/** @file usb.h
 * @brief USB register bit definition
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2010-8-6
 * @version 1.0
 */

#ifndef         __USB_H__
#define         __USB_H__
                                                        
#define USB_EP0_FORBID_WRITE                            (0x1)
#define USB_EP2_FORBID_WRITE                            (0x2)
#define USB_EP4_FORBID_WRITE                            (0x4)
                                                        
#define USB_EP0_START_PRE_READ                          (0x1)
#define USB_EP2_START_PRE_READ                          (0x2)
#define USB_EP4_START_PRE_READ                          (0x4)
                                                        
/** CSR0 bit masks mode-agnostic */                     
#define USB_CSR0_RXPKTRDY                               (1 << 0)
#define USB_CSR0_TXPKTRDY                               (1 << 1)

/** CSR0 bit masks IN SLAVE MODE */
#define USB_CSR0_P_SENTSTALL                            (1 << 2)
#define USB_CSR0_P_DATAEND                              (1 << 3)
#define USB_CSR0_P_SETUPEND                             (1 << 4)
#define USB_CSR0_P_SENDSTALL                            (1 << 5)
#define USB_CSR0_P_SVDRXPKTRDY                          (1 << 6)
#define USB_CSR0_P_SVDSETUPEND                          (1 << 7)

/**  CSR0 bit masks in MASTER mode   */
#define USB_CSR0_H_RXSTALL                              (1 << 2)
#define USB_CSR0_H_SETUPPKT                             (1 << 3)
#define USB_CSR0_H_ERROR                                (1 << 4)
#define USB_CSR0_H_REQPKT                               (1 << 5)
#define USB_CSR0_H_STATUSPKT                            (1 << 6)
#define USB_CSR0_H_NAKTIMEOUT                           (1 << 7)
                                                                                                                
#define USB_CSR0_FLUSHFIFO                              (0x1)
                                                                                                       
/** TXCSR1 bit masks mode-agnostic */                   
#define USB_TXCSR1_TXPKTRDY                             (1 << 0)
#define USB_TXCSR1_FIFONOTEMPTY                         (1 << 1)
#define USB_TXCSR1_FLUSHFIFO                            (1 << 3)
#define USB_TXCSR1_CLRDATATOG                           (1 << 6)
                                                        
/** TXCSR1 bit masks slave mode */                      
#define USB_TXCSR1_P_UNDERRUN                           (1 << 2)
#define USB_TXCSR1_P_SENDSTALL                          (1 << 4)
#define USB_TXCSR1_P_SENTSTALL                          (1 << 5)
                                                        
/** TXCSR1 bit masks MASTER MODE */                     
#define USB_TXCSR1_H_ERROR                              (0x04)
#define USB_TXCSR1_H_RXSTALL                            (0x20)
                                                        
/** RXCSR1 bit masks mode-agnostic */                   
#define USB_RXCSR1_RXPKTRDY                             (1 << 0)
#define USB_RXCSR1_FIFOFULL                             (1 << 1)
#define USB_RXCSR1_NAKTIMEOUT                           (1 << 3)
#define USB_RXCSR1_FLUSHFIFO                            (1 << 4)
#define USB_RXCSR1_CLRDATATOG                           (1 << 7)
                                                        
/** RXCSR1 bit masks slave mode */                      
#define USB_RXCSR1_P_OVERRUN                            (1 << 2) //0x04
#define USB_RXCSR1_P_SENDSTALL                          (1 << 5) //0x20
#define USB_RXCSR1_P_SENTSTALL                          (1 << 6) //0x40
                                                        
/** RXCSR1 bit masks MASTER MODE */                     
#define USB_RXCSR1_H_ERROR                              (1 << 2) //0x04
#define USB_RXCSR1_H_REQPKT                             (1 << 5) //0x20
#define USB_RXCSR1_H_RXSTALL                            (1 << 6) //0x40

/** TXCSR2 bit masks** THERE IS NO DIFFERENCE FOR MASTER AND SLAVE MODE */
#define USB_TXCSR2_FRCDT                                (1 << 3)
#define USB_TXCSR2_DMAMODE                              (1 << 2)
#define USB_TXCSR2_DMAENAB                              (1 << 4)
#define USB_TXCSR2_MODE                                 (1 << 5)
#define USB_TXCSR2_ISO                                  (1 << 6)
#define USB_TXCSR2_AUTOSET                              (1 << 7)
                                                        
/** RXCSR2 bit masks */                                 
#define USB_RXCSR2_AUTOCLEAR                            (1 << 7)
#define USB_RXCSR2_AUTOREQ                              (1 << 6)
#define USB_RXCSR2_DMAENAB                              (1 << 5)
#define USB_RXCSR2_DISNYET                              (1 << 4)
#define USB_RXCSR2_DMAMODE                              (1 << 3)
#define USB_RXCSR2_INCOMPRX                             (1 << 0)
                                                                                       
/** POWER REGISTER  */                                  
#define USB_POWER_ENSUSPEND                             (1 << 0)
#define USB_POWER_SUSPENDM                              (1 << 1)
#define USB_POWER_RESUME                                (1 << 2)
#define USB_POWER_RESET                                 (1 << 3)
#define USB_POWER_HSMODE                                (1 << 4)
#define USB_POWER_HSENABLE                              (1 << 5)
#define USB_POWER_SOFTCONN                              (1 << 6)
#define USB_POWER_ISOUP                                 (1 << 7)
                                                                                                    
/** EPn ENABLE */                                       
#define USB_EP0_ENABLE                                  (1 << 0)
#define USB_EP2_TX_ENABLE                               (1 << 2)
#define USB_EP4_TX_ENABLE                               (1 << 4)
                                                        
#define USB_EP1_RX_ENABLE                               (1 << 1)
#define USB_EP3_RX_ENABLE                               (1 << 3)
#define USB_EP4_RX_ENABLE                               (1 << 4)
                                                                                                                
#define USB_EP0_INDEX                                   (0)
#define USB_EP1_INDEX                                   (1 << 0)
#define USB_EP2_INDEX                                   (1 << 1)
#define USB_EP3_INDEX                                   ((1 << 1)|(1 << 0))
#define USB_EP4_INDEX                                   (1 << 2)
#define USB_EP5_INDEX                                   ((1 << 2)|(1 << 0))
#define USB_EP6_INDEX                                   ((1 << 2)|(1 << 1))
#define USB_EP7_INDEX                                   ((1 << 2)|(1 << 1)|(1 << 0))

/** EP INTERRUPT in INTR register */
#define USB_INTR_EP0                                    (1 << 0)
#define USB_INTR_EP1                                    (1 << 1)
#define USB_INTR_EP2                                    (1 << 2)
#define USB_INTR_EP3                                    (1 << 3)
#define USB_INTR_EP4                                    (1 << 4)
#define USB_INTR_EP5                                    (1 << 5)
#define USB_INTR_EP6                                    (1 << 6)
#define USB_INTR_EP7                                    (1 << 7)
                                                                                                                
/** dma control register */                             
#define USB_CTL_DMA_ENA                                 (1 << 0)
#define USB_CTL_DMA_DIRE_IN                             (1 << 1)
#define USB_CTL_DMA_MODE                                (1 << 2)
#define USB_CTL_DMA_INT_ENA                             (1 << 3)
#define USB_CTL_DMA_BUS_ERR                             (1 << 7)
                                                        
/* usb control and status register */                   
#define USB_REG_RXCSR1_RXSTALL                          (1 << 6)
#define USB_REG_RXCSR1_REQPKT                           (1 << 5)
                                                        
#define USB_TXCSR_AUTOSET                               (0x80)
#define USB_TXCSR_ISO                                   (0x40)
#define USB_TXCSR_MODE1                                 (0x20)
#define USB_TXCSR_DMAREQENABLE                          (0x10)
#define USB_TXCSR_FRCDATATOG                            (0x8)
#define USB_TXCSR_DMAREQMODE1                           (0x4)
#define USB_TXCSR_DMAREQMODE0                           (0x0)
                                                                                                                
#define USB_RXCSR_AUTOSET                               (0x80)
#define USB_RXCSR_ISO                                   (0x40)
#define USB_RXCSR_DMAREQENAB                            (0x20)
#define USB_RXCSR_DISNYET                               (0x10)
#define USB_RXCSR_DMAREQMODE1                           (0x8)
#define USB_RXCSR_DMAREQMODE0                           (0x0)
#define USB_RXCSR_INCOMPRX                              (0x1)

#define USB_ENABLE_DMA                                  (1)
#define USB_DIRECTION_RX                                (0<<1)
#define USB_DIRECTION_TX                                (1<<1)
#define USB_DMA_MODE1                                   (1<<2)
#define USB_DMA_MODE0                                   (0<<2)
#define USB_DMA_INT_ENABLE                              (1<<3)
#define USB_DMA_INT_DISABLE                             (0<<3)
#define USB_DMA_BUS_ERROR                               (1<<8)
#define USB_DMA_BUS_MODE0                               (0<<9)
#define USB_DMA_BUS_MODE1                               (1<<9)
#define USB_DMA_BUS_MODE2                               (2<<9)
#define USB_DMA_BUS_MODE3                               (3<<9)

#define DMA_CHANNEL1_INT                                (1)
#define DMA_CHANNEL2_INT                                (2)
#define DMA_CHANNEL3_INT                                (4)
#define DMA_CHANNEL4_INT                                (8)
#define DMA_CHANNEL5_INT                                (0x10)
                                                        
/**Interrupt register bit masks */                      
#define USB_INTR_SUSPEND                                (1 << 0)
#define USB_INTR_RESUME                                 (1 << 1)
#define USB_INTR_RESET                                  (1 << 2)
#define USB_INTR_SOF                                    (1 << 3)
#define USB_INTR_CONNECT                                (1 << 4)
#define USB_INTR_DISCONNECT                             (1 << 5)
#define USB_INTR_SESSREQ                                (1 << 6)
#define USB_INTR_VBUSERROR                              (1 << 7)   /* #FOR SESSION END  */
                                                        
/** Interrupt register bit enable */                    
#define USB_INTR_SUSPEND_ENA                            (1 << 0)
#define USB_INTR_RESUME_ENA                             (1 << 1)
#define USB_INTR_RESET_ENA                              (1 << 2)
#define USB_INTR_SOF_ENA                                (1 << 3)
#define USB_INTR_CONNECT_ENA                            (1 << 4)
#define USB_INTR_DISCONNECT_ENA                         (1 << 5)
#define USB_INTR_SESSREQ_ENA                            (1 << 6)
#define USB_INTR_VBUSERROR_ENA                          (1 << 7)   /* #FOR SESSION END  */
                                                        
/**Reset mode */                                        
#define RESET_CTRL_USB                                  (1 << 5)
#define RESET_CTRL_USB_FS                               (1 << 23)

#endif//__USB_H__

