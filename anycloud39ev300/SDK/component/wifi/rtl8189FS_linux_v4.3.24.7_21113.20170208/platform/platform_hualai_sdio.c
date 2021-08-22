#include <linux/mmc/host.h>
#include <linux/fs.h>
#include <linux/wlan_plat.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>
//#include <mach/jzmmc.h>
//#include <linux/bcm_pm_core.h>

//#include <gpio.h>
//#include <soc/gpio.h>

#define GPIO_WIFI_WAKEUP     	GPIO_PC(17)
#define GPIO_WIFI_RST_N			GPIO_PC(16)
#define SDIO_WIFI_POWER         GPIO_PB(30)
#define WLAN_SDIO_INDEX			1

/* static int wl_pw_en = 0; */
extern void rtc32k_enable(void);
extern void rtc32k_disable(void);

void wlan_pw_en_enable(void)
{
}

void wlan_pw_en_disable(void)
{
}


/*
    It calls jzmmc_manual_detect() to re-scan SDIO card
*/
int platform_wifi_power_on(void)
{
    //gpio_request(SDIO_WIFI_POWER, "sdio_wifi_power_on");
	//gpio_direction_output(SDIO_WIFI_POWER, 1);
	printk("wlan power on\n");
	msleep(10);

	//jzmmc_manual_detect(WLAN_SDIO_INDEX, 1);

	return 0;
}

int platform_wifi_power_off(void)
{
	//gpio_direction_output(SDIO_WIFI_POWER, 0);
    //gpio_free(SDIO_WIFI_POWER);
	printk("wlan power off\n");
	return 0;
}
