#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <regex.h>

#include "record_fs.h"
#include "ak_dvr_common.h"
#include "ak_dvr_file.h"
//#include "printcolor.h"

#define RECORD_FS_REPLAY_DEBUG            0

#define OUR_PHOTO_PREFIX                "PHOTO_"
#define AUDIO_FILE_PREFIX                "AUDIO_"
#define PHOTO_SAVE_SUF_NAME                ".jpg"

#define PATTERN_RECORD_DIR              "^([0-9]{8})|([0-9]{2})$"
#define PATTERN_RECORD_FILE             "^.*[0-9]{8}-[0-9]{6}@[0-9]+\\.(avi|mp4)$"
#define PATTERN_RECORD_AVI              "^.*\\.avi$"
#define PATTERN_RECORD_MP4              "^.*\\.mp4$"
#define REGEX_FLAGS                     REG_EXTENDED|REG_ICASE|REG_NOSUB
#define LEN_REGEX_ERROR                 1024

#ifndef FREE_POINT
#define FREE_POINT(POINT)  \
if(POINT != NULL) {\
    free(POINT);\
    POINT = NULL;\
}
#endif

/* record file total size in SD card */
static unsigned long rec_total_size = 0;
static unsigned char rec_sub_dir = AK_FALSE;

/**
 * get_record_file_size - get a file size according path and file name
 * @file[IN]: appointed path and file name
 * return: success >=0 for file size; othrewise -1(AK_FAILED)
 * notes:
 */
long get_record_file_size(const char *file)
{
    struct stat statbuf;

    /* get file information */
    if (stat(file, &statbuf) == -1) {
        ak_print_normal_ex("stat file: %s error\n", file);
        ak_print_normal_ex("errno=%d, desc: %s\n ", errno, strerror(errno));

        /* No such file or directory . means card is unplug */
        if (ENOENT == errno) {
            ak_print_error_ex("errno=%d\n", errno);
        }

        return AK_FAILED;
    }

    return (statbuf.st_blocks >> 1); //each block 512 bytes
}

/**
 * check_record_file_name - check record file name according to name and type
 * @file_name[IN]: appointed file name
 * @prefix[IN]: record file name prefix
 * return: record file type
 * notes: CYC_DV_20160504-095310.mp4
 *         prefix: CYC_DV_
 *        posfix: .mp4
 */
unsigned char check_record_file_name(char *file_name, const char *prefix)
{
    int posfix_len = FILE_EXT_NAME_MAX_LEN;
    int prefix_len = strlen(prefix);
    int file_len = strlen(file_name);
    int record_file_len = 6 + prefix_len + posfix_len;

    if(file_len < record_file_len){
        ak_print_normal_ex("%s isn't appointed video record!\n", file_name);
        ak_print_normal_ex("file_len=%d, record_file_len=%d\n",
            file_len, record_file_len);
        return DVR_FILE_TYPE_NUM;
    }

    // first compare prefix
    if(memcmp(file_name, prefix, prefix_len)){
        ak_print_normal_ex("%s isn't appointed video record(%s)\n",
            file_name, prefix);
        return DVR_FILE_TYPE_NUM;
    }

    unsigned char i = 0;
    char posfix[FILE_EXT_NAME_MAX_LEN] = {0};

    for (i = 0; i < FILE_EXT_NAME_MAX_LEN; ++i) {
        file_name[file_len-i] = tolower(file_name[file_len-i]);
    }

    for (i = 0; i < DVR_FILE_TYPE_NUM; ++i) {
        switch(i) {
        case DVR_FILE_TYPE_MP4:
            memcpy(posfix, ".mp4", posfix_len);
            break;
        case DVR_FILE_TYPE_AVI:
            memcpy(posfix, ".avi", posfix_len);
            break;
        default:
            ak_print_normal_ex("%s isn't appointed video record\n", file_name);
            return DVR_FILE_TYPE_NUM;
        }

        // then compare suffix
        if (0 == memcmp(&file_name[file_len - posfix_len], posfix, posfix_len)) {
            return i;
        }
    }

    return DVR_FILE_TYPE_NUM;
}

int check_record_ext_name( char *file_name, int i_len )                         //检查录像文件的扩展名是否符合要求
{
    if( i_len < LEN_RECORD_EXT_NAME ) {
        return DVR_FILE_TYPE_NUM;
    }
    if ( ( memcmp( file_name + i_len - LEN_RECORD_EXT_NAME , ".mp4" , LEN_RECORD_EXT_NAME ) == 0 ) ||
         ( memcmp( file_name + i_len - LEN_RECORD_EXT_NAME , ".MP4" , LEN_RECORD_EXT_NAME ) == 0 ) ) {
        return DVR_FILE_TYPE_MP4;
    }
    else if ( ( memcmp( file_name + i_len - LEN_RECORD_EXT_NAME , ".avi" , LEN_RECORD_EXT_NAME ) == 0 ) ||
              ( memcmp( file_name + i_len - LEN_RECORD_EXT_NAME , ".AVI" , LEN_RECORD_EXT_NAME ) == 0 ) ) {
        return DVR_FILE_TYPE_AVI;
    }
    else {
        return DVR_FILE_TYPE_NUM;
    }

}

int regexpr_compare(char *pattern, int flags, char *buff)
{
    int status;
    regex_t regex_t_pattern;
    char ac_error[ LEN_REGEX_ERROR ];

    if ((status = regcomp(&regex_t_pattern, pattern, flags)) != REG_NOERROR) {
        regerror(status, &regex_t_pattern, ac_error, LEN_REGEX_ERROR);
        ak_print_normal_ex("status= %d pattern= '%s' ac_error= '%s'", status, pattern, ac_error);
        return REG_NOMATCH;
    }
    status = regexec(&regex_t_pattern, buff, (size_t)0, NULL, 0);
    regfree(&regex_t_pattern);

    return status;
}

int record_sleep_ms = SEARCH_RECORD_MS_FAST;

void set_record_sleep_ms( int sleep_ms )
{
    record_sleep_ms = sleep_ms;
}

/**
 * search_record_file - search type matched record files in appointed path
 * @search[IN]: search record file param
 * @callback: sort callback
 * return: record files total size of appointed path
 */
static unsigned long search_record_file(struct search_file_param *search, search_record_callback callback)    //对全局录像文件进行索引的函数
{

    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "search->path= %s", search->path )
    /*
    DIR *dirp = opendir(search->path);
    if (!dirp){
        ak_print_normal_ex("it fails to open directory %s\n", search->path);
        return 0;
    }
    */

    int ret = 0;
    int count = 0;
    char file_path[256] = {0};
    unsigned long cur_size = 0;
    unsigned long total_size = 0;
    unsigned char file_type = DVR_FILE_TYPE_NUM;
    int i_num_scandir, i ;
    struct dirent **pp_dirent_list = NULL;
    //struct dirent *dir_ent = NULL;

    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "search->path= %s", search->path )
    /* search all of the files in appointed dir */
    //while (search->run_flag && ((pp_dirent_list[ i ] = readdir(dirp)) != NULL)) {
    i_num_scandir = scandir( search->path, &pp_dirent_list, 0 , alphasort );
    for( i = 0 ; i < i_num_scandir ; i ++ ) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
        if( search->run_flag == AK_FALSE ) {
            break;
        }
        if(pp_dirent_list[ i ]->d_name == NULL) {
            continue;
        }
        if ( ( strcmp(".", pp_dirent_list[ i ]->d_name) == 0) ||
             ( strcmp("..", pp_dirent_list[ i ]->d_name) == 0 )) {
            continue;
        }
        switch( pp_dirent_list[ i ]->d_type ){
            case DT_DIR:                                                        //不匹配命名规则的目录
                if (regexpr_compare(PATTERN_RECORD_DIR, REGEX_FLAGS, pp_dirent_list[ i ]->d_name) == REG_NOMATCH) {
                    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
                    continue;
                }
                //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
                break;
            case DT_REG:                                                        //不匹配命名规则的文件
                if (regexpr_compare(PATTERN_RECORD_FILE, REGEX_FLAGS, pp_dirent_list[ i ]->d_name) == REG_NOMATCH) {
                    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
                    continue;
                }
                //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
                break;
            default:
                continue;
        }
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
        /* find next when we get dir */
        if ((pp_dirent_list[ i ]->d_type & DT_DIR)) {
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
            if ( ( strcmp(".", pp_dirent_list[ i ]->d_name) == 0) ||
                 ( strcmp("..", pp_dirent_list[ i ]->d_name) == 0 )) {
                continue;
            }

            struct search_file_param param = {0};

            param.run_flag = search->run_flag;
            strcpy(param.prefix, search->prefix);
            sprintf(param.path, "%s%s/", search->path, pp_dirent_list[ i ]->d_name);
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= %s", pp_dirent_list[ i ]->d_name )
            rec_sub_dir = AK_TRUE;

            total_size = search_record_file(&param, callback);
            /* this is an empty sub dir, we'll delete it. */
            if (0 == total_size) {
                ak_print_notice_ex("delete empty dir: %s\n", param.path);
                remove(param.path);
            } else {
                rec_total_size += total_size;
            }
            rec_sub_dir = AK_FALSE;
            continue;
        }

        /* get absolutely file full path, including file name */
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "search->path= '%s' pp_dirent_list[ i ]->d_name= '%s'", search->path, pp_dirent_list[ i ]->d_name )
        sprintf(file_path, "%s%s", search->path, pp_dirent_list[ i ]->d_name);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_path= '%s'", file_path )

        /* find next directly when we don't get a regular file */
        ret = ak_is_regular_file(file_path);
        if (AK_TRUE != ret) {
            if((AK_FAILED == ret) && (ENOENT == errno)) {
                goto search_file_end;
            }
            continue;
        }

        /* compare current file */
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= '%s' search->prefix= '%s'", pp_dirent_list[ i ]->d_name , search->prefix )
        //file_type = check_record_file_name(pp_dirent_list[ i ]->d_name, search->prefix);
        file_type = check_record_ext_name( pp_dirent_list[ i ]->d_name, strlen( pp_dirent_list[ i ]->d_name ) );
        /*
        if (regexpr_compare(PATTERN_RECORD_AVI, REGEX_FLAGS, pp_dirent_list[ i ]->d_name) != REG_NOMATCH) {
            file_type = DVR_FILE_TYPE_AVI;
        }
        else if (regexpr_compare(PATTERN_RECORD_MP4, REGEX_FLAGS, pp_dirent_list[ i ]->d_name) != REG_NOMATCH) {
            file_type = DVR_FILE_TYPE_MP4;
        }
        else {
            continue;
        }
        */

        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "pp_dirent_list[ i ]->d_name= '%s' search->prefix= '%s' file_type= %d", pp_dirent_list[ i ]->d_name , search->prefix , file_type )
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_type= %d", file_type )
        if(DVR_FILE_TYPE_NUM == file_type) {
            continue;
        }

        cur_size = get_record_file_size(file_path);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "cur_size= %lu", cur_size )
        if (AK_FAILED == cur_size) {
            goto search_file_end;
        }

        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_path= '%s'", file_path )
        callback(file_path, cur_size, file_type);                               //回调添加到列表中
        total_size += cur_size;

        if (++count >= 10) {
            count = 0;
            ak_sleep_ms( record_sleep_ms );
        }
    }

search_file_end:
    for( i = 0 ; i < i_num_scandir ; i ++ ) {
        FREE_POINT( pp_dirent_list[ i ] );
    }
    FREE_POINT( pp_dirent_list );

    ak_print_info_ex("leave, total_size=%ld, rec_total_size=%ld(KB)\n",
        total_size, rec_total_size);
    /*
    if (closedir(dirp)){
        ak_print_normal_ex("close dir %s error, %s\n ",
            search->path, strerror(errno));
    }
    */

    if (rec_sub_dir) {
        return total_size;
    } else {
        return rec_total_size;
    }
}

/**
 * search_dir_record_file - search type matched record files in appointed path
 * @path[IN]: path for search record file
 * @search[IN]: search record file param
 * @replay[IN]: search replay record file param of dir by time
 * @callback: after search callback function
 * return: record files number of appointed path
 */

#if 0
//使用readdir进行遍历
static inline int search_dir_record_file(const char *path, struct search_file_param *search, struct search_replay_param *replay, search_replay_callback callback)
{
    if (!path || !search || !replay || !callback) {
        return AK_FAILED;
    }

    DIR *dirp = opendir(path);
    if (!dirp){
        ak_print_normal_ex("it fails to open directory %s\n", path);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "path= '%s'", path )
        return AK_FAILED;
    }
    else {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "path= '%s'", path )
    }

    struct dirent *dir_ent = NULL;
    char file_path[ LEN_PATH_FILE ] = {0};
    int file_count = 0;
    time_t calendar_time = 0;
    time_t calendar_end_time = 0;
    unsigned long cur_size = 0;
    int i_len_path ;                                                            //当前搜索目录的长度

    i_len_path = snprintf( file_path , LEN_PATH_FILE , "%s" , path );           //获取当前搜索目录的字符串长度
    while ((dir_ent = readdir(dirp)) != NULL) {
        if( check_record_ext_name( dir_ent->d_name, strlen( dir_ent->d_name ) ) != replay->type ) { //判断当前文件名是否avi|mp4后缀
            //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
            continue;
        }

        memcpy( file_path + i_len_path , dir_ent->d_name , strlen( dir_ent->d_name ) + 1 );         //拷贝拼接当前文件的全路径

        if ( get_record_file_size(file_path) <= 0) {
            continue;
        }

        calendar_time = record_fs_get_time_by_name(file_path);
        calendar_end_time = calendar_time + get_entry_total_time( file_path ) / 1000 ;

        if(((replay->start_time <= calendar_time) && (replay->end_time > calendar_time) ) ||
            ((replay->start_time < calendar_end_time) && (replay->end_time >= calendar_end_time)) ||
            ((replay->start_time >= calendar_time) && (replay->end_time <= calendar_end_time))) {
            if (callback(file_path, cur_size, calendar_time, replay)) {
                ak_print_error_ex("add %s failed\n", file_path);
            }
            else {
                file_count ++;
            }
        }
    }

    if (closedir(dirp)){
        ak_print_normal_ex("close dir %s error, %s\n ", path, strerror(errno));
    }

    return file_count;
}
#else
//使用scandir进行遍历
static inline int search_dir_record_file(const char *path, struct search_file_param *search, struct search_replay_param *replay, search_replay_callback callback)
{
    struct dirent **pp_dirent_list = NULL;
    char file_path[ LEN_PATH_FILE ] = {0};
    int file_count = 0;
    time_t calendar_time = 0;
    time_t calendar_end_time = 0;
    unsigned long cur_size = 0;
    int i_len_path ;                                                            //当前搜索目录的长度
    int i_num_scandir;                                                          //扫描目录后的长度
    int i;

    if (!path || !search || !replay || !callback) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "path= '%s'", path )
        return AK_FAILED;
    }
    else {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "path= '%s'", path )
    }

    i_len_path = snprintf( file_path , LEN_PATH_FILE , "%s" , path );           //获取当前搜索目录的字符串长度
    i_num_scandir = scandir( path, &pp_dirent_list, 0 , alphasort );
    for( i = 0; i < i_num_scandir; i ++ ) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "path= '%s' i= %d i_num_scandir= %d", path, i, i_num_scandir )
        if( check_record_ext_name( pp_dirent_list[ i ]->d_name, strlen( pp_dirent_list[ i ]->d_name ) ) != replay->type ) { //判断当前文件名是否avi|mp4后缀
            FREE_POINT( pp_dirent_list[ i ] );
            continue;
        }
        memcpy( file_path + i_len_path , pp_dirent_list[ i ]->d_name , strlen( pp_dirent_list[ i ]->d_name ) + 1 );         //拷贝拼接当前文件的全路径
        if ( get_record_file_size(file_path) <= 0) {                            //获取文件长度
            FREE_POINT( pp_dirent_list[ i ] );
            continue;
        }
        calendar_time = record_fs_get_time_by_name(file_path);
        calendar_end_time = calendar_time + get_entry_total_time( file_path ) / 1000 ;
        /*
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE,
                     "file_path= '%s' calendar_time= %lu calendar_end_time= %lu replay->start_time= %lu replay->end_time= %lu\n",
                     file_path , calendar_time , calendar_end_time , replay->start_time , replay->end_time )
                     */
        /*
        if(((replay->start_time <= calendar_time) && (replay->end_time > calendar_time) ) ||
             ((replay->start_time < calendar_end_time) && (replay->end_time >= calendar_end_time)) ||
             ((replay->start_time >= calendar_time) && (replay->end_time <= calendar_end_time))) {
             */
        //通过视频文件中带的两个时间和视频请求列表中带的时间进行比较，判断是否满足时间要求
        if ( check_time_axis( replay->start_time , replay->end_time , calendar_time , calendar_end_time ) == AK_TRUE ) {
            /*
            DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN,
                         "file_path= '%s' calendar_time= %lu calendar_end_time= %lu replay->start_time= %lu replay->end_time= %lu\n",
                         file_path , calendar_time , calendar_end_time , replay->start_time , replay->end_time )
                         */
            if (callback(file_path, cur_size, calendar_time, replay)) {         //回调函数,添加到列表中
                ak_print_error_ex("add %s failed\n", file_path);
            }
            else {
                file_count ++;
            }
        }
        FREE_POINT( pp_dirent_list[ i ] );
    }
    FREE_POINT( pp_dirent_list );

    return file_count;
}
#endif

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
 * record_fs_get_time_by_name: get record file calendar time from file name
 * @file_path[IN]: file path including full file name
 * return: 0 success; otherwise -1
 * notes: file name ex.: /mnt/CYC_DV/CYC_DV_20160504-095310@300000.mp4
 */
time_t record_fs_get_time_by_name(const char *file_path)
{
    time_t calendar_time = -1;

    char *str = strchr(file_path, '-');
    if(str) {
        str -= 8;
        struct ak_date date;
        ak_string_to_date(str, &date);
        calendar_time = ak_date_to_seconds(&date);
    }

    return calendar_time;
}

/**
 * record_fs_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB
 */
unsigned long record_fs_get_disk_free_size(const char *path)
{
    struct statfs disk_statfs;
    unsigned long free_size;

    memset(&disk_statfs, 0x00, sizeof(struct statfs));

    while ( statfs( path, &disk_statfs ) == -1 ) {
        if ( errno != EINTR ) {
            ak_print_normal( "statfs: %s Last error == %s\n", path, strerror(errno) );
            return 0;
        }
    }

    free_size = disk_statfs.f_bsize;
    free_size = free_size / 512;
    free_size = free_size * disk_statfs.f_bavail / 2;

    return free_size;
}

/**
 * record_fs_get_disk_total_size - get disk total size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk total size in KB, 0 failed
 */
unsigned long record_fs_get_disk_total_size(const char *path)
{
    struct statfs disk_statfs;
    unsigned long total_size;

    memset(&disk_statfs, 0x00, sizeof(struct statfs));

    while ( statfs( path, &disk_statfs ) == -1 ) {
        if ( errno != EINTR ) {
            ak_print_normal( "statfs: %s Last error == %s\n", path, strerror(errno) );
            return 0;
        }
    }

    total_size = (disk_statfs.f_bsize / 512) * disk_statfs.f_blocks / 2;

    return total_size;
}

/**
 * record_fs_search_by_time - search replay record file by time
 * @search[IN]: search record file param
 * @replay[IN/OUT]: replay param
 * @callback[IN]: replay entry callback
 * return: record files total number of first day appointed to given time
 * notes: find all of the record files in one day dir, add to replay list.
 */
int record_fs_search_by_time(struct search_file_param *search,
                            struct search_replay_param *replay,
                            search_replay_callback callback)
{
    if (!search || !replay || !callback) {
        return 0;
    }

    char dir_path[ LEN_PATH_FILE ] = {0};
    struct ak_date date;
    int file_count = 0;
    int cur_count = 0;

	//must be the same as ak_dvr_file.c line:1460
    time_t start_time = replay->start_time - replay->start_time % ONE_HOUR_SECOND - ONE_DAY_SECOND;//开始搜索范围扩大1小时
    time_t end_time = replay->end_time - replay->end_time % ONE_HOUR_SECOND + ONE_HOUR_SECOND ;     //结束搜索范围扩大1小时

    /*
    struct tm tm_res;
    localtime_r( &start_time , &tm_res ) ;
    DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                         "start_time= %lu '%04d-%02d-%02d %02d:%02d:%02d'\n",
                         start_time, tm_res.tm_year + 1900 , tm_res.tm_mon + 1 , tm_res.tm_mday ,
                         tm_res.tm_hour, tm_res.tm_min , tm_res.tm_sec )
    localtime_r( &end_time , &tm_res ) ;
    DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                         "end_time= %lu '%04d-%02d-%02d %02d:%02d:%02d'\n",
                         end_time, tm_res.tm_year + 1900 , tm_res.tm_mon + 1 , tm_res.tm_mday ,
                         tm_res.tm_hour, tm_res.tm_min , tm_res.tm_sec )
    */

    while(start_time <= end_time) {
        ak_seconds_to_date(start_time, &date);
        snprintf(dir_path, LEN_PATH_FILE , "%s%4.4d%2.2d%2.2d/%02d/", search->path, date.year, (date.month + 1), (date.day + 1), date.hour );
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s'", dir_path )

#if RECORD_FS_REPLAY_DEBUG
        ak_print_normal_ex("start_time=%lu, str: %s\n", start_time, ak_seconds_to_string(start_time));
        ak_print_normal_ex("end_time=%lu, str: %s\n", end_time, ak_seconds_to_string(end_time));
        ak_print_normal_ex("dir_path: %s\n", dir_path);
#endif
        /* check appointed dir */
        if (!ak_check_file_exist(dir_path)) {
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s'", dir_path )
            cur_count = search_dir_record_file(dir_path, search, replay, callback);
            if (cur_count < 0) {
                goto search_by_time_end;
            }

            file_count += cur_count;
        }

        //start_time += ONE_DAY_SECOND;
        start_time += ONE_HOUR_SECOND;
    }

search_by_time_end:
    return file_count;
}

/**
 * record_fs_init_record_list: init record file list
 * @search[IN]: search record file param
 * @pcallback: sort callback
 * return: record files total size of appointed path
 * notes: find all of the record files, add to management list.
 *        stat record files total size for cycle recording.
 */
unsigned long record_fs_init_record_list(struct search_file_param *search,
                                        search_record_callback callback)
{
    rec_total_size = 0;
    rec_sub_dir = AK_FALSE;
    return search_record_file(search, callback);
}

/**
 * record_fs_get_video_name - generate record file name according to param
 * @path[IN]: record file saving path
 * @prefix[IN]: record file name prefix
 * @ext_name[IN]: record file name ext
 * @date[IN]: appointed file date time
 * @duration[IN]: appointed file duration time
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_video_name(const char *path,
                                    const char *prefix,
                                    const char *ext_name,
                                    const struct ak_date *date,
                                    unsigned long duration,
                                    char *file_name)
{
    int file_index = 0;
    char time_str[20] = {0};
    char file[ LEN_PATH_FILE ] = {0};
    char dir_name[ LEN_PATH_RECORD ] = {0};

    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "path= '%s' prefix= '%s' ext_name= '%s' duration= '%lu' file_name= '%s'", path, prefix, ext_name, duration, file_name )
    ak_date_to_string(date, time_str);
    if (prefix) {
        sprintf(file, "%s%s", prefix, time_str);
    } else {
        sprintf(file, "%s", time_str);
    }

    snprintf(dir_name, LEN_PATH_RECORD, "%.4d%.2d%.2d/", date->year, (date->month + 1), (date->day + 1) );
    snprintf(file_name, LEN_PATH_FILE, "%s%s%.2d/%s@%.6lu%s", path, dir_name, date->hour, file, duration, ext_name);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_name= '%s'", file_name )
    /* check whether the file was exist */
    while (!ak_check_file_exist(file_name)){
        ++file_index;
        if (file_index > 999) {
            ak_print_normal_ex("it fail to get record name!\n");
            return AK_FAILED;
        }

        //sprintf(file_name, "%s/%.2d/%s_%03d%s", path, date->hour, file, file_index, ext_name);
        sprintf(file_name, "%s%s%02d/%s@%.6lu_%03d%s", path, dir_name, date->hour, file, duration, file_index, ext_name);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_name= '%s'", file_name )
        ak_sleep_ms(10);
        //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
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
        //ak_sleep_ms(10);
        //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
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
        //ak_sleep_ms(10);
        //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
    }

    return AK_SUCCESS;
}
