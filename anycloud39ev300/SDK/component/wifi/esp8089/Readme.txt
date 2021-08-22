
操作方法：
1.将ak_es8089.tar.gz 解压到 /kernel/drivers/net/wireless/   参考附件中的Makefile及其Kconfig  修改其中的Makefile 及其Kconfig。
2.根据es8089mmcpatch手动修改里面的文件。
3.我是利用开发板测试的，配置开发板板载文件mach-cloud39e_ak3916e_128pin_mnbd.c  ,可参考附件中的.c文件。
/* akwifi platform data */
struct akwifi_platform_data akwifi_pdata = {
	.gpio_init = ak_gpio_set,
	.gpio_cs = {    
         .pin        = -1,
         .pulldown   = AK_PULLDOWN_DISABLE,
         .pullup     = -1,
         .value      = AK_GPIO_OUT_HIGH,   // cs óDD§?μ
         .dir        = AK_GPIO_DIR_OUTPUT,
		.int_pol    = -1,
    },
	.gpio_on = {
		.pin		= AK_GPIO_50,
		.pulldown	= -1,
		.pullup		= AK_PULLUP_DISABLE,
		.value		= AK_GPIO_OUT_HIGH,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.gpio_off = {
		.pin		= AK_GPIO_50,
		.pulldown	= -1,
		.pullup		= AK_PULLUP_DISABLE,
		.value		= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.power_on_delay   = 2000,
	.power_off_delay  = 200,
};



内核配置：
Linux/arm 3.4.35 Kernel Configuration 
                  ---》Device Drivers     
                         --》 MMC/SD/SDIO card support
                               <*>   SDIO WIFI support  
                               [*]   ANYKA MMC/SD/SDIO Card Interface support  