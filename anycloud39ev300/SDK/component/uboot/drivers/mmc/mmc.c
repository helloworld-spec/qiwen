/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <linux/list.h>
#include <div64.h>
#include "mmc_private.h"
#include <asm/ak_sdhsmmc.h>
#include <asm/arch-ak39/anyka_cpu.h>
#include <asm/io.h>

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

/* cdh:add If we fail after 1 second wait, something is really bad */
#define MMC_MAX_RETRY_MS	1000000

static struct list_head mmc_devices;
static int cur_dev_num = -1;

int __weak board_mmc_getwp(struct mmc *mmc)
{
	return -1;
}

int mmc_getwp(struct mmc *mmc)
{
	int wp;

	wp = board_mmc_getwp(mmc);

	if (wp < 0) {
		if (mmc->getwp)
			wp = mmc->getwp(mmc);
		else
			wp = 0;
	}

	return wp;
}

int __board_mmc_getcd(struct mmc *mmc) {
	return -1;
}

int board_mmc_getcd(struct mmc *mmc)__attribute__((weak,
	alias("__board_mmc_getcd")));

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;

#ifdef CONFIG_MMC_TRACE
	int i;
	u8 *ptr;

	printf("CMD_SEND:%d\n", cmd->cmdidx);
	printf("\t\tARG\t\t\t 0x%08X\n", cmd->cmdarg);
	ret = mmc->send_cmd(mmc, cmd, data);
	switch (cmd->resp_type) {
		case MMC_RSP_NONE:
			printf("\t\tMMC_RSP_NONE\n");
			break;
		case MMC_RSP_R1:
			printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R1b:
			printf("\t\tMMC_RSP_R1b\t\t 0x%08X \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R2:
			printf("\t\tMMC_RSP_R2\t\t 0x%08X \n",
				cmd->response[0]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[1]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[2]);
			printf("\t\t          \t\t 0x%08X \n",
				cmd->response[3]);
			printf("\n");
			printf("\t\t\t\t\tDUMPING DATA\n");
			for (i = 0; i < 4; i++) {
				int j;
				printf("\t\t\t\t\t%03d - ", i*4);
				ptr = (u8 *)&cmd->response[i];
				ptr += 3;
				for (j = 0; j < 4; j++)
					printf("%02X ", *ptr--);
				printf("\n");
			}
			break;
		case MMC_RSP_R3:
			printf("\t\tMMC_RSP_R3,4\t\t 0x%08X \n",
				cmd->response[0]);
			break;
		default:
			printf("\t\tERROR MMC rsp not supported\n");
			break;
	}
#else
	ret = mmc->send_cmd(mmc, cmd, data);
#endif
	return ret;
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err, retries = 5;
#ifdef CONFIG_MMC_TRACE
	int status;
#endif

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	if (!mmc_host_is_spi(mmc))
		cmd.cmdarg = mmc->rca << 16;

	do {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (!err) {
			if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
			    (cmd.response[0] & MMC_STATUS_CURR_STATE) !=
			     MMC_STATE_PRG)
				break;
			else if (cmd.response[0] & MMC_STATUS_MASK) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
				printf("Status Error: 0x%08X\n",
					cmd.response[0]);
#endif
				return COMM_ERR;
			}
		} else if (--retries < 0)
			return err;

		udelay(1000);

	} while (timeout--);

#ifdef CONFIG_MMC_TRACE
	status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
	printf("CURR STATE:%d\n", status);
#endif
	if (timeout <= 0) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("Timeout waiting card ready\n");
#endif
		return TIMEOUT;
	}

	return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;

	return mmc_send_cmd(mmc, &cmd, NULL); // cdh:check ok
}

struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->block_dev.dev == dev_num)
			return m;
	}

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
	printf("MMC Device %d not found\n", dev_num);
#endif

	return NULL;
}


static int mmc_read_blocks(struct mmc *mmc, void *dst, lbaint_t start,
			   lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;
	
	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity){
		debug("cdh:yes high capacity!\n");
		cmd.cmdarg = start;
	}
	else {
		debug("cdh:no high capacity!\n");
		cmd.cmdarg = start * mmc->read_bl_len;
	}
	//debug("cdh:%s, cmd.cmdarg:0x%x, start:0x%x, mmc->read_bl_len:0x%x\n", __func__, cmd.cmdarg, start, mmc->read_bl_len);
	
	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data))
		return 0;

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			printf("mmc fail to send stop cmd\n");
#endif
			return 0;
		}
	}

	/* cdh:add Waiting for the ready status */
	if (mmc_send_status(mmc, timeout)) {
		//printf("wait block read finish failed!\n");
		return 0;
	}else{
		//printf("wait block read finish OK!\n");
	}
	
	return blkcnt;
}

static ulong mmc_bread(int dev_num, lbaint_t start, lbaint_t blkcnt, void *dst)
{
	lbaint_t cur, blocks_todo = blkcnt;

	if (blkcnt == 0)
		return 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
		return 0;

	if ((start + blkcnt) > mmc->block_dev.lba) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
			start + blkcnt, mmc->block_dev.lba);
#endif
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len))
		return 0;

	do {
		cur = (blocks_todo > mmc->b_max) ?  mmc->b_max : blocks_todo;
		if(mmc_read_blocks(mmc, dst, start, cur) != cur)
			return 0;
		blocks_todo -= cur;
		start += cur;
		dst += cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	
	return blkcnt;
}

static int mmc_go_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = mmc_send_cmd(mmc, &cmd, NULL); // cdh:check ok

	if (err)
		return err;

	
	return 0;
}

static int sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	// cdh:send ACMD41
	do {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL); // cdh:check ok

		if (err)
			return err;

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
			(mmc->voltages & 0xff8000);

		if (mmc->version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		err = mmc_send_cmd(mmc, &cmd, NULL); // cdh:check ok

		if (err)
			return err;

		udelay(1000);
	} while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0)
		return UNUSABLE_ERR;

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	printf("cdh:%s, mmc->high_capacity=%d\n", __func__, mmc->high_capacity);
	mmc->rca = 0;

	return 0;
}

/* We pass in the cmd since otherwise the init seems to fail */
static int mmc_send_op_cond_iter(struct mmc *mmc, struct mmc_cmd *cmd,
		int use_arg)
{
	int err;

	cmd->cmdidx = MMC_CMD_SEND_OP_COND;
	cmd->resp_type = MMC_RSP_R3;
	cmd->cmdarg = 0;
	if (use_arg && !mmc_host_is_spi(mmc)) {
		cmd->cmdarg =
			(mmc->voltages &
			(mmc->op_cond_response & OCR_VOLTAGE_MASK)) |
			(mmc->op_cond_response & OCR_ACCESS_MODE);

		if (mmc->host_caps & MMC_MODE_HC)
			cmd->cmdarg |= OCR_HCS;
	}
	err = mmc_send_cmd(mmc, cmd, NULL);
	if (err)
		return err;
	mmc->op_cond_response = cmd->response[0];
	return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err, i;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

 	/* Asking to the card its capabilities */
	mmc->op_cond_pending = 1;
	for (i = 0; i < 2; i++) {
		err = mmc_send_op_cond_iter(mmc, &cmd, i != 0);
		if (err)
			return err;

		/* exit if not busy (flag seems to be inverted) */
		if (mmc->op_cond_response & OCR_BUSY)
			return 0;
	}
	return IN_PROGRESS;
}

int mmc_complete_op_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	uint start;
	int err;

	mmc->op_cond_pending = 0;
	start = get_timer(0);
	do {
		err = mmc_send_op_cond_iter(mmc, &cmd, 1);
		if (err)
			return err;
		if (get_timer(start) > timeout)
			return UNUSABLE_ERR;
		udelay(100);
	} while (!(mmc->op_cond_response & OCR_BUSY));

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}


static int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	data.dest = (char *)ext_csd;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}


static int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int ret;

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				 (index << 16) |
				 (value << 8);

	ret = mmc_send_cmd(mmc, &cmd, NULL);

	/* Waiting for the ready status */
	if (!ret)
		ret = mmc_send_status(mmc, timeout);

	return ret;

}

static int mmc_change_freq(struct mmc *mmc)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
	char cardtype;
	int err;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xf;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

	if (err)
		return err;

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	/* No high-speed support */
	if (!ext_csd[EXT_CSD_HS_TIMING])
		return 0;

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & MMC_HS_52MHZ)
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	else
		mmc->card_caps |= MMC_MODE_HS;

	return 0;
}

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
	switch (part_num) {
	case 0:
		mmc->capacity = mmc->capacity_user;
		break;
	case 1:
	case 2:
		mmc->capacity = mmc->capacity_boot;
		break;
	case 3:
		mmc->capacity = mmc->capacity_rpmb;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		mmc->capacity = mmc->capacity_gp[part_num - 4];
		break;
	default:
		return -1;
	}

	//printf("cdh:part_num=%d, mmc->capacity=0x%llx, mmc->capacity_user=0x%llx\n", part_num, mmc->capacity, mmc->capacity_user);
	mmc->block_dev.lba = lldiv(mmc->capacity, mmc->read_bl_len);
	//printf("cdh:part_num=%d, mmc->block_dev.lba=0x%llx\n", part_num, mmc->block_dev.lba);
	
	return 0;
}

int mmc_switch_part(int dev_num, unsigned int part_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);
	int ret;

	if (!mmc)
		return -1;

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 (mmc->part_config & ~PART_ACCESS_MASK)
			 | (part_num & PART_ACCESS_MASK));
	if (ret)
		return ret;

	return mmc_set_capacity(mmc, part_num);
}

int mmc_getcd(struct mmc *mmc)
{
	int cd;

	cd = board_mmc_getcd(mmc);

	if (cd < 0) {
		printf("cdh:use ak mmc cd!\n");
		if (mmc->getcd) {
			cd = mmc->getcd(mmc);
			printf("cdh:ak getcd=%d\n", cd);
		}
		else
			cd = 1;
	}

	printf("%s, cd=%d\n", __func__, cd);
	return cd;
}

static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.cmdidx = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = (mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);

	data.dest = (char *)resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}


static int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	ALLOC_CACHE_ALIGN_BUFFER(uint, scr, 2);
	ALLOC_CACHE_ALIGN_BUFFER(uint, switch_status, 16);
	struct mmc_data data;
	int timeout;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	cmd.cmdidx = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	timeout = 3;

retry_scr:
	data.dest = (char *)scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		return err;
	}

	
	/* cdh:add Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	mmc->scr[0] = __be32_to_cpu(scr[0]);
	mmc->scr[1] = __be32_to_cpu(scr[1]);
	printf("cdh:mmc->scr[0]=0x%x, mmc->scr[1]=0x%x\n", mmc->scr[0], mmc->scr[1]);
	
	switch ((mmc->scr[0] >> 24) & 0xf) {
		case 0:
			mmc->version = SD_VERSION_1_0;
			break;
		case 1:
			mmc->version = SD_VERSION_1_10;
			break;
		case 2:
			mmc->version = SD_VERSION_2;
			if ((mmc->scr[0] >> 15) & 0x1)
				mmc->version = SD_VERSION_3;
			break;
		default:
			mmc->version = SD_VERSION_1_0;
			break;
	}
	printf("cdh:mmc->version=0x%x\n", mmc->version);
	
	if (mmc->scr[0] & SD_DATA_4BIT) {
		mmc->card_caps |= MMC_MODE_4BIT;
		printf("cdh:SD_DATA_4BIT, mmc->card_caps=0x%x\n", mmc->card_caps);
	}

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

#if 1

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)switch_status);

		if (err)
			return err;

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY)) {
			printf("cdh:SD_SWITCH_CHECK ready\n");
			break;
		}
	}

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED)) {
		printf("cdh:SD_SWITCH_CHECK not support high-speed!\n");
		return 0;
	}
	
	/*
	 * If the host doesn't support SD_HIGHSPEED, do not switch card to
	 * HIGHSPEED mode even if the card support SD_HIGHSPPED.
	 * This can avoid furthur problem when the card runs in different
	 * mode between the host.
	 */
	if (!((mmc->host_caps & MMC_MODE_HS_52MHz) &&
		(mmc->host_caps & MMC_MODE_HS))) {
		printf("cdh:MMC Controller mmc->host_caps:0x%x, not support high-speed!\n", mmc->host_caps);
		return 0;
	}else {
		printf("cdh:MMC Controller mmc->host_caps:0x%x, yes support high-speed!\n", mmc->host_caps);
	}
	
	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)switch_status);
	if (err){
		printf("cdh:SD_SWITCH_SWITCH fail!\n");
		return err;
	}else {
		printf("cdh:SD_SWITCH_SWITCH OK!\n");
	}

	// cdh:modify switch_status[4] to switch_status[11]
	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000) {
		mmc->card_caps |= MMC_MODE_HS;
		printf("cdh:mmc->card_caps:0x%x, MODE HS OK!\n", mmc->card_caps);
	}else {
		printf("cdh:mmc->card_caps:0x%x, MODE HS fail!\n", mmc->card_caps);
	}
#endif

	return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
	0,	/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

static void mmc_set_ios(struct mmc *mmc)
{
	mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, uint clock)
{
	if (clock > mmc->f_max)
		clock = mmc->f_max;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	mmc->clock = clock;
	mmc->iosset_flag = 1; // cdh:add for clock set
	mmc_set_ios(mmc);
	mmc->iosset_flag = 0; // cdh:add for clean flag
}

static void mmc_set_bus_width(struct mmc *mmc, uint width)
{
	mmc->bus_width = width;
	mmc->iosset_flag = 2; // cdh:add for bus width set
	mmc_set_ios(mmc);
	mmc->iosset_flag = 0; // cdh:add for clean flag
}



/**
 * @brief Get the value according some bits
 *
 * Called when get information about cid and csd.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The  csd or cid buffer 
 * @param start[in] The start bit
 * @param resp[in] The number of bits
 * @return The value according the  bits
 */
static signed long stuff_bits (unsigned short *resp, signed long start, signed long size)
{
    unsigned short __size = size;
    unsigned short __mask = (__size < 16 ? 1 << __size : 0) - 1;
    signed long __off = ((start) / 16);
    signed long __shft = (start) & 15;
    unsigned short __res = 0;

    __res = resp [__off] >> __shft;
    if (__size + __shft > 16)
    {
            __res = resp [__off] >> __shft;
            __res |= resp[__off+1] << ((16 - __shft) % 16);
    }

    return (__res & __mask);
}

/*for csd cid.... register decoder*/
#define UNSTUFF_BITS(resp,start,size)   stuff_bits(resp,start,size)


/**
 * @brief set the sd interface clk
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param clock[in] clock to set
 * @return T_BOOL
 */
#if 0
T_BOOL sd_set_clock(struct mmc *mmc,T_U32 clock)
{
    T_BOOL ret = AK_TRUE;

    // cdh:set mclk default 400khz, bus mode 1 wire
	//mmc_set_bus_width(mmc, 1);
	//mmc_set_clock(mmc, 1);
	
    //for some emmc inand, should decrease card clock before mode switch
    //set_clock(1000000, get_asic_freq(), SD_POWER_SAVE_ENABLE);   //1Mhz
    mmc_set_clock(mmc, 1);
    
    //switch hs
    if (clock > SD_DEFAULT_MODE_25M && clock <= SD_HS_MODE_50M)
    {                
        if(!sd_switch_hs(SD_HIGH_SPEED_MODE))
        {
            //HS SD @50M
            akprintf(C1, M_DRVSYS, "sd switch hs fail\n");
            ret = AK_FALSE;
        }
    }
    else if (clock > SD_HS_MODE_50M)
    {
        akprintf(C1, M_DRVSYS, "HS SD clk exceed 50M\n");
        ret = AK_FALSE;
    }
    else
    {
        if(!sd_switch_hs(SD_DEFAULT_SPEED_MODE))
        {
            akprintf(C1, M_DRVSYS, "sd switch default speed fail\n");
            ret = AK_FALSE;
        }
    }
 
    if(ret)
    {
        //change sd clock
        g_pCurSdDevice->ulClock =clock;
    }    
    set_clock(g_pCurSdDevice->ulClock, get_asic_freq(), SD_POWER_SAVE_ENABLE);
    

    return ret;
}
#endif

static int mmc_startup(struct mmc *mmc)
{
	int err, i;
	uint mult;
	//uint freq;
	//u64 cmult, csize, capacity;
	uint cmult, csize, capacity;
	uint blocknr=0;
	struct mmc_cmd cmd;
	
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
	ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);
	int timeout = 1000;

#ifdef CONFIG_MMC_SPI_CRC_ON
	if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
		cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 1;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}
#endif

	/* Put the Card in Identify Mode , CDH:GET CID*/
	cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
		MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);  // cdh:check ok
	if (err) {
		printf("cdh:MMC_CMD_ALL_SEND_CID Err!\n");
		return err;
	}else {
		printf("cdh:MMC_CMD_ALL_SEND_CID OK!\n");
	}

	//printf("cdh:debug wait!\n");
	//while(1);
	
	memcpy(mmc->cid, cmd.response, 16);
	printf("cdh:mmc cid[0]=%d\n", mmc->cid[0]);
	printf("cdh:mmc cid[1]=%d\n", mmc->cid[1]);
	printf("cdh:mmc cid[2]=%d\n", mmc->cid[2]);
	printf("cdh:mmc cid[3]=%d\n", mmc->cid[3]);
	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.cmdarg = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R6;

		err = mmc_send_cmd(mmc, &cmd, NULL);  // cdh:check ok

		if (err)
			return err;

		if (IS_SD(mmc)){
			mmc->rca = (cmd.response[0] >> 16) & 0xffff;
			printf("cdh:sd mmc->rca=0x%x\n", mmc->rca);
		}
	}

	/* Get the Card-Specific Data */
	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	mmc_set_clock(mmc, 1000000);
	mmc_set_clock(mmc, 2000000);
	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	if (err) {
		printf("cdh:MMC_CMD_SEND_CSD Err!\n");
		return err;
	}else {
		printf("cdh:MMC_CMD_SEND_CSD OK!\n");
	}
		
	//printf("cdh:debug wait!\n");
	//while(1);
	
	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];
	printf("cmd csdlong response[0]:0x%x!\n", mmc->csd[0]);
	printf("cmd csdlong response[1]:0x%x!\n", mmc->csd[1]);
	printf("cmd csdlong response[2]:0x%x!\n", mmc->csd[2]);
	printf("cmd csdlong response[3]:0x%x!\n", mmc->csd[3]);

	if (mmc->version == MMC_VERSION_UNKNOWN) {
		printf("cdh:mmc->version == MMC_VERSION_UNKNOWN!\n");
		int version = (cmd.response[0] >> 26) & 0xf;

		switch (version) {
			case 0:
				mmc->version = MMC_VERSION_1_2;
				break;
			case 1:
				mmc->version = MMC_VERSION_1_4;
				break;
			case 2:
				mmc->version = MMC_VERSION_2_2;
				break;
			case 3:
				mmc->version = MMC_VERSION_3;
				break;
			case 4:
				mmc->version = MMC_VERSION_4;
				break;
			default:
				mmc->version = MMC_VERSION_1_2;
				break;
		}
	}

	/* divide frequency by 10, since the mults are 10x bigger */
	//freq = fbase[(cmd.response[0] & 0x7)];
	mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

	mmc->tran_speed = UNSTUFF_BITS((unsigned short *)mmc->csd, 96, 8); //freq * mult;
	printf("cdh:mmc->tran_speed:0x%x!\n", mmc->tran_speed);
	
	mmc->read_bl_len = 1 << UNSTUFF_BITS((unsigned short *)mmc->csd, 80, 4); // 1 << ((cmd.response[1] >> 16) & 0xf);

	if (IS_SD(mmc)){
		mmc->write_bl_len = mmc->read_bl_len;
		printf("cdh:sd card, mmc->write_bl_len:%d!\n", mmc->write_bl_len);
	}
	else
		mmc->write_bl_len = 1<< UNSTUFF_BITS((unsigned short *)mmc->csd, 22, 4); // 1 << ((cmd.response[3] >> 22) & 0xf);

	if (mmc->high_capacity) {
		printf("cdh:yes mmc->high_capacity!\n");
		#if 0
		csize = (mmc->csd[1] & 0x3f) << 16
			| (mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
		#else
		csize = ((mmc->csd[2]<<16) &0x3f0000)+(mmc->csd[1] >> 16);
		cmult = 8;
		#endif
		mmc->capacity_user = (u64)((csize + 1) << (cmult + 2));
	} else {
		printf("cdh:no mmc->high_capacity!\n");
		#if 0
		csize = (mmc->csd[1] & 0x3ff) << 2
			| (mmc->csd[2] & 0xc0000000) >> 30;
		cmult = (mmc->csd[2] & 0x00038000) >> 15;
		#else
		mult = 1 << (UNSTUFF_BITS((unsigned short *)mmc->csd, 47, 3)+2);
		csize = UNSTUFF_BITS((unsigned short *)mmc->csd, 62, 2) + (UNSTUFF_BITS((unsigned short *)mmc->csd, 64, 10) << 2);
        blocknr = ( csize + 1 ) * mult;
        //capacity is the num of 512bytes
        mmc->capacity_user = blocknr * (mmc->read_bl_len >> 9);
		#endif
	}

	//mmc->capacity_user = (csize + 1) << (cmult + 2);
	mmc->capacity_user *= mmc->read_bl_len;
	mmc->capacity_boot = 0;
	mmc->capacity_rpmb = 0;
	printf("cdh:sd card, mmc->capacity_user:0x%llx blocks!\n", mmc->capacity_user);
	
	for (i = 0; i < 4; i++)
		mmc->capacity_gp[i] = 0;

	if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

	if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
	printf("cdh:sd card, mmc->write_bl_len2:%d!\n", mmc->write_bl_len);
	
	/* Select the card, and put it into Transfer Mode */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = MMC_CMD_SELECT_CARD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = mmc->rca << 16;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			printf("cdh:MMC_CMD_SELECT_CARD Err!\n");
			return err;
		}else {
			printf("cdh:MMC_CMD_SELECT_CARD OK!\n");
		}
	}

	// cdh:add
	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = MMC_MAX_BLOCK_LEN;
	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		printf("cdh:MMC_CMD_SET_BLOCKLEN Err!\n");
		return err;
	}else {
		printf("cdh:MMC_CMD_SET_BLOCKLEN OK!\n");
	}


	/*
	 * For SD, its erase group is always one sector
	 */
	mmc->erase_grp_size = 1;
	mmc->part_config = MMCPART_NOAVAILABLE;
	if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
		printf("cdh:mmc_send_ext_csd!\n");
		/* check  ext_csd version and capacity */
		err = mmc_send_ext_csd(mmc, ext_csd);
		if (!err && (ext_csd[EXT_CSD_REV] >= 2)) {
			/*
			 * According to the JEDEC Standard, the value of
			 * ext_csd's capacity is valid if the value is more
			 * than 2GB
			 */
			capacity = ext_csd[EXT_CSD_SEC_CNT] << 0
					| ext_csd[EXT_CSD_SEC_CNT + 1] << 8
					| ext_csd[EXT_CSD_SEC_CNT + 2] << 16
					| ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
			capacity *= MMC_MAX_BLOCK_LEN;
			if ((capacity >> 20) > 2 * 1024)
				mmc->capacity_user = capacity;
		}

		switch (ext_csd[EXT_CSD_REV]) {
		case 1:
			mmc->version = MMC_VERSION_4_1;
			break;
		case 2:
			mmc->version = MMC_VERSION_4_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_4_3;
			break;
		case 5:
			mmc->version = MMC_VERSION_4_41;
			break;
		case 6:
			mmc->version = MMC_VERSION_4_5;
			break;
		}

		/*
		 * Check whether GROUP_DEF is set, if yes, read out
		 * group size from ext_csd directly, or calculate
		 * the group size from the csd value.
		 */
		if (ext_csd[EXT_CSD_ERASE_GROUP_DEF]) {
			mmc->erase_grp_size =
				ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
					MMC_MAX_BLOCK_LEN * 1024;
		} else {
			int erase_gsz, erase_gmul;
			erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
			erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
			mmc->erase_grp_size = (erase_gsz + 1)
				* (erase_gmul + 1);
		}

		/* store the partition info of emmc */
		if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
		    ext_csd[EXT_CSD_BOOT_MULT])
			mmc->part_config = ext_csd[EXT_CSD_PART_CONF];

		mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

		mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

		for (i = 0; i < 4; i++) {
			int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
			mmc->capacity_gp[i] = (ext_csd[idx + 2] << 16) +
				(ext_csd[idx + 1] << 8) + ext_csd[idx];
			mmc->capacity_gp[i] *=
				ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
			mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
		}
	}

	err = mmc_set_capacity(mmc, mmc->part_num);
	printf("cdh:mmc->capacity:0x%llx !\n", mmc->capacity);
	if (err) {
		printf("cdh:part_num:%d, mmc_set_capacity Err!\n", mmc->part_num);
		return err;
	} else {
		printf("cdh:part_num:%d, mmc_set_capacity OK!\n", mmc->part_num);
	}
	
	//printf("cdh:debug wait!\n");
	//while(1);
	
	// cdh:切换到高速
	if (IS_SD(mmc))
		err = sd_change_freq(mmc);
	else
		err = mmc_change_freq(mmc);

	if (err)
		return err;

	/* Restrict card's capabilities by what the host can do */
	mmc->card_caps &= mmc->host_caps;

	if (IS_SD(mmc)) {
		if (mmc->card_caps & MMC_MODE_4BIT) {
			cmd.cmdidx = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = mmc->rca << 16;

			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err)
				return err;

			cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = 2;
			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err)
				return err;
			printf("### set bus width ### %s:%d\n", __func__, __LINE__);
			mmc_set_bus_width(mmc, 4);  // cdh:切换到4线带宽
		} else {
			printf("### set bus width ### %s:%d\n", __func__, __LINE__);
			mmc_set_bus_width(mmc, 1);  // cdh:切换到1线带宽
		}

		if (mmc->card_caps & MMC_MODE_HS) {
			mmc->tran_speed = 50000000;
		}
		else {
			mmc->tran_speed = 25000000;
		}
		printf("cdh:mmc->tran_speed=%d\n", mmc->tran_speed);
	} else {
		int idx;

		/* An array of possible bus widths in order of preference */
		static unsigned ext_csd_bits[] = {
			EXT_CSD_BUS_WIDTH_8,
			EXT_CSD_BUS_WIDTH_4,
			EXT_CSD_BUS_WIDTH_1,
		};

		/* An array to map CSD bus widths to host cap bits */
		static unsigned ext_to_hostcaps[] = {
			[EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
			[EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
		};

		/* An array to map chosen bus width to an integer */
		static unsigned widths[] = {
			8, 4, 1,
		};

		for (idx=0; idx < ARRAY_SIZE(ext_csd_bits); idx++) {
			unsigned int extw = ext_csd_bits[idx];

			/*
			 * Check to make sure the controller supports
			 * this bus width, if it's more than 1
			 */
			if (extw != EXT_CSD_BUS_WIDTH_1 &&
					!(mmc->host_caps & ext_to_hostcaps[extw]))
				continue;

			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_BUS_WIDTH, extw);

			if (err)
				continue;

			mmc_set_bus_width(mmc, widths[idx]);

			err = mmc_send_ext_csd(mmc, test_csd);
			if (!err && ext_csd[EXT_CSD_PARTITIONING_SUPPORT] \
				    == test_csd[EXT_CSD_PARTITIONING_SUPPORT]
				 && ext_csd[EXT_CSD_ERASE_GROUP_DEF] \
				    == test_csd[EXT_CSD_ERASE_GROUP_DEF] \
				 && ext_csd[EXT_CSD_REV] \
				    == test_csd[EXT_CSD_REV]
				 && ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] \
				    == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
				 && memcmp(&ext_csd[EXT_CSD_SEC_CNT], \
					&test_csd[EXT_CSD_SEC_CNT], 4) == 0) {

				mmc->card_caps |= ext_to_hostcaps[extw];
				break;
			}
		}

		if (mmc->card_caps & MMC_MODE_HS) {
			if (mmc->card_caps & MMC_MODE_HS_52MHz)
				mmc->tran_speed = 52000000;
			else
				mmc->tran_speed = 26000000;
		}
	}

	/* because for all sd card commuciation stable, we force set max transpeed as 25Mhz */
#if 0
	mmc_set_clock(mmc, mmc->tran_speed); // cdh:调整控制器MCLK
#else
	mmc->tran_speed = 25000000;
	mmc_set_clock(mmc, mmc->tran_speed); // cdh:调整控制器MCLK
#endif

	/* fill in device description */
	mmc->block_dev.lun = 0;
	mmc->block_dev.type = 0;
	mmc->block_dev.blksz = mmc->read_bl_len;
	mmc->block_dev.log2blksz = LOG2(mmc->block_dev.blksz);
	mmc->block_dev.lba = lldiv(mmc->capacity, mmc->read_bl_len);
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
	sprintf(mmc->block_dev.vendor, "Man %06x Snr %04x%04x",
		mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
		(mmc->cid[3] >> 16) & 0xffff);
	sprintf(mmc->block_dev.product, "%c%c%c%c%c%c", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
		(mmc->cid[2] >> 24) & 0xff);
	sprintf(mmc->block_dev.revision, "%d.%d", (mmc->cid[2] >> 20) & 0xf,
		(mmc->cid[2] >> 16) & 0xf);
#else
	mmc->block_dev.vendor[0] = 0;
	mmc->block_dev.product[0] = 0;
	mmc->block_dev.revision[0] = 0;
#endif
	//printf("cdh:mmc->block_dev.blksz=0x%x\n", mmc->block_dev.blksz);
	//printf("cdh:mmc->block_dev.log2blksz=0x%x\n", mmc->block_dev.log2blksz);
	//printf("cdh:mmc->block_dev.lba=0x%x\n", mmc->block_dev.lba);
	//printf("cdh:mmc->block_dev.vendor=%s\n", mmc->block_dev.vendor);
	//printf("cdh:mmc->block_dev.product=%s\n", mmc->block_dev.product);
	//printf("cdh:mmc->block_dev.revision=%s\n", mmc->block_dev.revision);
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBDISK_SUPPORT)
	init_part(&mmc->block_dev);
#endif
	//printf("cdh:debug wait!\n");
	//while(1);
	return 0;
}

static int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;
	int  i;

	cmd.cmdidx = SD_CMD_SEND_IF_COND; // cdh:CMD8
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;
	for(i = 0; i < 3; i++){
		err = mmc_send_cmd(mmc, &cmd, NULL); // cdh:check ok
		if(!err)
			break;
		printf("%s, Err cnt:%d\n", __func__, i);
	}
 
	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		mmc->version = SD_VERSION_2;

	return 0;
}

int mmc_register(struct mmc *mmc)
{
	/* Setup the universal parts of the block interface just once */
	mmc->block_dev.if_type = IF_TYPE_MMC;
	mmc->block_dev.dev = cur_dev_num++;
	mmc->block_dev.removable = 1;
	mmc->block_dev.block_read = mmc_bread;
	mmc->block_dev.block_write = mmc_bwrite;
	mmc->block_dev.block_erase = mmc_berase;
	if (!mmc->b_max)
		mmc->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	INIT_LIST_HEAD (&mmc->link);

	list_add_tail (&mmc->link, &mmc_devices);

	return 0;
}

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *mmc_get_dev(int dev)
{
	struct mmc *mmc = find_mmc_device(dev);
	if (!mmc || mmc_init(mmc))
		return NULL;

	return &mmc->block_dev;
}
#endif

int mmc_start_init(struct mmc *mmc)
{
	int err;

	// cdh:if (mmc_getcd(mmc) == 0)
	if (mmc_getcd(mmc) > 0) {
		mmc->has_init = 0;
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		printf("MMC: no card present\n");
#endif
		return NO_CARD_ERR;
	}else {
		printf("MMC: detect card present\n");
	}

	if (mmc->has_init) {
		printf("MMC: has init done, return!\n");
		return 0;
	}else {
		printf("MMC: no init, start init!\n");
	}

	// cdh:initial mmc/sd sharepin, open mmc mode system clock and reset mmc controller
	err = mmc->init(mmc);
	if (err) {
		printf("MMC: ak mmc driver init Err!\n");
		return err;
	}else {
		printf("MMC: ak mmc driver init OK!\n");
	}

	// cdh:set mclk default 400khz, bus mode 1 wire
	mmc_set_clock(mmc, 1);
	//mmc_set_bus_width(mmc, 1);
	
	printf("MMC: default init bus width=1, clock=400khz, OK!\n");
	mmc->bus_width = 1;
	
	/* cdh: CMD0 Reset the Card */
	err = mmc_go_idle(mmc);
	if (err) {
		printf("MMC: ak mmc drmmc_go_idle Err!\n");
		return err;
	}else {
		printf("MMC: ak mmc drmmc_go_idle OK!\n");
	}

	/* The internal partition reset to user partition(0) at every CMD0*/
	mmc->part_num = 0;

	/* cdh:must ver2 , to be Test for SD version 2 */
	err = mmc_send_if_cond(mmc);
	if (err) {
		printf("MMC: ak mmc mmc_send_if_cond Err!\n");
		return err;
	}else {
		printf("MMC: ak mmc mmc_send_if_cond OK!\n");
	}

	//printf("cdh:debug wait!\n");
	//while(1);
	
	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond(mmc);
	if (err) {
		printf("MMC: ak mmc sd_send_op_cond Err!\n");
		return err;
	}else {
		printf("MMC: ak mmc sd_send_op_cond OK!\n");
	}

	//printf("cdh:debug wait!\n");
	//while(1);
	
	/* If the command timed out, we check for an MMC card */
	if (err == TIMEOUT) {
		err = mmc_send_op_cond(mmc);

		if (err && err != IN_PROGRESS) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			printf("Card did not respond to voltage select!\n");
#endif
			return UNUSABLE_ERR;
		}
	}

	if (err == IN_PROGRESS)
		mmc->init_in_progress = 1;
	printf("%s: init OK!\n", __func__);
	
	return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
	int err = 0;

	if (mmc->op_cond_pending)
		err = mmc_complete_op_cond(mmc);

	if (!err)
		err = mmc_startup(mmc);
	if (err)
		mmc->has_init = 0;
	else
		mmc->has_init = 1;  // cdh:初始化完成
	mmc->init_in_progress = 0;
	return err;
}

int mmc_init(struct mmc *mmc)
{
	int err = IN_PROGRESS;
	unsigned start = get_timer(0);

	if (mmc->has_init) {
		debug("%s: mmc->has_init:yes, return\n", __func__);
		return 0;
	}else {
		debug("%s: mmc->has_init:no, start\n", __func__);
	}
	
	if (!mmc->init_in_progress) {
		debug("%s: mmc->init_in_progress=0, start init\n", __func__);
		err = mmc_start_init(mmc);
	} else {
		debug("%s: mmc->init_in_progress=1\n", __func__);
	}
	
	if (!err || err == IN_PROGRESS) {
		debug("%s: err<=0.start complete init\n", __func__);
		err = mmc_complete_init(mmc);
	} else {
		debug("%s: err=1\n", __func__);
	}

	//printf("cdh:debug wait!\n");
	//while(1);
	
	debug("%s: %d, time %lu\n", __func__, err, get_timer(start));
	return err;
}

/*
 * CPU and board-specific MMC initializations.  Aliased function
 * signals caller to move on
 */
static int __def_mmc_init(bd_t *bis)
{
	return -1;
}

int cpu_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));
int board_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)

void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		printf("%s: %d", m->name, m->block_dev.dev);

		if (entry->next != &mmc_devices)
			printf("%c ", separator);
	}

	printf("\n");
}

#else
void print_mmc_devices(char separator) { }
#endif

int get_mmc_num(void)
{
	return cur_dev_num;
}

void mmc_set_preinit(struct mmc *mmc, int preinit)
{
	mmc->preinit = preinit;
}

static void do_preinit(void)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->preinit)
			mmc_start_init(m);
	}
}


int mmc_initialize(bd_t *bis)
{
	INIT_LIST_HEAD (&mmc_devices);
	cur_dev_num = 0;

// cdh:add modify
#if 0
	if (board_mmc_init(bis) < 0)
		cpu_mmc_init(bis);
#else
	if (ak_sdhsmmc_init(0, 0, 0, 41, -1) < 0) {
		//printf("%s: ak_sdhsmmc_init Err!\n", __func__);
		return -1;
	}else {
		//printf("%s: ak_sdhsmmc_init OK!\n", __func__);
	}
#endif

#ifndef CONFIG_SPL_BUILD
	print_mmc_devices(',');
#endif

	do_preinit();
	return 0;
}

#ifdef CONFIG_SUPPORT_EMMC_BOOT
/*
 * This function changes the size of boot partition and the size of rpmb
 * partition present on EMMC devices.
 *
 * Input Parameters:
 * struct *mmc: pointer for the mmc device strcuture
 * bootsize: size of boot partition
 * rpmbsize: size of rpmb partition
 *
 * Returns 0 on success.
 */

int mmc_boot_partition_size_change(struct mmc *mmc, unsigned long bootsize,
				unsigned long rpmbsize)
{
	int err;
	struct mmc_cmd cmd;

	/* Only use this command for raw EMMC moviNAND. Enter backdoor mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error1 = %d\n", err);
		return err;
	}

	/* Boot partition changing mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG2;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error2 = %d\n", err);
		return err;
	}
	/* boot partition size is multiple of 128KB */
	bootsize = (bootsize * 1024) / 128;

	/* Arg: boot partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = bootsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error3 = %d\n", err);
		return err;
	}
	/* RPMB partition size is multiple of 128KB */
	rpmbsize = (rpmbsize * 1024) / 128;
	/* Arg: RPMB partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = rpmbsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		debug("mmc_boot_partition_size_change: Error4 = %d\n", err);
		return err;
	}
	return 0;
}

/*
 * This function shall form and send the commands to open / close the
 * boot partition specified by user.
 *
 * Input Parameters:
 * ack: 0x0 - No boot acknowledge sent (default)
 *	0x1 - Boot acknowledge sent during boot operation
 * part_num: User selects boot data that will be sent to master
 *	0x0 - Device not boot enabled (default)
 *	0x1 - Boot partition 1 enabled for boot
 *	0x2 - Boot partition 2 enabled for boot
 * access: User selects partitions to access
 *	0x0 : No access to boot partition (default)
 *	0x1 : R/W boot partition 1
 *	0x2 : R/W boot partition 2
 *	0x3 : R/W Replay Protected Memory Block (RPMB)
 *
 * Returns 0 on success.
 */
int mmc_boot_part_access(struct mmc *mmc, u8 ack, u8 part_num, u8 access)
{
	int err;
	struct mmc_cmd cmd;

	/* Boot ack enable, boot partition enable , boot partition access */
	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;

	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
			(EXT_CSD_PART_CONF << 16) |
			((EXT_CSD_BOOT_ACK(ack) |
			EXT_CSD_BOOT_PART_NUM(part_num) |
			EXT_CSD_PARTITION_ACCESS(access)) << 8);

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		if (access) {
			debug("mmc boot partition#%d open fail:Error1 = %d\n",
			      part_num, err);
		} else {
			debug("mmc boot partition#%d close fail:Error = %d\n",
			      part_num, err);
		}
		return err;
	}

	if (access) {
		/* 4bit transfer mode at booting time. */
		cmd.cmdidx = MMC_CMD_SWITCH;
		cmd.resp_type = MMC_RSP_R1b;

		cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				(EXT_CSD_BOOT_BUS_WIDTH << 16) |
				((1 << 0) << 8);

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err) {
			debug("mmc boot partition#%d open fail:Error2 = %d\n",
			      part_num, err);
			return err;
		}
	}
	return 0;
}
#endif
