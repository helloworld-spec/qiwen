#include <config/driver_config.h>
#include <config/app_config.h>


/* 
 * ToolChain builtin macro
 */
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

	// GCC < 4.8.2 (Before BSP400 gcc version)
	// check the GCC version for toolchain macro compatible issue
	#if GCC_VERSION < 40802
	
	#ifdef NDS32_BASELINE_V3
	#define __NDS32_ISA_V3__	NDS32_BASELINE_V3
	#endif

	#ifdef NDS32_BASELINE_V3M
	#define __NDS32_ISA_V3M__	NDS32_BASELINE_V3M
	#endif
	
	
	#ifdef NDS32_BASELINE_V2
	#define __NDS32_ISA_V2__       NDS32_BASELINE_V2
	#endif

	#endif


/*
 * Select Platform
 */
#define CONFIG_HEARTBEAT_LED 1
#undef CONFIG_HEARTBEAT_UART
#undef CONFIG_PLAT_AG101_4GB
#define CONFIG_PLAT_AG101P_16MB 1
#undef CONFIG_PLAT_AG101P_4GB
#undef CONFIG_PLAT_QEMU
//#undef CONFIG_CACHE_SUPPORT
#undef CONFIG_CPU_ICACHE_ENABLE
#undef CONFIG_CPU_DCACHE_ENABLE
#undef CONFIG_CPU_DCACHE_WRITETHROUGH
#undef CONFIG_CHECK_RANGE_ALIGNMENT
#undef CONFIG_CACHE_L2
#undef CONFIG_FULL_ASSOC

/*
 * Interrupt Option
 */
#ifdef CONFIG_EXT_INTC
	/* 6IVIC + EXT_INTC */
        #define NO_EXTERNAL_INT_CTL 0
#else
	/* 32IVIC */
        #define NO_EXTERNAL_INT_CTL 1
#endif

#undef CONFIG_HW_PRIO_SUPPORT 


/*
 * Debugging Options
 */
#undef CONFIG_DEBUG
#undef CONFIG_WERROR
