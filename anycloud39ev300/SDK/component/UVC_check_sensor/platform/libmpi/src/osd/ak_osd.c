#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "osd_font_data.h"
#include "osd_draw.h"
#include "internal_error.h"
#include "ak_isp_char.h"
#include "akuio.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_osd.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define OPAQUE_ALPHA					0		/* not transparent alpha */
#define CRYSTAL_CLEAR_ALPHA				15		/* full transparent alpha */

#define CRYSTAL_CLEAR_COLOR_TABLE_INDEX	0		/*full transparentcolor table */
#define WHITE_COLOR_TABLE_INDEX 		1		/* white color table */
#define BLACK_COLOR_TABLE_INDEX 		2		/* black color table  */

#define DEF_ALPHA		((OPAQUE_ALPHA + CRYSTAL_CLEAR_ALPHA) * 5 / 10)	/* global alpha */
#define DEF_FONTS_COLOR_TABLE_INDEX 	WHITE_COLOR_TABLE_INDEX			/* line  font front color: white */
#define DEF_GROUND_COLOR_TABLE_INDEX	BLACK_COLOR_TABLE_INDEX			/* line  font ground color: black */
#define DISP_INTERNEL_DOP   			2

#define OSD_MAIN_MAX_WIDTH		(1280)
#define OSD_MAIN_MAX_HEIGHT 	(960)
#define OSD_SUB_MAX_WIDTH		(640)
#define OSD_SUB_MAX_HEIGHT 		(480)

/* it saves  osd info */
struct osd_channel_param {
	int resolution_w;    	/*video width , pixel*/
	int resolution_h;    	/*video hight , pixel*/
	int osd_w;    			/*osd width , pixel*/
	int osd_h;    			/*osd hight , pixel*/
	int xpos;
	int ypos;
	unsigned char *p_osd_buffer;
	unsigned char *p_osd_phy;
	void *dma_vaddr;
	int font_size;    			/* in display*/
	ak_mutex_t lock;
};

/* it saves common osd info */
struct osd_comm_param {
	unsigned int *p_color_tables;
	int color_tab_front_id;
	int color_tab_ground_id;
	int alpha;
};

/*it is to covert font data to osd data*/
struct font_hw_ctrl {
	unsigned char *p_context;
	unsigned short osd_w;
	unsigned short font_size;
	unsigned short x_offset_pixels;
	unsigned short color_tab_front_id;
	unsigned short color_tab_ground_id;
};

static struct osd_channel_param osd[VPSS_OSD_CHANNELS_MAX];
static struct osd_comm_param osd_com;
static void * vi_handle = NULL;

static unsigned int def_color_tables[] =
{
	0x000000, 0xff7f7f, 0x007f7f, 0x266ac0, 0x71408a, 0x4b554a, 0x599540, 0x0ec075,
	0x34aab5, 0x786085, 0x2c8aa0, 0x68d535, 0x34aa5a, 0x43e9ab, 0x4b55a5, 0x008080
};

static const char *osd_version = "libmpi_osd V2.0.03";

/*get position of left pixel which is set */
static int FontDisp_F_DispEmpty(unsigned char *FontBuf,
				unsigned short Font_Height, unsigned short Font_Width)
{
	int i = 0;
	int j = 0;
	int curPixelPos = 0;

	for(i = 0; i < Font_Width; i ++){
		for(j = 0; j < Font_Height; j ++){
			curPixelPos = j * Font_Height + i;
			if ((FontBuf[curPixelPos>>3] << (curPixelPos & 7)) & 0x80){
				break;
			}
		}
		if(j != Font_Height){
			return i;
		}
	}

	return i;
}

/*get position of right  pixel which is set */
static int FontDisp_B_DispEmpty(unsigned char *FontBuf, unsigned short Font_Height, unsigned short Font_Width)
{
	int i = 0;
	int j = 0;
	int curPixelPos = 0;

	for(i = Font_Width - 1; i > 0; i--){
		for( j = Font_Height - 1; j >= 0; j--){
			curPixelPos = j * Font_Height + i;
			if ((FontBuf[curPixelPos>>3] << (curPixelPos & 7)) & 0x80){
				break;
			}
		}
		if(j >= 0){
			return i;
		}
	}

	return i;
}

/*enlarg font*/
static int linear_scaling(unsigned char *src, int width, int height, unsigned char *dst, int dst_width, int dst_height)
{
	int src_line_num;
	unsigned char *src_line_off;
	unsigned char *dst_line_off;
	unsigned char *src_col_off;
	unsigned char *dst_col_off;
	int src_bit;

	for (int i = 0; i < dst_height; ++i) {
		src_line_num = (i * height / dst_height);
		src_line_off = src + width * src_line_num / 8;
		dst_line_off = dst + dst_width * i / 8;
		for (int ii = 0; ii < dst_width; ++ii) {
			src_col_off = src_line_off + (ii * width / dst_width) / 8;
			src_bit = ( (*src_col_off) >> (7 - ((ii * width / dst_width) % 8)) ) & 0x01;
			dst_col_off = dst_line_off + ii / 8;
			if (src_bit)
				*dst_col_off |= 1 << (7 - (ii % 8));
			else
				*dst_col_off &= ~(1 << (7 - (ii % 8)));
		}
	}

	return 0;
}


/*draw one pixel to osd buf*/
static int draw_one_pixel(struct font_hw_ctrl *ctrl, unsigned short pos_x,
									unsigned short pos_y, char pixel_value)
{
	unsigned char *p_osd_context = ctrl->p_context;
	int osd_w = ctrl->osd_w;
	unsigned char *which_byte = p_osd_context + ctrl->x_offset_pixels / 2 + pos_y * osd_w / 2 + pos_x / 2;
	int is_higher_bits = pos_x & 0x01;
	int value = pixel_value ? ctrl->color_tab_front_id : ctrl->color_tab_ground_id;

	if (pos_x + ctrl->x_offset_pixels >= osd_w)
	{
		ak_print_error_ex("SetFontsMatrix pos_x too large pos_x:%d,x_offset_pixels:%d\n",pos_x, ctrl->x_offset_pixels);
		return -1;
	}
	/*4bit  stand for  1 pixel*/
	*which_byte = is_higher_bits ? (*which_byte & 0xf) | (value << 4) : (*which_byte & 0xf0) | value;

	return 0;
}

/**
* draw_one_font: draw one font to osd buf
* @param[IN]:   font hw ctrl
* @channel[IN]:  display channel[0,1]
* @code[IN]:  char code,unicode
* @font_dot[IN]: font size
* return: width of font to display;
*/
static int draw_one_font(struct font_hw_ctrl * param,
		unsigned int channel,unsigned short code,int font_dot)
{
	signed short  i, j;
	int  xPos;
	unsigned char   *FontMatrix = NULL;
	unsigned char   fontHeight;
	unsigned short  ch;
	int  	curPixelPos = 0, prePixelNum=0;
	int  	font_w_left	= 0;
	int  	font_w_right = 0;
	unsigned char   *array = NULL;
	int   	file_font_size ;

	file_font_size = get_file_font_size();
	fontHeight = font_dot;
	xPos = 0;
	ch = code;

	array = calloc(1, (fontHeight * fontHeight / 8));
	if (!array) {
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_print_error_ex(" calloc failed\n");
		goto end;
	}

	if(ch == 0x20){// space char
		font_w_left  = 0;
		font_w_right = fontHeight / 2;
		memset(array, 0, fontHeight * fontHeight / 8);
		FontMatrix = array;
		goto fill_osd;
	}

	font_w_left  = 0;
	font_w_right = fontHeight;
	FontMatrix = get_one_font_data(ch);

	if (NULL == FontMatrix)	{
		ak_print_error_ex("Er:NoFont 0x%x,%c, size %d\n",ch,ch, fontHeight);
		goto end;
	}
	if (fontHeight > file_font_size) {
		linear_scaling(FontMatrix, file_font_size, file_font_size, array, fontHeight, fontHeight);
		FontMatrix = array;
	}
	if (ch >= '0' && ch <= '9' ){
		font_w_right = fontHeight / 2;
	}
	else if(ch < 0x80 && ch != 0x20)
	{
		font_w_left = FontDisp_F_DispEmpty(FontMatrix, fontHeight, fontHeight);
		font_w_right = FontDisp_B_DispEmpty(FontMatrix, fontHeight, fontHeight);
		if(font_w_left > DISP_INTERNEL_DOP)
		{
			font_w_left -= DISP_INTERNEL_DOP ;
		}
		font_w_right += DISP_INTERNEL_DOP;
		if(font_w_right > fontHeight)
		{
			font_w_right = fontHeight;
		}
	}

fill_osd:
	if ((NULL != FontMatrix))
	{
		if(font_w_left>2)
			font_w_left -= 2;

		/* make sure width of each font is double */
		if ((font_w_right - font_w_left) & 0x1)
		{
			if (font_w_right < fontHeight)
				font_w_right++;
			else
				font_w_left--;
		}

		for (i=0,prePixelNum =0; i<fontHeight; i++, prePixelNum+=fontHeight)// scan height
		{
			for (j=font_w_left; j < font_w_right; j++) //scan  width
			{
				curPixelPos = prePixelNum + j;
				int pixel_value = (FontMatrix[curPixelPos>>3] << (curPixelPos & 7)) & 0x80;
				if (draw_one_pixel(param, (unsigned short)(j - font_w_left), (unsigned short)i, pixel_value))
					goto end;
			}
		}
	}
	xPos = ((font_w_right - font_w_left)> 0)? font_w_right - font_w_left : 0;

end:
	free(array);

	return xPos;
}

/* get resolution from system */
static int get_resolution(void *handle, int channel,int *p_width, int *p_height)
{
	struct video_channel_attr attr;
	enum video_channel ch =  channel ? VIDEO_CHN_SUB : VIDEO_CHN_MAIN;

	if (ak_vi_get_channel_attr(handle, &attr) < 0) {
		ak_print_error_ex("ak_vi_get_channel_attr fail");
		return -1;
	}
	*p_width =  attr.res[ch].width;
	*p_height = attr.res[ch].height;

	return 0;
}

/*set color table to driver*/
static int set_osd_color_table(void)
{
	int ret;
	unsigned int *p_color_tables = osd_com.p_color_tables;
	struct vpss_osd_param param;

	CLEAR(param);
	param.id = OSD_SET_COLOR_TABLE;
	memcpy((char *)param.data, (char *)p_color_tables, 16 * sizeof(unsigned int));
	ret = ak_vpss_osd_set_param(vi_handle, &param);

	return ret;
}

/**
 * request_osd_dma_mem: request dma and set
 * @channel[IN]: 0-main, 1-sub
 * @width[IN]: osd width
 * @height[IN]: osd height
 * return: 0 success, -1 failed
 */
static int request_osd_dma_mem(int channel, int width, int height)
{
	if ((channel < 0) || (channel > VPSS_OSD_CHANNELS_MAX)) {
		return AK_FAILED;
	}

	if (osd[channel].dma_vaddr) {
		akuio_free_pmem(osd[channel].dma_vaddr);
		osd[channel].dma_vaddr = NULL;
	}

	unsigned int size = (width * height / 2);
	void *vaddr = akuio_alloc_pmem(size);
	if(!vaddr) {
		set_error_no(ERROR_TYPE_PMEM_MALLOC_FAILED);
		ak_print_error_ex("alloc dma fail. w:%d h:%d\n", width, height);
		return AK_FAILED;
	}

	void *phyaddr = (void *)akuio_vaddr2paddr(vaddr);
	if (!phyaddr) {
		ak_print_error_ex("vaddr to paddr failed, w:%d h:%d\n", width, height);
		return AK_FAILED;
	}

	struct vpss_osd_param param = {0};
	struct isp_osd_mem_attr *osd_mem_attr = (struct isp_osd_mem_attr *)param.data;

	osd_mem_attr->dma_paddr = phyaddr;
	osd_mem_attr->size = size;
	osd[channel].dma_vaddr = vaddr;

	if (0 == channel)
		param.id = OSD_SET_MAIN_DMA_MEM_REQUST;
	if (1 == channel)
	   	param.id = OSD_SET_SUB_DMA_MEM_REQUST;

	ak_print_normal_ex("alloc osd dma.dma: %p size: %d w: %d h: %d\n",
			phyaddr, osd_mem_attr->size, width, height);

	return ak_vpss_osd_set_param(vi_handle, &param);
}

static int draw_osd_matrix(int channel,int xoffset, int yoffset,int font_w,
    int font_h, const unsigned char *dot_buf, unsigned int buf_len )
{
	struct osd_channel_param *p_osd = &osd[channel];
	unsigned char *p_osd_buffer = p_osd->p_osd_buffer
		+ (p_osd->osd_w * yoffset + xoffset) / 2;
	const unsigned char *FontMatrix = dot_buf;
	int height = font_h;
	int osd_row_bytes = p_osd->osd_w / 2;
	int matrix_row_bytes = font_w / 2;
	int odd = font_w % 2;

	/* draw matrix data to osd buf */
	int i;
	for (i = 0; i < height; i++)// scan height
	{
		memcpy(p_osd_buffer + i * osd_row_bytes,
			FontMatrix + i * matrix_row_bytes + odd * (i+1) / 2,
			matrix_row_bytes);
	}

	return 0;
}

/**
* draw_osd_canvas: draw font of string to osd buf.
* @channel[IN]:  display channel[0,1]
* @xoffset[IN]: offset to x-ray in osd rect
* @yoffset[IN]: offset to y-ray in osd rect
* @dispstr[IN]: display string code,unicode
* @strlen[IN]: display string len
* return: 0 - success; otherwise -1;
*/
static int draw_osd_canvas(int channel, int xoffset, int yoffset,
		const unsigned short *disp_string, int len)
{
	struct osd_channel_param *p_osd = &osd[channel];
	struct osd_comm_param *p_osd_com = &osd_com;
	unsigned char *p_osd_buffer;
	int font_size,i,xpos,width;
	struct font_hw_ctrl hw_osd_ctrl;

	font_size = p_osd->font_size;
	p_osd_buffer = p_osd->p_osd_buffer + (p_osd->osd_w * yoffset ) / 2;
	xpos = xoffset;

	hw_osd_ctrl.p_context = p_osd_buffer;
	hw_osd_ctrl.font_size = font_size;
	hw_osd_ctrl.osd_w = p_osd->osd_w;
	hw_osd_ctrl.color_tab_front_id = p_osd_com->color_tab_front_id;
	hw_osd_ctrl.color_tab_ground_id = p_osd_com->color_tab_ground_id;
	for (i = 0; i < len; i++)
	{
		hw_osd_ctrl.x_offset_pixels = xpos;
		/* draw one font data to osd buf */
		width = draw_one_font(&hw_osd_ctrl, channel, disp_string[i],
				hw_osd_ctrl.font_size);
		if (!width)
			break;
		xpos += width;
	}

	if (i != len)
		ak_print_normal_ex("Small buffer to disp osd, "
				"only disp %d characters of total %d\n", i,len);

	return 0;
}


/**
* clean_osd_canvas: clean osd buf.
* @channel[IN]:  display channel[0,1]
* @xoffset[IN]: offset to x-ray in osd rect
* @yoffset[IN]: offset to y-ray in osd rect
* @width[IN]: width of rect to clean
* @height[IN]: height  of rect to clean
* return: 0 - success; otherwise -1;
*/
static int clean_osd_canvas(int channel,int xoffset, int yoffset, int width, int height)
{
	struct osd_channel_param *p_osd = &osd[channel];
	int font_size,i;
	int per_pixel_row_bytes;
	int offset_bytes;
	unsigned char *p_osd_buffer;

	font_size = p_osd->font_size;
	per_pixel_row_bytes = p_osd->osd_w / 2;

	/*2 pixel == 1 byte*/
	offset_bytes = width / 2;
	p_osd_buffer = p_osd->p_osd_buffer + (p_osd->osd_w * yoffset + xoffset) / 2;

	/*osd dma buf  set 0     */
	for (i = 0; i < font_size; i++)
		 memset(p_osd_buffer + i * per_pixel_row_bytes,0, offset_bytes);

	return 0;
}

/**
* draw osd buf data to isp
*/
static int draw_osd(unsigned int channel)
{
	struct osd_channel_param *p_osd = &osd[channel];
	int alpha = osd_com.alpha;
	int ret;
	struct vpss_osd_param param;
	struct isp_osd_context_attr *p_osd_context_attr;

	CLEAR(param);
	p_osd_context_attr = (struct isp_osd_context_attr *)param.data;

	p_osd_context_attr->osd_context_addr = p_osd->p_osd_phy;
	p_osd_context_attr->osd_width	= p_osd->osd_w;
	p_osd_context_attr->osd_height = p_osd->osd_h;
	p_osd_context_attr->start_xpos = p_osd->xpos;
	p_osd_context_attr->start_ypos = p_osd->ypos;
	p_osd_context_attr->alpha	= alpha;
	p_osd_context_attr->enable = 1;

	param.id = (channel == 0) ? OSD_SET_MAIN_CHANNEL_DATA : OSD_SET_SUB_CHANNEL_DATA;
	ret = ak_vpss_osd_set_param(vi_handle, &param);

	return ret;
}


/**
* clear osd buf in ISP
*/
static int clear_osd(unsigned int channel)
{
	int ret = 0;
	struct vpss_osd_param param;

	if (channel > 1) 
		return -1;
	CLEAR(param);
	param.id = (channel == 0) ? OSD_SET_MAIN_CHANNEL_DATA : OSD_SET_SUB_CHANNEL_DATA;
	ret = ak_vpss_osd_set_param(vi_handle, &param);

	return ret;
}

static int request_osd_mem(unsigned int channel,int width, int height)
{
	struct osd_channel_param *p_osd = &osd[channel];


	if (p_osd->p_osd_buffer) {
		akuio_free_pmem(p_osd->p_osd_buffer);
		p_osd->p_osd_buffer = NULL;
	}
	
	unsigned int size = (width * height / 2);
	void *vaddr = akuio_alloc_pmem(size);
	if(!vaddr) {
		set_error_no(ERROR_TYPE_PMEM_MALLOC_FAILED);
		ak_print_error_ex("alloc osd uio fail. w:%d h:%d\n", width, height);
		return AK_FAILED;
	}
	p_osd->p_osd_buffer = vaddr;
	p_osd->p_osd_phy = (void *)akuio_vaddr2paddr(vaddr);
	memset(vaddr, 0, size);

	ak_print_normal_ex("ch%d p_osd_buffer:%p\n",channel,p_osd->p_osd_buffer);

	return 0;
}

int osd_set_rect(unsigned int channel, int xstart, int ystart,
		int width, int height)
{
	int ret = 0;
	struct osd_channel_param *p_osd = &osd[channel];

	ak_thread_mutex_lock(&p_osd->lock);
	p_osd->xpos = xstart;
	p_osd->ypos = ystart;

	if (((p_osd->osd_w * p_osd->osd_h) != (width * height))
		|| (NULL == p_osd->p_osd_buffer)) {
		if ((ret = request_osd_mem(channel, width, height)) == 0) {
			ret = request_osd_dma_mem(channel, width, height);
		} else
			ak_print_error_ex("ch:%d fail\n", channel);
	}

	p_osd->osd_w = width;
	p_osd->osd_h = height;
	ak_thread_mutex_unlock(&p_osd->lock);

	return  ret;
}

const char* ak_osd_get_version(void)
{
	return osd_version;
}

/**
 * ak_osd_init - init osd para, include screen resolution,
 *			default color table and color id ;malloc osd buf.
 * @handle[IN]: vi handle
 * return: 0 - success; otherwise -1;
 */
int ak_osd_init(void *handle)
{
	int width, height;
	unsigned int file_font_size;
	struct osd_channel_param *p_osd;

	if (NULL == handle) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("param err.\n");
		return -1;
	}
	vi_handle = handle;
	file_font_size = get_file_font_size();

	int i;
	for (i = 0; i < VPSS_OSD_CHANNELS_MAX; i++) {
		p_osd = &osd[i];
		memset(p_osd, 0, sizeof(struct osd_channel_param));
		/*2 channel only use 1 font file*/
		/*if file font is not init, use default set*/
		if (file_font_size == 0) {
			ak_print_error_ex("chn %d font file not init, use default\n", i);
			p_osd->font_size = i ? FONT_SIZE_24X24 : FONT_SIZE_48X48;
		} else
			p_osd->font_size = i ? file_font_size : file_font_size * 2;
		width = 0;
		height = 0;

		if (get_resolution(vi_handle, i, &width, &height) < 0) {
			ak_print_error_ex("get_resolution fail.\n");
		}
		p_osd->resolution_w = width;
		p_osd->resolution_h = height;
		ak_thread_mutex_init(&p_osd->lock);

		ak_print_normal_ex("ch:%d x:%d y:%d w:%d h:%d"
				"fontS:%d res_w:%d res_h:%d\n",
				i, p_osd->xpos ,p_osd->ypos,p_osd->osd_w ,p_osd->osd_h,
				p_osd->font_size, p_osd->resolution_w, p_osd->resolution_h);
	}
	osd_com.p_color_tables		= def_color_tables;
	osd_com.color_tab_front_id	= DEF_FONTS_COLOR_TABLE_INDEX;
	osd_com.color_tab_ground_id	= DEF_GROUND_COLOR_TABLE_INDEX;
	osd_com.alpha				= DEF_ALPHA;
	set_osd_color_table();

#if 0
	/* use maximum width and height to init the osd DMA memory */
	if (request_osd_dma_mem(0, OSD_MAIN_MAX_WIDTH, OSD_MAIN_MAX_HEIGHT))
		ak_print_error_ex("set main channel osd DMA memory fail\n");	
	if (request_osd_dma_mem(1, OSD_SUB_MAX_WIDTH, OSD_SUB_MAX_HEIGHT))
		ak_print_error_ex("set sub channel osd DMA memory fail\n");	
#endif

	return 0;
}

int ak_osd_get_max_rect(int channel, int *width, int *height)
{
	if (channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err.\n");
		return -1;
	}

	if(get_resolution(vi_handle, channel, width, height) < 0){
		ak_print_error_ex("get_resolution fail.\n");
		return -1;
	}

	return 0;
}

/**
 * ak_osd_set_rect - set osd rect position.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @xstart[IN]:  start pixel x-ray
 * @ystart[IN]:  start pixel y-ray
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_rect(void *vi_handle, int channel, int xstart, int ystart,
		int width, int height)
{
	struct osd_channel_param *p_osd;
	int font_size;

	if (channel < 0 || channel > 1) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("channel param err: %d\n", channel);
		return -1;
	}

	p_osd = &osd[channel];
	font_size = p_osd->font_size;
	ak_print_normal_ex("xstart: %d ystart: %d w: %d h: %d\n",
			xstart, ystart, width,  height);
	if (xstart < 0 || ystart < 0 ||
			xstart > (p_osd->resolution_w - 2 * font_size)
			|| width < font_size || height < font_size)
	{
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("err, x: %d, y: %d, r_w: %d, w: %d, h: %d, f: %d\n",
				xstart, ystart, p_osd->resolution_w, width, height, font_size);
		return -1;
	}

	if (get_resolution(vi_handle, channel, &p_osd->resolution_w,
				&p_osd->resolution_h) < 0) {
		ak_print_error_ex("get_resolution fail.\n");
	}

	if (p_osd->resolution_h - ystart < height)
		height = p_osd->resolution_h - ystart;

	if (p_osd->resolution_w - xstart < width)
		width = p_osd->resolution_w - xstart;

	if (width < font_size  || height < font_size) {
		ak_print_error_ex("param err, w: %d, h: %d, font: %d\n",
				width, height, font_size);
		return -1;
	}

	ak_print_notice_ex("x:%d, y:%d, w:%d, h:%d\n", xstart, ystart, width, height);

	return osd_set_rect(channel, xstart, ystart, width, height);
}


/**
 * ak_osd_set_color - set osd color param.
 * @front_color[IN]:   front color of osd,  [0,15]
 * @bg_color[IN]:     back color of osd, [0,15] 
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_color(int font_color, int bg_color)
{
	struct osd_comm_param *p_osd = &osd_com;

	if (font_color > 15 || bg_color > 15 ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_normal_ex("para err\n");
		return -1;
	}

	p_osd->color_tab_front_id = font_color;
	p_osd->color_tab_ground_id	= bg_color;
	
	return 0;
}

/**
 * ak_osd_draw_matrix - draw appointed matrix on screen.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @xoffset[IN]:  x offset of rect
 */
int ak_osd_draw_matrix(int channel, int xoffset, int yoffset, int font_w,
    int font_h, const unsigned char *dot_buf, unsigned int buf_len)
{
	int ret = 0;
	struct osd_channel_param *p_osd;

	if( channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
		return -1;
	}
	p_osd = &osd[channel];
	if(xoffset < 0 || yoffset < 0 || xoffset > (p_osd->osd_w - font_w) ||
		yoffset > (p_osd->osd_h - font_h) || (font_w * font_h)/ 2 > buf_len){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(" para err, osd_w: %d, osd_h: %d\n", p_osd->osd_w,
				p_osd->osd_h);
		return AK_FAILED;
	}
	ak_thread_mutex_lock(&osd[channel].lock);
	if((ret = draw_osd_matrix(channel,xoffset, yoffset,
		 font_w, font_h, dot_buf, buf_len)) == 0){
		 ret = draw_osd(channel);
	}
	ak_thread_mutex_unlock(&osd[channel].lock);

	return ret;
}

/**
 * ak_osd_draw_str - draw string on screen.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @xoffset[IN]:   x offset of rect
 * @yoffset[IN]:  y offset of rect
 * @disp_str[IN]: display string code, unicode
 * @str_len[IN]: display string len
 * return: 0 - success; otherwise -1;
 */
int ak_osd_draw_str(int channel, int xoffset, int yoffset,
		const unsigned short *disp_str, int str_len)
{
	int ret = 0;
	struct osd_channel_param *p_osd ;
	int font_size ;

	if (channel < 0 || channel > 1) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
		return -1;
	}
	p_osd = &osd[channel];
	font_size = p_osd->font_size;
	if( xoffset < 0 || yoffset < 0 || xoffset > (p_osd->osd_w - font_size) ||
		yoffset > (p_osd->osd_h - font_size) ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err, xo:%d, yo:%d, osd_w:%d, osd_h:%d, font_s:%d\n",
				xoffset, yoffset, p_osd->osd_w, p_osd->osd_h, font_size);
		return -1;
	}
	ak_thread_mutex_lock(&osd[channel].lock);
	if((ret = draw_osd_canvas(channel, xoffset, yoffset,
		disp_str, str_len)) == 0){
		ret = draw_osd(channel);
	}
	ak_thread_mutex_unlock(&osd[channel].lock);

	return ret;
}

int ak_osd_clean_str(int channel, int xoffset, int yoffset, int width, int height)
{
	int ret = 0;
	struct osd_channel_param *p_osd;

	if( channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("para err\n");
		return -1;
	}
	p_osd = &osd[channel];
	if( xoffset < 0 || yoffset < 0 || xoffset >= p_osd->osd_w ||
		yoffset >= p_osd->osd_h ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		 ak_print_error_ex(" param err\n");
		return -1;
	}
	if(p_osd->osd_w - xoffset < width){
		width = p_osd->osd_w - xoffset;
	}
	if(p_osd->osd_h - yoffset < height){
		height = p_osd->osd_h - yoffset;
	}
	ak_thread_mutex_lock(&osd[channel].lock);
	if((ret = clean_osd_canvas(channel,xoffset,yoffset,width,height)) == 0)
		 ret = draw_osd(channel);
	ak_thread_mutex_unlock(&osd[channel].lock);

	return ret;
}

#if 0
/**
* ak_osd_set_color_tab: set color table.
* @color_tab[IN]:  it is an array,size of array is 16
* return: 0 - success; otherwise -1;
*/
void ak_osd_set_color_tab(unsigned int *color_tab)
{
	unsigned int *p = color_tab;
	int i;
	for(i = 0; i < 16 ; i++){
		def_color_tables[i] = *p;
		p++;
	}
	set_osd_color_table();
}
#endif

/**
 * ak_osd_destroy - free osd resource
 * return: none
 */
void ak_osd_destroy(void)
{
	int i=0;

	for(i=0; i<VPSS_OSD_CHANNELS_MAX; ++i){
		clear_osd(i);

		ak_thread_mutex_lock(&osd[i].lock);
		if (osd[i].p_osd_buffer) {			
			akuio_free_pmem(osd[i].p_osd_buffer);			
			osd[i].p_osd_buffer = NULL;
		}

		/* release dma memory */
		if(osd[i].dma_vaddr){
			akuio_free_pmem(osd[i].dma_vaddr);
			osd[i].dma_vaddr = NULL;
		}
		ak_thread_mutex_unlock(&osd[i].lock);
	}

#ifndef AK_RTOS	
	free_font_data();
#endif//
}

int ak_osd_set_alpha(int alpha)
{
	int alpha_denominator = 100;

	if (alpha > alpha_denominator || alpha < 0) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument, allow alpha[0,100]\n");
		return -1;
	}	
	osd_com.alpha				= (OPAQUE_ALPHA + CRYSTAL_CLEAR_ALPHA)
									* alpha / alpha_denominator;
	ak_print_info_ex("setting alpha: %d, internal: %d\n", alpha, osd_com.alpha);

	return 0;
}
