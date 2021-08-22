/*****************************************************************************
 *
 *            Copyright Andes Technology Corporation 2007-2008
 *                         All Rights Reserved.
 *
 *  Revision History:
 *
 *    Aug.21.2007     Created.
 *    Nov.22.2009     Add queue HAL API
 ****************************************************************************/

#ifndef __HAL_H__
#define __HAL_H__

#include "os_cpu.h"
#include "bsp_defs.h"
//#include "bsp_regs.h"

#include "n12_def.h"

#include "debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#include <string.h>


typedef struct hal_semaphore {

    void    *obj;

} hal_semaphore_t;

typedef struct hal_mutex {

    void    *obj;

} hal_mutex_t;

typedef struct hal_mbox {

    void    *obj;

} hal_mbox_t;

typedef struct hal_queue {

    void    *obj;
    void    **start;
    void    *msg;
    INT32U  size;
    INT32U  opt;
    INT32U  timeout;
    void    *event;
    INT32S  err;

} hal_queue_t;

typedef struct hal_message {

    void    *obj;

} hal_message_t;

typedef struct hal_thread {

    void        (*fn)(void *);
    void        *arg;
    INT8U       prio;
    void        *ptos;      /* high_address should be assigned to NULL 
                       if you want kernel to allocate stack. */
    INT32U      stack_size; /* in bytes */
    char        *name;
    void        *task;

} hal_thread_t;

typedef struct hal_bh {

    hal_semaphore_t sem;
    hal_thread_t    th;

} hal_bh_t;

typedef struct hal_event {

    void    *obj;

} hal_event_t;

#define HAL_DISABLE_INTERRUPTS          0
#define HAL_ENABLE_INTERRUPTS           1

#define HAL_NULL                        0
#define HAL_TRUE                        1
#define HAL_FALSE                       0

#define HAL_SUSPEND                     -1 /* wait for object timeout */
#define HAL_DEL_NO_PEND                 0
#define HAL_DEL_ALWAYS                  1

#define HAL_SUCCESS                     0
#define HAL_FAILURE                     -1

#define HAL_ERR_UNHANDLED_INTERRUPT     -1
#define HAL_ERR_INVALID_POINTER         -2
#define HAL_ERR_NOT_PRESENT             -3
#define HAL_ERR_UNAVAILABLE             -4
#define HAL_ERR_TIMEOUT                 -5
#define HAL_ERR_NO_MEMORY               -6
#define HAL_ERR_INVALID_ENTRY           -7
#define HAL_ERR_INVALID_OPERATION       -8
#define HAL_ERR_INVALID_DRIVER          -9
#define HAL_ERR_INVALID_START           -23

typedef isr_t       hal_isr_t;

inline int              hal_register_isr(int vector, hal_isr_t isr, hal_isr_t *old);
inline void             hal_system_error(int err);
inline unsigned int     hal_global_int_ctl(int int_op);
inline unsigned int     hal_ms2ticks(INT32U timeout);
inline int              hal_sleep(int ms);
inline void*            hal_malloc(int size);
inline void             hal_free(void *ptr);
inline void*            hal_get_pid_current();
inline void*            hal_current();
inline int              hal_create_semaphore(hal_semaphore_t *sem, int num, const void *param);
inline int              hal_destroy_semaphore(hal_semaphore_t *sem);
inline int              hal_pend_semaphore(hal_semaphore_t *sem, unsigned int timeout);
inline int              hal_post_semaphore(hal_semaphore_t *sem);
inline int              hal_post_semaphore_in_isr(hal_semaphore_t *sem);
inline int              hal_create_mutex(hal_mutex_t *mutex, const void *param);
inline int              hal_destroy_mutex(hal_mutex_t *mutex);
inline int              hal_wait_for_mutex(hal_mutex_t *mutex, unsigned int timeout);
inline int              hal_release_mutex(hal_mutex_t *mutex);
inline int              hal_create_queue(hal_queue_t *queue);
inline int              hal_destroy_queue(hal_queue_t *queue);
inline int              hal_pend_queue(hal_queue_t *queue);
inline int              hal_post_queue(hal_queue_t *queue);
inline int              hal_post_queue_from_lisr(hal_queue_t *p_queue);
#if 0
inline int              hal_create_mbox(hal_mbox_t *mbox, const void *param);
inline int              hal_destroy_mbox(hal_mbox_t *mbox);
inline int              hal_pend_mbox(hal_mbox_t *mbox, void **msg, unsigned int timeout);
inline int              hal_try_pend_mbox(hal_mbox_t *mbox, void **msg);
inline int              hal_post_mbox(hal_mbox_t *mbox, void *msg);
#endif
inline int              hal_create_thread(hal_thread_t *th);
inline int              hal_destroy_thread(hal_thread_t *th);
inline int              hal_create_bh(hal_bh_t *bh);
inline int              hal_destroy_bh(hal_bh_t *bh);
inline int              hal_register_bh(hal_bh_t *bh, void *param);
inline int              hal_raise_bh(hal_bh_t *bh);
inline void             hal_init_os();
inline void             hal_start_os();


typedef enum
{
    serCOM1,
    serCOM2,
    serCOM3,
    serCOM4,
    serCOM5,
    serCOM6,
    serCOM7,
    serCOM8
} eCOMPort;

typedef enum
{
    serNO_PARITY,
    serODD_PARITY,
    serEVEN_PARITY,
    serMARK_PARITY,
    serSPACE_PARITY
} eParity;

typedef enum
{
    serSTOP_1,
    serSTOP_2
} eStopBits;

typedef enum
{
    serBITS_5,
    serBITS_6,
    serBITS_7,
    serBITS_8
} eDataBits;

typedef enum
{
    ser50,
    ser75,
    ser110,
    ser134,
    ser150,
    ser200,
    ser300,
    ser600,
    ser1200,
    ser1800,
    ser2400,
    ser4800,
    ser9600,
    ser19200,
    ser38400,
    ser57600,
    ser115200
} eBaud;

typedef struct _sSerialDrv
{
    // Initialize
    void (*init)(eBaud baud, eDataBits data_bits, eStopBits stop_bits, eParity parity);
    // Send char
    void (*tx_char) (char c);
    // Receive char
    int (*rx_char) (void);
    // Send multiple char
    void (*tx_multi) (const char *s, size_t length);
    // Get UART FIFO length
    int (*get_fifo_length) (void);
    // Check FIFO full
    int (*is_tx_fifo_full) (void);
    // Register TX interrupt
    void (*register_tx_int) (void (*)(void));
    // Deregister TX interrupt
    void (*deregister_tx_int) (void);
    // Register RX interrupt
    void (*register_rx_int) (void (*)(void));
    // Deregister RX interrupt
    void (*deregister_rx_int) (void);
    // Enable TX interrupt
    void (*enable_tx_int) (void);
    // Disable TX interrupt
    void (*disable_tx_int) (void);
    // Enable RX interrupt
    void (*enable_rx_int) (void);
    // Disable RX interrupt
    void (*disable_rx_int) (void);
} sSerialDrv;

inline void     hal_console_init (const sSerialDrv *ser_drv);
inline void     hal_console_puts (const signed char * s, unsigned int l);
inline int      hal_console_getc (signed char *pcRxedChar);
inline int      hal_console_putc (signed char cOutChar);
inline void     hal_console_begin_fatal (void);
inline void     hal_console_end_fatal (void);
#endif /* __HAL_H__ */
