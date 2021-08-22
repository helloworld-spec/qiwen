#ifndef __IRQ_H__
#define __IRQ_H__

/**
 *  Definition of first-level Interrupt Request Number
 */
#define IRQ_WDT                             0
#define IRQ_SWI                             1
#define IRQ_SPI_SDIO_WAKE                   2
#define IRQ_SPI_DONE                        3
#define IRQ_IPC                             4
#define IRQ_BTCX                            5
#define IRQ_FLASH_DMA_DONE                  6
#define IRQ_BEACON_DONE                     7
#define IRQ_BEACON_DTIM                     8
#define IRQ_BEACON                          9
#define IRQ_FBUSDMA                         9
#define IRQ_DMA                             10
#define IRQ_USB                             11
#define IRQ_RTC                             12
#define IRQ_SYSCTL                          13
#define IRQ_PMU                             14
#define IRQ_15_GROUP                        15
#define IRQ_PREBEACON                       16
#define IRQ_I2C                             16
#define IRQ_MTX_STAT_INT                    17
#define IRQ_CPU_INT_ALT                     18
#define IRQ_MBOX                            19
#define IRQ_US_TIMER0                       20
#define IRQ_US_TIMER1                       21
#define IRQ_US_TIMER2                       22
#define IRQ_US_TIMER3                       23
#define IRQ_MS_TIMER0                       24 //USB1.1 RX use it
#define IRQ_MS_TIMER1                       25
#define IRQ_MS_TIMER2                       26
#define IRQ_MS_TIMER3                       27
#define IRQ_MRX                             28
#define IRQ_HCI                             29
#define IRQ_CO_DMA                          30
#define IRQ_31_GROUP                        31

//IRQ 2 group
#define IRQ_2_SPI_CLK_EN_INT                0
#define IRQ_2_SDIO_WAKE                     1

// IRQ 15 Group
#define IRQ_15_DMN_NOHIT                    0
#define IRQ_15_ALC_ERR                      1
#define IRQ_15_RLS_ERR                      2
#define IRQ_15_ALC_ABT                      3

#define IRQ_15_ALC_TIMEPUT                  8
#define IRQ_15_REQ_LOCK                     9
#define IRQ_15_TX_LIMIT                     10
#define IRQ_15_ID_THOLD_RX                  11
#define IRQ_15_ID_THOLD_TX                  12
#define IRQ_15_DOUBLE_RLS                   13
#define IRQ_15_RX_ID_LEN_THOLD              14
#define IRQ_15_TX_ID_LEN_THOLD              15
#define IRQ_15_ALL_ID_LEN_THOLD             16
#define IRQ_15_TRASH_CAN                    17
#define IRQ_15_MB_LOWTHOLD                  18

#define IRQ_15_EDCA0_LOW_THRESHOLD          20
#define IRQ_15_EDCA1_LOW_THRESHOLD          21
#define IRQ_15_EDCA2_LOW_THRESHOLD          22
#define IRQ_15_EDCA3_LOW_THRESHOLD          23
#define IRQ_15_EDCA_BK                      24
#define IRQ_15_EDCA_BE                      25
#define IRQ_15_EDCA_VI                      26
#define IRQ_15_EDCA_VO                      27
#define IRQ_15_EDCA_MNT                     28
#define IRQ_15_EDCA_BCN                     29

// IRQ 31 Group
#define IRQ_31_GPI0                         0
#define IRQ_31_GPI1                         1
#define IRQ_31_GPI2                         2
#define IRQ_31_GPI3                         3
#define IRQ_31_GPI4                         4
#define IRQ_31_GPI5                         5
#define IRQ_31_GPI6                         6
#define IRQ_31_GPI7                         7
#define IRQ_31_GPI8                         8
#define IRQ_31_GPI9                         9
#define IRQ_31_GPI10                        10
#define IRQ_31_GPI11                        11
#define IRQ_31_GPI12                        12
#define IRQ_31_GPI13                        13
#define IRQ_31_GPI14                        14
#define IRQ_31_GPI15                        15
#define IRQ_31_GPI16                        16
#define IRQ_31_GPI17                        17
#define IRQ_31_GPI18                        18
#define IRQ_31_GPI19                        19
#define IRQ_31_GPI20                        20
#define IRQ_31_GPI21                        21
#define IRQ_31_GPI22                        22
#define IRQ_31_WIFI_PHY                     23
#define IRQ_31_UART_DBG_TX                  24
#define IRQ_31_UART_DBG_RX                  25
#define IRQ_31_UART_RX_TOUT                 26
#define IRQ_31_UART_DBG_MULTI               27
#define IRQ_31_UART_DATA_TX                 28
#define IRQ_31_UART_DATA_RX                 29
#define IRQ_31_UART_DATA_RX_TOUT            30
#define IRQ_31_UART_DATA_MULTI              31

#endif /* __IRQ_H__ */

