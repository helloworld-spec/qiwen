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
#include "osd_ipcsrv.h"
#include "ak_osd.h"

#define MPI_OSD			                "<mpi_osd>"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define OPAQUE_ALPHA					0		/* not transparent alpha */
#define CRYSTAL_CLEAR_ALPHA				15		/* full transparent alpha */

#define CRYSTAL_CLEAR_COLOR_TABLE_INDEX	0		/*full transparentcolor table */
#define WHITE_COLOR_TABLE_INDEX 		1		/* white color table */
#define BLACK_COLOR_TABLE_INDEX 		2		/* black color table  */

/* global alpha */
#define DEF_ALPHA		0	//((OPAQUE_ALPHA + CRYSTAL_CLEAR_ALPHA) * 5 / 10)
/* line  font front color: white */
#define DEF_FONTS_COLOR_TABLE_INDEX 	WHITE_COLOR_TABLE_INDEX
/* line  font ground color: black */
#define DEF_GROUND_COLOR_TABLE_INDEX	BLACK_COLOR_TABLE_INDEX
#define DISP_INTERNEL_DOP   			2

/*driver limit: donot modify it*/
#define CH0_OSD_MAX_WIDTH	1024
#define CH1_OSD_MAX_WIDTH	640

#define OSD_STR_POLICY_SKIP_NO_CHANGE

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
#define OSD_STR_POLICY_BUF_LEN	128
struct osd_str_draw_policy_attr {
	unsigned short font_size;
	unsigned short osd_w;
	unsigned short front_id;
	unsigned short ground_id;

	unsigned short str_len;
	unsigned short xpos[OSD_STR_POLICY_BUF_LEN];
	unsigned short str[OSD_STR_POLICY_BUF_LEN];
	unsigned short str_pixels_w[OSD_STR_POLICY_BUF_LEN];

	char be_clear;
};
#endif

/* it saves osd info, channel 0 and channel 1 maybe different */
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

/* it saves common osd info, channel 0 and channel 1 is same setting */
struct osd_comm_param {
	unsigned int *p_color_tables;
	int color_tab_front_id;
	int color_tab_ground_id;
	int color_tab_edge_id;
	/*alpha: [0,15]. new picture pixsel = picture pixsel * alpha/15  + osd pixsel * (1 - alpha/15)*/
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
	unsigned short color_tab_edge_id;
};

static struct osd_channel_param osd[VIDEO_CHN_NUM][VPSS_OSD_NUM_PER_VIDEO_CHN];
static struct osd_comm_param osd_com;
static void * vi_handle = NULL;

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
static struct osd_str_draw_policy_attr 
	all_osd_str_draw_policy_attr[VIDEO_CHN_NUM][VPSS_OSD_NUM_PER_VIDEO_CHN];
#endif

/*YUV format: Y8U8V8*/
static unsigned int def_color_tables[] = {
	0x000000, 0xff7f7f, 0x007f7f, 0x4c54ff, 0x952b15, 0x1dff6b, 0x599540, 0x0ec075,
	0x34aab5, 0x786085, 0x2c8aa0, 0x68d535, 0x34aa5a, 0x43e9ab, 0x4b55a5, 0x008080
};

static const char *osd_version = "libmpi_osd V1.1.03";

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
		ak_print_error_ex(MPI_OSD "SetFontsMatrix pos_x too large "
		    "pos_x:%d, x_offset_pixels:%d\n",
		    pos_x, ctrl->x_offset_pixels);
		return -1;
	}

	/*4bit  stand for  1 pixel*/
	*which_byte = is_higher_bits ?
	    ((*which_byte & 0xf) | (value << 4)) : ((*which_byte & 0xf0) | value);

	return 0;
}

/**
* draw_one_font: draw one font to osd buf
* @param[IN]:   font hw ctrl
* @channel[IN]:  display channel[0,1]
* @code[IN]:  char code,gb2312
* @font_dot[IN]: font size
* return: width of font to display;
*/
static int draw_one_font(struct font_hw_ctrl * param,
		unsigned int channel,unsigned short code,int font_dot)
{
	signed short  i, j;
	int  xPos;
	unsigned char   *FontMatrix = NULL;
	unsigned char   fontHeight = 0;
	unsigned short  ch = 0;
	int  	curPixelPos = 0, prePixelNum=0;	
	int  	font_w_right = 0;
	unsigned char   *array = NULL;
	int   	file_font_size = 0;

	file_font_size = get_file_font_size();
	fontHeight = font_dot;
	xPos = 0;
	ch = code;

	array = calloc(1, (fontHeight * fontHeight / 8));
	if (!array) {
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_print_error_ex(MPI_OSD "calloc failed\n");
		goto end;
	}
	
	font_w_right = fontHeight;
	/*gb2312. if it is asc char, width  is half of height*/
	if (ch < 0x80)
		font_w_right = fontHeight / 2;
	FontMatrix = get_one_font_data(ch);

	if (NULL == FontMatrix)	{
		ak_print_error_ex(MPI_OSD "Er:NoFont 0x%x,%c, size %d\n",
		    ch, ch, fontHeight);
		goto end;
	}
	if (fontHeight > file_font_size) {
		/* (hight / width) value*/
		int w_h = 1;
		if(code < 0x80) w_h = 2;
		linear_scaling(FontMatrix, file_font_size / w_h, file_font_size,
		    array, fontHeight / w_h, fontHeight);
		FontMatrix = array;
	}
	
	if ((NULL != FontMatrix))
	{
		/* make sure width of each font is double */		
        /* scan height */
		for (i = 0, prePixelNum = 0; i<fontHeight; i++ ) {
		    /* scan  width */
			for (j = 0; j < font_w_right; j++) {
				curPixelPos = prePixelNum + j;
				int pixel_value = (FontMatrix[curPixelPos>>3]
				    << (curPixelPos & 7)) & 0x80;
				if (draw_one_pixel(param, (unsigned short)(j),
				    (unsigned short)i, pixel_value))
					goto end;
			}
			prePixelNum += font_w_right;
		}
	}
	xPos = ((font_w_right)> 0)? font_w_right : 0;

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
		ak_print_error_ex(MPI_OSD "ak_vi_get_channel_attr fail");
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
 * request_osd_dma_mem: request dma and set this dma buf is used in isp.
 * @channel[IN]: 0-main, 1-sub
 * @width[IN]: osd width
 * @height[IN]: osd height
 * return: 0 success, -1 failed
 */
static int request_osd_dma_mem(int channel, int osd_rect, int width, int height)
{
	if ((channel < 0) || (channel >= VIDEO_CHN_NUM)) {
		return AK_FAILED;
	}

	if (osd[channel][osd_rect].dma_vaddr) {
		akuio_free_pmem(osd[channel][osd_rect].dma_vaddr);
		osd[channel][osd_rect].dma_vaddr = NULL;
	}

	//unsigned int size = (width * height / 2);
	unsigned int size = width * height;	/* double osd size for pingpong buffers */
	void *vaddr = akuio_alloc_pmem(size);
	if(!vaddr) {
		set_error_no(ERROR_TYPE_PMEM_MALLOC_FAILED);
		ak_print_error_ex(MPI_OSD "alloc dma fail. w:%d h:%d\n", width, height);
		return AK_FAILED;
	}

	void *phyaddr = (void *)akuio_vaddr2paddr(vaddr);
	if (!phyaddr) {
		ak_print_error_ex(MPI_OSD "vaddr to paddr failed, w:%d h:%d\n",
		    width, height);
		return AK_FAILED;
	}

	struct vpss_osd_param param = {0};
	struct isp_osd_mem_attr *osd_mem_attr = (struct isp_osd_mem_attr *)param.data;

	osd_mem_attr->chn = osd_rect;
	osd_mem_attr->dma_paddr = phyaddr;
	osd_mem_attr->size = size;
	osd[channel][osd_rect].dma_vaddr = vaddr;

	if (0 == channel)
		param.id = OSD_SET_MAIN_DMA_MEM_REQUST;
	if (1 == channel)
	   	param.id = OSD_SET_SUB_DMA_MEM_REQUST;

	ak_print_info_ex(MPI_OSD "alloc osd dma.dma: %p size: %d w: %d h: %d\n",
		phyaddr, osd_mem_attr->size, width, height);

	return ak_vpss_osd_set_param(vi_handle, &param);
}

/**
 * draw_osd_matrix - copy appointed matrix data to osd buf.
 */
static int draw_osd_matrix(int channel, int osd_rect, int xoffset, int yoffset,
    int font_w, int font_h, const unsigned char *dot_buf, unsigned int buf_len)
{
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];
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

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
static int osd_str_draw_policy_init(void)
{
	int i, j;

	for (i = 0; i < VIDEO_CHN_NUM; i++)
		for (j = 0; j < VPSS_OSD_NUM_PER_VIDEO_CHN; j++) {
			all_osd_str_draw_policy_attr[i][j].font_size = 0;
			all_osd_str_draw_policy_attr[i][j].osd_w = 0;
			all_osd_str_draw_policy_attr[i][j].str_len = 0;
			all_osd_str_draw_policy_attr[i][j].be_clear = 1;
		}

	return 0;
}

static int osd_str_draw_policy_must_redraw_all(
		const struct font_hw_ctrl *hw_osd_ctrl,
		struct osd_str_draw_policy_attr *p_osd_str_draw_policy_attr)
{
	int ret = 0;

	if (p_osd_str_draw_policy_attr->be_clear)
		return 1;

	if (hw_osd_ctrl->font_size != p_osd_str_draw_policy_attr->font_size ||
			hw_osd_ctrl->osd_w != p_osd_str_draw_policy_attr->osd_w ||
			hw_osd_ctrl->color_tab_front_id != p_osd_str_draw_policy_attr->front_id ||
			hw_osd_ctrl->color_tab_ground_id != p_osd_str_draw_policy_attr->ground_id
			//hw_psd_ctrl->str_len != p_osd_str_draw_policy_attr->str_len
			) {
		p_osd_str_draw_policy_attr->font_size = hw_osd_ctrl->font_size;
		p_osd_str_draw_policy_attr->osd_w = hw_osd_ctrl->osd_w;
		p_osd_str_draw_policy_attr->front_id = hw_osd_ctrl->color_tab_front_id;
		p_osd_str_draw_policy_attr->ground_id = hw_osd_ctrl->color_tab_ground_id;

		ret = 1;
	}

	return ret;
}

static int osd_draw_policy_current_character_no_change(
		const unsigned short *disp_character,
		const int str_index,
		const int xpos,
		const struct osd_str_draw_policy_attr *p_osd_str_draw_policy_attr)
{
	/*
	int str_len;
	int xpos[50];
	int str[50];
	int str_pixels_w[50];
	*/

	if (str_index >= p_osd_str_draw_policy_attr->str_len)
		return 0;

	if ((disp_character[str_index] != p_osd_str_draw_policy_attr->str[str_index])
		|| ((str_index > 0) && (disp_character[str_index - 1] != p_osd_str_draw_policy_attr->str[str_index - 1]))
		|| ((str_index < p_osd_str_draw_policy_attr->str_len - 1) && (disp_character[str_index + 1] != p_osd_str_draw_policy_attr->str[str_index + 1])))
		return 0;

	if (xpos != p_osd_str_draw_policy_attr->xpos[str_index])
		return 0;

	return 1;
}
#endif

/**
* is_pixel_font_color: judge one pixel is font color or not
* @p_osd_buffer[IN]:  osd buf
* @width[IN]:  osd buf width
* @height[IN]:  height
* @x[IN]: offset to x-ray in osd rect
* @y[IN]: offset to y-ray in osd rect
* return: 0 is not font color; 1 is font color;
*/
static int is_pixel_font_color(unsigned char *p_osd_buffer, int width, int height, unsigned short x, unsigned short y)
{
	unsigned char *which_byte = NULL;
	int is_higher_bits = 0;
	int pixel_value = 0;
	struct osd_comm_param *p_osd_com = &osd_com;

	if (NULL == p_osd_buffer)
		return AK_FALSE;

	which_byte = p_osd_buffer + y * width / 2 + x / 2;
	is_higher_bits = x & 0x01;

	if (is_higher_bits)
		pixel_value = (*which_byte & 0xf0) >> 4;
	else
		pixel_value = *which_byte & 0x0f;

	if (pixel_value == p_osd_com->color_tab_front_id)
		return AK_TRUE;
	else
		return AK_FALSE;
}

/**
* draw_pixel_edge_color: draw edge one pixel
* @p_osd_buffer[IN]:  osd buf
* @width[IN]:  osd buf width
* @height[IN]:  height
* @x[IN]: offset to x-ray in osd rect
* @y[IN]: offset to y-ray in osd rect
* return: 0 - success; otherwise -1;
*/
static int draw_pixel_edge_color(unsigned char *p_osd_buffer, int width, int height, unsigned short x, unsigned short y)
{
	unsigned char *which_byte = NULL;
	int is_higher_bits = 0;
	int value = 0;
	struct osd_comm_param *p_osd_com = &osd_com;

	if (NULL == p_osd_buffer)
		return -1;

	value = p_osd_com->color_tab_edge_id;

	which_byte = p_osd_buffer  + y * width / 2 + x / 2;
	is_higher_bits = x & 0x01;

	/*4bit  stand for  1 pixel*/
	*which_byte = is_higher_bits ?
	    ((*which_byte & 0xf) | (value << 4)) : ((*which_byte & 0xf0) | value);

	return 0;
}

/**
* draw_edge: draw font edge to osd buf.
* @p_osd_buffer[IN]:  osd buf
* @buf_width[IN]:  osd buf width
* @width[IN]:  width
* @height[IN]:  height
* return: 0 - success; otherwise -1;
*/
static int draw_edge(unsigned char *p_osd_buffer, int buf_width, int width, int height)
{
	int value_top = 0;
	int value_bottom = 0;
	int value_left = 0;
	int value_right = 0;
	int value = 0;
	int i = 0;
	int j = 0;

	if (NULL == p_osd_buffer)
		return -1;

	/*if the pixel is next to font, draw edge.*/

	for (i = 0; i<height; i++ ) {
		for (j = 0; j < width; j++) {
			if (is_pixel_font_color(p_osd_buffer, buf_width, height, j, i))
				continue;
			
			/*top pixel*/
			if (i > 0) {
				value_top = is_pixel_font_color(p_osd_buffer, buf_width, height, j, i - 1);
			} else
				value_top = 0;

			/*bottom pixel*/
			if (i < height - 1) {
				value_bottom = is_pixel_font_color(p_osd_buffer, buf_width, height, j, i + 1);
			} else 
				value_bottom = 0;

			/*left pixel*/
			if (j > 0) {
				value_left = is_pixel_font_color(p_osd_buffer, buf_width, height, j - 1, i);
			} else
				value_left = 0;

			/*right pixel*/
			if (j < width - 1) {
				value_right = is_pixel_font_color(p_osd_buffer, buf_width, height, j + 1, i);
			} else
				value_right = 0;

			value = value_top | value_bottom | value_left | value_right;

			if (value)
				draw_pixel_edge_color(p_osd_buffer, buf_width, height, (unsigned short)(j), (unsigned short)i);
			
		}
	}

	return 0;
}


/**
* draw_osd_canvas: draw font of string to osd buf.
* @channel[IN]:  display channel[0,1]
* @xoffset[IN]: offset to x-ray in osd rect
* @yoffset[IN]: offset to y-ray in osd rect
* @dispstr[IN]: display string code,gb2312
* @strlen[IN]: display string len
* return: 0 - success; otherwise -1;
*/
static int draw_osd_canvas(int channel, int osd_rect, int xoffset, int yoffset,
		const unsigned short *disp_string, int len)
{
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];
	struct osd_comm_param *p_osd_com = &osd_com;
	unsigned char *p_osd_buffer;
	int font_size,i,xpos;
	int width = 0;
	struct font_hw_ctrl hw_osd_ctrl;
#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	int redraw_all;
	struct osd_str_draw_policy_attr *p_cur_osd_str_draw_policy_attr;
#endif

	font_size = p_osd->font_size;
	p_osd_buffer = p_osd->p_osd_buffer + (p_osd->osd_w * yoffset ) / 2;
	xpos = xoffset;

	hw_osd_ctrl.p_context = p_osd_buffer;
	hw_osd_ctrl.font_size = font_size;
	hw_osd_ctrl.osd_w = p_osd->osd_w;
	hw_osd_ctrl.color_tab_front_id = p_osd_com->color_tab_front_id;
	hw_osd_ctrl.color_tab_ground_id = p_osd_com->color_tab_ground_id;
	hw_osd_ctrl.color_tab_edge_id = p_osd_com->color_tab_edge_id;

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	p_cur_osd_str_draw_policy_attr = &all_osd_str_draw_policy_attr[channel][osd_rect];

	if (osd_str_draw_policy_must_redraw_all(
				&hw_osd_ctrl, p_cur_osd_str_draw_policy_attr)) {
		redraw_all = 1;
	} else {
		redraw_all = 0;
	}
#endif
	for (i = 0; i < len; i++)
	{
#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
		if (!redraw_all && osd_draw_policy_current_character_no_change(
					disp_string,
					i,
					xpos,
					p_cur_osd_str_draw_policy_attr)) {
			xpos += p_cur_osd_str_draw_policy_attr->str_pixels_w[i];
			//ak_print_error("skip:%d\n", i);
			continue;
		} else {
			//ak_print_error("noskip:%d\n", i);
		}
#endif
		hw_osd_ctrl.x_offset_pixels = xpos;
		/* draw one font data to osd buf */
		width = draw_one_font(&hw_osd_ctrl, channel, disp_string[i],
				hw_osd_ctrl.font_size);
		if (!width)
			break;

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	/*
	int str_len;
	int xpos[50];
	int str[50];
	int str_pixels_w[50];
	*/
		p_cur_osd_str_draw_policy_attr->xpos[i] = xpos;
		p_cur_osd_str_draw_policy_attr->str[i] = disp_string[i];
		p_cur_osd_str_draw_policy_attr->str_pixels_w[i] = width;
#endif

		xpos += width;
	}

	int str_width = xpos - xoffset;
	int str_height = font_size;
	int x = xoffset;
	int y = yoffset;

	if (yoffset >= 1) {
		y = yoffset - 1;
		str_height++;
	}

	if (yoffset < p_osd->osd_h - font_size)
		str_height++;

	if (xoffset >= 1) {
		x = xoffset - 1;
		str_width++;
	}

	if (xpos < p_osd->osd_w)
		str_width++;

	unsigned char *p_buffer = p_osd->p_osd_buffer + (p_osd->osd_w * y) / 2 + x / 2;
	 
	draw_edge(p_buffer, p_osd->osd_w, str_width, str_height);
	

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	/* save strings length */
	p_cur_osd_str_draw_policy_attr->str_len = i;
	p_cur_osd_str_draw_policy_attr->be_clear = 0;
#endif

	if (i != len)
		ak_print_normal_ex(MPI_OSD "Small buffer to disp osd, "
				"only disp %d characters of total %d\n", i, len);

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
static int clean_osd_canvas(int channel, int osd_rect, int xoffset, int yoffset,
                int width, int height)
{
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];
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
static int draw_osd(unsigned int channel, int osd_rect)
{
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];
	int alpha = osd_com.alpha;
	int ret;
	struct vpss_osd_param param;
	struct isp_osd_context_attr *p_osd_context_attr;

	CLEAR(param);
	p_osd_context_attr = (struct isp_osd_context_attr *)param.data;

	p_osd_context_attr->chn = osd_rect;
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
static int clear_osd(unsigned int channel, int osd_rect)
{
	int ret = 0;
	struct vpss_osd_param param;
	struct isp_osd_context_attr *p_osd_context_attr;

	if (channel > 1)
		return -1;
	CLEAR(param);
	param.id = (channel == 0) ? OSD_SET_MAIN_CHANNEL_DATA : OSD_SET_SUB_CHANNEL_DATA;
	p_osd_context_attr = (struct isp_osd_context_attr *)param.data;
	p_osd_context_attr->chn = osd_rect;
	ret = ak_vpss_osd_set_param(vi_handle, &param);

	return ret;
}

/**
 * request_osd_mem - malloc osd buf which  pass osd data to isp dma buf.
 *                              caution,it malloc from dma. it can be used in user-space
 *                              and kernel space. this can be saved memory and
 *                              also can reduce copy data from user space to kerne space
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise -1;
 */
static int request_osd_mem(unsigned int channel, int osd_rect, int width, int height)
{
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];


	if (p_osd->p_osd_buffer) {
		akuio_free_pmem(p_osd->p_osd_buffer);
		p_osd->p_osd_buffer = NULL;
	}

	unsigned int size = (width * height / 2);
	void *vaddr = akuio_alloc_pmem(size);
	if(!vaddr) {
		set_error_no(ERROR_TYPE_PMEM_MALLOC_FAILED);
		ak_print_error_ex(MPI_OSD "alloc osd uio fail. w:%d h:%d\n",
		    width, height);
		return AK_FAILED;
	}
	p_osd->p_osd_buffer = vaddr;
	p_osd->p_osd_phy = (void *)akuio_vaddr2paddr(vaddr);
	memset(vaddr, 0, size);

	ak_print_info_ex(MPI_OSD "video_ch%d, osd_rect:%d, p_osd_buffer:%p, "
	    "width:%d, height:%d\n",
	    channel, osd_rect, p_osd->p_osd_buffer, width, height);

	return 0;
}

/**
 * osd_set_rect - set osd rect position, malloc buf.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @xstart[IN]:  start pixel x-ray
 * @ystart[IN]:  start pixel y-ray
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise -1;
 */
static int osd_set_rect(unsigned int channel, int osd_rect,
            int xstart, int ystart,	int width, int height)
{
	int ret = 0;
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];

	ak_thread_mutex_lock(&p_osd->lock);
	p_osd->xpos = xstart;
	p_osd->ypos = ystart;

	if (((p_osd->osd_w * p_osd->osd_h) != (width * height))
		|| (NULL == p_osd->p_osd_buffer)) {
		if ((ret = request_osd_mem(channel, osd_rect, width, height)) == 0) {
			ret = request_osd_dma_mem(channel, osd_rect, width, height);
		} else
			ak_print_error_ex(MPI_OSD "ch:%d fail\n", channel);
	}

	p_osd->osd_w = width;
	p_osd->osd_h = height;
	ak_thread_mutex_unlock(&p_osd->lock);

	return  ret;
}

int osd_get_common_value(int *alpha, int *front_color, int *ground_color)
{
	struct osd_comm_param *p_osd = &osd_com;
	
	*alpha = p_osd->alpha;
	*front_color = p_osd->color_tab_front_id;
	*ground_color = p_osd->color_tab_ground_id;

	return 0;
}

int osd_get_area_value(int channel, int osd_rect, int *xstart, int *ystart, 
	int *width, int *height, int *disp_font_size)
{	
	struct osd_channel_param *p_osd;

	if( channel < 0 || channel > 1 ||  osd_rect < 0 || osd_rect > 2){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err\n");
		return -1;
	}
	p_osd = &osd[channel][osd_rect];
	*xstart = p_osd->xpos;
	*ystart = p_osd->ypos;
	*width = p_osd->osd_w;
	*height = p_osd->osd_h;
	*disp_font_size = p_osd->font_size;

	return 0;
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
		ak_print_error_ex(MPI_OSD "param err.\n");
		return -1;
	}
	vi_handle = handle;
	file_font_size = get_file_font_size();

#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	osd_str_draw_policy_init();
#endif

	int i, j;
	for (i = 0; i < VIDEO_CHN_NUM; i++)
	for (j = 0; j < VPSS_OSD_NUM_PER_VIDEO_CHN; j++) {
		p_osd = &osd[i][j];
		memset(p_osd, 0, sizeof(struct osd_channel_param));
		/*2 channel only use 1 font file*/
		/*if file font is not init, use default set*/
		if (file_font_size == 0) {
			ak_print_info_ex(MPI_OSD "ch%d font file not init, use default\n", i);
			p_osd->font_size = i ? FONT_SIZE_16X16 : FONT_SIZE_48X48;
		} else
			p_osd->font_size = i ? file_font_size : file_font_size * 2;

		width = 0;
		height = 0;

		if (get_resolution(vi_handle, i, &width, &height) < 0) {
			ak_print_error_ex(MPI_OSD "get_resolution fail.\n");
		}
		p_osd->resolution_w = width;
		p_osd->resolution_h = height;
		ak_thread_mutex_init(&p_osd->lock, NULL);

		ak_print_info_ex(MPI_OSD "ch:%d rect:%d x:%d y:%d w:%d h:%d"
			" fontS:%d res_w:%d res_h:%d\n",
			i, j, p_osd->xpos, p_osd->ypos, p_osd->osd_w, p_osd->osd_h,
			p_osd->font_size, p_osd->resolution_w, p_osd->resolution_h);
	}
	osd_com.p_color_tables		= def_color_tables;
	osd_com.color_tab_front_id	= DEF_FONTS_COLOR_TABLE_INDEX;
	osd_com.color_tab_ground_id	= DEF_GROUND_COLOR_TABLE_INDEX;
	osd_com.color_tab_edge_id	= BLACK_COLOR_TABLE_INDEX;
	osd_com.alpha				= DEF_ALPHA;
	set_osd_color_table();
	osd_sys_ipc_register();
	
	return 0;
}

/**
 * ak_osd_get_max_rect - get channel resolution.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise -1;
 */
int ak_osd_get_max_rect(int channel, int *width, int *height)
{
	enum video_channel ch =  channel ? VIDEO_CHN_SUB : VIDEO_CHN_MAIN;

	if (channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err.\n");
		return -1;
	}

	if(get_resolution(vi_handle, channel, width, height) < 0){
		ak_print_error_ex(MPI_OSD "get_resolution fail.\n");
		return -1;
	}
	if (ch == VIDEO_CHN_SUB) {
		if ((*width) > CH1_OSD_MAX_WIDTH)
			*width = CH1_OSD_MAX_WIDTH;
	} else {
		if ((*width) > CH0_OSD_MAX_WIDTH)
			*width = CH0_OSD_MAX_WIDTH;
	}

	return 0;
}

/**
 * ak_osd_set_rect - set osd rect position.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @osd_rect[IN]: [0,2], osd display area number. 
 * @xstart[IN]:  start pixel x-ray
 * @ystart[IN]:  start pixel y-ray
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_rect(void *vi_handle, int channel, int osd_rect,
        int xstart, int ystart, int width, int height)
{
	struct osd_channel_param *p_osd;
	int font_size;

	if (channel < 0 || channel > 1) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "channel param err: %d\n", channel);
		return -1;
	}

	p_osd = &osd[channel][osd_rect];
	font_size = p_osd->font_size;
	ak_print_info_ex(MPI_OSD "xstart: %d ystart: %d w: %d h: %d\n",
			xstart, ystart, width,  height);
	if (xstart < 0 || ystart < 0 ||
			xstart > (p_osd->resolution_w - 2 * font_size)
			|| width < font_size || height < font_size)
	{
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "err, x: %d, y: %d, r_w: %d, w: %d, h: %d, f: %d\n",
				xstart, ystart, p_osd->resolution_w, width, height, font_size);
		return -1;
	}

	if (get_resolution(vi_handle, channel, &p_osd->resolution_w,
				&p_osd->resolution_h) < 0) {
		ak_print_error_ex(MPI_OSD "get_resolution fail.\n");
	}

	if (p_osd->resolution_h - ystart < height)
		height = p_osd->resolution_h - ystart;

	if (p_osd->resolution_w - xstart < width)
		width = p_osd->resolution_w - xstart;

	if (width < font_size  || height < font_size) {
		ak_print_error_ex(MPI_OSD "param err, w: %d, h: %d, font: %d\n",
				width, height, font_size);
		return -1;
	}

	ak_print_info_ex(MPI_OSD "x:%d, y:%d, w:%d, h:%d\n",
	    xstart, ystart, width, height);

	return osd_set_rect(channel, osd_rect, xstart, ystart, width, height);
}

/**
 * ak_osd_draw_matrix - draw appointed matrix on screen.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]:  x offset of rect
 * @yoffset[IN]:  y offset of rect
 * @font_w[IN]:  width of font to display
 * @font_h[IN]:   height of font to display
 * @dot_buf[IN]: font data to display on osd
 * @buf_len[IN]: length of font data
 */
int ak_osd_draw_matrix(int channel, int osd_rect, int xoffset, int yoffset,
       int font_w, int font_h, const unsigned char *dot_buf, unsigned int buf_len)
{
	int ret = 0;
	struct osd_channel_param *p_osd = &osd[channel][osd_rect];

	if( channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err\n");
		return -1;
	}

	if(xoffset < 0 || yoffset < 0 || xoffset > (p_osd->osd_w - font_w) ||
		yoffset > (p_osd->osd_h - font_h) || (font_w * font_h)/ 2 > buf_len){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD " para err, x=%d, y=%d width=%d/%d height=%d/%d buf_len=%d\n",
				xoffset, yoffset, font_w, p_osd->osd_w, font_h, p_osd->osd_h, buf_len);
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&osd[channel][osd_rect].lock);
	if((ret = draw_osd_matrix(channel, osd_rect, xoffset, yoffset,
		 font_w, font_h, dot_buf, buf_len)) == 0){
		 int width = font_w;
		 int height = font_h;
		 int x = xoffset;
		 int y = yoffset;

		 if (yoffset >= 1) {
		 	y = yoffset - 1;
			height++;
		 }

		 if (yoffset < p_osd->osd_h - font_h)
		 	height++;

		 if (xoffset >= 1) {
		 	x = xoffset - 1;
			width++;
		 }

		 if (xoffset < p_osd->osd_w - font_w)
		 	width++;
		 	
		 
		 unsigned char *p_osd_buffer = p_osd->p_osd_buffer + (p_osd->osd_w * y) / 2 + x / 2;
		 draw_edge(p_osd_buffer, p_osd->osd_w, width, height);
		 ret = draw_osd(channel, osd_rect);
	}
	ak_thread_mutex_unlock(&osd[channel][osd_rect].lock);

	return ret;
}

/**
 * ak_osd_draw_str - draw string on screen.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]:   x offset of rect
 * @yoffset[IN]:  y offset of rect
 * @disp_str[IN]: display string code, gb2312
 * @str_len[IN]: display string len
 * return: 0 - success; otherwise -1;
 */
int ak_osd_draw_str(int channel, int osd_rect, int xoffset, int yoffset,
		const unsigned short *disp_str, int str_len)
{
	int ret = 0;
	struct osd_channel_param *p_osd ;
	int font_size ;

	if (channel < 0 || channel > 1) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err\n");
		return -1;
	}

	p_osd = &osd[channel][osd_rect];
	font_size = p_osd->font_size;
	if( xoffset < 0 || yoffset < 0 || xoffset > (p_osd->osd_w - font_size) ||
		yoffset > (p_osd->osd_h - font_size) ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err, xo:%d, yo:%d, "
		    "osd_w:%d, osd_h:%d, font_s:%d\n",
		    xoffset, yoffset, p_osd->osd_w, p_osd->osd_h, font_size);
		return -1;
	}

	ak_thread_mutex_lock(&osd[channel][osd_rect].lock);
	if((ret = draw_osd_canvas(channel, osd_rect, xoffset, yoffset,
		disp_str, str_len)) == 0){
		ret = draw_osd(channel, osd_rect);
	}
	ak_thread_mutex_unlock(&osd[channel][osd_rect].lock);

	return ret;
}

/**
 * ak_osd_clean_str - clean rect of osd.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]:  x offset of rect
 * @yoffset[IN]:  y offset of rect
 * @width[IN]:  width of font to display
 * @height[IN]:   height of font to display
 *  return: 0 - success; otherwise -1;
 */
int ak_osd_clean_str(int channel, int osd_rect, int xoffset, int yoffset,
        int width, int height)
{
	int ret = 0;
	struct osd_channel_param *p_osd;
#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	struct osd_str_draw_policy_attr *p_cur_osd_str_draw_policy_attr;
	p_cur_osd_str_draw_policy_attr = &all_osd_str_draw_policy_attr[channel][osd_rect];
#endif

	if( channel < 0 || channel > 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "para err\n");
		return -1;
	}
	p_osd = &osd[channel][osd_rect];
	if( xoffset < 0 || yoffset < 0 || xoffset >= p_osd->osd_w ||
		yoffset >= p_osd->osd_h ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		 ak_print_error_ex(MPI_OSD " param err\n");
		return -1;
	}
	if(p_osd->osd_w - xoffset < width){
		width = p_osd->osd_w - xoffset;
	}
	if(p_osd->osd_h - yoffset < height){
		height = p_osd->osd_h - yoffset;
	}

	ak_thread_mutex_lock(&osd[channel][osd_rect].lock);
	if((ret = clean_osd_canvas(channel, osd_rect, xoffset,yoffset,width,height)) == 0)
		 ret = draw_osd(channel, osd_rect);
#ifdef OSD_STR_POLICY_SKIP_NO_CHANGE
	p_cur_osd_str_draw_policy_attr->be_clear = 1;
#endif
	ak_thread_mutex_unlock(&osd[channel][osd_rect].lock);

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
		ak_print_normal_ex(MPI_OSD "para err\n");
		return -1;
	}

	p_osd->color_tab_front_id = font_color;
	p_osd->color_tab_ground_id	= bg_color;

	return 0;
}

/**
 * ak_osd_set_edge_color - set osd font edge color param.
 * @front_color[IN]:   front color of osd,  [0,15]
 * @bg_color[IN]:     back color of osd, [0,15]
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_edge_color(int edge_color)
{
	struct osd_comm_param *p_osd = &osd_com;

	if (edge_color > 15) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_normal_ex(MPI_OSD "para err\n");
		return -1;
	}

	p_osd->color_tab_edge_id	= edge_color;

	return 0;
}


/**
 * ak_osd_set_alpha - set alpha for osd.
 * @alpha[IN]:  alpha to be set
 *  return: 0 - success; otherwise -1;
 */
int ak_osd_set_alpha(int alpha)
{
	int alpha_denominator = 100;

	if (alpha > alpha_denominator || alpha < 0) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "Invalid argument, allow alpha[0,100]\n");
		return -1;
	}

	osd_com.alpha = (((OPAQUE_ALPHA + CRYSTAL_CLEAR_ALPHA) * alpha)
						/ alpha_denominator);
	ak_print_info_ex(MPI_OSD "setting alpha: %d, internal: %d\n",
	    alpha, osd_com.alpha);

	return 0;
}

/**
 * ak_osd_set_font_size - set osd display font size.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @size[IN]:    display font size 
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_font_size(int channel, int size)
{
	struct osd_channel_param *p_osd;
	
	if (channel > 1 || channel < 0 || size > 96 || size < 16 ) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_normal_ex(MPI_OSD "para err\n");
		return -1;
	}

	for (int j = 0; j < VPSS_OSD_NUM_PER_VIDEO_CHN; j++) {
		p_osd = &osd[channel][j];
		p_osd->font_size = size;		
	}

	return 0;
}

/**
 * ak_osd_set_rect_enable - set osd rect display or not.
 * @channel[IN]:  [0,1],  0 -> main channel, 1 -> sub channel
 * @osd_rect[IN]: [0,2], osd display area number. 
 * @enable[IN]:  1 enable, 0 disable
 * return: 0 - success; otherwise -1;
 */
int ak_osd_set_rect_enable(int channel, int osd_rect, int enable)
{
    int ret;

	if (channel < 0 || channel > 1) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "channel param err: %d\n", channel);
		return -1;
	}

    if (osd_rect < 0 || osd_rect > 2) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex(MPI_OSD "osd_rect param err: %d\n", osd_rect);
		return -1;
	}

	if (enable)
        ret = draw_osd(channel, osd_rect);
    else
        ret = clear_osd(channel, osd_rect);

    return ret;	
}


/**
 * ak_osd_destroy - free osd resource
 * return: none
 */
void ak_osd_destroy(void)
{
	int i, j;

	osd_sys_ipc_unregister();
	for(i=0; i<VIDEO_CHN_NUM; ++i)
	for(j=0; j<VPSS_OSD_NUM_PER_VIDEO_CHN; j++) {
		clear_osd(i, j);

		ak_thread_mutex_lock(&osd[i][j].lock);
		if (osd[i][j].p_osd_buffer) {
			akuio_free_pmem(osd[i][j].p_osd_buffer);
			osd[i][j].p_osd_buffer = NULL;
		}

		/* release dma memory */
		if(osd[i][j].dma_vaddr){
			akuio_free_pmem(osd[i][j].dma_vaddr);
			osd[i][j].dma_vaddr = NULL;
		}
		ak_thread_mutex_unlock(&osd[i][j].lock);

		ak_thread_mutex_destroy(&osd[i][j].lock);
	}

	free_font_data();
}
