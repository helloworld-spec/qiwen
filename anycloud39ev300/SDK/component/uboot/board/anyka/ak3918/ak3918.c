/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/l2buf.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

/*
 * Miscellaneous platform dependent initialisations
 */

int board_early_init_f (void)
{
	l2_init();
	return 0;
}

int board_init (void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	return 0;
}


int misc_init_r (void)
{
#if 0
	setenv("verify", "n");
#endif
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
			PHYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_AK_ETHER
	rc = ak39_ethernet_initialize(0, 0x20300000);
#endif

	return rc;
}
#endif
