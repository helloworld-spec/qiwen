#ifndef _ISP_CONF_FILE_H_
#define _ISP_CONF_FILE_H_


#define ISP_CFG_MAX_SIZE		(160*1024)



struct cfg_file_head {
	unsigned int main_version;
	char file_version[16];
	unsigned int sensor_id;

	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char subfile_id;

	unsigned char style_id;
	unsigned char reserve1;
	unsigned short reserve2;
	unsigned int subfile_len;

	unsigned char reserve3[88];

	char notes[384];
};


enum load_mode {
	LOAD_MODE_DAY_OUTDOOR,
	LOAD_MODE_NIGHT_ISP,
	LOAD_MODE_CUSTOM_2,
	LOAD_MODE_CUSTOM_3,
	LOAD_MODE_WHOLE_FILE,
	LOAD_MODE_NUM
};

/************************** for  isptuner **********************************/
/**  
 * brief: load config file data
 * @mode[IN]:mode in enum load_mode
 * @buf[OUT]:config data buf
 * @size[OUT]: size of config data
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_cfg_file_load(enum load_mode mode, char *buf, unsigned int *size);

/**  
 * brief: store config file data
 * @buf[IN]:config data buf
 * @size[IN]: size of config data
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_cfg_file_store(char *buf, unsigned int size);

/**  
 * brief: set config file path
 * @config_file[IN]:the config file name(include path and name), must be rw area
 * return: 0 success, -1 failed
 * notes: 
 */
void isp_cfg_file_set_path(const char *config_file);

/************************** for  internal other modules **********************************/
/**  
 * brief: get style id
 * @void
 * return: style id
 * notes: 
 */
unsigned char isp_cfg_file_get_style_id(void);

/**  
 * brief: set style id
 * @style_id[IN]: id to set
 * return: 0 success, -1 failed
 * notes: 
 */
int isp_cfg_file_set_style_id(unsigned char style_id);


/**
 * brief: get config file head info
 * @subfile_idx[IN]:subfile index
 * @headinfo[OUT]:head info
 * return: 0 success, -1 failed
 * notes:
 */
int isp_cfg_file_get_headinfo(unsigned int subfile_id, struct cfg_file_head *headinfo);


#endif
