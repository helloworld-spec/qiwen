#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>

#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_cmd_exec.h"
#include "ak_config.h"
#include "dana_wifi.h"

#define ITEM_LEN		81
#define AP_NUM			32
#define AP_LIST_FILE	"/etc/jffs2/ap_list"
#define LINE_BYTE 		1024
#define NUM_WIFI_SCAN           32
#define LEN_WIFI_CHK            1024
#define NUM_TRY_WIFI_CONNECT    5                                                                   //wifi检测连接次数
#define LEN_BUFF 65536
#define NUM_WIFI_CHK            5                                                                   //wifi驱动加载的检测次数
                                                                                                    //wpa_cli -iwlan0 status如果输入了错误的ssid和passwd，依然可以获取到ssid
                                                                                                    //不适合用在此处判断是否能够成功连接到wifi网络
#define CMD_WIFI_STAT_FILE "cat /sys/class/net/wlan0/operstate"                                     //目前改用operstate去进行判断
#define WIFI_STAT_SUCCESS  "up"                                                                     //wifi网卡运行的标志
#define FILE_WIFI_TMP      "/tmp/wifi_info"                                                         //暂存的wifi配置文件
/*
 * wifi ap data struct.
 * it is original and get by iwlist.
 */
struct ap {
	int index;
	int security;
	char address[ITEM_LEN];
	char ssid[ITEM_LEN];
	char password[ITEM_LEN];
	char protocol[ITEM_LEN];
	char mode[ITEM_LEN];
	char frequency[ITEM_LEN];
	char en_key[ITEM_LEN];
	char bit_rates[ITEM_LEN];
	char sig_level[ITEM_LEN];
};

/*
 * wifi ap data struct array.
 * it is original and get by iwlist.
 */
struct ap_shop {
	int ap_num;
	struct ap ap_list[AP_NUM];
};

/**
 * get_ap_item - get  one item of wifi ap info.
 * return: 1, success; 0, failed.
 */
static int get_ap_item(const char *str, const char *name, char *val)
{
	str = strstr(str, name);
	if(str)	{
		for(str += strlen(name); (*str) && (*str != '"') && (*str != '\n'); ){
			*val++ = *str++;
		}
	}
	*val = '\0';

	return str != NULL;
}

/**
 * scan_ap2file - start search wifi and save wifi ap info to file.
 * return:  void.
 */
static void scan_ap2file(void)
{
	char cmd[100];

	/* scan  AP to ap_list */
	sprintf(cmd, "iwlist wlan0 scanning > %s", AP_LIST_FILE);
	system(cmd);
}

/**
 * get_security - get wifi ap security.
 * return:  enctype value.
 */
static int get_security(char *buf)
{
	int val = 0;

	if(strstr(buf, "Encryption key:on")){
		if (strstr(buf, "WPA"))	{
			val = WIFI_ENCTYPE_WPA_TKIP;
		}
		else if(strstr(buf, "WPA2")){
			val = WIFI_ENCTYPE_WPA2_TKIP;
		}
	}
	else if(strstr(buf, "Encryption key:off")){
		val = WIFI_ENCTYPE_NONE;
	}

	return val;
}

/**
 * get_one_ap - get  one wifi ap info from string.
 * return:   length of ssid.
 */
static int get_one_ap(char *buf, struct ap *ap)
{
	get_ap_item(buf, "Address: ", ap->address);
	get_ap_item(buf, "ESSID:\"", ap->ssid);
	get_ap_item(buf, "Protocol:", ap->protocol);
	get_ap_item(buf, "Mode:", ap->mode);
	get_ap_item(buf, "Frequency:", ap->frequency);
	get_ap_item(buf, "Encryption key:", ap->en_key);
	get_ap_item(buf, "Bit Rates:", ap->bit_rates);
	get_ap_item(buf, "level=", ap->sig_level);
	ap->security = get_security(buf);
	return strlen(ap->ssid);
}

/**
 * scan_ap - start search wifi and get  wifi ap list  from file.
 *                         the list is original.
 * return:   0, success; -1, failed.
 */
static int scan_ap(struct ap_shop *info)
{
	char *line;
	char *buffer;
	FILE *ap_file;
	struct ap ap_temp;

	line   = (char *)calloc(1, LINE_BYTE);
	buffer = (char *)calloc(1, LINE_BYTE*10);

	ak_print_normal_ex("Scanning other available wifi AP...\n");
	scan_ap2file();
	ap_file = fopen(AP_LIST_FILE, "r");
	if (NULL == ap_file){
		ak_print_normal_ex("open file %s fail!\n",  AP_LIST_FILE);
		goto fail;
	}

	memset(line, '\0', LINE_BYTE);
	while ((!feof(ap_file)) && (strstr(line, "Address:") == NULL)){
		fgets(line, LINE_BYTE, ap_file);
	}

	for (; (!feof(ap_file)) && (strstr(line, "Address:") != NULL); ){
		memset(buffer, '\0', sizeof(LINE_BYTE*10));
		strcat(buffer, line);

		/* get one AP info to buffer */
		for (fgets(line, LINE_BYTE, ap_file); (!feof(ap_file)) && (strstr(line, "Address:") == NULL); /*null*/)
		{
			strcat(buffer, line);
			fgets(line, LINE_BYTE, ap_file);
		}

		/* copy one ap info to list */
		memset(&ap_temp, 0, sizeof(struct ap));
		if (get_one_ap(buffer, &ap_temp)){
			info->ap_num++;
			if (info->ap_num >= sizeof(info->ap_list)/sizeof(info->ap_list[0]))	{
				info->ap_num--;
				ak_print_normal_ex("ap list=%d is full!", info->ap_num);
				break;
			}
			memcpy(&info->ap_list[info->ap_num], &ap_temp, sizeof(struct ap));
			info->ap_list[info->ap_num].index = info->ap_num;
		}
	}

	free(line);
	free(buffer);
	fclose(ap_file);

	return 0;
fail:
	free(line);
	free(buffer);

	return -1;
}

/**
 * do_wifi_search - start search and get  wifi ap list .
 *                         the list is original.
 * return:   0, success; -1, failed.
 */
static int do_wifi_search(struct ap_shop *ap)
{
	int error;
	struct ap_shop *ap_info = ap;

	memset(ap_info, 0, sizeof(struct ap_shop));
	ap_info->ap_num = -1;
	error = scan_ap(ap_info);

	return error;
}

/**
 * dana_wifi_get_ap_list - get  wifi ap list .
 *                            the list have been arranged.
 * return:   wifi ap list or null
 */
struct ak_ap_list* dana_wifi_get_ap_list(void)
{
	int i;
	struct ak_ap_list * ap_list;
	struct ap_shop soft_ap_list;

	if(do_wifi_search(&soft_ap_list)) {
		return NULL;
	}

	if(soft_ap_list.ap_num > NUM_WIFI_SCAN ){
		soft_ap_list.ap_num = NUM_WIFI_SCAN;
	}
	if(soft_ap_list.ap_num < 0 ){
		soft_ap_list.ap_num = 0;
	}
	ap_list = (struct ak_ap_list *)calloc(1, sizeof(struct ak_ap_list));
	if( NULL == ap_list) {
		ak_print_normal_ex("calloc fail\n");
		return NULL;
	}
	ap_list->ap_num = soft_ap_list.ap_num;

	ak_print_normal_ex("\nWifi list:\n");
	for(i = 0; i < soft_ap_list.ap_num; i++)  {
		ap_list->ap_info[i].enc_type = soft_ap_list.ap_list[i].security;
		ap_list->ap_info[i].quality = atoi(soft_ap_list.ap_list[i].sig_level);
		ap_list->ap_info[i].essid[32] = 0;
		strncpy(ap_list->ap_info[i].essid, soft_ap_list.ap_list[i].ssid,32);

		ak_print_normal("id:%d, enc:%d,ssid:%s, quality:%d\n", i+1, soft_ap_list.ap_list[i].security,
			soft_ap_list.ap_list[i].ssid, atoi(soft_ap_list.ap_list[i].sig_level));
	}

	return ap_list;
}

/**
 * dana_wifi_net_chk - check net configure if it is ok.
 * @if_name[IN]: 	 net point name
 * return:   1, net is ok;  0 , net is  failed;
 */
int dana_wifi_net_chk(const char *if_name)
{
	struct ifreq ifr;
	struct sockaddr_in *sai = NULL;
	int sockfd;
	int ret = 0;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	/*if ip is not configure , ioctl(sockfd, SIOCGIFADDR, &ifr) return fail*/
	bzero(&ifr,sizeof(struct ifreq));
	strcpy(ifr.ifr_name,if_name);
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) >=0){
		sai = (struct sockaddr_in *) &ifr.ifr_addr;
		ak_print_normal_ex(" %s ip:%s \n ", if_name, inet_ntoa(sai->sin_addr));
		if(strlen(inet_ntoa(sai->sin_addr)) > 6){
			ret = 1;
		}
	}
	close(sockfd);

	return ret;
}

/**
 * dana_wifi_driver_chk - check wifi driver if it install successfully
 * return:     0 , success ;  -1 , failed;
 */

int dana_wifi_driver_chk(void)
{
	char result[ LEN_WIFI_CHK ] ;
	int ret = AK_FAILED , i ;

	for( i = 0 ; i < NUM_WIFI_CHK ; i ++ ) {
		memset( result , 0 , LEN_WIFI_CHK ) ;
		ak_cmd_exec("ifconfig | grep wlan0", result, sizeof(result));
		if(0 == strlen(result)) {                                                           //找不到wlan0设备,wifi未启动
			ak_print_normal_ex("the wifi device is not working, start it.\n");
			memset( result , 0 , LEN_WIFI_CHK ) ;
			ak_cmd_exec("wifi_driver.sh station", result , LEN_WIFI_CHK );              //加载驱动
			memset( result , 0 , LEN_WIFI_CHK ) ;
			ak_cmd_exec("ifconfig wlan0 up", result , LEN_WIFI_CHK );                   //启动网卡
		} else {                                                                            //找到wlan0设备,退出
			ak_print_normal_ex("the wifi dev has running.\n");
			ret = AK_SUCCESS ;
			break ;
		}
	}
	return ret;

	/*
	ak_cmd_exec("ifconfig | grep wlan0", result, sizeof(result));
	if(0 == strlen(result)) {
		ak_print_normal_ex("the wifi device is not working, start it.\n");
		// install wifi driver
		ak_cmd_exec("wifi_driver.sh station", result , LEN_WIFI_CHK );
		bzero(result, sizeof(result));
		ak_cmd_exec("ifconfig wlan0 up", result , LEN_WIFI_CHK );
		sleep(5);
		while(i--) {
			bzero(result, sizeof(result));
			ak_cmd_exec("ifconfig | grep wlan0", result, LEN_WIFI_CHK);
			if(strlen(result) == 0){
				sleep(1);
			}
		}
		if (i == 0) {
			ak_print_normal_ex("the wifi device is still not working,  return error.\n");
			ret = AK_FAILED ;
		}
		else {
			ak_print_normal_ex("the wifi dev has running.\n");
		}
	} else {
		ak_print_normal_ex("the wifi dev has running.\n");
	}
	return ret;
	*/
}

static void save_wifi_info2tmp(const char *ssid, const char* pswd)
{
	int fd = open( FILE_WIFI_TMP , O_CREAT | O_RDWR | 0644);

	if (fd < 0) {
		ak_print_normal_ex("open failed, %s\n", strerror(errno));
		return;
	}

	char buf1[100] = {0};
	char buf2[100] = {0};
	char buf3[100] = {0};

	sprintf(buf1, "ssid=%s\n", ssid);
	sprintf(buf2, "pswd=%s\n", pswd);
	if (strlen(pswd))
		sprintf(buf3, "sec=%s\n", "wpa");
	else
		sprintf(buf3, "sec=%s\n", "open");

	write(fd, buf1, strlen(buf1));
	write(fd, buf2, strlen(buf2));
	write(fd, buf3, strlen(buf3));

	close(fd);

}


/**
 * check_wifi_connection - check ssid if it is in use ok.
 * @ssid[IN]: 	 ssid string
 * return:   1, connected;  0 , not;
 */
static int check_wifi_connection(const char *ssid)
{
	char result[100] = {0};
	int i = 0;

	/* check ssid */
	//ak_cmd_exec( "wpa_cli -iwlan0 status | grep '^ssid=' | sed 's/ssid=//g'" , result, sizeof(result));
	ak_cmd_exec( CMD_WIFI_STAT_FILE , result, sizeof(result));
	while(result[i]){                                                                           //替换第一个回车为字符串结束符
		if (result[i]=='\n' || result[i]=='\r'){
			result[i] = '\0';
			break;
		}
		i++;
	}
	if( strcmp( result , WIFI_STAT_SUCCESS ) == 0 ) {
		return AK_TRUE ;
	}
	else {
		return AK_FALSE ;
	}
	/*
	int same_ssid = 1;
	if (strlen(result) > 0) {
		char *p = strstr(result, "ssid=");
		if (p) {
			// if it is same, same_ssid eq 0
			same_ssid = strcmp(p + 5, ssid);
			ak_print_normal("[%s] ##### now: %s, ssid: %s\n", __func__, p + 5, ssid);
		}
	}
	return !(same_ssid);
	*/
}

/**
 * dana_wifi_conn_new_ssid - use new ssid to wifi.
 * @ssid[IN]: 	 ssid string
 * @pswd[IN]: passwd string
 * return:   1, connected;  0 , not;
 */

int dana_wifi_conn_new_ssid(const char *ssid, const char* pswd)
{
	int ret = 0 , i , i_wifi_curret ;
	char ac_buff_shellres[ LEN_BUFF ] , ac_cmd[ LEN_BUFF ] ; //, result[5] ;

	i_wifi_curret = check_wifi_connection( NULL ) ;                                             //获取当前的wifi连接状态
	/* save wireless ssid and password to tmp file,
	 * script will use it to connect wifi. if it success,
	 * ssid will be set to configure file.
	 */
	save_wifi_info2tmp(ssid, pswd);                                                             //将当前的wifi保存为暂存配置文件中去

	ak_print_normal_ex(" want to set this info: ESSID: %s, PSK: %s\n",
				 ssid, pswd);
	/* waiting for driver install and wpa_supplicant run */
	ak_cmd_exec( "wifi_manage.sh start" , ac_buff_shellres , LEN_BUFF ) ;                       //执行wifi连接脚本

	for( i = 0 ; i < NUM_TRY_WIFI_CONNECT ; i ++ ) {                                            //循环判断是否能够连接上wifi
		if ( ( ret = check_wifi_connection( NULL ) ) == AK_TRUE ) {
			break ;
		}
		sleep( 1 ) ;
	}
	if ( ret == AK_FALSE ) {
		snprintf( ac_cmd , LEN_BUFF , "rm %s" , FILE_WIFI_TMP ) ;
		ak_cmd_exec( ac_cmd , ac_buff_shellres , LEN_BUFF ) ;                               //删除暂存的wifi配置文件
		if( i_wifi_curret == AK_TRUE ) {                                                    //当前连接是使用wifi连接的
			ak_cmd_exec( "wifi_manage.sh start" , ac_buff_shellres , LEN_BUFF ) ;       //如果不能连接则使用配置文件的wifi配置进行重新连接
		}
		else {
			ak_cmd_exec( "wifi_manage.sh stop" , ac_buff_shellres , LEN_BUFF ) ;        //卸载wifi驱动和相应进程
			ak_cmd_exec( "eth_manage.sh start" , ac_buff_shellres , LEN_BUFF ) ;        //启动以太网
		}
	}
	/*
	if (0 == ret) {
		//ak_cmd_exec("wifi_station.sh connect", result, sizeof(result));
	}
	*/

	return ret;
}
