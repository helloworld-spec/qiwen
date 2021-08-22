/**
 * @file interrupt.c
 * @brief interrupt function file
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#include "interrupt.h" 
#include "drv_api.h"

#define MAX_STACK_DEPTH         (10)

#define MAX_REGISTER_INTR_NUM   (64)
#define MAX_INTR_HANDLER_NUM    (16)
#define INTR_NBITS              (3)

static volatile unsigned long gb_interrupt_val[MAX_STACK_DEPTH] = {0};
static volatile unsigned long gb_fastinterrupt_val[MAX_STACK_DEPTH] = {0};
static volatile unsigned long   ucStackTop = 0;
static volatile unsigned long   ucStackTop_fiq = 0;

static volatile unsigned char registered_irqs[MAX_REGISTER_INTR_NUM]={0xff};
static volatile unsigned char registered_fiqs[MAX_REGISTER_INTR_NUM]={0xff};

typedef struct _tagint
{
    T_INTR_HANDLER 	ptr;
    struct _tagint 	*next;
    INT_VECTOR     	v;

	char 			name[8]; //irq name
	unsigned long 	cnt; //irq occured count
}T_INTR_NODE;

static volatile T_INTR_NODE intr_pointers[INTR_NBITS*MAX_INTR_HANDLER_NUM]={0};

/**
* @brief store the irq mask to global variable and disable all irq interrupt
* @author liao_zhijun
* @date 2010-06-18
* @return void
*/
void store_all_int(void)
{
    unsigned long mask_reg;

    if (ucStackTop >= MAX_STACK_DEPTH)
    {
        akprintf(C3, M_DRVSYS, "-->store_all_int(): interrupt stack overfollow, ucStackTop=%d\r\n", ucStackTop);
        while(1);
    }

    irq_mask();
    
    mask_reg = REG32(IRQINT_MASK_REG);
    REG32(IRQINT_MASK_REG) = 0x0;

    gb_interrupt_val[ucStackTop] = mask_reg;

    ucStackTop++;

    irq_unmask();
}

/**
* @brief recover irq interrupt from last store_all_int call
* @author liao_zhijun
* @date 2010-06-18
* @return void
*/
void restore_all_int(void)
{
    if (ucStackTop == 0)
    {
        akprintf(C3, M_DRVSYS, "-->restore_all_int(): Reach interrupt stack bottom\r\n");
        while(1);
    }

    irq_mask();

    ucStackTop--;
    REG32(IRQINT_MASK_REG) = gb_interrupt_val[ucStackTop];

    irq_unmask();
}

/* 
map to real vector by different chips 
the vector is the status bits location
*/
static unsigned char map2real_vector(INT_VECTOR v)
{
    const unsigned char vector_table[INT_VECTOR_MAX - INT_VECTOR_MIN] = {
        0/*mem*/, 1/*camera*/, 
        2/*VEDIO_ENCODE*/, 4/*MMCSD*/,
        5/*SDIO*/, 6/*adc2*/,

		7/*DAC*/,8/*SPI1*/,
        9/*SPI2*/, 10/*UART1*/,
        11/*UART2*/, 12/*L2*/,
        
        13/*IIC*/, 
        14/*IRDA*/, 15/*GPIO*/,
        16/*MAC*/, 17/*ENCRYPT*/,
        18/*USB*/, 19/*USBDMA*/,
        
        3/*SAR_ADC*/, 3/*TIMER*/,
        3/*WAKEUP_TRIGGER*/, 3/*RTC*/
        };
    return vector_table[v-(INT_VECTOR_MIN + 1)];
}

static char* map2name(INT_VECTOR v)
{
    const char vector_table[INT_VECTOR_MAX - INT_VECTOR_MIN][8] = {
        "mem"/*mem*/, "isp"/*camera*/, 
        "venc"/*VEDIO_ENCODE*/, "mmc"/*MMCSD*/,
        "sdio"/*SDIO*/, "adc2"/*adc2*/,

		"dac"/*DAC*/,"spi1"/*SPI1*/,
        "spi2"/*SPI2*/, "uart1"/*UART1*/,
        "uart2"/*UART2*/, "l2"/*L2*/,
        
        "i2c"/*IIC*/, 
        "irda"/*IRDA*/, "gpio"/*GPIO*/,
        "mac"/*MAC*/, "encryt"/*ENCRYPT*/,
        "usb"/*USB*/, "usbdma"/*USBDMA*/,
        
        "sardac"/*SAR_ADC*/, "timer"/*TIMER*/,
        "wakeup"/*WAKEUP_TRIGGER*/, "rtc"/*RTC*/
        };
    return vector_table[v-(INT_VECTOR_MIN + 1)];

}

/* map to mask bit by different chips */
static unsigned long map2mask_bit(INT_VECTOR v)
{
    const unsigned long mask_table[INT_VECTOR_MAX - INT_VECTOR_MIN] = {
        IRQ_MASK_MEM_BIT/*mem*/, IRQ_MASK_CAMERA_BIT/*camera*/, 
        IRQ_MASK_VENC_BIT/*VIDEO_ENCODE*/, IRQ_MASK_MMCSD_BIT/*MMCSD*/,
        IRQ_MASK_SDIO_BIT/*SDIO*/, IRQ_MASK_ADC_BIT/*adc*/,

		IRQ_MASK_DAC_BIT/*DAC*/,IRQ_MASK_SPI1_BIT/*SPI1*/,
        IRQ_MASK_SPI2_BIT/*SPI2*/, IRQ_MASK_UART1_BIT/*UART1*/,
        IRQ_MASK_UART2_BIT/*UART2*/, IRQ_MASK_L2_BIT/*L2*/,
        
        IRQ_MASK_IIC_BIT/*IIC*/, 
        IRQ_MASK_IRDA_BIT/*IRDA*/, IRQ_MASK_GPIO_BIT/*GPIO*/,
        IRQ_MASK_MAC_BIT/*MAC*/, IRQ_MASK_ENCRYPT_BIT/*ENCRYPT*/,
        IRQ_MASK_USB_BIT/*USB*/, IRQ_MASK_USBDMA_BIT/*USBDMA*/,
        
        IRQ_MASK_SYS_MODULE_BIT/*SAR_ADC*/, IRQ_MASK_SYS_MODULE_BIT/*TIMER*/,
        IRQ_MASK_SYS_MODULE_BIT/*WAKEUP_TRIGGER*/, IRQ_MASK_SYS_MODULE_BIT/*RTC*/

        };

    return mask_table[v-(INT_VECTOR_MIN + 1)];
}

static T_INTR_NODE* check_node(T_INTR_NODE *node, INT_VECTOR v)
{
    T_INTR_NODE *p;

    p = node;
    while(p)
    {
        if (p->v == v)
            break;
        p = p->next;
    }
    return p;
}

static T_INTR_NODE* alloc_node(T_INTR_NODE *head)
{
    unsigned char index;
    T_INTR_NODE *p=NULL, *node; 

    /* find an empty place first */
    for (index=MAX_INTR_HANDLER_NUM; index<MAX_INTR_HANDLER_NUM*INTR_NBITS; index++)
    {
        if (!intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE *)&intr_pointers[index];
            break;
        }
    }
    /* no more empty place */
    if (!p)
        return NULL;
    
    if (head)
    {
        /* there is a head, link the empty node to it */
        node = head;
        while(node->next)
        {
            node = node->next;
        };
        node->next = p;     
    }
    return p;
}

/**
* @brief interrupt init, called before int_register_irq()
* @author liao_zhijun
* @date 2010-06-18
* @return void
*/
void interrupt_init(void)
{
    unsigned long i;
    for (i=0; i<MAX_REGISTER_INTR_NUM; i++)
    {
        registered_irqs[i] = 0xff;
        registered_fiqs[i] = 0xff;
    }
}

/**
* @brief register irq interrupt
* @author liao_zhijun
* @data 2010-06-18
* @return bool
*/
bool int_register_irq(INT_VECTOR v, T_INTR_HANDLER handler)
{
    unsigned char    index, vector;
    unsigned long   mask;
    T_INTR_NODE *p;

    /* check vector valid */
    if (v >= INT_VECTOR_MAX)
    {
        akprintf(C3, M_DRVSYS, "invalid vector %d\n", v);
        return false;
    }
    
    vector = map2real_vector(v);

    if (handler)
    {
        /* check its fiq has registered or not */
        if (registered_fiqs[vector] != 0xff)
        {
            akprintf(C3, M_DRVSYS, "its fiq has regisetered, exit!!\n");
            return false;
        }

        /* register it now */
        if (registered_irqs[vector] != 0xff)
        {
            index = registered_irqs[vector];
            /* check it has registered or not */
            p = check_node((T_INTR_NODE*)&intr_pointers[index], v);
            if (p)
            {
                /* it has registered, just replace the handler*/
                p->ptr = handler;
            }
            else
            {
                /* alloc a node and add it to the tail of list */               
                p = alloc_node((T_INTR_NODE*)&intr_pointers[index]);
                if (p)
                {
                    p->ptr  = handler;
                    p->next = NULL;
                    p->v = v;
                    
                }
                else
                {
                    akprintf(C3, M_DRVSYS, "no more place for registeration, exit!!\n");
                    return false;
                }
            }
        }
        else
        {
            /* find an empty place for it */
            index = 0;
            for (index=0; index<MAX_INTR_HANDLER_NUM; index++)
            {
                if (!intr_pointers[index].ptr)
                    break;
            }
            if (index < MAX_INTR_HANDLER_NUM)
            {
                intr_pointers[index].ptr = handler;
                intr_pointers[index].next= NULL;
                intr_pointers[index].v = v;
				strcpy(intr_pointers[index].name, map2name(v));
				intr_pointers[index].cnt = 0;
                registered_irqs[vector] = index;
            }
            else
            {
                akprintf(C3, M_DRVSYS, "no more place for register, exit1!!\n");
                return false;
            }
        }
        /* enable interrupt */
        INTR_ENABLE(map2mask_bit(v));
    }
    else
    {
        /* unregister it */
        
    }
    return true;
}

void irq_dispatch_handler(unsigned char irq)
{
    unsigned char index;
    T_INTR_NODE *p;
    
    //akprintf(C3, M_DRVSYS, "irq number %d\n", irq);
    
    if (irq >= MAX_REGISTER_INTR_NUM)
    {
        akprintf(C3, M_DRVSYS, "irq number %d is invalid!!\n", irq);
        return;
    }
    
    index = registered_irqs[irq];
    if (index < MAX_INTR_HANDLER_NUM)
    {
        if (intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE*)&intr_pointers[index];
			p->cnt++; //increase irq count
            do
            {
                /* if handled it, exit */
                if ((*(p->ptr))() == true)
                    return;
                p = p->next;
            }while(p);
            /* WRONG: irq not be handled!! */
            if (irq != 27)  /* 27 is gpio */
                akprintf(C3, M_DRVSYS, "irq %d not handled!!\n", irq);
        }
        else
            akprintf(C3, M_DRVSYS, "unregistered irq %d\n", irq);
    }
    else if (irq < INT_STATUS_NBITS)
    {
        akprintf(C3, M_DRVSYS, "irq %d %x not register and it comes!!\n", irq, REG32(IRQINT_MASK_REG));
        while(1);
    }

}

void fiq_dispatch_handler(unsigned char fiq)
{
    unsigned char index;
    T_INTR_NODE *p;

    //akprintf(C3, M_DRVSYS, "fiq number %d\n", fiq);
    if (fiq >= MAX_REGISTER_INTR_NUM)
    {
        akprintf(C3, M_DRVSYS, "fiq number %d is invalid!!\n", fiq);
        return;
    }
    
    index = registered_fiqs[fiq];
    if (index < MAX_INTR_HANDLER_NUM)
    {
        if (intr_pointers[index].ptr)
        {
            p = (T_INTR_NODE*)&intr_pointers[index];
            do
            {
                /* if handled it, exit */
                if ((*(p->ptr))() == true)
                    return;
                p = p->next;
            }while(p);
            /* WRONG: fiq not be handled!! */
            akprintf(C3, M_DRVSYS, "fiq %d not handled!!\n", fiq);
            
        }
        else
            akprintf(C3, M_DRVSYS, "unregistered fiq %d\n", fiq);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "fiq not register and it comes!!\n");
        while(1);
    }

}

void irq_show()
{
	int index;

	akprintf(C3, M_DRVSYS, "[%lu]\r\n", get_tick_count());
	
	for (index=0; index<MAX_INTR_HANDLER_NUM; index++)
	{
		if (intr_pointers[index].ptr)
		{
			akprintf(C3, M_DRVSYS, "%d:\t%d\t%s\n",	map2real_vector(intr_pointers[index].v),
				intr_pointers[index].cnt, intr_pointers[index].name);
		}
	}
}

