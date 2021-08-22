#ifndef _DANA_OTHERS_H_
#define _DANA_OTHERS_H_

/**
 * dana_others_get_verion - get system version  
 * return:   version string;
 */
char* dana_others_get_verion();

/**
 * dana_others_settime - set device time   
 * @time_arg[IN]: the time to set   
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_settime(long long time_arg);

/**
 * dana_others_ptz_init - int ptz for danale  
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_ptz_init(void);

/**
 * dana_others_ptz_exit - exit ptz for danale  
 * return:   void;
 */
void dana_others_ptz_exit(void);

/**
 * dana_others_set_ptz_cmd - set cmd to ptz  and  ptz run    
 * @code[IN]: cmd   
 * @para1[IN]:  reserved
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_set_ptz_cmd( int code , int para1);

/**
 * dana_others_cloud_init - int cloud for danale  
 * return:   void;
 */
void dana_others_cloud_init(void);

/**
 * dana_others_upgrade_init - int OTA upgrade for danale  
 * @danale_path[IN]:  danale conf path
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_upgrade_init(const char *danale_path);

/**
 * dana_others_send_alarm - send alarm info to danale  
 * @type[IN]:  alarm type
 * @level[IN]:  sensitivity
 * @start_time[IN]:  time that alarm triggered
 * @time_len[IN]:  reserved
 * return:   void;
 */
void dana_others_send_alarm(int type,int level,int start_time, int time_len);

/**
 * dana_others_alarm_init - init alarm  
 * @vi_handle[IN]:  vi handle 
 * @ai_handle[IN]:  ai handle 
 * return:   void;
 */
void dana_others_alarm_init(void *vi_handle, void *ai_handle);

/**
 * dana_others_u2g - convert "utf-8" to "gbk"  
 * @inbuf[IN]:  utf-8  code string
 * @inlen[IN]:  strlen
 * @outbuf[OUT]:  gbk code string
 * @outlen[IN]:  strlen
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

/**
 * dana_others_mt_init - mt init
 * @vi_handle[IN]: opened vi_handle
 * return:  0 success, -1 failed
 */
int dana_others_mt_init(void *vi_handle);

/**
 * dana_others_mt_exit - mt exit
 * return:  void
 */
void dana_others_mt_exit(void);

#endif
