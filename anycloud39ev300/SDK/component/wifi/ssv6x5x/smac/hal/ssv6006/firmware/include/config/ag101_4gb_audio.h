#include <config/driver_config.h>
#include <config/app_config.h>
/*
 * Select Platform
 */
#define CONFIG_HEARTBEAT_LED 1
#undef CONFIG_HEARTBEAT_UART
#undef CONFIG_PLAT_AG101_4GB
#undef CONFIG_PLAT_AG101P_16MB
#define CONFIG_PLAT_AG101P_4GB 1
#undef CONFIG_PLAT_QEMU
#define CONFIG_CACHE_SUPPORT 1
#define CONFIG_CPU_ICACHE_ENABLE 1
#define CONFIG_CPU_DCACHE_ENABLE 1
//#define CONFIG_CPU_DCACHE_WRITETHROUGH 1
#undef CONFIG_CHECK_RANGE_ALIGNMENT
#undef CONFIG_CACHE_L2
#undef CONFIG_FULL_ASSOC

/*
 * Library Configuration
 */
#undef CONFIG_PFM_COUNTER
#undef CONFIG_USE_LCD

/*
 * Debugging Options
 */
#undef CONFIG_DEBUG
#undef CONFIG_WERROR
