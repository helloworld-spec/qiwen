
#ifndef __USB_COMMON_H__
#define __USB_COMMON_H__

#include "anyka_cpu.h"
#include "anyka_types.h"

/** @{@name USB Define
 *    Define the USB character
 */
#define USB_ENDPOINT_MAX_NUM      3     //end point max num
#define EP0_MAX_PAK_SIZE        64      //end point 0 FIFO size
#define EP_BULK_HIGHSPEED_MAX_PAK_SIZE    512      //end point bulk FIFO size in high speed mode
#define EP_BULK_FULLSPEED_MAX_PAK_SIZE    64      //end point bulk FIFO size in full speed mode


/** @{@name Buffer length define
 *  Define buffer length of EPx
 */
#define     EP0_IBUF_MAX_LEN    64  //the max packet size end point 0 in buffer can take
#define     EP0_OBUF_MAX_LEN    64  //the max packet size end point 0 out buffer can take
#define     EP1_BUF_MAX_LEN     64   //the max packet size end point 1 buffer can take
#define     EP2_BUF_MAX_LEN     EP_BULK_HIGHSPEED_MAX_PAK_SIZE  //the max packet size end point 2 buffer can take
#define     EP3_BUF_MAX_LEN     EP_BULK_HIGHSPEED_MAX_PAK_SIZE  //the max packet size end point 3 buffer can take

#endif

