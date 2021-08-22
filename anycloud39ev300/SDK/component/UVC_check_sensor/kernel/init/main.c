#include <stdarg.h>
#include "drv_api.h"
#include "command.h"
#include "drv_gpio.h"

#include "anyka_bsp.h"
#include "os_malloc.h"

#ifdef AKOS
#include "akos_api.h"
#endif

#ifdef SUPPORT_FS    
#include "fat_sysif.h"		
#endif
#include "partition_port.h"
#include "utils.h"

#include "platform_devices.h"
#include "dev_drv.h"

#define PLATFORM_VERSION "Sky39EV200 V1.2.01"


#define INIT_TASK_STACK_SIZE     (16<<10)
unsigned char initTaskStack[INIT_TASK_STACK_SIZE];
static const unsigned char initTaskName[] = "CMain";


void CMain();

void cmd_help()
{
    printk( "system information:\n");

    //display chip type
    printk( "chip = AK%x\n", drv_get_chip_type());
    printk( "ver  = V%x\n", drv_get_chip_version());

    //display freq info
    printk( "pll  = %dMhz\n", MAIN_CLK/(1000*1000));
    printk( "asic = %dMhz\n", get_asic_freq()/(1000*1000));
    printk( "cpu  = %dMhz\n", get_cpu_freq()/(1000*1000));
    printk( "mem  = %dMhz\n", get_mem_freq()/(1000*1000));

    //display ram info
    printk( "ram  = %dMBytes\n", drv_get_ram_size()/(1024*1024));

    printk( "\ncommand list:\n");
    cmd_show_help(NULL);
}

void command_task(unsigned long argc, void *argv)
{	
    //
    // enter command parse loop
    //
    cmd_loop();
}

#ifdef AKOS
//!!!!!don't enable interrupt in this function!!!!!
void Application_Initialize(void *first_available_memory)
{
    //config as jtag if
	//*(volatile unsigned long*)(0x08000074) |= (1<<0) | (1<<5) | (1<<6);
	//*(volatile unsigned long*)(0x08000078) |= (3<<4) | (3<<6) | (3<<8) | (3<<10);
	
    //
    // init MMU
    //
	MMU_Init(_MMUTT_STARTADDRESS); 

	//
	// init AKOS
	//
    AK_System_Init((void *)CMain, (unsigned char*)initTaskName, initTaskStack, INIT_TASK_STACK_SIZE);
}
#endif

void except_handler(unsigned char type)
{
    printk( "except_handler(): type=%d\n", type);
}

static void init_locks_in_locksection()
{
    //define in section.lds
    extern int __lock_init_start, __lock_init_end;

    int *addr;
    long * lock;
    for (addr = &__lock_init_start; addr < &__lock_init_end; addr++) 
    {
        printk("\ninit lock %d in locksection\n", addr - &__lock_init_start);
        lock = (long *)*addr;
       	*lock = AK_Create_Semaphore(1, AK_PRIORITY);
        
    }

}
void CMain()
{
	int tmp;
	int fd;
#ifdef AKOS
    #define COMMAND_STACK_SIZE (16*1024)
    unsigned long *Command_Task_Stack;
    T_hTask handle;
#endif
    T_DRIVE_INITINFO drv_info;

#if defined(CHIP_AK3918E)
    drv_info.chip = CHIP_3918E;
#else
    while(1);
#endif

    //
	// init mm
    //
    Fwl_MallocInit();

    //
    // init idle task
    //
    idle_thread_create();

 	//
 	// init drvlib
 	//
 	
    drv_info.fRamAlloc = (void *)Fwl_MallocAndTrace;
    drv_info.fRamFree = (void *)Fwl_FreeAndTrace;
    drv_init(&drv_info);
    exception_set_callback(except_handler);

	set_asic_pll(400);//only adjust asic_pll=400mhz => vclk=200mhz. no change cpu & ddr.
    //
    // init uart/gpio/timer
    //
    vtimer_init();
    console_init(uiUART0 ,CONSOLE_UART, 115200);

	//register devcies
	dev_do_register();
	
    printk( "start kernel...\n");
    printk( "chip_type=AK%x\n", drv_get_chip_type());

    printk( "gpio pin init.\n");
	gpio_init();

	codec_intr_enable();

    printk("platform version:%s\n", PLATFORM_VERSION);

	
    //
    // init shell command
    //
    cmd_utils_reg();//register utils 
	cmd_do_register();
    cmd_init(cmd_help);

    init_locks_in_locksection();
    
#ifdef AKOS
    Command_Task_Stack = Fwl_Malloc(COMMAND_STACK_SIZE);
    handle = AK_Create_Task((void *)command_task, (unsigned char*)"commandtask", 
            0, NULL,
            Command_Task_Stack, COMMAND_STACK_SIZE,
            90, 2,
            AK_PREEMPT, AK_START);
	
    if(AK_IS_INVALIDHANDLE(handle))
    {
        printk("Invalid Handle!");
        while(1);
    }
#else
    command_task(0, NULL);
#endif

    
    //
    // init flash mtd partition
    //
	//partition_port_init();


    //
    // init rtc
    //
#if 0 //  def SUPPORT_RTC
	rtc_init(0);
#endif

#if 0  //  def SUPPORT_WIFI  
    //
    // init network
    // 
	fd = dev_open(DEV_WIFI);
	if(fd < 0)
	{
		printk("open wifi faile\r\n");
		while(1);
	}
	dev_read(fd,  &tmp, 1);
	gpio_share_pin_set( ePIN_AS_SDIO , tmp);
	
    if(wifi_init() == 0)
    {
        unsigned char mac[6] = {0,0,0,0,0,0};
		wifi_get_mac(mac);
		printk("\nmac addr=%x:%x:%x:%x:%x:%x\n\n"
			, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		//init_wifi_lwip();
		tcpip_init(NULL, NULL); //wmj
    }
	else
	{
		printk("init wifi error!!!\n");
	}
#endif

    //
    // init filesystem
    //
#if 0 //  def SUPPORT_FS    
	if (init_fs())
	{
		printk("\ninit file system ok!\n");
	}else
	{
		printk("\ninit file system fail!\n");
	}	
	//dev_open(DEV_TCARD);
#endif

	cmd_uvc_demo(0, 0);

    while(1)
    {
        AK_Sleep(1000);
    }
    return;
        
}



