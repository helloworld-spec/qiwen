/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef SSVSDIOBRIDGE_H
#define SSVSDIOBRIDGE_H

#include <linux/etherdevice.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/leds.h>
#include <linux/completion.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#include "sdiobridge_pub.h"

//#include "debug.h"
//#include "common.h"

struct ssv_sdiobridge_glue
{
    struct device *dev;

    //
	// Send data transactions in byte mode (0) or block mode (1)
	//
	u8 blockMode;
	u16 blockSize;

	u8 autoAckInt;
    
    /* for ssv SDIO */
    unsigned int		dataIOPort;
    unsigned int		regIOPort;
    u8		            funcFocus;

    atomic_t irq_handling;
    wait_queue_head_t irq_wq;

    /* for rx buf */
    wait_queue_head_t read_wq;
    spinlock_t rxbuflock;
    void *bufaddr;
	struct list_head rxbuf;
    struct list_head rxreadybuf;

    /* for debug */
    struct dentry *debugfs;
    struct dentry *dump_entry;
    u32 dump;
};


#define MANUFACTURER_SSV_CODE              0x3030	/* South Silicon Valley */
#define MANUFACTURER_ID_CABRIO_BASE        0x3030

#endif /* SSVSDIOBRIDGE_H */
