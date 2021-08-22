#ifndef _DRV_USB_H_
#define _DRV_USB_H_

//USB speed
#define HSUSB  0
#define FSUSB  1

#define FS_BULK_EP_MAX_PKT_SZ       (0x40)

//Bit define
#define BIT0   0x01
#define BIT1   0x02
#define BIT2   0x04
#define BIT3   0x08
#define BIT4   0x10
#define BIT5   0x20
#define BIT6   0x40
#define BIT15  0x8000  //(1 << 15)
#define BIT16  0x10000 //(1 << 16)
#define BIT17  0x20000 //(1 << 17)

#define REQLENGTH(x)    ((x)<<16)
#define TXBUFFSEL(x)    ((x)<<8)
#define DMA_IDLE        (0x0)
#define DMA_OUT_EN      ((0x1)<<4)
#define DMA_IN_EN       ((0x2)<<4)
#define RXZEROBIRQ_EN   (1<<3)
#define IN_PREBUFFING   (0x1)
#define IN_POSTBUFFING  (0x0)

//USB register
#define	USB2_BASE                   (0x70004000)

//USB control register
#define	ADR_USB2_ACR                (USB2_BASE+0x00000000)
#define	ADR_USB2_MDAR               (USB2_BASE+0x00000004)
#define	ADR_USB2_UDCR               (USB2_BASE+0x00000008)
#define	ADR_USB2_PRIR               (USB2_BASE+0x00000014)
#define	ADR_USB2_TX_BCR1            (USB2_BASE+0x00000034)
#define	ADR_USB2_TX_BCR2            (USB2_BASE+0x00000038)
#define	ADR_USB2_IER                (USB2_BASE+0x00000050)
#define	ADR_USB2_ISR                (USB2_BASE+0x00000058)

//USB ACC register
#define	ADR_USB2_ACCR               (USB2_BASE+0x000001AC)

//USB to HCI register
#define	ADR_USB2HCI_REG0            (USB2_BASE+0x000001F0)
#define	ADR_USB2HCI_REG1            (USB2_BASE+0x000001F4)
#define	ADR_USB2HCI_REG2            (USB2_BASE+0x000001F8)
#define	ADR_HCI_DATA_PORT_ADDR      (0x7FFF0000)

//0x70004000
#define GET_ADR_USB2_ACR            (REG32(ADR_USB2_ACR))
#define SET_ADR_USB2_ACR(_VAL)      (REG32(ADR_USB2_ACR)) = (_VAL)
//0x70004004
#define GET_ADR_USB2_MDAR           (REG32(ADR_USB2_MDAR))
#define SET_ADR_USB2_MDAR(_VAL)     (REG32(ADR_USB2_MDAR)) = (_VAL)
//0x70004008
#define GET_ADR_USB2_UDCR           (REG32(ADR_USB2_UDCR))
#define SET_ADR_USB2_UDCR(_VAL)     (REG32(ADR_USB2_UDCR)) = (_VAL)
//0x70004014
#define GET_ADR_USB2_PRIR           (REG32(ADR_USB2_PRIR))
#define SET_ADR_USB2_PRIR(_VAL)     (REG32(ADR_USB2_PRIR)) = (_VAL)
//0x70004034
#define GET_ADR_USB2_TX_BCR1        (REG32(ADR_USB2_TX_BCR1))
#define SET_ADR_USB2_TX_BCR1(_VAL)  (REG32(ADR_USB2_TX_BCR1)) = (_VAL)
//0x70004038
#define GET_ADR_USB2_TX_BCR2        (REG32(ADR_USB2_TX_BCR2))
#define SET_ADR_USB2_TX_BCR2(_VAL)  (REG32(ADR_USB2_TX_BCR2)) = (_VAL)
//0x70004050
#define GET_ADR_USB2_IER            (REG32(ADR_USB2_IER))
#define SET_ADR_USB2_IER(_VAL)      (REG32(ADR_USB2_IER)) = (_VAL)
//0x70004058
#define GET_ADR_USB2_ISR            (REG32(ADR_USB2_ISR))
#define SET_ADR_USB2_ISR(_VAL)      (REG32(ADR_USB2_ISR)) = (_VAL)

//0x700041AC
#define GET_ADR_USB2_ACCR           (REG32(ADR_USB2_ACCR))
#define SET_ADR_USB2_ACCR(_VAL)     (REG32(ADR_USB2_ACCR)) = (_VAL)

//0x700041F0
#define GET_ADR_USB2HCI_REG0        (REG32(ADR_USB2HCI_REG0))
#define SET_ADR_USB2HCI_REG0(_VAL)  (REG32(ADR_USB2HCI_REG0)) = (_VAL)

//0x700041F4
#define GET_ADR_USB2HCI_REG1        (REG32(ADR_USB2HCI_REG1))
#define SET_ADR_USB2HCI_REG1(_VAL)  (REG32(ADR_USB2HCI_REG1)) = (_VAL)

// PRIr (RO)
#define CUREP_NUM                   ((GET_ADR_USB2_PRIR & 0xff0000) >> 16)
#define BYTECNT_CTN                 (GET_ADR_USB2_PRIR & 0x7ff)

// ACr
enum tx_buff {
    TXBUFF0		= 0,
    TXBUFF1		= 1,
    TXBUFF2		= 2,
    TXBUFF3		= 3,
    TXBUFF4		= 4,
    TXBUFF5		= 5,
    TXBUFF6		= 6,
    TXBUFF7		= 7,
    TXBUFF8		= 8
};

#define NOTIFY_HCI_FIRST_PKT()       SET_ADR_USB2HCI_REG0(GET_ADR_USB2HCI_REG0 |= BIT0)
#define NOTIFY_HCI_LAST_PKT()        SET_ADR_USB2HCI_REG0(GET_ADR_USB2HCI_REG0 |= BIT1)

#define GET_HCI_IN_DATA_LEN()        (GET_ADR_USB2HCI_REG1 & 0x7FFF) 
//0:HCI I/F not yet update the length, 1: HCI I/F update the length. 
//Write 1 to clear the register after read the status.
#define GET_HCI_UPDATE_LEN_STATE()   (GET_ADR_USB2HCI_REG1 & BIT16) >>16
#define CLEAR_HCI_UPDATE_LEN_STATE() (REG32(ADR_USB2HCI_REG1) |= BIT16)
#define FORCE_HCI_DATA_IN()          (REG32(ADR_USB2HCI_REG1) |= BIT17)

enum intr_usb_sts {
	/* Normal EP Evt */
	PEP8XFEREVT      = (1<<31),
	PEP7XFEREVT      = (1<<30),
	PEP6XFEREVT      = (1<<29),
	PEP5XFEREVT      = (1<<28),
	PEP4XFEREVT      = (1<<27),
	PEP3XFEREVT      = (1<<26),
	PEP2XFEREVT      = (1<<25),
	PEP1XFEREVT      = (1<<24),
	/* EP0 Evt */
	EP0QUERYEVT      = (1<<19),
	EP0INXFEREVT     = (1<<18),
	EP0OUTXFEREVT    = (1<<17),
	EP0SETUPEVT      = (1<<16),
	/* USB Command Evt */
	SETCONFIGEVT     = (1<<15),
	SETINTERFACEEVT  = (1<<14),
	/* Xfer Evt */
	DMAERREVT        = (1<<9),
	DMADONEEVT       = (1<<8),
	/* USB Evt */
	SOFEVT           = (1<<5),
	RESUMEEVT        = (1<<4),
	SUSPENDEVT       = (1<<3),
	RSTEVT           = (1<<2),
	DISCONNEVT       = (1<<1),
	CONNECTEVT       = 1
};

// TX buffer config
#define TxBuff_PreINQueueMsk    (0x7<<12) // Mask for b[12...14]

// I-comm Host Command 
#define USB_CMD_WRITE_REG     0x01
#define USB_CMD_READ_REG      0x02

#define USB_CMD_SET_TEST_FRM  0xF0

struct ssv6200_read_reg {
    volatile u32 *regs[0];
    u32 reserved;
}__attribute__ ((packed));

struct ssv6200_read_reg_result {
    u32 vals[0];
}__attribute__ ((packed));

struct ssv6200_write_reg {
    struct {
        volatile u32 *addr;
        u32 val;
    } regs[0];
}__attribute__ ((packed));

//Test command
struct ssv6200_set_test_frm {
    u32 len;
}__attribute__ ((packed));

struct command_hdr {
    u8  len;      //command body length
    u8  cmd;      //command type
    u16 padding;  //command sequence number
}__attribute__ ((packed));

struct __icomm_host_cmd {
    struct command_hdr hdr;
    union {
        struct ssv6200_read_reg         rreg;
        struct ssv6200_read_reg_result  rreg_res;
        struct ssv6200_write_reg        wreg;
        struct ssv6200_set_test_frm     test_frm;
    };
}__attribute__ ((packed));

s32 drv_usb_init(void);

#endif /* _DRV_USB_H_ */

