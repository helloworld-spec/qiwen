#ifndef _DRV_EFUSEE_H_
#define _DRV_EFUSEE_H_

#include <regs.h>
#include <uart/drv_uart.h>

void read_efuse_identify(u32 *chip_identity);

#endif /* _DRV_EFUSEE_H_ */

