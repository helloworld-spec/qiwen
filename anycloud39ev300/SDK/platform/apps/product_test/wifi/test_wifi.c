#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_net.h"
#include "test_common.h"

#define BUF_SIZE 		64
#define AP  			"/var/ap_list"
#define LEN				81
#define LINE 				1024
#define AP_SIZE 			8096

typedef enum {
	WIFI_ENCTYPE_INVALID,
	WIFI_ENCTYPE_NONE,
	WIFI_ENCTYPE_WEP,
	WIFI_ENCTYPE_WPA_TKIP,
	WIFI_ENCTYPE_WPA_AES,
	WIFI_ENCTYPE_WPA2_TKIP,
	WIFI_ENCTYPE_WPA2_AES
} WIFI_ENCTYPE;//the type of wifi


struct ap {
	int index;
	int security;
	char address[LEN];
	char ssid[LEN];
	char password[LEN];
	char protocol[LEN];
	char mode[LEN];
	char frequency[LEN];
	char en_key[LEN];
	char bit_rates[LEN];
	char sig_level[LEN];
};

typedef struct _anyka_ap_info {
	char essid[33];
	unsigned int enc_type;
	int quality;
} anyka_ap_info, *Panyka_ap_info;

typedef struct _anyka_ap_list{
	int ap_num;
	anyka_ap_info ap_info[100];
} anyka_ap_list, *Panyka_ap_list;

struct ap_shop {
	int ap_num;
	struct ap ap_list[LEN];
};

char *wifi_conn_ssid_pre = "ak";
char wifi_conn_ssid[33];
char *wifi_conn_pwd = "123456789akspc";

static struct wifi_list_info test_wifi_info;
static int wifi_test_finish = 0;


void ScanOtherAp2File(void)
{
	char cmd[BUF_SIZE] = {0};

	// scan other AP to ap_list
	sprintf(cmd, "iwlist wlan0 scanning > %s", AP);
	system(cmd);
}

int cgiCheckSecurity(char *buf)
{
	int val = 0;

	if(strstr(buf, "Encryption key:on"))
	{
		if (strstr(buf, "WPA"))
		{
			val = WIFI_ENCTYPE_WPA_TKIP;
		}
		else if(strstr(buf, "WPA2"))
		{
			val = WIFI_ENCTYPE_WPA2_TKIP;
		}

	}
	else if(strstr(buf, "Encryption key:off"))
	{
		val = WIFI_ENCTYPE_NONE;
	}

	return val;
}

static int cgiGetVal(const char *str, const char *name, char *val)
{
	str = strstr(str, name);
	if(str)
	{
		for(str += strlen(name); (*str) && (*str != '"') && (*str != '\n'); )
		{
			*val++ = *str++;
		}
	}

	*val = '\0';
	return str != NULL;
}

static int cgiGetOneAp(char *buf, struct ap *ap)
{
	cgiGetVal(buf, "Address: ", ap->address);
	cgiGetVal(buf, "ESSID:\"", ap->ssid);
	cgiGetVal(buf, "Protocol:", ap->protocol);
	cgiGetVal(buf, "Mode:", ap->mode);
	cgiGetVal(buf, "Frequency:", ap->frequency);
	cgiGetVal(buf, "Encryption key:", ap->en_key);
	cgiGetVal(buf, "Bit Rates:", ap->bit_rates);
	cgiGetVal(buf, "level=", ap->sig_level);
	ap->security = cgiCheckSecurity(buf);
	return strlen(ap->ssid);
}

static int cgiAllocOneAp(struct ap_shop *info)
{
	info->ap_num++;
	if (info->ap_num >= sizeof(info->ap_list))
	{
		info->ap_num--;
		ak_print_notice_ex("ap list=%d is full!",info->ap_num);
		return -1;
	}
	return 0;
}

static int cgiScanOtherAp(struct ap_shop *info)
{
	int error = 0;
	char *line;
	char *buffer;
	FILE *ap_file;
	struct ap ap_temp;

	line   = (char *)calloc(1,LINE);
	buffer = (char *)calloc(1,LINE*10);

	ak_print_info_ex("Scanning available wifi AP.\n");

	ScanOtherAp2File();

	ap_file = fopen(AP, "r");
	if (NULL == ap_file)
	{
		ak_print_error_ex("open file %s fail!\n", AP);
		goto fail;
	}

	while ((!feof(ap_file)) && (strstr(line, "Address:") == NULL))
	{
		fgets(line, LINE, ap_file);
	}

	for (; (!feof(ap_file)) && (strstr(line, "Address:") != NULL); )
	{
		memset(buffer, '\0', sizeof(LINE*10));
		strcat(buffer, line);

		/* get one AP info to buffer*/
		for (fgets(line, LINE, ap_file); (!feof(ap_file)) && (strstr(line, "Address:") == NULL); /*nul*/)
		{
			strcat(buffer, line);
			fgets(line, LINE, ap_file);
		}

		/* copy one ap info to list*/
		memset(&ap_temp, 0, sizeof(struct ap));
		if (cgiGetOneAp(buffer, &ap_temp))
		{
			error = cgiAllocOneAp(info);

			if (error)
			{
				ak_print_error_ex("cgi Alloc One Ap fail!\n");
				fclose(ap_file);
				goto fail;
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

static void InitApInfo(struct ap_shop *info)
{
	memset(info, 0, sizeof(struct ap_shop));
	info->ap_num = -1;
}

static int doWirelessSearch(struct ap_shop *ap)
{
	int error;
	struct ap_shop *ap_info = ap;

	InitApInfo(ap_info);
	error = cgiScanOtherAp(ap_info);

	return error;
}

/**
 * get_ap_list - start to search and get wifi ap list
 * return:  wifi ap list
 */
Panyka_ap_list get_ap_list()
{
	int i;
	Panyka_ap_list ap_list;
	struct ap_shop soft_ap_list;

	if(doWirelessSearch(&soft_ap_list))
	{
		return NULL;
	}

	ap_list = (Panyka_ap_list)malloc(sizeof(anyka_ap_list));
	ap_list->ap_num = soft_ap_list.ap_num;

    ak_print_normal_ex(" ap_num : %d\n",soft_ap_list.ap_num);
	if(soft_ap_list.ap_num < 0)    soft_ap_list.ap_num = 0;
	if(soft_ap_list.ap_num > 100)    soft_ap_list.ap_num = 100;
	ak_print_normal_ex("Wifi list:\n");
	ak_print_normal_ex("total val : %d\n",soft_ap_list.ap_num);
	for(i = 0; i < soft_ap_list.ap_num; i++)
	{
		ap_list->ap_info[i].enc_type = soft_ap_list.ap_list[i].security;
		ap_list->ap_info[i].quality = atoi(soft_ap_list.ap_list[i].sig_level);
		strcpy(ap_list->ap_info[i].essid, soft_ap_list.ap_list[i].ssid);

		ak_print_normal_ex("id:%d, enc:%d,ssid:%s, quality:%d\n", i+1, soft_ap_list.ap_list[i].security,
				soft_ap_list.ap_list[i].ssid, atoi(soft_ap_list.ap_list[i].sig_level));
	}

	memset(&test_wifi_info, 0, sizeof(test_wifi_info));

	test_wifi_info.cnt = soft_ap_list.ap_num;
	if (soft_ap_list.ap_num > 0) {
		for (i=0; i<test_wifi_info.cnt; i++) {
			memcpy(test_wifi_info.info[i].wify_ssid_name, ap_list->ap_info[i].essid, sizeof(test_wifi_info.info[i].wify_ssid_name));
			test_wifi_info.info[i].quality = ap_list->ap_info[i].quality;
		}
	}
	
	return ap_list;
}

int get_wifi_info_list(struct wifi_list_info *wifi_info)
{
	if (NULL == wifi_info) {
		ak_print_error_ex("wifi_info NULL!\n");
		return -1;
	}

	memcpy(wifi_info, &test_wifi_info, sizeof(struct wifi_list_info));
	ak_print_normal("test_wifi_info.cnt : %d\n", test_wifi_info.cnt);

	return 0;	
}


static int do_syscmd(char *cmd, char *result)
{
	char buf[512];
	FILE *filp;

	filp = popen(cmd, "r");
	if (NULL == filp) {
		ak_print_error_ex("popen fail!\n");
		return -2;
	}

	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf)-1, filp);

	sprintf(result, "%s", buf);

	pclose(filp);
	return strlen(result);
}

/**
 * save_ap_list - save ap list to tmp file.
 * return:  void
 */
void save_ap_list(Panyka_ap_list ap_list)
{
	Panyka_ap_list list = ap_list;
	int wifi_conn_quality = -100;

	FILE *filp = fopen("/tmp/ap_list.txt", "w+");
	int count = list->ap_num;

	char buf[100] = {0};
	sprintf(buf, "total:%d\n", count);
	fwrite(buf, 1, strlen(buf), filp);

	while (count > 0) {
		bzero(buf, 100);
		sprintf(buf, "ssid:%s;quality:%d\n",
			   	list->ap_info[count - 1].essid,
				list->ap_info[count - 1].quality);
		fwrite(buf, 1, strlen(buf), filp);

		if(strncmp(list->ap_info[count - 1].essid,wifi_conn_ssid_pre,2) == 0){
			if(list->ap_info[count - 1].quality > wifi_conn_quality){
				wifi_conn_quality = list->ap_info[count - 1].quality;
				strcpy(wifi_conn_ssid, list->ap_info[count - 1].essid);
			}
		}

		count--;
	}
	fclose(filp);
}

/**
 * save_wifi_info2tmp - save ssid and passwd to file.
 * return:  void
 */
static void save_wifi_info2tmp(const char *ssid, const char* pswd)
{
	int fd = open("/tmp/wifi_info", O_CREAT | O_RDWR | 0644);

	if (fd < 0) {
		ak_print_error_ex("open failed, %s\n",strerror(errno));
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
 * check_wifi_connection - check ssid if connection ok.
 * return:  1, conneceted;  otherwise 0
 */
static int check_wifi_connection(const char *ssid)
{
	char result[100] = {0};
	int i = 0;

	/* check ssid */
	do_syscmd("wpa_cli -iwlan0 status | grep '^ssid='", result);
	while(result[i])
    {
        if (result[i]=='\n' || result[i]=='\r')
        {
            result[i] = '\0';
			break;
        }

        i++;
    }
	int same_ssid = 1;
	if (strlen(result) > 0) {
		char *p = strstr(result, "ssid=");
		if (p) {
			/** if it is same, same_ssid eq 0 **/
			same_ssid = strcmp(p + 5, ssid);
			ak_print_info_ex(" now: %s, ssid: %s\n", p + 5, ssid);
		}
	}

	return !(same_ssid);
}


int connect_new_wifi(const char *ssid, const char* pswd)
{
    int ret = 0;
    save_wifi_info2tmp(ssid, pswd);
		/* save wireless ssid and password config */
    ak_print_info_ex(" want to set this info: ESSID: %s, PSK: %s\n",
				 ssid, pswd);
		/* waiting for driver install and wpa_supplicant run */
	sleep(1);
	ret = check_wifi_connection(ssid);
    if( 0 == ret){
		sleep(1);
		ret = check_wifi_connection(ssid);;
    }
    return ret;
}

int is_test_wifi_finish(void)
{
	return wifi_test_finish;
}

void test_wifi_clean_flag(void)
{
	wifi_test_finish = 0;
}


/**
 * test_wifi - test wifi function.
 * return:  0, sucess; -1, failed
 */
int test_wifi( void)
{
	char result[512] = {0};
	int ret = 0, i = 5, j = 0;
	int wifi_status = 0;
	int eth0_status = 1;
	int search_ssid = 0;
	char ip[16];
	Panyka_ap_list ap_list;

	memset(&test_wifi_info, 0, sizeof(test_wifi_info));

	ak_print_normal_ex(" start.\n");
    system("rm -f /tmp/ap_list.txt");

	do_syscmd("ifconfig | grep eth0", result);
	if(strlen(result) == 0) {
		/*no wire net*/
		eth0_status = 0;
		do{
			sleep(1);
			do_syscmd("ifconfig | grep wlan0", result);
		}while(strlen(result) == 0);
	}else{
		/*have wire net*/
		do{
			sleep(1);
			j++;
			do_syscmd("ifconfig eth0 | grep RUNNING", result);
		}while((strlen(result) == 0) && (j<8));
		if(j < 8)
			ak_print_info_ex("eth0 is running.\n");
		else{
			eth0_status = 0;
			ak_print_info_ex("eth0 not running.\n");
		}
	}

	do_syscmd("ifconfig | grep wlan0", result);
	/* eth0 not up, wait wireless up */
	while( (0 == eth0_status) && (strlen(result) == 0)){
		ak_print_normal_ex("wait wlan0 up.\n");
		sleep(1);
		do_syscmd("ifconfig | grep wlan0", result);
	}
	if(strlen(result) == 0) {
		ak_print_normal_ex("the wifi device is not working, start it.\n");

		/* install wifi driver */
		system("wifi_driver.sh station");

		system("ifconfig wlan0 up");

		sleep(5);
		while(i--) {
			bzero(result, sizeof(result));
			do_syscmd("ifconfig | grep wlan0", result);
			if(strlen(result) == 0){
				sleep(1);
			}
		}
		if (i == 0) {
			ak_print_error_ex("the wifi device is still not working, finish test and return error.\n");
			return -1;
		}
	} else {
		wifi_status = 1;
		ak_print_normal_ex("the wifi dev has running.\n");
	}
RE_SEARCH_TEST_SSID:
	ap_list = get_ap_list();

	wifi_test_finish = 1;
	
	if (!ap_list) {
		ak_print_error_ex(" fail to get ap list\n");
		ret = -1;
		goto fail;
	}

	if(ap_list->ap_num == 0) {
		ak_print_normal_ex("cannot find any ssid .\n");
		ret = -1;
	} else {
		ret = 0;
	}

	memset(wifi_conn_ssid , 0, 33);
	/*
	 * save ap list to tmp file
	 */
	save_ap_list(ap_list);

	if(ap_list)
		free(ap_list);

fail:
	if(!wifi_status) {
		system("ifconfig wlan0 down");

		/* uninstall wifi driver */
		system("wifi_driver.sh uninstall");
	}

	else{
		/* if  do re-test-wifi and wifi have been configure, donot configure wifi */
		if(ak_net_get_ip("wlan0", ip)){
			if(wifi_conn_ssid[0] != 0)
			{
				ak_print_normal_ex("ssid:%s pwd:%s.\n",wifi_conn_ssid,wifi_conn_pwd);
				connect_new_wifi(wifi_conn_ssid,wifi_conn_pwd);
			}
			else{
				if(search_ssid++ < 3){
					ak_print_normal_ex("count:%d .\n", search_ssid);
					goto RE_SEARCH_TEST_SSID;
				}
			}
		}
	}

	return ret;
}
