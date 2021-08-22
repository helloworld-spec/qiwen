#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#include "record_fs.h"
#include "ak_record_common.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define OUR_PHOTO_PREFIX				"PHOTO_"
#define AUDIO_FILE_PREFIX				"AUDIO_"
#define PHOTO_SAVE_SUF_NAME            	".jpg"

/**
 * check_record_file_name - check record file name according to name and type
 * @file_name[IN]: appointed file name
 * @prefix[IN]: record file name prefix
 * return: record file type
 * notes: CYC_DV_20160504-095310.mp4
 * 		prefix: CYC_DV_
 *		posfix: .mp4
 */
static unsigned char check_record_file_name(char *file_name, const char *prefix)
{
	int posfix_len = FILE_EXT_NAME_MAX_LEN;
	int prefix_len = strlen(prefix);
    int file_len = strlen(file_name);
    int record_file_len = 6 + prefix_len + posfix_len;
    
    if(file_len < record_file_len){
        ak_print_normal_ex("%s isn't appointed video record!\n", file_name);
		ak_print_normal_ex("file_len=%d, record_file_len=%d\n",
			file_len, record_file_len);
        return RECORD_FILE_TYPE_NUM;
    }
	
	/* first compare prefix */
    if(memcmp(file_name, prefix, prefix_len)){
        ak_print_normal_ex("%s isn't appointed video record(%s)\n",
			file_name, prefix);
        return RECORD_FILE_TYPE_NUM;
    }

	unsigned char i = 0;
	char posfix[FILE_EXT_NAME_MAX_LEN] = {0};

	for(i=0; i<FILE_EXT_NAME_MAX_LEN; ++i){
		file_name[file_len-i] = tolower(file_name[file_len-i]);
	}
	
	for(i=0; i<RECORD_FILE_TYPE_NUM; ++i){
		switch(i){
		case RECORD_FILE_TYPE_MP4:
			memcpy(posfix, ".mp4", posfix_len);
			break;
		case RECORD_FILE_TYPE_AVI:
			memcpy(posfix, ".avi", posfix_len);
			break;
		default:
			ak_print_normal_ex("%s isn't appointed video record\n", file_name);
        	return RECORD_FILE_TYPE_NUM;
		}

		/* then compare suffix */
	    if(0 == memcmp(&file_name[file_len - posfix_len], posfix, posfix_len)){
	        return i;
	    }
	}
    
    return RECORD_FILE_TYPE_NUM;
}

/**
 * search_record_file - search type matched record files in appointed path
 * @search[IN]: search record file param
 * @callback: sort callback
 * return: record files total size of appointed path
 */
static unsigned long search_record_file(struct search_file_param *search,
							search_record_callback callback)
{
	ak_print_info_ex("enter...\n");
	DIR *dirp = opendir(search->path);
    if (!dirp){  
        ak_print_normal_ex("it fails to open directory %s\n", search->path);
        return 0;
    }

	int ret = 0;
	char astrFile[300];
	struct stat statbuf;
    struct dirent *direntp = NULL; 
    unsigned long totalSize = 0; 
    unsigned long cur_size = 0;
    unsigned char file_type = RECORD_FILE_TYPE_NUM;

	/* search all of the files in appointed dir */
    while(search->run_flag && (direntp = readdir(dirp)) != NULL){
        if(direntp->d_name == NULL)
        	continue;

		/* get absolutely file full path, including file name */
		sprintf(astrFile, "%s%s", search->path, direntp->d_name);

		/* find next directly when we don't get a regular file */
		ret = ak_is_regular_file(astrFile);
		if (AK_TRUE != ret) {
			if((AK_FAILED == ret) && (ENOENT == errno)) {
				goto search_file_end;
			}
			continue;
		}
        
		/* compare current file */
		file_type = check_record_file_name(direntp->d_name, search->prefix);
       	if(RECORD_FILE_TYPE_NUM == file_type)
            continue;

		/* get file information */
		if (stat(astrFile, &statbuf) == -1) {
			ak_print_normal_ex("stat file: %s error\n", astrFile); 
			ak_print_normal_ex("errno=%d, desc: %s\n ", errno, strerror(errno)); 

			/* No such file or directory . means card is unplug*/
			if(ENOENT == errno) {
				ak_print_error_ex("errno=%d\n", errno);
				goto search_file_end;
			}
			
			ak_sleep_ms(5);
			continue; 
		}
		
		cur_size = (statbuf.st_blocks >> 1); //each block 512 bytes
		callback(astrFile, cur_size, file_type);
		totalSize += cur_size;
		ak_sleep_ms(5);
	}

search_file_end:
	ak_print_info_ex("leave, totalSize=%ld\n", totalSize);
    if (closedir(dirp)){
    	ak_print_normal_ex("close dir %s error, %s\n ", 
    		search->path, strerror(errno));
    }
    
    return totalSize;
}

/**
 * record_fs_create_dir - create dir
 * @rec_path[IN]: record full path
 * return: 0 success, -1 failed
 */
int record_fs_create_dir(const char *rec_path)
{
	char *tmp_path = (char *)calloc(1, (strlen(rec_path) + 1 ));
	if (NULL == tmp_path) {
		ak_print_normal_ex("Out of memory!\n" );
		return AK_FAILED;
	}

	strcpy(tmp_path, rec_path);

	int ret = AK_SUCCESS;
	char *back_slash = NULL;
	char *tmp = strchr(tmp_path, '/');

	/* create dir according to / recursively */
	while(AK_TRUE) {
		back_slash = strchr(tmp + 1, '/' );
		if ( NULL == back_slash )
			break;
		
		*back_slash= '\0';

		if (!ak_check_file_exist(tmp_path)){
			*back_slash = '/';
			tmp = back_slash;
			continue;
		}

		if (mkdir(tmp_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
			ak_print_normal_ex("mkdir %s error, %s\n", tmp_path, strerror(errno));
			ret = AK_FAILED;
			goto create_dir_end;
		}

		*back_slash = '/';
        tmp = back_slash;
	}

	if ((mkdir(tmp_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0) 
		&& (errno != EEXIST)) {
		ak_print_normal_ex("can't complete create dir %s! error = %s!\n", 
			tmp_path, strerror(errno));
		ret = AK_FAILED;
	}
	
create_dir_end:
	free(tmp_path);
	tmp_path = NULL;
	
	return ret;
}

/**
 * record_fs_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB, -1 failed
 */
unsigned long record_fs_get_disk_free_size(const char *path)
{
	struct statfs disk_statfs;
    unsigned long free_size;
	
	memset(&disk_statfs, 0x00, sizeof(struct statfs));

	while ( statfs( path, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			ak_print_normal( "statfs: %s Last error == %s\n", path, strerror(errno) );
			return AK_FAILED;
		}
	}
	
    free_size = disk_statfs.f_bsize;
    free_size = free_size / 512;
    free_size = free_size * disk_statfs.f_bavail / 2;
    
	return free_size;
}

/**
 * record_fs_init_record_list: init record file list
 * @search[IN]: search record file param
 * @pcallback: sort callback
 * return: record files total size of appointed path
 * notes: find all of the record files, add to management list.
 *		stat record files total size for cycle recording.
 */
unsigned long record_fs_init_record_list(struct search_file_param *search,
										search_record_callback callback)
{
    return search_record_file(search, callback);	
}

/**
 * record_fs_get_video_name - generate record file name according to param
 * @path[IN]: record file saving path
 * @prefix[IN]: record file name prefix
 * @ext_name[IN]: record file name ext
 * @date[IN]: appointed file date time
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_video_name(const char *path, 
									const char *prefix,
									const char *ext_name,
									const struct ak_date *date, 
									char *file_name)
{
	int file_index = 0;
	char time_str[20] = {0};
	char file[100] = {0};

	ak_date_to_string(date, time_str);
	sprintf(file, "%s%s", prefix, time_str);
    sprintf(file_name, "%s%s%s", path, file, ext_name);

	/* check whether the file was exist */
	while (!ak_check_file_exist(file_name)){
		++file_index;
		if (file_index > 999) {
			ak_print_normal_ex("it fail to get record name!\n");
			return AK_FAILED;
		}
		
		sprintf(file_name, "%s%s_%03d%s", path, file, file_index, ext_name);
		ak_sleep_ms(10);
	}
	
    return AK_SUCCESS;
}

/**
 * record_fs_get_audio_name: generate audio file name according to param
 * @path[IN]: audio record file saving path
 * @ext_name[IN]: audio record file name ext
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_audio_name(const char *path, 
									const char *ext_name,
									char *file_name)
{
	int file_index = 0;
	char file[100] = {0};
	struct ak_date date;

	ak_get_localdate(&date);
	sprintf(file, "%s%02d%02d%02d", AUDIO_FILE_PREFIX, 
		date.hour, date.minute, date.second);
    sprintf(file_name, "%s%s%s", path, file, ext_name);
    
	while (!ak_check_file_exist(file_name)){
		++file_index;
		if (file_index > 999) {
			ak_print_normal_ex("it fail to get record name!\n");
			return AK_FAILED;
		}
		
		sprintf(file_name, "%s%s_%03d%s", path, file, file_index, ext_name);
		ak_sleep_ms(10);
	}
	
    return AK_SUCCESS;
}

/**
 * record_fs_get_photo_name: generate photo file name according to param
 * @path[IN]: photo file saving path
 * @file_name[OUT]: generated photo file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_photo_name(const char *path, char *file_name)
{
	int file_index = 0;
	char time_str[20] = {0};
	char file[100] = {0};
	struct ak_date date;

	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);    
	sprintf(file, "%s%s", OUR_PHOTO_PREFIX, time_str);	
    sprintf(file_name, "%s%s%s", path, file, PHOTO_SAVE_SUF_NAME);

	while (!ak_check_file_exist(file_name)){
		++file_index;
		if (file_index > 999) {
			ak_print_normal_ex("it fail to get record name!\n");
			return AK_FAILED;
		}
		
		sprintf(file_name, "%s%s_%03d%s", 
			path, file, file_index, PHOTO_SAVE_SUF_NAME);			
		ak_sleep_ms(10);
	}
	
    return AK_SUCCESS;
}
