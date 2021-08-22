#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <signal.h>
#include <linux/netlink.h>

#include "ssv_smartlink.h"

void main(void)
{
    int ret=-1;
    int timeout=0;
    int status = 0;
    char status_str[128];
    char ssid[128];
    char pass[128];

    printf("%s\n", ssv_smartlink_version());

    ret = smaricomm_start();
    if (ret < 0)
    {
        printf("smaricomm_start fail\n");
        return;
    }

    printf("smarticomm_set_si_cmd\n");
    status = START_SMART_ICOMM;
    smarticomm_set_si_cmd(status);

    do
    {
        sleep(2);
        memset(status_str,0x00,128);
        printf("smarticomm_get_si_status\n");
        smarticomm_get_si_status(status_str);
        printf("status string = %s\n",status_str);
        timeout++;
    }while(strcmp(status_str,"OK"));

    smarticomm_get_si_ssid(ssid);
    printf("ssid = %s\n",ssid);
    smarticomm_get_si_pass(pass);
    printf("pass = %s\n",pass);

   smarticomm_stop();
}

