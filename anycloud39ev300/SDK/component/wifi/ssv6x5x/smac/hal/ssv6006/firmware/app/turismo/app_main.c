#include <config.h>
#include <msgevt.h>
#include <pbuf.h>
#include <hal.h>
#include <ssv_version.h>
#include <ssv_chip.h>
#include <mac80211/soft-main.h>
#include <mac80211/cmd_engine.h>
#include <mac80211/tx_stuck.h>
#include <mac80211/soft_beacon.h>
#include <cmd_def.h>
#include <log.h>
#include <ssv_lib.h>
#include <regs.h>

#include <stdio.h>
#include <stdarg.h>

#if (CONFIG_SIM_PLATFORM==0 && CLI_ENABLE == 1)       
#include <cli.h>
#endif

/**
 *  Task Information Table:
 *
 *  Each Task registered here is created in system initial time. After a task
 *  is created, a Message Queue for the task is also created. Once the 
 *  task starts, the task is blocked and waitting for message from the 
 *  Message queue.
 */
struct task_info_st g_soc_task_info[] = {
    { "Soft-Mac",       (OsMsgQ)0, 40, OS_TASK_PRIO3, 256, NULL, soft_mac_task      },
    { "CmdEngine",      (OsMsgQ)0, 4, OS_TASK_PRIO2, 256, NULL, CmdEngine_Task      },
#if (CONFIG_SIM_PLATFORM==0 && CLI_ENABLE == 1)
    { "cli_task",       (OsMsgQ)0, 0, OS_TASK_PRIO1, 256, NULL, Cli_Task            },
#endif
};


#if (CONFIG_SIM_PLATFORM == 0)
#if defined(ENABLE_FW_SELF_CHECK)
#include <ssv_regs.h>
void _calc_fw_checksum (void)
{
    volatile u32 *FW_RDY_REG = (volatile u32 *)(SPI_REG_BASE + 0x10);
#if 0
    u32          block_count = (*FW_RDY_REG >> 16) & 0x0FF;
    u32          fw_checksum = FW_CHECKSUM_INIT;
    u32         *addr = 0;
    u32          total_words = block_count * CHECKSUM_BLOCK_SIZE / sizeof(u32);

    printf("Calculating checksum from %d blocks...", block_count);
    // Don't calculate checksum if host does not provide block count.
    if (block_count == 0)
    {
        printf("skip.\n");
        return;
    }
        
    while (total_words--) {
        fw_checksum += *addr++;
    }
    fw_checksum = ((fw_checksum >> 24) + (fw_checksum >> 16) + (fw_checksum >> 8) + fw_checksum) & 0x0FF;

    printf("done with %02X.\n", fw_checksum);

    fw_checksum = (fw_checksum << 16) & 0x00FF0000; 

    *FW_RDY_REG = (*FW_RDY_REG & 0xFF00FFFF) | fw_checksum;
    // wait until host confirm the checksum.
    fw_checksum = ~fw_checksum & 0x00FF0000;

    printf("Waiting host's acknowledge...\n");
    do {
        if ((*FW_RDY_REG & 0x00FF0000) == fw_checksum)
            break;
    } while (1);

#else
    extern u32 firmware_checksum;
    extern u32 firmware_block_count;
    u32 fw_checksum;

    // Don't calculate checksum if host does not provide block count.
    if (firmware_block_count == 0)
    {
        printf("Checksum check skipped.\n");
        return;
    }

    printf("Calculated checksum from %d blocks into %02X.\n", firmware_block_count, firmware_checksum);

    // wait until host confirm the checksum.
    fw_checksum = ~firmware_checksum & 0x00FF0000;

    printf("Waiting host's acknowledge...");
    do {
        if ((*FW_RDY_REG & 0x00FF0000) == fw_checksum)
            break;
    } while (1);
    printf("OK.\n");

    #endif
}
#endif // ENABLE_FW_SELF_CHECK

#if 0
static void _zero_bss (void)
{
    extern u32   __bss_beg__;
    extern u32   __bss_end__;
    u8          *p_bss = (u8 *)&__bss_beg__;
    u8          *p_bss_end = (u8 *)&__bss_end__;

    while (p_bss != p_bss_end)
        *p_bss++ = 0; 
}
#endif // 0
#endif

/**
 *  Entry point of the firmware code. After system booting, this is the
 *  first function to be called from boot code. This function need to 
 *  initialize the chip register, software protoctol, RTOS and create
 *  tasks. Note that, all memory resource needed for each software
 *  module shall be pre-allocated in initialization time.
 *
 */
u32 efuse_chip_identity = 0x0;
void APP_Init (void)
{
    volatile u32 *FW_RDY_REG = (volatile u32 *)(SPI_REG_BASE + 0x10);
    register u32 i;

    /* Check integraty of downloaded firmware */
    #if (CONFIG_SIM_PLATFORM == 0)
    #ifdef ENABLE_FW_SELF_CHECK
    _calc_fw_checksum();
    #endif // ENABLE_FW_SELF_CHECK
    /* Fill BSS with 0 */
    //_zero_bss();
    #endif
    /* Initialize hardware & device drivers */
    // ASSERT(bsp_init() == OS_SUCCESS);

#if (CONFIG_SIM_PLATFORM == 0)
    /**
     * Initialize RTOS before starting use it. If in simulation/emulation platform,
     * we shall ignore this initialization because the simulation/emulation has
     * initialized.
     */
    ASSERT( OS_Init() == OS_SUCCESS );

    #if 0
    LOG_init(true, true, LOG_LEVEL_ON, LOG_MODULE_MASK(LOG_MODULE_EMPTY) | LOG_MODULE_MASK(LOG_MODULE_ALL), false);
    LOG_OUT_DST_OPEN(LOG_OUT_SOC_TERM);
    LOG_OUT_DST_CUR_ON(LOG_OUT_SOC_TERM);
    #endif // 0
#endif

    /**
     * Initialize software components.
     */
    ASSERT( msg_evt_init() == OS_SUCCESS ); 
    ASSERT( PBUF_Init() == OS_SUCCESS );
    ASSERT( soft_mac_init() == OS_SUCCESS );
    ASSERT( tx_stuck_init() == OS_SUCCESS );
    ASSERT( soft_beacon_init() == OS_SUCCESS );
    ASSERT( cmd_engine_init() == OS_SUCCESS);

#ifndef IGNORE_CHIP_ID

    if (CHIP_IDENTITY == efuse_chip_identity)
    {
        //Write firmware Version(SVN version)
        *FW_RDY_REG = ssv_firmware_version;
    }
    else
    {
        *FW_RDY_REG = FIRWARE_NOT_MATCH_CODE;
        //LOG_FATAL("Firmware not match efuse_chip_identity[%08x]\n",efuse_chip_identity);

        do {
            if (*FW_RDY_REG == 0)
                *FW_RDY_REG = efuse_chip_identity;
        } while (1);
    }
#else
    //Write firmware Version(SVN version)
    *FW_RDY_REG = ssv_firmware_version;
#endif

    /**
     * Create Tasks and Message Queues. Creating Message Queue or not
     * depends on the xxx.qevt field is zero or not. If zero, no need to
     * create a message queue for the task.
     */
    for (i = 0; i < (sizeof(g_soc_task_info)/sizeof(g_soc_task_info[0])); i++) {
        /* Create Registered Task: */
        OS_TaskCreate(g_soc_task_info[i].task_func, 
            g_soc_task_info[i].task_name,
            g_soc_task_info[i].stack_size,
            g_soc_task_info[i].args, 
            g_soc_task_info[i].prio, 
            NULL); 

        if ((u32)g_soc_task_info[i].qlength> 0) 
        {
            ASSERT(OS_MsgQCreate(&g_soc_task_info[i].qevt, 
            (u32)g_soc_task_info[i].qlength)==OS_SUCCESS);
        }
    }

    LOG_PRINTF("Initialzation done. \nStarting OS scheduler...\n");
#if (CONFIG_SIM_PLATFORM == 0)

    /* Start the scheduler so our tasks start executing. */
    OS_StartScheduler();
#endif
}

