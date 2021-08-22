#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os_malloc.h"
#include "isp_struct.h"
//#include "isp_h42.conf.h"
#include "ispdrv_modules_interface.h"
#include "isp_conf.h"

#define anyka_err	printf
#define anyka_print	printf
#define malloc	Fwl_Malloc
#define free	Fwl_Free

#define ISP_MODULE_ID_SIZE		2
#define ISP_MODULE_LEN_SIZE		2
//#define ISP_MODULE_HEAD_SIZE	(ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE)
#define SUBFILE_NUM_MAX			5
#define NOTES_DISABLE			1
#define NOTES_ENABLE			2
#define STYLEID_MIN				3
#define CFG_FILE_NOTES_LEN		300
#define FVPR_AWB_STEP			10

typedef struct subfile_info               
{
	unsigned long	cnt;
	signed long	offset[SUBFILE_NUM_MAX];
	unsigned long	filelen[SUBFILE_NUM_MAX];
}SUBFILE_INFO;


struct vfpr_info {
	int default_auto_awb_step;
	int is_init;
};

static unsigned char *g_modules_ptr_day[ISP_HUE+2];
static unsigned char *g_modules_ptr_night[ISP_HUE+2];
static unsigned char *g_isp_conf_cache = NULL;
static struct vfpr_info g_vfpr;

/**
 * @brief calulate modules configuration address
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] cfgBuf address of the configuration
 * @param[out] modules_ptr all modules address
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
static bool isp_modules_offset(unsigned char* cfgBuf, unsigned char **modules_ptr)
{
	unsigned char i = 0;
	unsigned long total = 0;
	unsigned short moduleId = 0;
	unsigned short length = 0;
	unsigned long offset = 0;

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//anyka_print("i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if (moduleId != i)
		{
			anyka_print("[%s] data err!\n", __func__);
			return false;
		}

		modules_ptr[i] = cfgBuf + offset;// + ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE;

		offset += length;
		total += length;


#if 0
		if (i==ISP_BB) {
			AK_ISP_BLC *blc = cfgBuf - base + modules_ptr[i] + 2 + sizeof(AK_ISP_BLC);
			printf("blc offset:%d+%d+%d, black_level_enable:%u,bl_r_a:%u,bl_gr_a:%u,bl_gb_a:%u,bl_b_a:%u,bl_r_offset:%d,bl_gr_offset:%d,bl_gb_offset:%d,bl_b_offset:%d\n",
				   	modules_ptr[i], 2 , sizeof(AK_ISP_BLC), blc->black_level_enable,blc->bl_r_a,blc->bl_gr_a,blc->bl_gb_a,blc->bl_b_a,blc->bl_r_offset,blc->bl_gr_offset,blc->bl_gb_offset,blc->bl_b_offset);

			printf("sizeof(AK_ISP_BLC_ATTR):%d", sizeof(AK_ISP_BLC_ATTR));
		}
#endif
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		anyka_print("[%s] sensor id err!\n", __func__);
		return false;
	}

	modules_ptr[i] = cfgBuf + offset;// + ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE;

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;


	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;

	//anyka_print("[%s] Check OK!\n", __func__);

	return true;
}

/**
 * @brief check modules of isp configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] cfgBuf address of the configuration
 * @param[in] size sizeof the memory
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
static bool isp_module_cfg_check(unsigned char* cfgBuf, unsigned long* size)
{
	unsigned char i = 0;
	unsigned long total = 0;
	unsigned short moduleId = 0;
	unsigned short length = 0;
	unsigned long offset = 0;

	if (NULL == cfgBuf || 0 == *size)
	{
		anyka_print("[%s] cfgBuf is null or size is 0, size:%lu!\n", __func__, *size);
		return false;
	}

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//anyka_print("i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if (moduleId != i)
		{
			anyka_print("[%s] data err!\n", __func__);
			return false;
		}

		offset += length;
		total += length;

		if (offset > *size)
		{
			anyka_print("[%s] size err:%lu!\n", __func__, *size);
			return false;
		}
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		anyka_print("[%s] sensor id err!\n", __func__);
		return false;
	}

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;

	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;

	if (offset > *size)
	{
		anyka_print("[%s] size err:%lu!\n", __func__, *size);
		return false;
	}

	*size = total;

	//anyka_print("[%s] Check OK!\n", __func__);

	return true;
}

/**
 * @brief check isp configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] mode day or night configuration
 * @param[in] isp_conf_buf memory address to the configuration
 * @param[in] isp_conf_len memory length
 * @param[out] isp_offset offset of the memory to the mode
 * @param[out] modules_ptr address of the memory to the mode
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
static bool isp_conf_data_check(T_LOAD_MODE mode, unsigned char *isp_conf_buf,
		unsigned long isp_conf_len, unsigned long *isp_offset, unsigned char **modules_ptr)
{
	unsigned long filesize = 0;
	unsigned char *cfgBuf = isp_conf_buf;
	CFGFILE_HEADINFO headinfo = {0};
	SUBFILE_INFO subfileinfo = {0};
	unsigned char i = 0;
	unsigned long total = 0;
	unsigned long offset = 0;
	unsigned long isp_size = 0;
	unsigned char filestyleid = 0;

	if ((NULL == isp_offset) || (mode >= LOAD_MODE_NUM))
	{
		anyka_err("[%s] param err\n", __func__);
		return false;
	}

	subfileinfo.cnt = 0;

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		subfileinfo.offset[i] = -1;
	}

	filesize = isp_conf_len;

CHECK_ONE_SUBFILE:
	memcpy(&headinfo, cfgBuf + offset, sizeof(CFGFILE_HEADINFO));

	filestyleid = 0;

	if (headinfo.styleId < STYLEID_MIN)
	{
		filestyleid = 0;
	}
	else
	{
		filestyleid = headinfo.styleId - STYLEID_MIN;
	}

	if (3 != headinfo.main_version)
	{
		if (headinfo.main_version & 0x0000ff00)
		{
			anyka_print("[%s] cfg file is old version, v2!\n", __func__);
		}
		else
		{
			anyka_print("[%s] cfg file is old version, v1!\n", __func__);
		}

		return false;
	}
	
	if ((headinfo.year < 1900)
		|| (headinfo.month > 12 || headinfo.month < 1)
		|| (headinfo.day > 31 || headinfo.day < 1)
		|| (headinfo.hour > 23)
		|| (headinfo.minute > 59)
		|| (headinfo.second > 59)
		|| (headinfo.subFileId > 4))
	{
		anyka_print("[%s] headinfo err!\n", __func__);
		return false;
	}
#if 0
	if (0 == headinfo.styleId)	//V1 version old cfg file
	{
		anyka_print("[%s] cfg file is old version!\n", __func__);
		return false;
	}
#endif
#if 0
	anyka_print("[%s] sensorId:0x%x, styleId:%d\n", 
			__func__, headinfo.sensorId, filestyleid);
#endif


	offset += sizeof(CFGFILE_HEADINFO);
	total += sizeof(CFGFILE_HEADINFO);
	subfileinfo.filelen[subfileinfo.cnt] += sizeof(CFGFILE_HEADINFO);

	isp_size = filesize - offset;
	
	if (!isp_module_cfg_check(cfgBuf + offset, &isp_size))
	{
		anyka_print("[%s] isp data err!\n", __func__);
		return false;
	}

	offset += isp_size;
	total += isp_size;
	subfileinfo.filelen[subfileinfo.cnt] += isp_size;

	if (NOTES_ENABLE <= headinfo.styleId)	//has notes
	{
		offset += CFG_FILE_NOTES_LEN;
		total += CFG_FILE_NOTES_LEN;
		subfileinfo.filelen[subfileinfo.cnt] += CFG_FILE_NOTES_LEN;
	}
	else if (NOTES_DISABLE == headinfo.styleId)
	{
		
	}

	if (0 == subfileinfo.cnt)
	{
		subfileinfo.offset[headinfo.subFileId] = 0;
	}
	else
	{
		subfileinfo.offset[headinfo.subFileId] = subfileinfo.filelen[subfileinfo.cnt-1];
	}

	subfileinfo.cnt++;

	if (filesize > total && subfileinfo.cnt < SUBFILE_NUM_MAX)
	{
		goto CHECK_ONE_SUBFILE;
	}

	if (LOAD_MODE_WHOLE_FILE == mode)
	{
		*isp_offset = 0;
	}
	else
	{
		if (-1 == subfileinfo.offset[mode])
		{
			anyka_print("[%s] no this subfile! mode : %d\n", __func__, mode);
			return false;
		}

		*isp_offset = subfileinfo.offset[mode] + sizeof(CFGFILE_HEADINFO);

		isp_modules_offset(cfgBuf + *isp_offset, modules_ptr);
	}

	if (subfileinfo.cnt < 2)
	{
		anyka_print("[%s] subfile is not enough, cnt : %lu\n", __func__, subfileinfo.cnt);
		return false;
	}

#if 0
	anyka_print("[isp.conf]version: %s, sensor id: 0x%x, style id: %d, modify time: %d-%d-%d %02d:%02d:%02d!\n", 
			headinfo.file_version, headinfo.sensorId, filestyleid, 
			headinfo.year, headinfo.month, headinfo.day,
			headinfo.hour, headinfo.minute, headinfo.second);

	anyka_print("[%s] OK subfile cnt = %lu!\n", __func__, subfileinfo.cnt);
#endif
	return true;
}

/**
 * @brief call ispdrv to set isp modules
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] cmd modue id
 * @param[in] cm_buf cmd's param
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
static bool isp_conf_call_set_ispdrv(unsigned long cmd, unsigned char *cmd_buf)
{
	bool ret = true;

	switch (cmd) {
		case ISP_BB:
			{
				AK_ISP_BLC *blc = (AK_ISP_BLC *)(cmd_buf + 4 + 2 + sizeof(AK_ISP_BLC));
				//printf(" black_level_enable:%u,bl_r_a:%u,bl_gr_a:%u,bl_gb_a:%u,bl_b_a:%u,bl_r_offset:%d,bl_gr_offset:%d,bl_gb_offset:%d,bl_b_offset:%d\n",
					//	blc->black_level_enable,blc->bl_r_a,blc->bl_gr_a,blc->bl_gb_a,blc->bl_b_a,blc->bl_r_offset,blc->bl_gr_offset,blc->bl_gb_offset,blc->bl_b_offset);
			}
			break;
		case ISP_LSC:
			break;
		case ISP_RAW_LUT:
			break;
		case ISP_NR:
			break;
		case ISP_3DNR:
			break;

		case ISP_GB:
			break;
		case ISP_DEMO:
			break;
		case ISP_GAMMA:
			break;
		case ISP_CCM:
			break;
		case ISP_FCS:
			break;

		case ISP_WDR:
			break;

		case ISP_SHARP:
			break;
		case ISP_SATURATION:
			break;
		case ISP_CONSTRAST:
			break;

		case ISP_RGB2YUV:
			break;
		case ISP_YUVEFFECT:
			break;
		case ISP_DPC:
			break;
		case ISP_WEIGHT:
			break;
		case ISP_AF:
			break;

		case ISP_WB:
			break;
		case ISP_EXP:
			break;
		case ISP_MISC:
			break;
		case ISP_Y_GAMMA:
			break;
		case ISP_HUE:
			break;

		case ISP_SENSOR:
			{
				int i;
				AK_ISP_INIT_SENSOR *p = (AK_ISP_INIT_SENSOR *)cmd_buf;
				unsigned short *psensor = (unsigned short *)(&p->p_sensor);
				//printf("ISP_SENSOR length:%d\n", p->length);
				//for (i=0;i<=(p->length - 4) / 2; i+=2)
					//printf("0x%02x, 0x%02x\n", psensor[i], psensor[i+1]);
			}
			break;
		default:
			ret = false;
			break;
	}

	if (ret == true)
	{
		ret = ispdrv_set_module(cmd, cmd_buf);
	}

	return ret;
}

/**
 * @brief set sensor initial configuration
 * @author ye_guohong   
 * @date 2017-03-14
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_sensor_init_conf(void)
{
	bool ret = false;
	int i;
	unsigned char **modules_ptr_day = &g_modules_ptr_day[0];

	for (i = ISP_BB; i <= ISP_HUE; i++) {
	}

	//sensor
	ret = isp_conf_call_set_ispdrv(ISP_SENSOR, modules_ptr_day[i - ISP_BB]);

	return ret;
}

/**
 * @brief set day configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_day(void)
{
	bool ret = false;
	int i;
	unsigned char **modules_ptr_day = &g_modules_ptr_day[0];

	for (i = ISP_BB; i <= ISP_HUE; i++) {
		ret = isp_conf_call_set_ispdrv(i, modules_ptr_day[i - ISP_BB]);
		if (ret == false) {
			printf("%s %d ,i:%d\n",__func__,__LINE__,i);
			goto end;
		}
	}

	//sensor
	//ret = isp_conf_call_set_ispdrv(ISP_SENSOR, modules_ptr_day[i - ISP_BB]);

end:
	return ret;
}

/**
 * @brief set night configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_night(void)
{
	bool ret = false;
	int i;
	unsigned char **modules_ptr_night = &g_modules_ptr_night[0];

	for (i = ISP_BB; i <= ISP_HUE; i++) {
		if (i == ISP_MISC)
			continue;
		ret = isp_conf_call_set_ispdrv(i, modules_ptr_night[i - ISP_BB]);
		if (ret == false)
			goto end;
	}

	//sensor ymx: delete. no night sensor.
	//ret = isp_conf_call_set_ispdrv(ISP_SENSOR, modules_ptr_night[i - ISP_BB]);	

end:
	return ret;
}

/**
 * @brief initiation for isp conf module
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_conf_buf basic memory address to isp_conf file
 * @param[in] len memory length
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_init(unsigned char *isp_conf_buf, unsigned long len)
{
	bool ret;
	unsigned long isp_offset_day;
	unsigned long isp_offset_night;
	unsigned char **modules_ptr_day = &g_modules_ptr_day[0];
	unsigned char **modules_ptr_night = &g_modules_ptr_night[0];

	memset(g_modules_ptr_day, 0, sizeof(g_modules_ptr_day)/sizeof(g_modules_ptr_day[0]));
	memset(g_modules_ptr_night, 0, sizeof(g_modules_ptr_night)/sizeof(g_modules_ptr_night[0]));

	g_isp_conf_cache = malloc(len);
	memset(g_isp_conf_cache, 0, len);
	if (g_isp_conf_cache == NULL) {
		printf("malloc fail\n");
		return false;
	}
	memcpy(g_isp_conf_cache, isp_conf_buf, len);

	ret = isp_conf_data_check(LOAD_MODE_DAY_ISP, g_isp_conf_cache, len, &isp_offset_day, modules_ptr_day);
	if (ret == false) {
		goto fail;
	}

	ret = isp_conf_data_check(LOAD_MODE_NIGHT_ISP, g_isp_conf_cache, len, &isp_offset_night, modules_ptr_night);
	if (ret == false) {
		goto fail;
	}

	//isp_conf_set_day(); //2017.2.7

	return true;

fail:
	free(g_isp_conf_cache);
	g_isp_conf_cache = NULL;
	return false;
}

/**
 * @brief deinitiation for isp conf module
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_deinit(void)
{
	if (g_isp_conf_cache) {
		free(g_isp_conf_cache);
		g_isp_conf_cache = NULL;
	}

	return true;
}

/**
 * @brief start fvpr
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_fvpr_start(void)
{
	unsigned int new_step = FVPR_AWB_STEP;
	AK_ISP_AWB_ATTR 		   awb_para;

	if (false == ispdrv_get_module(ISP_WB_SUB_AWB, (unsigned char *)&awb_para)) {
		printf("[%s] get ISP_WB_SUB_AWB fail\n", __func__);
		return false;
	}

	g_vfpr.default_auto_awb_step = awb_para.auto_wb_step;
	awb_para.auto_wb_step = new_step;

	if (false == ispdrv_set_module(ISP_WB_SUB_AWB, (unsigned char *)&awb_para)) {
		printf("[%s] set ISP_WB_SUB_AWB fail\n", __func__);
		return false;
	}

	g_vfpr.is_init = true;
	printf("%s get default awb_step:%u, set new_step:%u\n", __func__,\
			g_vfpr.default_auto_awb_step, new_step);
	return true;
}

/**
 * @brief finish fvpr
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_fvpr_finish(void)
{
	AK_ISP_AWB_ATTR 		   awb_para;

	if (g_vfpr.is_init != true) {
		printf("%s vfpr is not init\n", __func__);
		return false;
	}
		
	if (false == ispdrv_get_module(ISP_WB_SUB_AWB, (unsigned char *)&awb_para)) {
		printf("[%s] get ISP_WB_SUB_AWB fail\n", __func__);
		return false;
	}

	awb_para.auto_wb_step = g_vfpr.default_auto_awb_step;

	if (false == ispdrv_set_module(ISP_WB_SUB_AWB, (unsigned char *)&awb_para)) {
		printf("[%s] set ISP_WB_SUB_AWB fail\n", __func__);
		return false;
	}

	g_vfpr.is_init = false;
	printf("%s set default awb_step:%u\n", __func__, awb_para.auto_wb_step);

	return true;
}

/**
 * @brief pause isp
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true  successfully
 * @retval false  unsuccessfully
 */
bool isp_pause(void)
{
	return ispdrv_isp_pause();
}
/**
 * @brief resume isp
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true  successfully
 * @retval false  unsuccessfully
 */
bool isp_resume(void)
{
	return ispdrv_isp_resume();
}

#if 0
int main()
{
	isp_conf_init(isp_conf_file, sizeof(isp_conf_file));

#if 0

	AK_ISP_AE_ATTR          *ae_day = modules_ptr_day[ISP_EXP] + sizeof(AK_ISP_RAW_HIST_ATTR) +  \
						sizeof(AK_ISP_RGB_HIST_ATTR) + sizeof(AK_ISP_YUV_HIST_ATTR) + sizeof(AK_ISP_EXP_TYPE) + sizeof(AK_ISP_ME_ATTR);
	AK_ISP_AE_ATTR          *ae_night = modules_ptr_night[ISP_EXP] + sizeof(AK_ISP_RAW_HIST_ATTR) + \
						sizeof(AK_ISP_RGB_HIST_ATTR) + sizeof(AK_ISP_YUV_HIST_ATTR) + sizeof(AK_ISP_EXP_TYPE) + sizeof(AK_ISP_ME_ATTR);

	printf("offset:%u&%u,\
			max_exp:%u&%u,\
			\n",
			isp_offset_day, isp_offset_night,
			ae_day->exp_time_max, ae_night->exp_time_max
			);

#endif

	isp_conf_deinit();
}
#endif
