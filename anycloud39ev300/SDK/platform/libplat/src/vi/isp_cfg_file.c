#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "isp_basic.h"
#include "isp_cfg_file.h"

#include "ak_common.h"

#define ISP_CFG		                "<isp_cfg>"


#define SUBFILE_NUM_MAX					(5)		//style files counter
#define CFG_FILE_NOTES_LEN				(300)

#define SENSOE_ID_PATH					"/sys/ak_info_dump/sensor_id"
#define STYLE_ID_PATH					"/etc/jffs2/styleId"

#define PATH_SIZE			(256) 		//file's absolutely path size
#define ISP_VERSION_MAIN	(4)	  		//V4.x.xx
#define ISP_SUBFILE_NUM 	(2)	  		//now we have 2 sub file

static FILE *isp_cfg_fp = NULL;
static char isp_cfg_file_name[PATH_SIZE] = {0};
static char cfg_file_path[PATH_SIZE] = {0};

struct subfile_info {
	unsigned int cnt;
	int offset[SUBFILE_NUM_MAX];
	unsigned int filelen[SUBFILE_NUM_MAX];
};


/**
 * brief: create file for store  isp cfg data, if exist, overwrite
 * return: 0 success, -1 failed
 * notes:
 */
static int isp_cfg_create_file(void)
{
	if (isp_cfg_fp) {
		fclose(isp_cfg_fp);
	}

	isp_cfg_fp = fopen(isp_cfg_file_name, "w+b");
	if (!isp_cfg_fp) {
		ak_print_error_ex(ISP_CFG "open %s failed, %s\n", isp_cfg_file_name, strerror(errno));
		return -1;
	}

	return 0;
}

/**
 * brief: close isp cfg file
 * return: 0 success, -1 failed
 * notes:
 */
static void isp_cfg_close_file(void)
{
	if (isp_cfg_fp) {
		fclose(isp_cfg_fp);
		isp_cfg_fp = NULL;
	}
}

/**
 * brief: read data from isp cfg file
 * @buf[IN]: buf to load data
 * @size[IN]:   size to read
 * @offset[IN]: offset
 * return: 0 success, -1 failed
 * notes:
 */
static int read_data(char *buf, unsigned int size, unsigned int offset)
{
	if (!buf) {
		ak_print_error_ex(ISP_CFG "invalid argument, buf is null!\n");
		return -1;
	}

	if (!isp_cfg_fp) {
		ak_print_error_ex(ISP_CFG "%s read_data buf NULL!\n", __func__);
		return -1;
	}

	fseek(isp_cfg_fp, offset, SEEK_SET);
	if (size != fread(buf, 1, size, isp_cfg_fp)) {
		ak_print_error_ex(ISP_CFG "read failed!\n");
		return -1;
	}
	return 0;
}


/**
 * brief: open isp cfg file ,only read
 * @path[IN]:   file path
 * return: file handle
 * notes:
 */
static void *isp_config_file_open(const char *path)
{
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		ak_print_error_ex(ISP_CFG "open %s failed, %s\n", path, strerror(errno));
		return NULL;
	}

	return fp;
}


/**
 * brief: get file size
 * @file[IN]:   file handle
 * return: >0 file size; -1, failed
 * notes:
 */
static int isp_config_get_file_size(void *file)
{
	if (!file)
		return -1;

	FILE *fp = (FILE *)file;
	fseek(fp, 0, SEEK_END);
	
	return ftell(fp);
}


/**
 * brief: check data
 * @mode[IN]:  load mode
 * @path[IN]:  file path
 * @sensor_id[IN]:  sensor id for match
 * @style_id[IN]:  style id for match
 * @isp_offset[OUT]:  data offset
 * return: 0 success; -1 failed
 * notes:
 */
static int check_data(enum load_mode mode, const char *path,
	   	unsigned int sensor_id, unsigned char style_id, unsigned int *isp_offset)
{
	if (!path || !isp_offset || (mode >= LOAD_MODE_NUM)) {
		ak_print_error_ex(ISP_CFG "invalid argument, path: %s, offset: %p, mode: %d\n",
			path, isp_offset, mode);
		return AK_FAILED;
	}

	FILE *fp = isp_config_file_open(path);
	if (!fp)
		return AK_FAILED;

	int ret = AK_FAILED;
	char *buf = NULL;
	
	/* get file size and check it */
	int file_size = isp_config_get_file_size(fp);
	if (file_size <= sizeof(struct cfg_file_head)) {
		ak_print_error_ex(ISP_CFG "file_size too small");
		goto check_data_end;
	}

	int buf_len = file_size + 16;	//for what +16?
	buf = (char *)calloc(1, buf_len);
	if (!buf) {
		ak_print_error_ex(ISP_CFG "calloc failed, size: %d\n", buf_len);
		goto check_data_end;
	}
	
	/* load new config file to buf */ 
	rewind(fp);
	fread(buf, 1, file_size, fp);

	int i = 0;
	struct cfg_file_head headinfo = {0};
	struct subfile_info subfileinfo = {0};

	for (i = 0; i < SUBFILE_NUM_MAX; i++)
		subfileinfo.offset[i] = -1;

	unsigned int total = 0;		//total offset
	unsigned int offset = 0;	//subfile offset
	unsigned char file_style_id = 0;

	do {
		memcpy(&headinfo, buf + offset, sizeof(struct cfg_file_head));
		file_style_id = headinfo.style_id;

		if (headinfo.main_version != ISP_VERSION_MAIN) {
			ak_print_error_ex(ISP_CFG "config file: %s is old version, main_version:%d\n",
				path,headinfo.main_version);
			goto check_data_end;
		}

		if ((headinfo.year < 1900) || (headinfo.month > 12 || headinfo.month < 1) ||
				(headinfo.day > 31 || headinfo.day < 1) || (headinfo.hour > 23) ||
				(headinfo.minute > 59) || (headinfo.second > 59) || 
				(headinfo.subfile_id > 4)) {
			ak_print_error_ex(ISP_CFG "file: %s head info err!\n", path);
			goto check_data_end;
		}

		if ((sensor_id != headinfo.sensor_id) || (style_id != file_style_id)) {
			ak_print_warning_ex(ISP_CFG "%s is not need file!\n", path);
			ak_print_normal_ex(ISP_CFG "sensor_id:0x%x, 0x%x, style_id:%d, %d\n",
				sensor_id, headinfo.sensor_id, style_id, file_style_id);
			goto check_data_end;
		}

		offset += sizeof(struct cfg_file_head);
		total += sizeof(struct cfg_file_head);
		subfileinfo.filelen[subfileinfo.cnt] += sizeof(struct cfg_file_head);

		unsigned int isp_size = file_size - offset;

		if (isp_module_check_cfg(buf + offset, &isp_size) < 0) {
			ak_print_error_ex(ISP_CFG "%s isp data err!\n", path);
			goto check_data_end;
		}

		offset += isp_size;
		total += isp_size;
		subfileinfo.filelen[subfileinfo.cnt] += isp_size;

		if (0 == subfileinfo.cnt) {
			subfileinfo.offset[headinfo.subfile_id] = 0;
		} else {
			subfileinfo.offset[headinfo.subfile_id] = subfileinfo.offset[subfileinfo.cnt-1] + subfileinfo.filelen[subfileinfo.cnt-1];
		}

		subfileinfo.cnt++;
	} while (file_size > total && subfileinfo.cnt < SUBFILE_NUM_MAX);

	if (LOAD_MODE_WHOLE_FILE == mode) {
		*isp_offset = 0;
	} else {
		if (-1 == subfileinfo.offset[mode]) {
			ak_print_error_ex(ISP_CFG "%s no this subfile! mode : %d\n", path, mode);
			goto check_data_end;
		}
		*isp_offset = subfileinfo.offset[mode] + sizeof(struct cfg_file_head);
	}

	if (subfileinfo.cnt < 2) {
		ak_print_error_ex(ISP_CFG "subfile %s is not enough, cnt: %u\n",
				path, subfileinfo.cnt);
		goto check_data_end;
	}

	ret = AK_SUCCESS;
	ak_print_notice(ISP_CFG "[isp.conf]version: %s, sensor id: 0x%x, style id: %d\n",
		headinfo.file_version, headinfo.sensor_id, file_style_id);

	for (i=0; i<subfileinfo.cnt; i++) {
		memcpy(&headinfo, buf + subfileinfo.offset[i], sizeof(struct cfg_file_head));
		
		ak_print_normal(ISP_CFG "isp subfile %d, modify time: %d-%d-%d %02d:%02d:%02d\n", i, 
			headinfo.year, headinfo.month, headinfo.day,
			headinfo.hour, headinfo.minute, headinfo.second);
		ak_print_normal(ISP_CFG "%s\n\n", headinfo.notes);
	}
	
check_data_end:
	if (buf) {
		free(buf);
		buf = NULL;
	}
	if (fp) {
		fclose(fp);
		fp = NULL;
	}

	return ret;
}

/*
 * isp_get_sensor_id - get sensor id
 * return: on success return sensor id, failed return -1
 */
static int isp_cfg_get_sensor_id(void)
{
	FILE *fp = fopen(SENSOE_ID_PATH, "r");
	if ((!fp) && (errno == ENOENT)) {
		ak_print_error_ex(ISP_CFG "cannot get sensor id, check your camera driver\n");
		return -1;
	}

	char id[8] = {0};
	fread(id, sizeof(id), 1, fp);
	fclose(fp);

	int sensor_id = strtol(id, NULL, 16);
	if (sensor_id < 0) {
		ak_print_error_ex(ISP_CFG "sensor id error\n");
		return -1;
	}

	return sensor_id;
}



/**
 * brief: check file
 * @mode[IN]:  load mode
 * @path[IN]:  file path
 * @isp_offset[OUT]:  data offset
 * return: 0 success; -1 failed
 * notes:
 */
static int check_file(enum load_mode mode, const char *path,
	   	unsigned int *isp_offset)
{
	/*
	 * check file, 
	 * isp_offset will store the start position which 
	 * will be use for load data 
	 */
	int sensor_id = isp_cfg_get_sensor_id();
	unsigned char style_id = isp_cfg_file_get_style_id();

	if (check_data(mode, path, sensor_id, style_id, isp_offset)) {
		ak_print_normal_ex(ISP_CFG "read sensor fail\n");
		return -1;
	}
	/* 
	 * check config file,
	 * on success, store path to global val
	 */
	memset(isp_cfg_file_name, 0x00, sizeof(isp_cfg_file_name));
	strcpy(isp_cfg_file_name, path);
	ak_print_normal_ex(ISP_CFG "check isp cfg: %s OK\n", isp_cfg_file_name);

	return 0;
}


/**
 * brief: load file data without check
 * @buf[IN]:  buf to load
 * @size[OUT]:  data size
 * @isp_offset[IN]:  data offset
 * return: 0 success; -1 failed
 * notes:
 */
static int load_nocheck(char *buf, unsigned int *size, unsigned int isp_offset)
{
	int ret = -1;
	unsigned int filesize = 0;

	if (!buf || !size) {
		ak_print_error_ex(ISP_CFG "param NULL!\n");
		return ret;
	}

	if (!isp_cfg_fp) {
		if ((isp_cfg_fp = isp_config_file_open(isp_cfg_file_name)) == NULL)
			return -1;
	}

	filesize = isp_config_get_file_size(isp_cfg_fp);

	if (*size < filesize - isp_offset) {
		ak_print_error_ex(ISP_CFG "buf size is too small!, now: %u, should be %u\n",
				*size, filesize - isp_offset);
		*size = 0;
		isp_cfg_close_file();
		return ret;
	}
	*size = filesize - isp_offset;
	ret = read_data(buf, *size, isp_offset);

	isp_cfg_close_file();

	return ret;
}


/**
 * brief: write data to isp cfg file
 * @buf[IN]: data buf
 * @size[IN]:   size to write
 * @offset[IN]: offset
 * return: 0 success, -1 failed
 * notes:
 */
static int write_data(char *buf, unsigned int size, unsigned int offset)
{
	if (NULL == buf) {
		ak_print_error(ISP_CFG "%s write_data buf NULL!\n", __func__);
		return -1;
	}

	if (NULL == isp_cfg_fp)
		isp_cfg_fp = fopen (isp_cfg_file_name, "r+b");

	if (NULL != isp_cfg_fp) {
		fseek(isp_cfg_fp, offset, SEEK_SET);
		if (size == fwrite(buf, 1, size, isp_cfg_fp))
			return 0;
		else
			ak_print_error(ISP_CFG "%s isp_cfg_fp write failed!\n", __func__);
	} else 
		ak_print_error(ISP_CFG "%s isp_cfg_fp open failed!\n", __func__);

	return -1;
}

/**
 * brief: set config file path
 * @path[IN]:the config file name(include path and name), must be rw area
 * notes:
 */
void isp_cfg_file_set_path(const char *file_path)
{
	if (file_path) {
        memset(cfg_file_path, 0, PATH_SIZE);
		strncpy(cfg_file_path, file_path, strlen(file_path));
	}
}

void isp_cfg_file_clear_path(void)
{
	bzero(cfg_file_path, sizeof(cfg_file_path));
}
/**
 * brief: get style id
 * @void
 * return: style id
 * notes:
 */
unsigned char isp_cfg_file_get_style_id(void)
{
	unsigned char style_id = 0;
	FILE *fp = fopen (STYLE_ID_PATH, "rb");
	if (NULL != fp) {
		fread(&style_id, 1, 1, fp);
		fclose(fp);
	}

	return style_id;
}

/**
 * brief: set style id
 * @style_id[IN]: id to set
 * return: 0 success, -1 failed
 * notes:
 */
int isp_cfg_file_set_style_id(unsigned char style_id)
{
	unsigned char id = style_id;
	FILE *fp = fopen (STYLE_ID_PATH, "wb");
	if (NULL != fp) {
		fwrite(&id, 1, 1, fp);
		fclose(fp);
		return 0;
	}

	return -1;
}

/**
 * brief: load config file data
 * @mode[IN]:mode in enum load_mode
 * @buf[OUT]:config data buf
 * @size[OUT]: size of config data
 * return: 0 success, -1 failed
 * notes:
 */
int isp_cfg_file_load(enum load_mode mode, char *buf, unsigned int *size)
{
	unsigned int isp_offset = 0;

	if (check_file(mode, cfg_file_path, &isp_offset))
	{
	    if (LOAD_MODE_DAY_OUTDOOR == mode)
			isp_cfg_file_clear_path();
	}

	else
		return load_nocheck(buf, size, isp_offset);

	return -1;
}

/**
 * brief: store config file data
 * @buf[IN]:config data buf
 * @size[IN]: size of config data
 * return: 0 success, -1 failed
 * notes:
 */
int isp_cfg_file_store(char *buf, unsigned int size)
{
	int ret = -1;

	if (NULL == buf || 0 == size) {
		ak_print_error_ex(ISP_CFG "param NULL!\n");
	   	return ret;
	}

	if (0 == isp_cfg_create_file())
		ret = write_data(buf, size, 0);
	else
		ak_print_error(ISP_CFG "%s isp_cfg_fp open failed!\n", __func__);
	isp_cfg_close_file();

	return ret;
}

/**
 * brief: get config file head info
 * @subfile_idx[IN]:subfile index
 * @headinfo[OUT]:head info
 * return: 0 success, -1 failed
 * notes:
 */
int isp_cfg_file_get_headinfo(unsigned int subfile_id, struct cfg_file_head *headinfo)
{
	int ret = -1;
	int size = sizeof(struct cfg_file_head);
	int offset = 0;
	int i = 0;
	struct cfg_file_head head = {0};
	FILE *fp = NULL;

	if (NULL == headinfo) {
		ak_print_error_ex(ISP_CFG "headinfo NULL!\n");
	   	return ret;
	}


	if ((fp = isp_config_file_open(isp_cfg_file_name)) == NULL)
		return ret;


	while (i < subfile_id) {
		fseek(fp, offset, SEEK_SET);
		if (size != fread(&head, 1, size, fp)) {
			ak_print_error_ex(ISP_CFG "read failed!\n");
			goto exit;
		}

		offset += head.subfile_len;
		i++;
	}

	fseek(fp, offset, SEEK_SET);

	if (size != fread(headinfo, 1, size, fp)) {
		ak_print_error_ex(ISP_CFG "read failed!\n");
		goto exit;
	}

	ret = 0;

exit:
	fclose(fp);
		fp = NULL;
		
	return ret;
}

