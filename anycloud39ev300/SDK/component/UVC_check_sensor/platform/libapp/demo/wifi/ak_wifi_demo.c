/*
*wifi_demo.c    wifi operation demo
*/
#include "wifi.h"
#include "command.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "AKerror.h"


static char *help_ap[]={
	"wifi ap tools",
	"usage:ap start [ch] [ssid] <password>\n"
	"      ap stop\n"
};
static char *help_sta[]={
	"wifi sta tools",
	"usage: sta conn [ssid] <password>\n"
	"       sta disconn\n"
};

int sta_reconn_flag = AK_TRUE;
ak_pthread_t  g_sta_conn_thread_id = AK_INVALID_TASK;
struct _conn_param conn_param;

struct _conn_param
{
	unsigned char essid[MAX_SSID_LEN];
	unsigned char password[MAX_KEY_LEN];
};


/*keep connection thread*/
static void sta_conn_thread(void *args)
{
	struct _conn_param *conn_param = (struct _conn_param*)args;
	
	while(sta_reconn_flag == AK_TRUE)
	{
		if (!wifi_isconnected())
		{	
			ak_print_normal("Connect to SSID: < %s >  with password:%s\n", conn_param->essid,  conn_param->password);
			wifi_connect(conn_param->essid, conn_param->password);
			if (wifi_isconnected())
			{
				ak_print_normal("wifi connect ok\n");
				wifi_power_cfg(0);
				wifistation_netif_init();
			}
		}
		ak_sleep_ms(1000);
	}
	ak_print_normal("sta conn thread exit.\n");
	ak_thread_exit();
}


/**
 *测试WIFI连接,
 *连接到指定名字的路由器，加密模式和频道自动适应
 *密码长度在WPA或WPA2模式下8 <= len <= 64;在WEP模式下必须为5或13
 */
static void cmd_wifi_sta(int argc, char **args)
{
	char *essid = args[1];
	char *password =args[2];

	if (strcmp(args[0], "conn") == 0)
	{
		if (argc != 2 && argc != 3)
		{
			ak_print_normal("%s",help_sta[1]);
			return;
		}
		if(strlen(args[1]) > MAX_SSID_LEN)
		{
			ak_print_normal("ssid should less than 32 characters\n");
			return;
		}
		if(argc == 3 && strlen(args[2]) > MAX_KEY_LEN)
		{
			ak_print_normal("password should less than 64 characters\n");
			return;
		}
		
		strcpy(conn_param.essid, args[1]);
		
		if(argc == 3)
		{
			strcpy(conn_param.password, args[2]);
		}
		else
		{
			memset(conn_param.password, 0, MAX_KEY_LEN);
		}
		/*create a task to connetc AP,  so the STA can reconnect AP case the AP unavailable temporily for some reason*/
		if(g_sta_conn_thread_id == AK_INVALID_TASK)
		{
			wifi_set_mode(WIFI_MODE_STA);
			sta_reconn_flag = AK_TRUE;
			ak_thread_create(&g_sta_conn_thread_id , (void*)sta_conn_thread , &conn_param, 4096, 10);
		}
		else
		{
			ak_print_normal("sta is connecting, please disconnect it first\n");
		}
		
	}
	else if (strcmp(args[0], "disconn") == 0)
	{
		if (argc != 1)
		{
			ak_print_normal("%s",help_sta[1]);
			return;
		}

		ak_print_normal("disconnect wifi \n");
		
		sta_reconn_flag = AK_FALSE;
		wifi_disconnect();
		if(g_sta_conn_thread_id != AK_INVALID_TASK)
		{
			ak_thread_join(g_sta_conn_thread_id);
			g_sta_conn_thread_id = AK_INVALID_TASK;
		}
		
	}
	else
	{
		ak_print_normal("%s",help_sta[1]);
	}
}

static void cmd_wifi_ap(int argc, char **args)
{
	struct _apcfg  ap_cfg;

	
	if (strcmp(args[0], "start") == 0)
	{
		if (argc != 3 && argc != 4)
		{
			ak_print_normal("%s",help_ap[1]);
			return;
		}
		
		memset(&ap_cfg, 0, sizeof(struct _apcfg));
		ap_cfg.mode = 0; //0:bg mode  1:bgn mode
		
		ap_cfg.channel = atoi(args[1]);
		if(ap_cfg.channel < 1 || ap_cfg.channel > 13)
		{
			ak_print_normal("channel should be 1 ~ 13\n");
			return;
		}
		strcpy(ap_cfg.ssid, args[2]);
		if(strlen(ap_cfg.ssid) > MAX_SSID_LEN)
		{
			ak_print_normal("AP SSID should be less than 32 characters \n");
			return;
		}
		if (argc == 4)
		{
			if(strlen(args[3]) < MIN_KEY_LEN || strlen(args[3]) > MAX_KEY_LEN)
			{
				ak_print_normal("AP key should be %d ~ %d characters \n", MIN_KEY_LEN, MAX_KEY_LEN);
				return;
			}	
			strcpy(ap_cfg.key, args[3]);
			ap_cfg.enc_protocol = KEY_WPA2;
		}
		else
		{
			ap_cfg.enc_protocol = KEY_NONE;
		}
		
		if(g_sta_conn_thread_id != AK_INVALID_TASK)
		{	
		    sta_reconn_flag = AK_FALSE;
			wifi_disconnect();
			ak_thread_join(g_sta_conn_thread_id);
			g_sta_conn_thread_id = AK_INVALID_TASK;
		}
		ak_print_normal("Create AP SSID:%s, 11n: %s, key mode:%d, key:%s, channel:%d\n", 
			ap_cfg.ssid, ap_cfg.mode?"enable":"disable", 
			ap_cfg.enc_protocol, ap_cfg.enc_protocol?(char*)ap_cfg.key:"",
			ap_cfg.channel);
		
		wifi_set_mode(WIFI_MODE_AP);
		wifi_create_ap(&ap_cfg);
		
		wifi_netif_init();
	}
	else if (strcmp(args[0], "stop") == 0)
	{
		if (argc != 1)
		{
			ak_print_normal("%s",help_ap[1]);
			return;
		}
		ak_print_normal("stop wifi ap...\n");
		wifi_destroy_ap();
	}
	else
	{
		ak_print_normal("%s",help_ap[1]);
	}
	
}


int wifi_demo_init()
{
	cmd_register("ap", cmd_wifi_ap, help_ap);
	cmd_register("sta", cmd_wifi_sta, help_sta);
	return 0;
}

cmd_module_init(wifi_demo_init)


