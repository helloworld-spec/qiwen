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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/firmware.h>
#include <ssv6200.h>

#include <smac/dev.h>
#include <hal.h>
#include "hctrl.h"


MODULE_AUTHOR("iComm Semiconductor Co., Ltd");
MODULE_DESCRIPTION("HCI driver for SSV6xxx 802.11n wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("SSV6xxx WLAN cards");
MODULE_LICENSE("Dual BSD/GPL");


// Freddie ToDo: Make ctrl_hci with device instance instead of a global variable
//static struct ssv6xxx_hci_ctrl *__ctrl_hci = NULL;

static int ssv6xxx_hci_usb_tx_handler(struct ssv6xxx_hci_ctrl *ctrl_hci, void *dev, int max_count, int *err);

static int ssv6xxx_hci_irq_enable(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    /* enable interrupt */
	HCI_IRQ_SET_MASK(ctrl_hci, ~(ctrl_hci->int_mask));
	HCI_IRQ_ENABLE(ctrl_hci);
    return 0;
}



static int ssv6xxx_hci_irq_disable(struct ssv6xxx_hci_ctrl *ctrl_hci)
{

    /* disable interrupt */
	HCI_IRQ_SET_MASK(ctrl_hci, 0xffffffff);
	HCI_IRQ_DISABLE(ctrl_hci);
    return 0;
}

static void ssv6xxx_hci_irq_register(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 irq_mask)
{
    unsigned long flags;
    u32 regval;
    
    mutex_lock(&ctrl_hci->hci_mutex);

    spin_lock_irqsave(&ctrl_hci->int_lock, flags);  
    ctrl_hci->int_mask |= irq_mask;
    regval = ~ctrl_hci->int_mask;
    spin_unlock_irqrestore(&ctrl_hci->int_lock, flags);
    smp_mb();

    HCI_IRQ_SET_MASK(ctrl_hci, regval);
    mutex_unlock(&ctrl_hci->hci_mutex);
}

static inline u32 ssv6xxx_hci_get_int_bitno(struct ssv6xxx_hci_ctrl *ctrl_hci, int txqid)
{
    /*Workaround solution: We use this interrupt bit(bit 1) for Queue 4(MNG) 
        other interrupt bit you can reference */
    //#define SSV6XXX_INT_TX              0x00000002  //1<<1
    
    if(txqid ==  SSV_HW_TXQ_NUM-1)
        return 1;
    else
        return txqid+3;            
}

#ifndef SSV_SUPPORT_HAL
void ssv6xxx_hci_hci_inq_info(struct ssv6xxx_hci_ctrl *ctrl_hci, int *used_id)
{
    return;
}

void ssv6xxx_hci_load_fw_enable_mcu(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	if (HCI_REG_WRITE(ctrl_hci, ADR_BRG_SW_RST, 0x1));
}

int ssv6xxx_hci_load_fw_disable_mcu(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	u32 clk_en;
	
	if (HCI_REG_WRITE(ctrl_hci, ADR_BRG_SW_RST, 0x0));
	if (HCI_REG_WRITE(ctrl_hci, ADR_BOOT, 0x0));
	if (HCI_REG_READ(ctrl_hci, ADR_PLATFORM_CLOCK_ENABLE, &clk_en));
	if (HCI_REG_WRITE(ctrl_hci, ADR_PLATFORM_CLOCK_ENABLE, (clk_en | (1 << 2))));

    return 0;
}

int ssv6xxx_hci_load_fw_set_status(struct ssv6xxx_hci_ctrl *ctrl_hci, int status)
{
	return HCI_REG_WRITE(ctrl_hci, ADR_TX_SEG, status);
}

int ssv6xxx_hci_load_fw_get_status(struct ssv6xxx_hci_ctrl *ctrl_hci, int *status)
{
	return HCI_REG_READ(ctrl_hci, ADR_TX_SEG, status);
}

int ssv6xxx_hci_reset_cpu(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	u32 reset;

	if (HCI_REG_READ(ctrl_hci, ADR_PLATFORM_CLOCK_ENABLE, &reset));
	if (HCI_REG_WRITE(ctrl_hci, ADR_PLATFORM_CLOCK_ENABLE, reset & ~(1 << 24)));

    return 0;
}

void ssv6xxx_hci_load_fw_pre_config_device(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	HCI_LOAD_FW_PRE_CONFIG_DEVICE(ctrl_hci);
}

void ssv6xxx_hci_load_fw_post_config_device(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	HCI_LOAD_FW_POST_CONFIG_DEVICE(ctrl_hci);
}
#endif

static void *ssv6xxx_hci_open_firmware(char *user_mainfw)
{
    struct file *fp;

    fp = filp_open(user_mainfw, O_RDONLY, 0);
    if (IS_ERR(fp)) 
        fp = NULL;
      
	return fp;
}

static int ssv6xxx_hci_read_fw_block(char *buf, int len, void *image)
{
	struct file *fp = (struct file *)image;
	int rdlen;

	if (!image)
		return 0;

	rdlen = kernel_read(fp, fp->f_pos, buf, len);
	if (rdlen > 0)
		fp->f_pos += rdlen;

	return rdlen;
}

static void ssv6xxx_hci_close_firmware(void *image)
{
	if (image)
		filp_close((struct file *)image, NULL);
}

static int ssv6xxx_hci_load_firmware_openfile(struct ssv6xxx_hci_ctrl *hci_ctrl, u8 *firmware_name)
{
    int ret = 0;
    u8   *fw_buffer = NULL;
    u32   sram_addr = FW_START_SRAM_ADDR;
    u32   block_count = 0;
    u32   res_size=0, len=0, tolen=0;
    void *fw_fp = NULL;
    u8    interface = HCI_DEVICE_TYPE(hci_ctrl);
#ifdef ENABLE_FW_SELF_CHECK
    u32   checksum = FW_CHECKSUM_INIT;
    u32   fw_checksum, fw_clkcnt;
    u32   retry_count = 3;
    u32  *fw_data32;
#else
    int   writesize = 0;
    u32   retry_count = 1;
#endif
    u32   word_count, i;

    if (hci_ctrl->redownload == 1) {
        // Redownload firmware    
        HCI_DBG_PRINT(hci_ctrl, "Re-download FW\n");
        HCI_JUMP_TO_ROM(hci_ctrl);            
    }

    // Load firmware
    fw_fp = ssv6xxx_hci_open_firmware(firmware_name);
    if (!fw_fp) {
        HCI_DBG_PRINT(hci_ctrl, "failed to find firmware (%s)\n", firmware_name);
        ret = -1;
        goto out;
    }

   	// Allocate buffer firmware aligned with FW_BLOCK_SIZE and padding with 0xA5 in empty space.
   	fw_buffer = (u8 *)kzalloc(FW_BLOCK_SIZE, GFP_KERNEL);
   	if (fw_buffer == NULL) {
        HCI_DBG_PRINT(hci_ctrl, "Failed to allocate buffer for firmware.\n");
        goto out;
    }

    do {
		// Disable MCU (USB ROM code must be alive for downloading FW, so USB doesn't do it)
		if (!(interface == SSV_HWIF_INTERFACE_USB)) {
		    ret = SSV_LOAD_FW_DISABLE_MCU(hci_ctrl);
           if (ret == -1)
               goto out;
       } 
            
		// Write firmware to SRAM address 0
        HCI_DBG_PRINT(hci_ctrl, "Writing firmware to SSV6XXX...\n");
        memset(fw_buffer, 0xA5, FW_BLOCK_SIZE);            
        while ((len = ssv6xxx_hci_read_fw_block((char*)fw_buffer, FW_BLOCK_SIZE, fw_fp))) {
        	tolen += len;
            //printk("read len=%d,sram_addr=%d\n",len,sram_addr);
            if (len < FW_BLOCK_SIZE) {
            	res_size = len;
                break;
            }
			if ((ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer, FW_BLOCK_SIZE)) != 0)
              	break;
            
			sram_addr += FW_BLOCK_SIZE;
            word_count = (len / sizeof(u32));
            fw_data32 = (u32 *)fw_buffer;
            for (i = 0; i < word_count; i++) {
            	checksum += fw_data32[i];
            }
            //printk("\nper blk cks=%x\n",checksum);                
            memset(fw_buffer, 0xA5, FW_BLOCK_SIZE);          
    	}
            
        if(res_size)
        {
        	u32 cks_blk_cnt,cks_blk_res;
            cks_blk_cnt = res_size / CHECKSUM_BLOCK_SIZE;                    
            cks_blk_res = res_size % CHECKSUM_BLOCK_SIZE;
            //printk("res_size = %d,cks_blk_cnt=%d\n",res_size,cks_blk_cnt);
            ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer, (cks_blk_cnt+1)*CHECKSUM_BLOCK_SIZE);
			word_count = (cks_blk_cnt * CHECKSUM_BLOCK_SIZE / sizeof(u32));
            fw_data32 = (u32 *)fw_buffer;
            for (i = 0; i < word_count; i++)
            	checksum += *fw_data32++;
            
			//printk("last cks=%x,tolen=%d,res_size=%d\n",checksum,tolen,res_size);
            if(cks_blk_res) {
            	word_count = (CHECKSUM_BLOCK_SIZE / sizeof(u32));
                //fw_data32 = (u32 *)&fw_buffer[cks_blk_cnt * CHECKSUM_BLOCK_SIZE];
                for (i = 0; i < word_count; i++) {
                	checksum += *fw_data32++;
                }
            }
            //printk("over cks=%x\n",checksum);
       	}
            
        // Calculate the final checksum.
        checksum = ((checksum >> 24) + (checksum >> 16) + (checksum >> 8) + checksum) & 0x0FF;
        checksum <<= 16;

        if (ret == 0) {
            // Reset CPU for USB switching ROM to firmware
            if (interface == SSV_HWIF_INTERFACE_USB) {
                ret = SSV_RESET_CPU(hci_ctrl);
                if (ret == -1)
                    goto out;
            }

            SSV_SET_SRAM_MODE(hci_ctrl, SRAM_MODE_ILM_160K_DLM_32K);
            
			block_count = tolen / CHECKSUM_BLOCK_SIZE;
            res_size = tolen % CHECKSUM_BLOCK_SIZE;
            if(res_size)
            	block_count++;
            // Inform FW that how many blocks is downloaded such that FW can calculate the checksum.
            SSV_LOAD_FW_SET_STATUS(hci_ctrl, (block_count << 16));
			SSV_LOAD_FW_GET_STATUS(hci_ctrl, &fw_clkcnt);
			HCI_DBG_PRINT(hci_ctrl,	"(block_count << 16) = %x,reg =%x\n", (block_count << 16),fw_clkcnt);
            // Release reset to let CPU run.
			SSV_LOAD_FW_ENABLE_MCU(hci_ctrl);
			HCI_DBG_PRINT(hci_ctrl,	"Firmware \"%s\" loaded\n", firmware_name);
            // Wait FW to calculate checksum.
            msleep(50);
            // Check checksum result and set to complement value if checksum is OK.
            SSV_LOAD_FW_GET_STATUS(hci_ctrl, &fw_checksum);
            fw_checksum = fw_checksum & FW_STATUS_MASK;
            if (fw_checksum == checksum) {
            	SSV_LOAD_FW_SET_STATUS(hci_ctrl, (~checksum & FW_STATUS_MASK));
                ret = 0;
				HCI_DBG_PRINT(hci_ctrl,	"Firmware check OK.%04x = %04x\n", fw_checksum, checksum);
                break;
           	} else {
				HCI_DBG_PRINT(hci_ctrl, "FW checksum error: %04x != %04x\n", fw_checksum, checksum);
                ret = -1;
           	}

      	} else {
			HCI_DBG_PRINT(hci_ctrl, "Firmware \"%s\" download failed. (%d)\n", firmware_name, ret);
            ret = -1;
        }
  	} while (--retry_count);

    if (ret)
    	goto out;

    hci_ctrl->redownload = 1;
    
    ret = 0;

out:
    if(fw_fp)
        ssv6xxx_hci_close_firmware(fw_fp);
    
    if (fw_buffer != NULL)
        kfree(fw_buffer);

    return ret;
}

static int ssv6xxx_hci_get_firmware(struct device *dev, char *user_mainfw, const struct firmware **mainfw)
{
    int ret;

    //hBUG_ON(helper == NULL);
    BUG_ON(mainfw == NULL);

    /* Try user-specified firmware first */
    if (*user_mainfw) {
        ret = request_firmware(mainfw, user_mainfw, dev);
        if (ret) {
            goto fail;
        }
        if (*mainfw)
            return 0;
    }

fail:
    /* Failed */
    if (*mainfw) {
        release_firmware(*mainfw);
        *mainfw = NULL;
    }

    return -ENOENT;
}

static int ssv6xxx_hci_load_firmware_request(struct ssv6xxx_hci_ctrl *hci_ctrl, u8 *firmware_name)
{
    int ret = 0;
    const struct firmware *ssv6xxx_fw = NULL;
    u8   *fw_buffer = NULL;
    u32   sram_addr = FW_START_SRAM_ADDR;
    u32   block_count = 0;
    u32   block_idx = 0;
    u32   res_size;
    u8   *fw_data;
    u8    interface = HCI_DEVICE_TYPE(hci_ctrl);
#ifdef ENABLE_FW_SELF_CHECK
    u32   checksum = FW_CHECKSUM_INIT;
    u32   fw_checksum;
    u32   retry_count = 3;
    u32  *fw_data32;
#else
    int   writesize = 0;
    u32   retry_count = 1;
#endif

    if (hci_ctrl->redownload == 1) {
        // Redownload firmware    
        HCI_DBG_PRINT(hci_ctrl, "Re-download FW\n");
        HCI_JUMP_TO_ROM(hci_ctrl);    
    }

    // Load firmware
    ret = ssv6xxx_hci_get_firmware(hci_ctrl->shi->dev, firmware_name, &ssv6xxx_fw);
    if (ret) {
		HCI_DBG_PRINT(hci_ctrl, "failed to find firmware (%d)\n", ret);
        goto out;
    }

    // Allocate buffer firmware aligned with FW_BLOCK_SIZE and padding with 0xA5 in empty space.
    fw_buffer = (u8 *)kzalloc(FW_BLOCK_SIZE, GFP_KERNEL);
    if (fw_buffer == NULL) {
		HCI_DBG_PRINT(hci_ctrl, "Failed to allocate buffer for firmware.\n");
        goto out;
    }

#ifdef ENABLE_FW_SELF_CHECK
    block_count = ssv6xxx_fw->size / CHECKSUM_BLOCK_SIZE;
    res_size = ssv6xxx_fw->size % CHECKSUM_BLOCK_SIZE;
    {
    	int word_count = (int)(block_count * CHECKSUM_BLOCK_SIZE / sizeof(u32));
        int i;
        fw_data32 = (u32 *)ssv6xxx_fw->data;
        for (i = 0; i < word_count; i++)
        	checksum += fw_data32[i];

        if (res_size) {
        	memset(fw_buffer, 0xA5, CHECKSUM_BLOCK_SIZE);
            memcpy(fw_buffer, &ssv6xxx_fw->data[block_count * CHECKSUM_BLOCK_SIZE], res_size);

            // Accumulate checksum for the incomplete block
            word_count = (int)(CHECKSUM_BLOCK_SIZE / sizeof(u32));
            fw_data32 = (u32 *)fw_buffer;
            for (i = 0; i < word_count; i++) {
            	checksum += fw_data32[i];
            }
        }
	}

    // Calculate the final checksum.
    checksum = ((checksum >> 24) + (checksum >> 16) + (checksum >> 8) + checksum) & 0x0FF;
    checksum <<= 16;
#endif // ENABLE_FW_SELF_CHECK

    do {                
        // Disable MCU (USB ROM code must be alive for downloading FW, so USB doesn't do it)        
        if (!(interface == SSV_HWIF_INTERFACE_USB)) {
            ret = SSV_LOAD_FW_DISABLE_MCU(hci_ctrl);
            if (ret == -1)
                goto out;
        }

        // Write firmware to SRAM address 0
#ifdef ENABLE_FW_SELF_CHECK
        block_count = ssv6xxx_fw->size / FW_BLOCK_SIZE;
        res_size = ssv6xxx_fw->size % FW_BLOCK_SIZE;

		HCI_DBG_PRINT(hci_ctrl, "Writing %d blocks to SSV6XXX...", block_count);
        for (block_idx = 0, fw_data = (u8 *)ssv6xxx_fw->data, sram_addr = 0;block_idx < block_count;
        		block_idx++, fw_data += FW_BLOCK_SIZE, sram_addr += FW_BLOCK_SIZE) {
        	memcpy(fw_buffer, fw_data, FW_BLOCK_SIZE);
            if ((ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer, FW_BLOCK_SIZE)) != 0)
            	break;
       	}
        
		if(res_size) {
			memset(fw_buffer, 0xA5, FW_BLOCK_SIZE);
            memcpy(fw_buffer, &ssv6xxx_fw->data[block_count * FW_BLOCK_SIZE], res_size);
            if ((ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer,
					((res_size/CHECKSUM_BLOCK_SIZE)+1)*CHECKSUM_BLOCK_SIZE)) != 0)
				break;
        }
#else // ENABLE_FW_SELF_CHECK
        block_count = ssv6xxx_fw->size / FW_BLOCK_SIZE;
        res_size = ssv6xxx_fw->size % FW_BLOCK_SIZE;
        writesize = sdio_align_size(func,res_size);

		HCI_DBG_PRINT(hci_ctrl, "Writing %d blocks to SSV6XXX...", block_count);
        for (block_idx = 0, fw_data = (u8 *)ssv6xxx_fw->data, sram_addr = 0;block_idx < block_count;
        		block_idx++, fw_data += FW_BLOCK_SIZE, sram_addr += FW_BLOCK_SIZE) {
            memcpy(fw_buffer, fw_data, FW_BLOCK_SIZE);
            if ((ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer, FW_BLOCK_SIZE)) != 0)
            	break;
        }
        if(res_size) {
        	memcpy(fw_buffer, &ssv6xxx_fw->data[block_count * FW_BLOCK_SIZE], res_size);
            if ((ret = HCI_LOAD_FW(hci_ctrl, sram_addr, (u8 *)fw_buffer, writesize)) != 0)
            	break;
        }
#endif // ENABLE_FW_SELF_CHECK

        if (ret == 0) {
            // Reset CPU for USB switching ROM to firmware
            if (interface == SSV_HWIF_INTERFACE_USB) {
                ret = SSV_RESET_CPU(hci_ctrl);
                if (ret == -1)
                    goto out;
            }

            SSV_SET_SRAM_MODE(hci_ctrl, SRAM_MODE_ILM_160K_DLM_32K);
            
#ifdef ENABLE_FW_SELF_CHECK
        	block_count = ssv6xxx_fw->size / CHECKSUM_BLOCK_SIZE;
            res_size = ssv6xxx_fw->size % CHECKSUM_BLOCK_SIZE;
            if(res_size)
            	block_count++;
            // Inform FW that how many blocks is downloaded such that FW can calculate the checksum.
            SSV_LOAD_FW_SET_STATUS(hci_ctrl, (block_count << 16));
#endif // ENABLE_FW_SELF_CHECK

            // Release reset to let CPU run.
			SSV_LOAD_FW_ENABLE_MCU(hci_ctrl);
			HCI_DBG_PRINT(hci_ctrl, "Firmware \"%s\" loaded\n", firmware_name);
#ifdef ENABLE_FW_SELF_CHECK
            // Wait FW to calculate checksum.
            msleep(50);
            // Check checksum result and set to complement value if checksum is OK.
            SSV_LOAD_FW_GET_STATUS(hci_ctrl, &fw_checksum);
            fw_checksum = fw_checksum & FW_STATUS_MASK;
            if (fw_checksum == checksum) {
				SSV_LOAD_FW_SET_STATUS(hci_ctrl, (~checksum & FW_STATUS_MASK));
                ret = 0;
				HCI_DBG_PRINT(hci_ctrl, "Firmware check OK.\n");
                break;
            } else {
				HCI_DBG_PRINT(hci_ctrl,	"FW checksum error: %04x != %04x\n", fw_checksum, checksum);
                ret = -1;
            }
#endif
     	} else {
			HCI_DBG_PRINT(hci_ctrl, "Firmware \"%s\" download failed. (%d)\n", firmware_name, ret);
            ret = -1;
       	}
	} while (--retry_count);

    if (ret)
    	goto out;
    
    hci_ctrl->redownload = 1;
    
    ret = 0;    

out:
    if (ssv6xxx_fw)
        release_firmware(ssv6xxx_fw);

    if (fw_buffer != NULL)
        kfree(fw_buffer);

    return ret;
}

// Due to USB Rx task always keeps running in background
// It must start/stop USB acc of Rx endpoint(Ep4) while HCI start/stop
// Don't stop Ep 1,2 here for accessing register and don't stop Ep 3 here for flush buffered TX data in HCI.
static int ssv6xxx_hci_start_acc(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    HCI_START_USB_ACC(ctrl_hci, 4);
    return 0;
}

static int ssv6xxx_hci_stop_acc(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    HCI_STOP_USB_ACC(ctrl_hci, 4);    
    return 0;
}

static int ssv6xxx_hci_start(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    ssv6xxx_hci_irq_enable(ctrl_hci);
    ssv6xxx_hci_start_acc(ctrl_hci);
    ctrl_hci->hci_start = true;
    HCI_IRQ_TRIGGER(ctrl_hci);
    return 0;
}

static int ssv6xxx_hci_stop(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    ssv6xxx_hci_irq_disable(ctrl_hci);
    ssv6xxx_hci_stop_acc(ctrl_hci);
    ctrl_hci->hci_start = false;
    return 0;
}

static void ssv6xxx_hci_write_hw_config(struct ssv6xxx_hci_ctrl *ctrl_hci, int val)
{
    ctrl_hci->write_hw_config = val;
}

static int ssv6xxx_hci_read_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u32 *regval)
{
    int ret = HCI_REG_READ(ctrl_hci, addr, regval);
    return ret;
}

static int ssv6xxx_hci_safe_read_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u32 *regval)
{
    int ret = HCI_REG_SAFE_READ(ctrl_hci, addr, regval);
    return ret;
}

static int ssv6xxx_hci_write_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u32 regval)
{
    if (ctrl_hci->write_hw_config && (ctrl_hci->shi->write_hw_config_cb != NULL))
	    ctrl_hci->shi->write_hw_config_cb((void *)SSV_SC(ctrl_hci), addr, regval);
    
    return HCI_REG_WRITE(ctrl_hci, addr, regval);
}

static int ssv6xxx_hci_safe_write_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u32 regval)
{
    if (ctrl_hci->write_hw_config && (ctrl_hci->shi->write_hw_config_cb != NULL))
	    ctrl_hci->shi->write_hw_config_cb((void *)SSV_SC(ctrl_hci), addr, regval);
    
    return HCI_REG_SAFE_WRITE(ctrl_hci, addr, regval);
}

static int ssv6xxx_hci_burst_read_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 *addr, u32 *regval, u8 reg_amount)
{
    int ret = HCI_BURST_REG_READ(ctrl_hci, addr, regval, reg_amount);
    return ret;
}

static int ssv6xxx_hci_burst_write_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 *addr, u32 *regval, u8 reg_amount)
{
    return HCI_BURST_REG_WRITE(ctrl_hci, addr, regval, reg_amount);
}

static int ssv6xxx_hci_burst_safe_read_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 *addr, u32 *regval, u8 reg_amount)
{
    int ret = HCI_BURST_REG_SAFE_READ(ctrl_hci, addr, regval, reg_amount);
    return ret;
}

static int ssv6xxx_hci_burst_safe_write_word(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 *addr, u32 *regval, u8 reg_amount)
{
    return HCI_BURST_REG_SAFE_WRITE(ctrl_hci, addr, regval, reg_amount);
}

static int ssv6xxx_hci_load_fw(struct ssv6xxx_hci_ctrl *hci_ctrl, u8 *firmware_name, u8 openfile)
{
	int ret = 0;

    SSV_LOAD_FW_PRE_CONFIG_DEVICE(hci_ctrl);
		
	if (openfile)
		ret = ssv6xxx_hci_load_firmware_openfile(hci_ctrl, firmware_name);
	else
		ret = ssv6xxx_hci_load_firmware_request(hci_ctrl, firmware_name);
		
    // Sleep to let SSV6XXX get ready.
    msleep(50);

	if (ret == 0) 
    	SSV_LOAD_FW_POST_CONFIG_DEVICE(hci_ctrl);

	return ret;
}

static int ssv6xxx_hci_write_sram(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u8 *data, u32 size)
{
    return HCI_SRAM_WRITE(ctrl_hci, addr, data, size);
}

static int ssv6xxx_hci_pmu_wakeup(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    HCI_PMU_WAKEUP(ctrl_hci);
    return 0;
}

static int ssv6xxx_hci_interface_reset(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
	HCI_IFC_RESET(ctrl_hci);
	return 0;
}

static int ssv6xxx_hci_sysplf_reset(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 addr, u32 value)
{
	HCI_SYSPLF_RESET(ctrl_hci, addr, value);
	return 0;
}

static int ssv6xxx_hci_send_cmd(struct ssv6xxx_hci_ctrl *ctrl_hci, struct sk_buff *skb)
{
    int ret;
    ret = IF_SEND(ctrl_hci, (void *)skb, skb->len, 0);
    if (ret < 0) {
        HCI_DBG_PRINT(ctrl_hci, "ssv6xxx_hci_send_cmd fail......\n");
    }
    return ret;
}

static int ssv6xxx_hci_enqueue(struct ssv6xxx_hci_ctrl *ctrl_hci, struct sk_buff *skb, int txqid, u32 tx_flags)
{
    struct ssv_hw_txq *hw_txq;
    unsigned long flags;
    u32 status;
    int qlen = 0;
	
    BUG_ON(txqid >= SSV_HW_TXQ_NUM || txqid < 0);
    if (txqid >= SSV_HW_TXQ_NUM || txqid < 0)
        return -1;

    hw_txq = &ctrl_hci->hw_txq[txqid];

    hw_txq->tx_flags = tx_flags;

    if (tx_flags & HCI_FLAGS_ENQUEUE_HEAD)
        skb_queue_head(&hw_txq->qhead, skb);
    else
        skb_queue_tail(&hw_txq->qhead, skb);

    qlen = (int)skb_queue_len(&hw_txq->qhead);
	
    //Flow control check
    //spin_lock_irqsave(&hw_txq->txq_lock, flags);
    if (!(tx_flags & HCI_FLAGS_NO_FLOWCTRL)) {
        if (skb_queue_len(&hw_txq->qhead) >= hw_txq->max_qsize) {
            /* start tx flow control */
            ctrl_hci->shi->hci_tx_flow_ctrl_cb(
                (void *)SSV_SC(ctrl_hci),
                hw_txq->txq_no,
                true,2000
            );
        }
    }
    //spin_unlock_irqrestore(&hw_txq->txq_lock, flags);

	if (ctrl_hci->isr_disable == true) {
		/* if polling, wake up thread to send frame */
		wake_up_interruptible(&ctrl_hci->tx_wait_q);
	} else { 

	#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
    	mutex_lock(&ctrl_hci->hci_mutex);
	#endif
    	spin_lock_irqsave(&ctrl_hci->int_lock, flags);
    	status = ctrl_hci->int_mask /* | ctrl_hci->int_status*/;

	#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
    	/* Read ctrl_hci->int_mask to see if need to enable interrupt. */
    	if ((ctrl_hci->int_mask & SSV6XXX_INT_RESOURCE_LOW) == 0)
    	{
        	if (ctrl_hci->shi->if_ops->trigger_tx_rx == NULL)
        	{
            	u32 regval;

            	ctrl_hci->int_mask |= SSV6XXX_INT_RESOURCE_LOW;
            	regval = ~ctrl_hci->int_mask;

            	spin_unlock_irqrestore(&ctrl_hci->int_lock, flags);

            	HCI_IRQ_SET_MASK(ctrl_hci, regval);
            	mutex_unlock(&ctrl_hci->hci_mutex);
        	}
        	else
        	{
            	ctrl_hci->int_status |= SSV6XXX_INT_RESOURCE_LOW;
            	smp_mb();
            	spin_unlock_irqrestore(&ctrl_hci->int_lock, flags);
            	mutex_unlock(&ctrl_hci->hci_mutex);
            	ctrl_hci->shi->if_ops->trigger_tx_rx(ctrl_hci->shi->dev);
        	}
    	}
    	else
    	{
        	spin_unlock_irqrestore(&ctrl_hci->int_lock, flags);
        	mutex_unlock(&ctrl_hci->hci_mutex);
    	}

	#else // CONFIG_SSV_TX_LOWTHRESHOLD
    	{
        	u32 bitno;
        	bitno = ssv6xxx_hci_get_int_bitno(txqid);
        	/* Read ctrl_hci->int_mask to see if need to enable interrupt. */
        	if ((ctrl_hci->int_mask & BIT(bitno)) == 0)
        	{
            	if (ctrl_hci->shi->if_ops->trigger_tx_rx == NULL)
            	{
                	queue_work(ctrl_hci->hci_work_queue,&ctrl_hci->hci_tx_work[txqid]);
            	}
            	else
            	{
                	ctrl_hci->int_status |= BIT(bitno);
                	smp_mb();
                	ctrl_hci->shi->if_ops->trigger_tx_rx(ctrl_hci->shi->dev);
            	}
         	}
    	}
    	spin_unlock_irqrestore(&ctrl_hci->int_lock, flags);
	#endif // CONFIG_SSV_TX_LOWTHRESHOLD
	}

	return qlen;
    
}


static bool ssv6xxx_hci_is_txq_empty(struct ssv6xxx_hci_ctrl *ctrl_hci, int txqid)
{
    struct ssv_hw_txq *hw_txq;
    BUG_ON(txqid >= SSV_HW_TXQ_NUM);
    if (txqid >= SSV_HW_TXQ_NUM)
        return false;
    
    hw_txq = &ctrl_hci->hw_txq[txqid];
    if (skb_queue_len(&hw_txq->qhead) <= 0)
        return true;
    return false;
}


static int ssv6xxx_hci_txq_flush(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 txq_mask)
{
    struct ssv_hw_txq *hw_txq;
    struct sk_buff *skb = NULL;
    int txqid;

    for(txqid=0; txqid<SSV_HW_TXQ_NUM; txqid++) {
        if ((txq_mask & (1<<txqid)) != 0)
            continue;
        hw_txq = &ctrl_hci->hw_txq[txqid];
        while((skb = skb_dequeue(&hw_txq->qhead))) {
            ctrl_hci->shi->hci_tx_buf_free_cb (skb, (void *)SSV_SC(ctrl_hci));
        }
    }
   
    return 0;
}


static int ssv6xxx_hci_txq_flush_by_sta(struct ssv6xxx_hci_ctrl *ctrl_hci, int aid)
{

    return 0;
}



static int ssv6xxx_hci_txq_pause(struct ssv6xxx_hci_ctrl *ctrl_hci, u32 txq_mask)
{
    struct ssv_hw_txq *hw_txq;    
    int txqid;

#ifdef SSV_SUPPORT_HAL
    struct ssv_hw *sh;

    if (SSV_SC(ctrl_hci) == NULL){
		HCI_DBG_PRINT(ctrl_hci,	"%s: can't pause due to software structure not initialized !!\n", __func__);
		return 1;
    }    
    sh = ctrl_hci->shi->sh;
#endif

    mutex_lock(&ctrl_hci->txq_mask_lock);
    ctrl_hci->txq_mask |= (txq_mask & 0x1F);
    for(txqid=0; txqid<SSV_HW_TXQ_NUM; txqid++) {
        if ((ctrl_hci->txq_mask&(1<<txqid)) == 0)
            continue;
        hw_txq = &ctrl_hci->hw_txq[txqid];
        hw_txq->paused = true;
    }
    
    /* halt hardware tx queue */
#ifdef SSV_SUPPORT_HAL
    HAL_UPDATE_TXQ_MASK(sh, ctrl_hci->txq_mask);
#else
    HCI_REG_SET_BITS(ctrl_hci, ADR_MTX_MISC_EN,
        (ctrl_hci->txq_mask << MTX_HALT_Q_MB_SFT), MTX_HALT_Q_MB_MSK);
#endif
    mutex_unlock(&ctrl_hci->txq_mask_lock);
    
    
	//HCI_DBG_PRINT(ctrl_hci, "%s(): ctrl_hci->txq_mas=0x%x\n", __FUNCTION__, ctrl_hci->txq_mask);    
	return 0;
}



static int ssv6xxx_hci_txq_resume(struct ssv6xxx_hci_ctrl *hci_ctrl, u32 txq_mask)
{
    struct ssv_hw_txq *hw_txq;
    int txqid;

#ifdef SSV_SUPPORT_HAL
    struct ssv_hw *sh;
    
    if (SSV_SC(hci_ctrl) == NULL){
		HCI_DBG_PRINT(hci_ctrl, "%s: can't resume due to software structure not initialized !!\n", __func__);
		return 1;
    }    
    sh = hci_ctrl->shi->sh;
#endif
    /* resume hardware tx queue */

    mutex_lock(&hci_ctrl->txq_mask_lock);
    hci_ctrl->txq_mask &= ~(txq_mask & 0x1F);

#ifdef SSV_SUPPORT_HAL
    HAL_UPDATE_TXQ_MASK(sh, hci_ctrl->txq_mask);
#else
    HCI_REG_SET_BITS(hci_ctrl, ADR_MTX_MISC_EN,
        (hci_ctrl->txq_mask << MTX_HALT_Q_MB_SFT), MTX_HALT_Q_MB_MSK);
#endif
    
    for(txqid=0; txqid<SSV_HW_TXQ_NUM; txqid++) {
        if ((hci_ctrl->txq_mask&(1<<txqid)) != 0)
            continue;
        hw_txq = &hci_ctrl->hw_txq[txqid];
        hw_txq->paused = false;
    }
    mutex_unlock(&hci_ctrl->txq_mask_lock);

	
	//HCI_DBG_PRINT(hci_ctrl, "%s(): ctrl_hci->txq_mas=0x%x\n", __FUNCTION__, hci_ctrl->txq_mask);    
	return 0;
}



/**
* int ssv6xxx_hci_force_xmit() -send the specified number of frames for a specified 
*                          		tx queue to HWIF.
*
* @ struct ssv_hw_txq *hw_txq: the output queue to send.
* @ int max_count: the maximal number of frames to send.
*/
static int ssv6xxx_hci_force_xmit(struct ssv6xxx_hci_ctrl *hci_ctrl, struct ssv_hw_txq *hw_txq, 
    int max_count, int *err, int free_tx_page)
{
    struct sk_buff_head tx_cb_list;
    struct sk_buff *skb = NULL;
	int tx_count, ret;
	int    reason = -1 , page_count;
	struct ssv6xxx_hci_info *shi = hci_ctrl->shi;
    #ifdef SSV_SUPPORT_HAL
    struct ssv_hw *sh;
    #else
    struct ssv6200_tx_desc *tx_desc = NULL;
    #endif
    
    hci_ctrl->xmit_running = 1;
    skb_queue_head_init(&tx_cb_list);
    
    for (tx_count=0; tx_count<max_count; tx_count++) {
        if ((hci_ctrl->hci_start == false) || (hw_txq->paused)){
			HCI_DBG_PRINT(hci_ctrl, "ssv6xxx_hci_force_xmit - hci_start = false\n");
            *err = 1;
            goto xmit_out;
        }
        skb = skb_dequeue(&hw_txq->qhead);
        if (!skb) {
            goto xmit_out;
	    }
        
	    if (free_tx_page != TX_PAGE_NOT_LIMITED ) {   // start to check tx resource
	        page_count = (skb->len + SSV6200_ALLOC_RSVD);
	        if (page_count & HW_MMU_PAGE_MASK)
		        page_count = (page_count >> HW_MMU_PAGE_SHIFT) + 1;
	        else
		        page_count = page_count >> HW_MMU_PAGE_SHIFT;

	        if (page_count > (SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl) / 2))
			    HCI_DBG_PRINT(hci_ctrl, "Asking page %d(%d) exceeds resource limit %d.\n",
		        page_count, skb->len,(SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl) / 2));

	        if (free_tx_page < page_count){
		        skb_queue_head(&hw_txq->qhead, skb);
		        break;
            }

	        free_tx_page -= page_count;
	    }

	    
#ifdef SSV_SUPPORT_HAL
        sh = shi->sh;
        reason = HAL_GET_TX_DESC_REASON(sh, skb);
#else
		tx_desc = (struct ssv6200_tx_desc *)skb->data;
		reason = tx_desc->reason;
#endif

#if 1
        // Rate control update rate
        if (shi->hci_skb_update_cb != NULL && reason != ID_TRAP_SW_TXTPUT)
        {
            shi->hci_skb_update_cb(skb, (void *)(SSV_SC(hci_ctrl)));
        }
#endif   
        /**
         * send to ssv6xxx SoC through USB interface. If fail to send
         * the frame, try again next tx round.
         */
		if (shi->hci_pre_tx_cb)
        	shi->hci_pre_tx_cb(skb, (void *)(SSV_SC(hci_ctrl)));
		
		ret = IF_SEND(hci_ctrl, (void *)skb, skb->len, hw_txq->txq_no);
        if (ret < 0) {
			HCI_DBG_PRINT(hci_ctrl, "ssv6xxx_hci_force_xmit fail[%d]......\n", ret);
			*err = ret;
            skb_queue_head(&hw_txq->qhead, skb);
            break;
        }
        if (reason != ID_TRAP_SW_TXTPUT)
            skb_queue_tail(&tx_cb_list, skb);
        else
            shi->skb_free((void *)(SSV_SC(hci_ctrl)), skb);
        hw_txq->tx_pkt ++;
        
        /**
         * Notify upper layer of stopping flow control if the flow
         * control has been enabled by upper layer.
         */
        if (skb_queue_len(&hw_txq->qhead) < hw_txq->resum_thres) {
            shi->hci_tx_flow_ctrl_cb(
                (void *)(SSV_SC(hci_ctrl)),
                hw_txq->txq_no, false, 2000);
        }
    }
xmit_out:

    /* Report frames tx status to mac80211: for rate control */
    if (shi->hci_post_tx_cb && reason != -1 && reason != ID_TRAP_SW_TXTPUT) {
        shi->hci_post_tx_cb (&tx_cb_list, (void *)(SSV_SC(hci_ctrl)));
    }
    hci_ctrl->xmit_running = 0;
    return tx_count;
}

static int ssv6xxx_hci_force_tx_handler(struct ssv6xxx_hci_ctrl *hci_ctrl, void *dev,
                                        int max_count, int *err)
{
    struct ssv_hw_txq *hw_txq=dev;
    int tx_count=0;
    struct ssv6xxx_hci_info *shi = hci_ctrl->shi;
    int hci_free_id = 0, hci_used_id = -1;
    int free_tx_page = TX_PAGE_NOT_LIMITED, tx_use_page = -1;

    max_count = skb_queue_len(&hw_txq->qhead);
    if (max_count == 0)
        return 0;
   
    /* 
     * For USB, all endpoints get the conflict with firmware download. 
     * Therefore, it should check hci_start flags first.
     */
    if ((hci_ctrl->hci_start == false) || (hw_txq->paused)) {
        *err = 1;
        return 0;
    }

    /* take care of the resource of HCI INPUTQ */
    if (shi->sh->cfg.usb_hw_resource != USB_HW_RESOURCE_CHK_NONE ) {
       
        SSV_READRG_HCI_INQ_INFO(hci_ctrl, &hci_used_id, & tx_use_page);
        if ((hci_used_id == -1) || (tx_use_page == -1)) {
            *err = -EIO;
            return 0;
        }

        if (shi->sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_TXID ) {  // enable hci id check
            if (hci_used_id != -1) {
                hci_free_id = SSV6XXX_ID_HCI_INPUT_QUEUE - hci_used_id;
                if (hci_free_id == 0) {
                    *err = 2;
                    return 0;
                }

                if (max_count > hci_free_id)
                    max_count = hci_free_id;
            }
        }

        if ((shi->sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_TXPAGE) || // check tx page resource
            (shi->sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_SCAN)) { 
		    free_tx_page = SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl) - tx_use_page;
		    if (free_tx_page < 0) {
                *err = 2;
                return 0;
            }
		}
    }
    
	tx_count = ssv6xxx_hci_force_xmit(hci_ctrl, hw_txq, max_count, err, free_tx_page);
	// Check if queue is empty, call empty callback so the AMPDU TX can send its buffered frames down.
    if (   (shi->hci_tx_q_empty_cb != NULL)
        && (skb_queue_len(&hw_txq->qhead) == 0))
    {
        shi->hci_tx_q_empty_cb(hw_txq->txq_no, (void *)(SSV_SC(hci_ctrl)));
    }
    return tx_count;
}

static int _do_force_tx (struct ssv6xxx_hci_ctrl *hctl, int *err)
{
    int                q_num;
    int                tx_count = 0;
    struct ssv_hw_txq *hw_txq;
    u32 dev_type = HCI_DEVICE_TYPE(hctl);
    int (*handler)(struct ssv6xxx_hci_ctrl *, void *, int, int *);

    if ((dev_type == SSV_HWIF_INTERFACE_USB) &&
         ( (hctl->shi->sh->cfg.usb_hw_resource & USB_HW_RESOURCE_CHK_TXID ) == 0)) {
        handler = ssv6xxx_hci_force_tx_handler;
    } else {
        handler = ssv6xxx_hci_usb_tx_handler;//Check HW resource for WMM issue.
    }

    for (q_num = (SSV_HW_TXQ_NUM - 1); q_num >= 0; q_num--)
    {
        hw_txq = &hctl->hw_txq[q_num];

        tx_count += handler(hctl, hw_txq, 999, err);   
        if (*err < 0)
            break;
    }

    return tx_count;
}

/**
* int ssv6xxx_hci_xmit() - send the specified number of frames for a specified 
*                          tx queue to SDIO.
*
* @ struct ssv_hw_txq *hw_txq: the output queue to send.
* @ int max_count: the maximal number of frames to send.
*/
static int ssv6xxx_hci_xmit(struct ssv6xxx_hci_ctrl *hci_ctrl, struct ssv_hw_txq *hw_txq, int max_count, struct ssv6xxx_hw_resource *phw_resource)
{
    struct sk_buff_head tx_cb_list;
    struct sk_buff *skb = NULL;
	int tx_count = 0, ret = 0, page_count;
	int    reason = -1;
#ifdef SSV_SUPPORT_HAL
    struct ssv_hw *sh;
#else
    struct ssv6200_tx_desc *tx_desc = NULL;
#endif
    
    hci_ctrl->xmit_running = 1;
    skb_queue_head_init(&tx_cb_list);
    
    for(tx_count=0; tx_count<max_count; tx_count++) {
        if ((hci_ctrl->hci_start == false) || (hw_txq->paused)){
			HCI_DBG_PRINT(hci_ctrl, "ssv6xxx_hci_xmit - hci_start = false\n");
            goto xmit_out;
        }
        skb = skb_dequeue(&hw_txq->qhead);
        if (!skb){
			HCI_DBG_PRINT(hci_ctrl, "ssv6xxx_hci_xmit - queue empty\n");
            goto xmit_out;
	    }
	    page_count = (skb->len + SSV6200_ALLOC_RSVD);
	    if (page_count & HW_MMU_PAGE_MASK)
		    page_count = (page_count >> HW_MMU_PAGE_SHIFT) + 1;
	    else
		    page_count = page_count >> HW_MMU_PAGE_SHIFT;

	    if (page_count > (SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl) / 2))
			HCI_DBG_PRINT(hci_ctrl, "Asking page %d(%d) exceeds resource limit %d.\n",
		        page_count, skb->len,(SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl) / 2));

            if (page_count > SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl)) {
                printk("Asking page %d(%d) > %d is impossible to send. Drop it!\n", page_count, skb->len, SSV6XXX_PAGE_TX_THRESHOLD(hci_ctrl));
                hci_ctrl->shi->skb_free((void *)(SSV_SC(hci_ctrl)), skb);
                break;
            }

	    if ((phw_resource->free_tx_page < page_count) || (phw_resource->free_tx_id <= 0) || (phw_resource->max_tx_frame[hw_txq->txq_no] <= 0))
        {
		    skb_queue_head(&hw_txq->qhead, skb);
            /* 
             * Workaround: Reduce USB interference.
             * This can improve rx capability.
             */
		    udelay(1);
		    break;
        }

	    phw_resource->free_tx_page -= page_count;
	    phw_resource->free_tx_id--;
	    phw_resource->max_tx_frame[hw_txq->txq_no]--;

#ifdef SSV_SUPPORT_HAL
        sh = hci_ctrl->shi->sh;
        reason = HAL_GET_TX_DESC_REASON(sh, skb);
#else
		tx_desc = (struct ssv6200_tx_desc *)skb->data;
		reason = tx_desc->reason;
#endif
#if 1
        // Rate control update rate
        if (hci_ctrl->shi->hci_skb_update_cb != NULL && reason != ID_TRAP_SW_TXTPUT)
        {
            hci_ctrl->shi->hci_skb_update_cb(skb, (void *)(SSV_SC(hci_ctrl)));
        }
#endif   
        /**
         * send to ssv6xxx SoC through SDIO/SPI interface. If fail to send
         * the frame, try again next tx round.
         */
		if (hci_ctrl->shi->hci_pre_tx_cb)
		    hci_ctrl->shi->hci_pre_tx_cb(skb, (void *)(SSV_SC(hci_ctrl)));
        
		ret = IF_SEND(hci_ctrl, (void *)skb, skb->len, hw_txq->txq_no);
        if (ret < 0) {
            HCI_DBG_PRINT(hci_ctrl, "ssv6xxx_hci_xmit fail......\n");
            skb_queue_head(&hw_txq->qhead, skb);
            break;
        }
        if (reason != ID_TRAP_SW_TXTPUT)
            skb_queue_tail(&tx_cb_list, skb);
        else
            hci_ctrl->shi->skb_free((void *)(SSV_SC(hci_ctrl)), skb);
        hw_txq->tx_pkt ++;
#ifdef CONFIG_IRQ_DEBUG_COUNT
        if (hci_ctrl->irq_enable)
            hci_ctrl->irq_tx_pkt_count++;
#endif 
        
        /**
         * Notify upper layer of stopping flow control if the flow
         * control has been enabled by upper layer.
         */
        if (skb_queue_len(&hw_txq->qhead) < hw_txq->resum_thres) {
            hci_ctrl->shi->hci_tx_flow_ctrl_cb(
                    (void *)(SSV_SC(hci_ctrl)),
				    hw_txq->txq_no, false, 2000);
		}
	}
xmit_out:

    /* Report frames tx status to mac80211: for rate control */
    if (hci_ctrl->shi->hci_post_tx_cb && reason != -1 && reason != ID_TRAP_SW_TXTPUT) {
        hci_ctrl->shi->hci_post_tx_cb (&tx_cb_list, (void *)(SSV_SC(hci_ctrl)));
    }
    hci_ctrl->xmit_running = 0;
    return (ret == 0) ? tx_count : ret;
}

static int ssv6xxx_hci_tx_handler(struct ssv6xxx_hci_ctrl *ctrl_hci, void *dev, int max_count)
{
    struct ssv6xxx_hci_txq_info txq_info;
    struct ssv6xxx_hci_txq_info2 txq_info2; 
    struct ssv6xxx_hw_resource hw_resource;
    struct ssv_hw_txq *hw_txq=dev;
    int hci_used_id = -1;
	int ret, tx_count=0;

    max_count = skb_queue_len(&hw_txq->qhead);
    if ((max_count == 0) || (hw_txq->paused))
        return 0;

    if (hw_txq->txq_no == 4)
    {
#ifndef _x86_64
retry_read:
#endif
    #ifdef SSV_SUPPORT_HAL
        if (SSV_SC(ctrl_hci) != NULL)
            ret = HAL_READRG_TXQ_INFO2(ctrl_hci->shi->sh ,(u32 *)&txq_info2, &hci_used_id);
        else {
			HCI_DBG_PRINT(ctrl_hci, "%s: can't read txq_info2 due to software structure not initialized !!\n", __func__);
		    return 0;
        }
    #else
        ret = HCI_REG_READ(ctrl_hci, ADR_TX_ID_ALL_INFO2, (u32 *)&txq_info2);
    #endif
        if (ret < 0) {
            ctrl_hci->read_rs1_info_fail++;
            return 0;
        }
#ifdef _x86_64
        BUG_ON(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_page);
        BUG_ON(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_id);
#else
        if(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_page)
            goto retry_read;
        if(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_id)
            goto retry_read;
        /* 
         * If HCI input queue is full, host driver should wait for free txid.
         */
        if (hci_used_id == SSV6XXX_ID_HCI_INPUT_QUEUE)
            goto retry_read;
#endif

		hw_resource.free_tx_page =
		    SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) - txq_info2.tx_use_page;
		hw_resource.free_tx_id = SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) - txq_info2.tx_use_id;

        //    max_tx_frame[4] = SSV6XXX_ID_MANAGER_QUEUE(ctrl_hci)-(txq_info.tx_use_id-txq_info.txq0_size-txq_info.txq1_size-txq_info.txq2_size-txq_info.txq3_size);
		hw_resource.max_tx_frame[4] = SSV6XXX_ID_MANAGER_QUEUE(ctrl_hci) - txq_info2.txq4_size;
        if (hci_used_id != -1) // update max_count by free hci input id 
            max_count = SSV6XXX_ID_HCI_INPUT_QUEUE - hci_used_id;
    }
    else
    {
    #ifdef SSV_SUPPORT_HAL
        if (SSV_SC(ctrl_hci) != NULL)
            ret = HAL_READRG_TXQ_INFO(ctrl_hci->shi->sh ,(u32 *)&txq_info, &hci_used_id);
        else {
			HCI_DBG_PRINT(ctrl_hci, "%s: can't read txq_info due to software structure not initialized !!\n",  __func__);
		    return 0;
        }        
    #else
        ret = HCI_REG_READ(ctrl_hci, ADR_TX_ID_ALL_INFO, (u32 *)&txq_info);
    #endif
        if (ret < 0) {
            ctrl_hci->read_rs0_info_fail++;
            return 0;
        }

        if (hci_used_id == SSV6XXX_ID_HCI_INPUT_QUEUE)  //no free hci txid
            return 0;

        BUG_ON(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_page);
        BUG_ON(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_id);
		hw_resource.free_tx_page = SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) - txq_info.tx_use_page;
		hw_resource.free_tx_id = SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) - txq_info.tx_use_id;
		hw_resource.max_tx_frame[0] =
		    SSV6XXX_ID_AC_BK_OUT_QUEUE(ctrl_hci) - txq_info.txq0_size;
		hw_resource.max_tx_frame[1] =
		    SSV6XXX_ID_AC_BE_OUT_QUEUE(ctrl_hci) - txq_info.txq1_size;
		hw_resource.max_tx_frame[2] =
		    SSV6XXX_ID_AC_VI_OUT_QUEUE(ctrl_hci) - txq_info.txq2_size;
		hw_resource.max_tx_frame[3] =
		    SSV6XXX_ID_AC_VO_OUT_QUEUE(ctrl_hci) - txq_info.txq3_size;
        if (hci_used_id != -1) // update max_count by free hci input id 
            max_count = SSV6XXX_ID_HCI_INPUT_QUEUE - hci_used_id;
		//BUG_ON(hw_resource.max_tx_frame[3] < 0);
		//BUG_ON(hw_resource.max_tx_frame[2] < 0);
		//BUG_ON(hw_resource.max_tx_frame[1] < 0);
		//BUG_ON(hw_resource.max_tx_frame[0] < 0);
	}
	{
#ifdef CONFIG_IRQ_DEBUG_COUNT
		if(ctrl_hci->irq_enable)
			ctrl_hci->real_tx_irq_count++;
#endif
		tx_count = ssv6xxx_hci_xmit(ctrl_hci, hw_txq, max_count, &hw_resource);
	}
    // Check if queue is empty, call empty callback so the AMPDU TX can send its buffered frames down.
    if (   (ctrl_hci->shi->hci_tx_q_empty_cb != NULL) 
        && (skb_queue_len(&hw_txq->qhead) == 0))
    {
        ctrl_hci->shi->hci_tx_q_empty_cb(hw_txq->txq_no, SSV_SC(ctrl_hci));
    }
    return tx_count;
}

static int ssv6xxx_hci_usb_tx_handler(struct ssv6xxx_hci_ctrl *ctrl_hci, void *dev, int max_count, int *err)
{
    struct ssv6xxx_hci_txq_info txq_info;
    struct ssv6xxx_hci_txq_info2 txq_info2; 
    struct ssv6xxx_hw_resource hw_resource;
    struct ssv_hw_txq *hw_txq=dev;
    int hci_used_id = -1;
	int ret, tx_count=0;

    max_count = skb_queue_len(&hw_txq->qhead);
    if (max_count == 0)
        return 0;

    if ((ctrl_hci->hci_start == false) || (hw_txq->paused)) {
        *err = 1;
        return 0;
    }

    if (hw_txq->txq_no == 4)
    {
#ifndef _x86_64
retry_read:
#endif
    #ifdef SSV_SUPPORT_HAL
        if (SSV_SC(ctrl_hci) != NULL)
            ret = HAL_READRG_TXQ_INFO2(ctrl_hci->shi->sh ,(u32 *)&txq_info2, &hci_used_id);
        else {
			HCI_DBG_PRINT(ctrl_hci, "%s: can't read txq_info2 due to software structure not initialized !!\n", __func__);
                    *err = -EIO;
		    return 0;
        }
    #else
        ret = HCI_REG_READ(ctrl_hci, ADR_TX_ID_ALL_INFO2, (u32 *)&txq_info2);
    #endif
        if (ret < 0) {
            ctrl_hci->read_rs1_info_fail++;
            *err = -EIO;
            return 0;
        }
#ifdef _x86_64
        BUG_ON(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_page);
        BUG_ON(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_id);
#else
        if(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_page)
            goto retry_read;
        if(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info2.tx_use_id)
            goto retry_read;
        /* 
         * If HCI input queue is full, host driver should wait for free txid.
         */
        if (hci_used_id == SSV6XXX_ID_HCI_INPUT_QUEUE)
            goto retry_read;
#endif

		hw_resource.free_tx_page =
		    SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) - txq_info2.tx_use_page;
		hw_resource.free_tx_id = SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) - txq_info2.tx_use_id;

        //    max_tx_frame[4] = SSV6XXX_ID_MANAGER_QUEUE(ctrl_hci)-(txq_info.tx_use_id-txq_info.txq0_size-txq_info.txq1_size-txq_info.txq2_size-txq_info.txq3_size);
		hw_resource.max_tx_frame[4] = SSV6XXX_ID_USB_MANAGER_QUEUE(ctrl_hci) - txq_info2.txq4_size;
        if (hci_used_id != -1) // update max_count by free hci input id 
            max_count = SSV6XXX_ID_HCI_INPUT_QUEUE - hci_used_id;
    }
    else
    {
    #ifdef SSV_SUPPORT_HAL
        if (SSV_SC(ctrl_hci) != NULL)
            ret = HAL_READRG_TXQ_INFO(ctrl_hci->shi->sh ,(u32 *)&txq_info, &hci_used_id);
        else {
			HCI_DBG_PRINT(ctrl_hci, "%s: can't read txq_info due to software structure not initialized !!\n",  __func__);
                    *err = -EIO;
		    return 0;
        }        
    #else
        ret = HCI_REG_READ(ctrl_hci, ADR_TX_ID_ALL_INFO, (u32 *)&txq_info);
    #endif
        if (ret < 0) {
            ctrl_hci->read_rs0_info_fail++;
            *err = -EIO;
            return 0;
        }

        if (hci_used_id == SSV6XXX_ID_HCI_INPUT_QUEUE) {  //no free hci txid
            *err = 2;
            return 0;
        }

        if (SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_page) {
            *err = -EIO;
            //WARN_ON(SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_page);
            return 0;
        }
        if (SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_id) {
            *err = -EIO;
            //WARN_ON(SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) < txq_info.tx_use_id);
            return 0;
        }
		hw_resource.free_tx_page = SSV6XXX_PAGE_TX_THRESHOLD(ctrl_hci) - txq_info.tx_use_page;
		hw_resource.free_tx_id = SSV6XXX_ID_TX_THRESHOLD(ctrl_hci) - txq_info.tx_use_id;
		hw_resource.max_tx_frame[0] =
		    SSV6XXX_ID_USB_AC_BK_OUT_QUEUE(ctrl_hci) - txq_info.txq0_size;
		hw_resource.max_tx_frame[1] =
		    SSV6XXX_ID_USB_AC_BE_OUT_QUEUE(ctrl_hci) - txq_info.txq1_size;
		hw_resource.max_tx_frame[2] =
		    SSV6XXX_ID_USB_AC_VI_OUT_QUEUE(ctrl_hci) - txq_info.txq2_size;
		hw_resource.max_tx_frame[3] =
		    SSV6XXX_ID_USB_AC_VO_OUT_QUEUE(ctrl_hci) - txq_info.txq3_size;
        if (hci_used_id != -1) // update max_count by free hci input id 
            max_count = SSV6XXX_ID_HCI_INPUT_QUEUE - hci_used_id;
            if (hw_resource.max_tx_frame[3] < 0) {
                *err = -EIO;
                //WARN_ON(hw_resource.max_tx_frame[3] < 0);
                return 0;
            }
            if (hw_resource.max_tx_frame[2] < 0) {
                *err = -EIO;
                //WARN_ON(hw_resource.max_tx_frame[2] < 0);
                return 0;
            }
            if (hw_resource.max_tx_frame[1] < 0) {
                *err = -EIO;
                //WARN_ON(hw_resource.max_tx_frame[1] < 0);
                return 0;
            }
            if (hw_resource.max_tx_frame[0] < 0) {
                *err = -EIO;
                //WARN_ON(hw_resource.max_tx_frame[0] < 0);
                return 0;
            }
	}
	{
#ifdef CONFIG_IRQ_DEBUG_COUNT
		if(ctrl_hci->irq_enable)
			ctrl_hci->real_tx_irq_count++;
#endif
		tx_count = ssv6xxx_hci_xmit(ctrl_hci, hw_txq, max_count, &hw_resource);
	}
    // Check if queue is empty, call empty callback so the AMPDU TX can send its buffered frames down.
    if (   (ctrl_hci->shi->hci_tx_q_empty_cb != NULL) 
        && (skb_queue_len(&hw_txq->qhead) == 0))
    {
        ctrl_hci->shi->hci_tx_q_empty_cb(hw_txq->txq_no, SSV_SC(ctrl_hci));
    }
    return tx_count;
}

void ssv6xxx_hci_tx_work(struct work_struct *work)
{
    struct ssv6xxx_hci_ctrl *ctrl_hci;

    ctrl_hci = container_of(work, struct ssv6xxx_hci_ctrl, hci_tx_work);
#ifdef CONFIG_SSV_TX_LOWTHRESHOLD

    // enable interrupt bit
    ssv6xxx_hci_irq_register(ctrl_hci, SSV6XXX_INT_RESOURCE_LOW);
    // Trigger TX interrupt from host
    //HCI_IRQ_TRIGGER(ctrl_hci);

#else // CONFIG_SSV_TX_LOWTHRESHOLD
    int txqid;

    for(txqid = SSV_HW_TXQ_NUM - 1; txqid >= 0; txqid--) {
        u32 bitno;
        
        /* check which worker wake up this work queue. */
        if (&ctrl_hci->hci_tx_work[txqid] != work)
            continue;

        /* enable interrupt bit */
        bitno = ssv6xxx_hci_get_int_bitno(txqid);         
        ssv6xxx_hci_irq_register(1<<(bitno));              
        break;
    }
#endif // CONFIG_SSV_TX_LOWTHRESHOLD
}


static int _do_rx (struct ssv6xxx_hci_ctrl *hctl, u32 isr_status)
{
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    struct sk_buff_head         rx_list;
    #endif // USE_THREAD_RX
    struct sk_buff             *rx_mpdu;
    int                         ret = 0;
    int                         rx_cnt, next_pkt_len;
    size_t                      dlen;
    u32                         status = isr_status;
	u32							rx_mode, frame_size;
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    struct timespec             rx_io_start_time, rx_io_end_time, rx_io_diff_time;
    struct timespec             rx_proc_start_time, rx_proc_end_time, rx_proc_diff_time;
    #endif // CONFIG_SSV6XXX_DEBUGFS

    #ifdef CONFIG_SSV6XXX_DEBUGFS
    memset(&rx_io_end_time, 0 , sizeof(struct timespec));
    memset(&rx_io_start_time, 0 , sizeof(struct timespec));
    memset(&rx_proc_start_time, 0 , sizeof(struct timespec));
	#endif // CONFIG_SSV6XXX_DEBUGFS
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    skb_queue_head_init(&rx_list);
    #endif // USE_THREAD_RX
	rx_mode = hctl->shi->hci_rx_mode_cb((void *)(SSV_SC(hctl)));
	frame_size = (rx_mode & RX_HW_AGG_MODE) ? MAX_HCI_RX_AGGR_SIZE : MAX_FRAME_SIZE_DMG;
    frame_size += MAX_RX_PKT_RSVD;
    next_pkt_len = 0;

    for (rx_cnt = 0; (status & SSV6XXX_INT_RX) && (rx_cnt < 32/*999999*/); rx_cnt++)
    {
        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
            getnstimeofday(&rx_io_start_time);
        #endif // CONFIG_SSV6XXX_DEBUGFS
        
        dlen = next_pkt_len;
        ret = IF_RECV(hctl, hctl->rx_buf->data, &dlen, rx_mode);

        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
            getnstimeofday(&rx_io_end_time);
        #endif // CONFIG_SSV6XXX_DEBUGFS

        if (ret < 0 || dlen<=0)
        {
			HCI_DBG_PRINT(hctl, "%s(): IF_RECV() retruns %d (dlen=%d)\n", __FUNCTION__, ret, (int)dlen);
            if (ret != -84 || dlen>frame_size)
                break;
        }

        rx_mpdu = hctl->rx_buf;
        hctl->rx_buf = hctl->shi->skb_alloc((void *)(SSV_SC(hctl)), frame_size);
        if (hctl->rx_buf == NULL)
        {
			HCI_DBG_PRINT(hctl, "RX buffer allocation failure!\n");
            hctl->rx_buf = rx_mpdu;
            break;
        }
        hctl->rx_pkt++;

        #ifdef CONFIG_IRQ_DEBUG_COUNT
        if (hctl->irq_enable){
            hctl->irq_rx_pkt_count ++;
        }
        #endif

        skb_put(rx_mpdu, dlen);
        next_pkt_len = hctl->shi->hci_peek_next_pkt_len_cb(rx_mpdu, (void *)(SSV_SC(hctl))); 
        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
            getnstimeofday(&rx_proc_start_time);
        #endif // CONFIG_SSV6XXX_DEBUGFS
        #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
        __skb_queue_tail(&rx_list, rx_mpdu);
        #else
        hctl->shi->hci_rx_cb(rx_mpdu, (void *)(SSV_SC(hctl)));
        #endif // USE_THREAD_RX

        if (next_pkt_len == 0) //if no next packet info, it is necessary to check irq_status
            HCI_IRQ_STATUS(hctl, &status);
        
		#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
        {
            getnstimeofday(&rx_proc_end_time);
            hctl->isr_rx_io_count++;

            rx_io_diff_time = timespec_sub(rx_io_end_time, rx_io_start_time);
            hctl->isr_rx_io_time += timespec_to_ns(&rx_io_diff_time);

            rx_proc_diff_time = timespec_sub(rx_proc_end_time, rx_proc_start_time);
            hctl->isr_rx_proc_time += timespec_to_ns(&rx_proc_diff_time);
        }
        #endif // CONFIG_SSV6XXX_DEBUGFS
    }

    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    if (hctl->isr_mib_enable)
        getnstimeofday(&rx_proc_start_time);
    #endif // CONFIG_SSV6XXX_DEBUGFS
    hctl->shi->hci_rx_cb(&rx_list, (void *)(SSV_SC(hctl)));
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    if (hctl->isr_mib_enable)
    {
        getnstimeofday(&rx_proc_end_time);

        rx_proc_diff_time = timespec_sub(rx_proc_end_time, rx_proc_start_time);
        hctl->isr_rx_proc_time += timespec_to_ns(&rx_proc_diff_time);
    }
    #endif // CONFIG_SSV6XXX_DEBUGFS
    #endif // USE_THREAD_RX

    return ret;
}

static void ssv6xxx_hci_rx_work(struct work_struct *work)
{
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    struct sk_buff_head rx_list;
    #endif // USE_THREAD_RX
    struct sk_buff *rx_mpdu;
    int rx_cnt, ret, next_pkt_len;
    size_t dlen;
    int status;
	u32	rx_mode, frame_size;
#ifdef CONFIG_SSV6XXX_DEBUGFS
    struct timespec     rx_io_start_time, rx_io_end_time, rx_io_diff_time;
    struct timespec     rx_proc_start_time, rx_proc_end_time, rx_proc_diff_time;
#endif // CONFIG_SSV6XXX_DEBUGFS
    struct ssv6xxx_hci_ctrl *ctrl_hci;
    struct ssv6xxx_hci_info *shi;

    ctrl_hci = container_of(work, struct ssv6xxx_hci_ctrl, hci_rx_work);
    shi = ctrl_hci->shi;
	#ifdef CONFIG_SSV6XXX_DEBUGFS
    memset(&rx_io_end_time, 0 , sizeof(struct timespec));
    memset(&rx_io_start_time, 0 , sizeof(struct timespec));
    memset(&rx_proc_start_time, 0 , sizeof(struct timespec));
	#endif
    ctrl_hci->rx_work_running = 1;
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    skb_queue_head_init(&rx_list);
    #endif // USE_THREAD_RX
	rx_mode = shi->hci_rx_mode_cb((void *)(SSV_SC(ctrl_hci)));
	frame_size = (rx_mode & RX_HW_AGG_MODE) ? MAX_HCI_RX_AGGR_SIZE : MAX_FRAME_SIZE_DMG;
    frame_size += MAX_RX_PKT_RSVD;
    next_pkt_len = 0;
    
	status = SSV6XXX_INT_RX;
    for (rx_cnt = 0; (status & SSV6XXX_INT_RX) && (rx_cnt < 32/*999999*/); rx_cnt++) {
#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (ctrl_hci->isr_mib_enable)
            getnstimeofday(&rx_io_start_time);
#endif // CONFIG_SSV6XXX_DEBUGFS
		
        dlen = next_pkt_len;
        ret = IF_RECV(ctrl_hci, ctrl_hci->rx_buf->data, &dlen, rx_mode);

#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (ctrl_hci->isr_mib_enable)
            getnstimeofday(&rx_io_end_time);
#endif // CONFIG_SSV6XXX_DEBUGFS

        if (ret < 0 || dlen<=0) {
			HCI_DBG_PRINT(ctrl_hci, "%s(): IF_RECV() retruns %d (dlen=%d)\n", __FUNCTION__, ret, (int)dlen);
            if (ret != -84 || dlen>frame_size)
                break;
        }

        rx_mpdu = ctrl_hci->rx_buf;
        ctrl_hci->rx_buf = shi->skb_alloc((void *)(SSV_SC(ctrl_hci)), frame_size);
        if (ctrl_hci->rx_buf == NULL) {
			HCI_DBG_PRINT(ctrl_hci, "RX buffer allocation failure!\n");
            ctrl_hci->rx_buf = rx_mpdu;
            break;
        }
        ctrl_hci->rx_pkt ++;
#ifdef CONFIG_IRQ_DEBUG_COUNT
		if(ctrl_hci->irq_enable){
			ctrl_hci->irq_rx_pkt_count ++;
	    }
#endif
        skb_put(rx_mpdu, dlen);
        next_pkt_len = shi->hci_peek_next_pkt_len_cb(rx_mpdu, (void *)(SSV_SC(ctrl_hci))); 
#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (ctrl_hci->isr_mib_enable)
            getnstimeofday(&rx_proc_start_time);
#endif // CONFIG_SSV6XXX_DEBUGFS
        #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
        __skb_queue_tail(&rx_list, rx_mpdu);
        #else
        shi->hci_rx_cb(rx_mpdu, (void *)(SSV_SC(ctrl_hci)));
        #endif // USE_THREAD_RX

        if (next_pkt_len == 0) //if no next packet info, it is necessary to check irq_status
            HCI_IRQ_STATUS(ctrl_hci, &status);

#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (ctrl_hci->isr_mib_enable)
        {
            getnstimeofday(&rx_proc_end_time);
            ctrl_hci->isr_rx_io_count++;

            rx_io_diff_time = timespec_sub(rx_io_end_time, rx_io_start_time);
            ctrl_hci->isr_rx_io_time += timespec_to_ns(&rx_io_diff_time);

            rx_proc_diff_time = timespec_sub(rx_proc_end_time, rx_proc_start_time);
            ctrl_hci->isr_rx_proc_time += timespec_to_ns(&rx_proc_diff_time);
        }
#endif // CONFIG_SSV6XXX_DEBUGFS
    }
    #if !defined(USE_THREAD_RX) || defined(USE_BATCH_RX)
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    if (ctrl_hci->isr_mib_enable)
        getnstimeofday(&rx_proc_start_time);
    #endif // CONFIG_SSV6XXX_DEBUGFS
    shi->hci_rx_cb(&rx_list, (void *)(SSV_SC(ctrl_hci)));
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    if (ctrl_hci->isr_mib_enable)
    {
        getnstimeofday(&rx_proc_end_time);

        rx_proc_diff_time = timespec_sub(rx_proc_end_time, rx_proc_start_time);
        ctrl_hci->isr_rx_proc_time += timespec_to_ns(&rx_proc_diff_time);
    }
    #endif // CONFIG_SSV6XXX_DEBUGFS
    #endif // USE_THREAD_RX

    ctrl_hci->rx_work_running = 0;
}

#ifdef CONFIG_SSV6XXX_DEBUGFS
static void ssv6xxx_isr_mib_reset (struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    ctrl_hci->isr_mib_reset = 0;
    ctrl_hci->isr_total_time = 0;
    ctrl_hci->isr_rx_io_time = 0;
    ctrl_hci->isr_tx_io_time = 0;
    ctrl_hci->isr_rx_io_count = 0;
    ctrl_hci->isr_tx_io_count = 0;
    ctrl_hci->isr_rx_proc_time =0;
}

static int hw_txq_len_open(struct inode *inode, struct file *filp)
{
    filp->private_data = inode->i_private;
    return 0;
}

static ssize_t hw_txq_len_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
    ssize_t ret;
    struct ssv6xxx_hci_ctrl *hctl = (struct ssv6xxx_hci_ctrl *)filp->private_data;
    char *summary_buf = kzalloc(1024, GFP_KERNEL);
    char *prn_ptr = summary_buf;
    int   prt_size;
    int   buf_size = 1024;
    int   i=0;

    if (!summary_buf)
        return -ENOMEM;

    for (i=0; i<SSV_HW_TXQ_NUM; i++)
    {
        prt_size = snprintf(prn_ptr, buf_size, "\n\rhw_txq%d_len: %d", i, 
                            skb_queue_len(&hctl->hw_txq[i].qhead));
        prn_ptr  += prt_size;
        buf_size -= prt_size;
    }

    buf_size = 1024 - buf_size;

    ret = simple_read_from_buffer(buffer, count, ppos, summary_buf, buf_size);
    kfree(summary_buf);

    return ret;
}

#if 0
 static ssize_t hw_txq_len_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
    return 0;
}
#endif


struct file_operations hw_txq_len_fops = {
    .owner = THIS_MODULE,
    .open  = hw_txq_len_open,
    .read  = hw_txq_len_read,
//    .write = hw_txq_len_write,
};

bool ssv6xxx_hci_init_debugfs(struct ssv6xxx_hci_ctrl *ctrl_hci, struct dentry *dev_deugfs_dir)
{
    ctrl_hci->debugfs_dir = debugfs_create_dir("hci", dev_deugfs_dir);

    if (ctrl_hci->debugfs_dir == NULL)
    {
		HCI_DBG_PRINT(ctrl_hci, "Failed to create HCI debugfs directory.\n");
        return false;
    }

    debugfs_create_u32("TXQ_mask",           00444, ctrl_hci->debugfs_dir, &ctrl_hci->txq_mask);
    debugfs_create_u32("hci_isr_mib_enable", 00644, ctrl_hci->debugfs_dir, &ctrl_hci->isr_mib_enable);
    debugfs_create_u32("hci_isr_mib_reset",  00644, ctrl_hci->debugfs_dir, &ctrl_hci->isr_mib_reset);
    debugfs_create_u64("isr_total_time",     00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_total_time);
    debugfs_create_u64("tx_io_time",         00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_tx_io_time);
    debugfs_create_u64("rx_io_time",         00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_rx_io_time);
    debugfs_create_u32("tx_io_count",        00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_tx_io_count);
    debugfs_create_u32("rx_io_count",        00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_rx_io_count);
    debugfs_create_u64("rx_proc_time",       00444, ctrl_hci->debugfs_dir, &ctrl_hci->isr_rx_proc_time);
    debugfs_create_file("hw_txq_len",        00444, ctrl_hci->debugfs_dir, ctrl_hci, &hw_txq_len_fops);

    return true;
}

void ssv6xxx_hci_deinit_debugfs(struct ssv6xxx_hci_ctrl *ctrl_hci)
{
    if (ctrl_hci->debugfs_dir == NULL)
        return;

    //debugfs_remove_recursive(ctrl_hci->debugfs_dir);
    ctrl_hci->debugfs_dir = NULL;
}
#endif // CONFIG_SSV6XXX_DEBUGFS
//--------------------------------------------------------------
//                              ssv6xxx_hci_isr
//--------------------------------------------------------------
static int _isr_do_rx (struct ssv6xxx_hci_ctrl *hctl, u32 isr_status)
{
    int retval;
    //==============================================================>
    //DEBUG CODE
    u32 before = jiffies;
#ifdef CONFIG_IRQ_DEBUG_COUNT
    if (hctl->irq_enable)
        hctl->rx_irq_count++;
#endif
    if (hctl->isr_summary_eable
        && hctl->prev_rx_isr_jiffes) {


       if (hctl->isr_rx_idle_time){
           hctl->isr_rx_idle_time += (jiffies - hctl->prev_rx_isr_jiffes);
           hctl->isr_rx_idle_time = hctl->isr_rx_idle_time >>1;
       }
       else {
            //first time
            hctl->isr_rx_idle_time += (jiffies - hctl->prev_rx_isr_jiffes);
       }

    }
    //<==============================================================

    retval = _do_rx(hctl, isr_status);
    //ssv6xxx_hci_rx_work(&hctl->hci_rx_work);

    //==============================================================>
    //DEBUG CODE

    if(hctl->isr_summary_eable){

        //caculate how many time spend in handling rx frames
        if(hctl->isr_rx_time){
            hctl->isr_rx_time += (jiffies-before);
            hctl->isr_rx_time = hctl->isr_rx_time >>1;
        }
        else{
            hctl->isr_rx_time += (jiffies-before);
        }

        //log timestamp for calculating no interrupt space.
        hctl->prev_rx_isr_jiffes = jiffies;

    }
    //<=============================================================
    return retval;
} // end of - _do_rx -

static bool ssv6xxx_hci_is_frame_send(struct ssv6xxx_hci_ctrl *hci_ctrl)
{
    int	q_num;
    struct ssv_hw_txq *hw_txq;

    for (q_num = (SSV_HW_TXQ_NUM - 1); q_num >= 0; q_num--) {
        hw_txq = &hci_ctrl->hw_txq[q_num];

        if (!hw_txq->paused && !ssv6xxx_hci_is_txq_empty(hci_ctrl, q_num))
            return true;
    }

    return false;
}

#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
static int _do_tx (struct ssv6xxx_hci_ctrl *hctl, u32 status)
{
    int                q_num;
    int                tx_count = 0;
    unsigned long      flags;
    struct ssv_hw_txq *hw_txq;
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    struct timespec    tx_io_start_time, tx_io_end_time, tx_io_diff_time;
    #endif // CONFIG_SSV6XXX_DEBUGFS
    int                ret;

    #ifdef CONFIG_SSV6XXX_DEBUGFS
    memset(&tx_io_start_time, 0 , sizeof(struct timespec));
    #endif
    #ifdef CONFIG_IRQ_DEBUG_COUNT
    if ((!(status & SSV6XXX_INT_RX)) && hctl->irq_enable)
        hctl->tx_irq_count++;
    #endif // CONFIG_IRQ_DEBUG_COUNT

    if ((status & SSV6XXX_INT_RESOURCE_LOW) == 0)
         return 0;

    for (q_num = (SSV_HW_TXQ_NUM - 1); q_num >= 0; q_num--)
    {
        //==============================================================>
        //DEBUG CODE
        u32 before = jiffies;
        //<==============================================================

        hw_txq = &hctl->hw_txq[q_num];

        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
            getnstimeofday(&tx_io_start_time);
        #endif // CONFIG_SSV6XXX_DEBUGFS

        /* xmit tx frames */
        ret = ssv6xxx_hci_tx_handler(hctl, hw_txq, 999);
        if (ret < 0) {
			HCI_DBG_PRINT(hctl, "TX Handler failed.\n");
            break;
        } else
        	tx_count += ret;

        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
        {
            getnstimeofday(&tx_io_end_time);

            tx_io_diff_time = timespec_sub(tx_io_end_time, tx_io_start_time);
            hctl->isr_tx_io_time += timespec_to_ns(&tx_io_diff_time);
        }
        #endif // CONFIG_SSV6XXX_DEBUGFS

        //==============================================================>
        //DEBUG CODE
        if (hctl->isr_summary_eable)
        {
            //caculate how many time spend in handling tx frames
            if (hctl->isr_tx_time)
            {
                hctl->isr_tx_time += (jiffies-before);
                hctl->isr_tx_time = hctl->isr_tx_time >>1;
            }
            else
            {
                hctl->isr_tx_time += (jiffies-before);
            }
        }
    }

    // Check if all TX queue empty, then disable interrupt
    mutex_lock(&hctl->hci_mutex);
    spin_lock_irqsave(&hctl->int_lock, flags);


    if (!ssv6xxx_hci_is_frame_send(hctl))
    {
        u32 reg_val;

        hctl->int_mask &= ~SSV6XXX_INT_RESOURCE_LOW;
        reg_val = ~hctl->int_mask;
        spin_unlock_irqrestore(&hctl->int_lock, flags);
        HCI_IRQ_SET_MASK(hctl, reg_val);
    }
    else
    {
        spin_unlock_irqrestore(&hctl->int_lock, flags);
    }
    mutex_unlock(&hctl->hci_mutex);

    return ((ret < 0) ? ret : tx_count);
} // end of - _do_tx -

#else // CONFIG_SSV_TX_LOWTHRESHOLD

static int _do_tx (struct ssv6xxx_hci_ctrl *hctl, u32 status)
{
    int                q_num;
    int                tx_count = 0;
    int                ret;
    #ifdef CONFIG_SSV6XXX_DEBUGFS
    struct timespec    tx_io_start_time, tx_io_end_time, tx_io_diff_time;
    #endif // CONFIG_SSV6XXX_DEBUGFS

    #ifdef CONFIG_IRQ_DEBUG_COUNT
    if ((!(status & SSV6XXX_INT_RX)) && hctl->irq_enable)
        htcl->tx_irq_count++;
    #endif // CONFIG_IRQ_DEBUG_COUNT

    for (q_num = (SSV_HW_TXQ_NUM - 1); q_num >= 0; q_num--)
    {
        int                 bitno;
        struct ssv_hw_txq  *hw_txq;
        unsigned long       flags;

        //==============================================================>
        //DEBUG CODE
        u32                 before = jiffies;
        //<==============================================================

        hw_txq = &hctl->hw_txq[q_num];
        bitno = ssv6xxx_hci_get_int_bitno(hw_txq->txq_no);

        if ((status & BIT(bitno)) == 0)
            continue;

        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (htcl->isr_mib_enable)
        {
            getnstimeofday(&tx_io_start_time);
        }
        #endif // CONFIG_SSV6XXX_DEBUGFS

        /* xmit tx frames */
        ret = ssv6xxx_hci_tx_handler(hctl, hw_txq, 999);
        if (ret < 0) {
			HCI_DBG_PRINT(hci_ctrl, "TX handler failed.\n");
            break;
        } else
        	tx_count += ret;

        /*check if need to disable interrupt */
        mutex_lock(&hctl->hci_mutex);
        spin_lock_irqsave(&hctl->int_lock, flags);

        if (skb_queue_len(&hw_txq->qhead) <= 0)
        {
            u32 reg_val;

            hctl->int_mask &= ~(1<<bitno);
            reg_val = ~hctl->int_mask;
            spin_unlock_irqrestore(&hctl->int_lock, flags);

            HCI_IRQ_SET_MASK(hctl, reg_val);
        }
        else
        {
            spin_unlock_irqrestore(&hctl->int_lock, flags);
        }

        mutex_unlock(&hctl->hci_mutex);

        #ifdef CONFIG_SSV6XXX_DEBUGFS
        if (htcl->isr_mib_enable)
        {
            getnstimeofday(&tx_io_end_time);

            tx_io_diff_time = timespec_sub(tx_io_end_time, tx_io_start_time);
            htcl->isr_tx_io_time += timespec_to_ns(&tx_io_diff_time);
        }
        #endif // CONFIG_SSV6XXX_DEBUGFS

        //==============================================================>
        //DEBUG CODE

        //caculate how many time spend in handling tx frames
        if (htcl->isr_summary_eable)
        {
            if (htcl->isr_tx_time)
            {
                htcl->isr_tx_time += (jiffies - before);
                htcl->isr_tx_time = htcl->isr_tx_time >>1;
            }
            else
            {
                htcl->isr_tx_time += (jiffies - before);
            }
        }
    }

    return ((ret < 0) ? ret : tx_count);
} // end of - _do_tx -
#endif // CONFIG_SSV_TX_LOWTHRESHOLD

static void ssv6xxx_hci_isr_reset(struct work_struct *work)
{
    struct ssv6xxx_hci_ctrl *ctrl_hci;

    ctrl_hci = container_of(work, struct ssv6xxx_hci_ctrl, isr_reset_work);
    
	HCI_DBG_PRINT(ctrl_hci, "ISR Reset!!!");
	ssv6xxx_hci_irq_disable(ctrl_hci);
    ssv6xxx_hci_irq_enable(ctrl_hci);
}

irqreturn_t ssv6xxx_hci_isr(int irq, void *args)
{
    struct ssv6xxx_hci_ctrl *hctl = args;
    u32                      status;
    unsigned long            flags;
    int                      ret = IRQ_HANDLED;
	//==============================================================>
	//DEBUG CODE
	bool                     dbg_isr_miss = true;
	bool 					 isr_reset = false;
	bool                     first_time_check = true;

    if (hctl->isr_summary_eable
        && hctl->prev_isr_jiffes){

        if(hctl->isr_idle_time){
            hctl->isr_idle_time += (jiffies - hctl->prev_isr_jiffes);
            hctl->isr_idle_time = hctl->isr_idle_time >>1;
        }
        else{
            hctl->isr_idle_time += (jiffies - hctl->prev_isr_jiffes);
        }
    }
	//<==============================================================
    
    BUG_ON(!args);
#ifdef CONFIG_IRQ_DEBUG_COUNT
	if(hctl->irq_enable)
        hctl->irq_count++;
#endif    
    do {
#ifdef CONFIG_SSV6XXX_DEBUGFS
        struct timespec  start_time, end_time, diff_time;

        memset(&start_time, 0 , sizeof(struct timespec));
        if (hctl->isr_mib_reset)
            ssv6xxx_isr_mib_reset(hctl);

        if (hctl->isr_mib_enable)
            getnstimeofday(&start_time);
#endif // CONFIG_SSV6XXX_DEBUGFS

        mutex_lock(&hctl->hci_mutex);
        // Check if wake up by HCI. Enable the corresponding INT before reading status.
        if (hctl->int_status)
        {
            u32 regval;

            spin_lock_irqsave(&hctl->int_lock, flags);
            hctl->int_mask |= hctl->int_status;
            hctl->int_status = 0;
            regval = ~hctl->int_mask;
            smp_mb();
            spin_unlock_irqrestore(&hctl->int_lock, flags);

            HCI_IRQ_SET_MASK(hctl, regval);
        }
        // Get INT status from device
        ret = HCI_IRQ_STATUS(hctl, &status);
        if ((ret < 0) || ((status & hctl->int_mask) == 0)) {
            if (first_time_check){
#ifdef CONFIG_IRQ_DEBUG_COUNT
                if (hctl->irq_enable)
                    hctl->invalid_irq_count++;
#endif
                //printk("get irq status[%d] status[0x%08x]\n", ret, status);
                //WARN_ON(1);                      
                //spin_unlock_irqrestore(&hctl->int_lock, flags);
                ret = IRQ_NONE;
            }
            mutex_unlock(&hctl->hci_mutex);

            break;
        }
        //else
            //printk("get irq status[%d] status[0x%08x]\n", ret, status);
        spin_lock_irqsave(&hctl->int_lock, flags);
        status &= hctl->int_mask;
        spin_unlock_irqrestore(&hctl->int_lock, flags);
        mutex_unlock(&hctl->hci_mutex);
        
        hctl->isr_running = 1;

        /* handle TX interrupt */
#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
        if (status & SSV6XXX_INT_RESOURCE_LOW) {
#else
        if (status & (SSV6XXX_INT_TX|SSV6XXX_INT_LOW_EDCA_0|SSV6XXX_INT_LOW_EDCA_1|SSV6XXX_INT_LOW_EDCA_2|SSV6XXX_INT_LOW_EDCA_3)) {
#endif
            ret = _do_tx(hctl, status);
            if (ret > 0)
            {
                dbg_isr_miss = false;
            } else if (ret < 0) {
                isr_reset = true;
                hctl->isr_running = 0;
                break;
            }
        }

        /* handle Rx interrupt */
        if (status & SSV6XXX_INT_RX) {
            ret = _isr_do_rx(hctl, status);
            if (ret < 0) {
                isr_reset = true;
                hctl->isr_running = 0;
                HCI_DBG_PRINT(hctl, "do_rx failed\n");
                break;
            }
            dbg_isr_miss = false;
        } else {
            //Because only RX needs CPU very much, if no RX, give up CPU to avoid ksdioirqd busy loop.
            hctl->isr_running = 0;
            break;
        }
       
        hctl->isr_running = 0;
#ifdef CONFIG_SSV6XXX_DEBUGFS
        if (hctl->isr_mib_enable)
        {
            getnstimeofday(&end_time);

            diff_time = timespec_sub(end_time, start_time);
            hctl->isr_total_time += timespec_to_ns(&diff_time);
        }
#endif // CONFIG_SSV6XXX_DEBUGFS

        first_time_check = false;
    } while (1);

    //==============================================================>
    //DEBUG CODE
    if (hctl->isr_summary_eable ) {
        if(dbg_isr_miss)
            hctl->isr_miss_cnt++;

        hctl->prev_isr_jiffes = jiffies;
	}
    //<==============================================================
   	if (isr_reset == true) 
    	queue_work(hctl->hci_work_queue, &hctl->isr_reset_work);
    
    return ret;
}

static int ssv6xxx_hci_tx_task (void *data)
{
#define MAX_HCI_TX_TASK_SEND_FAIL       3
	struct ssv6xxx_hci_ctrl *hctl = (struct ssv6xxx_hci_ctrl *)data;
	unsigned long     wait_period = msecs_to_jiffies(200); 
	int err = 0, err_cnt = 0;

	txrxboost_init();
	printk("SSV6XXX HCI TX Task started.\n");

	while (!kthread_should_stop())
	{
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible_timeout(hctl->tx_wait_q,
									(  kthread_should_stop()
									|| ssv6xxx_hci_is_frame_send(hctl)),
                                    wait_period);

		if (kthread_should_stop())
		{
			hctl->hci_tx_task = NULL;
			HCI_DBG_PRINT(hctl, "Quit HCI TX task loop...\n");
			break;
		}
		set_current_state(TASK_RUNNING);

    txrxboost_change((u32)atomic_read(&SSV_SC(hctl)->ampdu_tx_frame),
			SSV_SC(hctl)->sh->cfg.txrxboost_low_threshold,
			SSV_SC(hctl)->sh->cfg.txrxboost_high_threshold,
			SSV_SC(hctl)->sh->cfg.txrxboost_prio);

        if ((hctl->hci_flags & SSV6XXX_HCI_OP_INVALID) ||
            (hctl->hci_flags & SSV6XXX_HCI_OP_IFERR) ||
            (err_cnt > MAX_HCI_TX_TASK_SEND_FAIL)) { 
                ssv6xxx_hci_txq_flush(hctl, (TXQ_EDCA_0|TXQ_EDCA_1|TXQ_EDCA_2|TXQ_EDCA_3|TXQ_MGMT));
                err_cnt = 0;
        } else { 
            if (ssv6xxx_hci_is_frame_send(hctl))
            {
                err = 0;
                _do_force_tx(hctl, &err);
                if ((err < 0) && (err != -1)) {
                    err_cnt++;
                    /* 
                    * If error, wait for next wake up.
                    * It should include the all error case, 
                    */
                } else if (err == -1) {
                    /* the error code means that hwif is disconnnect */
                    hctl->hci_flags |= SSV6XXX_HCI_OP_IFERR;
                }
                /* 
                 * err 0, success
                 * err = 1, hci stop or hwq paused
                 * err = 2, no hw resource
                 * It should wait the next round to send frame*/
            }
        }
    }

    return 0;
}

static struct ssv6xxx_hci_ops hci_ops = 
{
    .hci_start            = ssv6xxx_hci_start,
    .hci_stop             = ssv6xxx_hci_stop,
    .hci_write_hw_config  = ssv6xxx_hci_write_hw_config,
    .hci_read_word        = ssv6xxx_hci_read_word,
    .hci_write_word       = ssv6xxx_hci_write_word,
    .hci_safe_read_word   = ssv6xxx_hci_safe_read_word,
    .hci_safe_write_word  = ssv6xxx_hci_safe_write_word,
    .hci_burst_read_word  = ssv6xxx_hci_burst_read_word,
    .hci_burst_write_word = ssv6xxx_hci_burst_write_word,    
    .hci_burst_safe_read_word  = ssv6xxx_hci_burst_safe_read_word,
    .hci_burst_safe_write_word = ssv6xxx_hci_burst_safe_write_word,    
    .hci_tx               = ssv6xxx_hci_enqueue,
    .hci_tx_pause         = ssv6xxx_hci_txq_pause,
    .hci_tx_resume        = ssv6xxx_hci_txq_resume,
    .hci_txq_flush        = ssv6xxx_hci_txq_flush,
    .hci_txq_flush_by_sta = ssv6xxx_hci_txq_flush_by_sta,
    .hci_txq_empty        = ssv6xxx_hci_is_txq_empty,
    .hci_load_fw          = ssv6xxx_hci_load_fw,
    .hci_pmu_wakeup       = ssv6xxx_hci_pmu_wakeup,
    .hci_send_cmd         = ssv6xxx_hci_send_cmd,
    .hci_write_sram       = ssv6xxx_hci_write_sram,
#ifdef CONFIG_SSV6XXX_DEBUGFS
    .hci_init_debugfs     = ssv6xxx_hci_init_debugfs,
    .hci_deinit_debugfs   = ssv6xxx_hci_deinit_debugfs,
#endif
    .hci_interface_reset  = ssv6xxx_hci_interface_reset,
    .hci_sysplf_reset     = ssv6xxx_hci_sysplf_reset,
};


int ssv6xxx_hci_deregister(struct ssv6xxx_hci_info *shi)
{
    u32 regval;
    struct ssv6xxx_hci_ctrl *hci_ctrl;

    /**
        * Wait here until there is no frame on the hardware. Before
        * call this function, the RF shall be turned off to make sure
        * no more incoming frames. This function also disable interrupt
        * once no more frames.
        */
    printk("%s(): \n", __FUNCTION__);


    if (shi->hci_ctrl == NULL)
        return -1;
    
    hci_ctrl = shi->hci_ctrl;
    hci_ctrl->hci_flags |= SSV6XXX_HCI_OP_INVALID;
    regval = 1;
//    sido module may release before.
//    /* check mcu/hci/fragment/security/mrx/mic */
//    while(!regval) {
//        if(ctrl_hci->shi->if_ops->readreg(
//        ctrl_hci->shi->dev, ADR_RD_FFOUT_CNT1, 
//        &regval));
//        mdelay(10);
//    };
    // flush txq packet 
    ssv6xxx_hci_txq_flush(hci_ctrl, (TXQ_EDCA_0|TXQ_EDCA_1|TXQ_EDCA_2|TXQ_EDCA_3|TXQ_MGMT));
    ssv6xxx_hci_irq_disable(hci_ctrl);
    flush_workqueue(hci_ctrl->hci_work_queue);
    destroy_workqueue(hci_ctrl->hci_work_queue);
    if (hci_ctrl->hci_tx_task != NULL)
	{
		printk("Stopping HCI TX task...\n");
		kthread_stop(hci_ctrl->hci_tx_task);
		hci_ctrl->hci_tx_task = NULL;
		printk("Stopped HCI TX task.\n");
	}
	shi->hci_ctrl = NULL;
	
	kfree(hci_ctrl);

    return 0;
}
EXPORT_SYMBOL(ssv6xxx_hci_deregister);


int ssv6xxx_hci_register(struct ssv6xxx_hci_info *shi)
{
    int i, capability;
    struct ssv6xxx_hci_ctrl *hci_ctrl;
	    
    if (shi == NULL/* || ctrl_hci->shi*/) {
        printk(KERN_ERR "NULL sh when register HCI.\n");
        return -1;
    }

    hci_ctrl = kzalloc(sizeof(*hci_ctrl), GFP_KERNEL);
    if (hci_ctrl == NULL)
        return -ENOMEM;

    memset((void *)hci_ctrl, 0, sizeof(*hci_ctrl));

    /* HCI & hw/sw mac interface binding */
    shi->hci_ctrl = hci_ctrl;
    shi->hci_ops = &hci_ops;
    hci_ctrl->shi = shi;
    
    hci_ctrl->rx_buf = shi->skb_alloc((void *)(SSV_SC(hci_ctrl)), MAX_HCI_RX_AGGR_SIZE);
    if (hci_ctrl->rx_buf == NULL) {
        kfree(hci_ctrl);
        return -ENOMEM;
    }
   
    hci_ctrl->txq_mask = 0;
    mutex_init(&hci_ctrl->txq_mask_lock);
    mutex_init(&hci_ctrl->hci_mutex);

    spin_lock_init(&hci_ctrl->int_lock);
#ifdef CONFIG_IRQ_DEBUG_COUNT
    hci_ctrl->irq_enable = false;
    hci_ctrl->irq_count = 0;
    hci_ctrl->invalid_irq_count = 0;
    hci_ctrl->tx_irq_count = 0;
    hci_ctrl->real_tx_irq_count = 0;
    hci_ctrl->rx_irq_count = 0;
    hci_ctrl->irq_rx_pkt_count = 0;
    hci_ctrl->irq_tx_pkt_count = 0;
#endif
    /* TX queue initialization */
    for (i=0; i < SSV_HW_TXQ_NUM; i++) {
        memset(&hci_ctrl->hw_txq[i], 0, sizeof(struct ssv_hw_txq));
        skb_queue_head_init(&hci_ctrl->hw_txq[i].qhead);
        //spin_lock_init(&ctrl_hci->hw_txq[i].txq_lock);
        hci_ctrl->hw_txq[i].txq_no = (u32)i;
        hci_ctrl->hw_txq[i].max_qsize = SSV_HW_TXQ_MAX_SIZE;
        hci_ctrl->hw_txq[i].resum_thres = SSV_HW_TXQ_RESUME_THRES;
    }

    hci_ctrl->hci_work_queue = create_singlethread_workqueue("ssv6xxx_hci_wq");
	
	INIT_WORK(&hci_ctrl->isr_reset_work, ssv6xxx_hci_isr_reset);
    INIT_WORK(&hci_ctrl->hci_rx_work, ssv6xxx_hci_rx_work);
#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
	INIT_WORK(&hci_ctrl->hci_tx_work, ssv6xxx_hci_tx_work);
#else
	for(i=0; i<SSV_HW_TXQ_NUM; i++)
		INIT_WORK(&hci_ctrl->hci_tx_work[i], ssv6xxx_hci_tx_work);
#endif // CONFIG_SSV_TX_LOWTHRESHOLD
	capability = (HCI_HWIF_PROPERTY(hci_ctrl) & SSV_HWIF_CAPABILITY_MASK);
	switch (capability) {
		case SSV_HWIF_CAPABILITY_INTERRUPT:
			/* 
		 	 * For SDIO interface, we hold the interrupt routine to send/receive the frame.
		 	 */
		#ifdef CONFIG_SSV_TX_LOWTHRESHOLD
   			hci_ctrl->int_mask = SSV6XXX_INT_RX|SSV6XXX_INT_RESOURCE_LOW;
		#else
    		hci_ctrl->int_mask = SSV6XXX_INT_RX|SSV6XXX_INT_TX|SSV6XXX_INT_LOW_EDCA_0|
    			SSV6XXX_INT_LOW_EDCA_1|SSV6XXX_INT_LOW_EDCA_2|SSV6XXX_INT_LOW_EDCA_3;
		#endif // CONFIG_SSV_TX_LOWTHRESHOLD
			hci_ctrl->isr_disable = false;
    		hci_ctrl->int_status= 0;
    		HCI_IRQ_SET_MASK(hci_ctrl, 0xFFFFFFFF);
    		ssv6xxx_hci_irq_disable(hci_ctrl);
    		HCI_IRQ_REQUEST(hci_ctrl, ssv6xxx_hci_isr);
			break;

		case SSV_HWIF_CAPABILITY_POLLING:
			/* 
		 	 * For USB interface, we hold the tx/rx thread to send/receive the frame.
		 	 */
			hci_ctrl->isr_disable = true;
			init_waitqueue_head(&hci_ctrl->tx_wait_q);
			hci_ctrl->hci_tx_task = kthread_run(ssv6xxx_hci_tx_task, hci_ctrl, "ssv6xxx_hci_tx_task");
			HCI_RX_TASK(hci_ctrl, hci_ctrl->shi->hci_rx_cb, hci_ctrl->shi->hci_is_rx_q_full, (void *)(SSV_SC(hci_ctrl)), &hci_ctrl->rx_pkt);
			break;
		default:
            printk("Detect unknown hardware capability\n");
			return -1;	
	}
	#ifdef CONFIG_SSV6XXX_DEBUGFS
	hci_ctrl->debugfs_dir = NULL;
	hci_ctrl->isr_mib_enable = false;
    hci_ctrl->isr_mib_reset = 0;
    hci_ctrl->isr_total_time = 0;
    hci_ctrl->isr_rx_io_time = 0;
    hci_ctrl->isr_tx_io_time = 0;
    hci_ctrl->isr_rx_io_count = 0;
    hci_ctrl->isr_tx_io_count = 0;
    hci_ctrl->isr_rx_proc_time =0;
    #endif // CONFIG_SSV6XXX_DEBUGFS
    return 0;
}
EXPORT_SYMBOL(ssv6xxx_hci_register);











#if (defined(CONFIG_SSV_SUPPORT_ANDROID)||defined(CONFIG_SSV_BUILD_AS_ONE_KO))
int ssv6xxx_hci_init(void)
#else
static int __init ssv6xxx_hci_init(void)
#endif
{
#ifdef CONFIG_SSV6200_CLI_ENABLE
    //extern struct ssv6xxx_hci_ctrl *ssv_dbg_ctrl_hci;
#endif
#ifdef CONFIG_SSV6200_CLI_ENABLE
    //ssv_dbg_ctrl_hci = ctrl_hci;
#endif
    return 0;
}

#if (defined(CONFIG_SSV_SUPPORT_ANDROID)||defined(CONFIG_SSV_BUILD_AS_ONE_KO))
void ssv6xxx_hci_exit(void)
#else
static void __exit ssv6xxx_hci_exit(void)
#endif
{
#ifdef CONFIG_SSV6200_CLI_ENABLE
    //extern struct ssv6xxx_hci_ctrl *ssv_dbg_ctrl_hci;
#endif
    /*
    kfree(ctrl_hci);    
    ctrl_hci = NULL;
    */
#ifdef CONFIG_SSV6200_CLI_ENABLE
    //ssv_dbg_ctrl_hci = NULL;
#endif   
}


#if (!defined(CONFIG_SSV_SUPPORT_ANDROID) && !defined(CONFIG_SSV_BUILD_AS_ONE_KO))
module_init(ssv6xxx_hci_init);
module_exit(ssv6xxx_hci_exit);
#endif




