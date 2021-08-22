/*
* cmd_api.h
*/


#include "anyka_types.h"


#define STRUCT_PACKED		__attribute__((packed))

typedef struct _cmd_info {
	unsigned char preamble;
	unsigned char cmd_id;
	unsigned short cmd_len;
	unsigned short cmd_seq;
	unsigned short cmd_result;
	union {
		unsigned int event; /*event type param*/
		struct _video_param
		{
			unsigned char  resolution;
    		unsigned char  fps;
    		unsigned short kbps;
		}video_param;
		
	}STRUCT_PACKED param;
} STRUCT_PACKED CMD_INFO;


#define CMD_HEAD_LEN (4)

/*cmd_seq + cmd_result + event*/
#define EVENT_CMD_LEN (2 + 2 + 4)

/*cmd define 
*  H240 -> CC3200  odd number
*  CC3200 -> H240 oven number
*/
#define	CMD_WAKEUP_SRC_REQ   		0x01
#define	CMD_WAKEUP_SRC_RESP 		0x02

/*reset on CC3200 tell H240 to give out a audio prompt*/
#define CMD_SYS_CONFIG				0x04
#define CMD_SYS_CONFIG_COMPLETE 	0x06

#define CMD_SLEEP_REQ				0x05
#define CMD_BATTERY_LOW_ALARM 		0x07

#define CMD_RING_CALL_END			0x08
#define CMD_VIDEO_PREVIEW			0x0A
#define CMD_VIDEO_PREVIEW_END		0x0C
#define CMD_VIDEO_PARAM     		0x0E




/*event define*/
#define EVENT_RTC_WAKEUP  		  	0x01
#define EVENT_PIR_WAKEUP 			0x02
#define	EVENT_RING_CALL_WAKEUP 		0x03
#define	EVENT_VIDEO_PREVIEW_WAKEUP	0x04
#define	EVENT_SYS_CONFIG_WAKEUP		0x05

/*battery define*/
#define BATTERY_ALARM_THRESHOLD 	5


/*sys alarm event define*/
#define ALARM_BATTERY_TOO_LOW 		0x01
#define ALARM_MOVE_DETECT			0x02

/*spi stream define*/
typedef struct _spi_stream_send_info {
    char header_magic[4];
	unsigned int reserve;
	unsigned int pack_id;
	unsigned int data_len;
    //unsigned int check_sum;
} STRUCT_PACKED SPI_STREAM_SEND_INFO;


/*video stream define*/
typedef struct _stream_header {
	unsigned int header_magic;
    unsigned int frameType;
    unsigned int time;
	unsigned int frame_len;
} VIDEO_STREAM_HEADER,AUDIO_STREAM_HEADER;

/*video stream define*/
typedef struct _stream_end {
	unsigned int frame_crc;
	unsigned int end_magic;
}  VIDEO_STREAM_END,AUDIO_STREAM_END;





#if 0
/** 16 bits byte swap */
#define swap_byte_16(x) \
((T_U16)((((T_U16)(x) & 0x00ffU) << 8) | \
         (((T_U16)(x) & 0xff00U) >> 8)))

/** 32 bits byte swap */
#define swap_byte_32(x) \
((T_U32)((((T_U32)(x) & 0x000000ffUL) << 24) | \
         (((T_U32)(x) & 0x0000ff00UL) <<  8) | \
         (((T_U32)(x) & 0x00ff0000UL) >>  8) | \
         (((T_U32)(x) & 0xff000000UL) >> 24)))


#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif /* BYTE_ORDER */

#define BYTE_SWAP 0

#if (BYTE_SWAP == 1)
#define ntohs(x)  swap_byte_16(x)
#define htons(x)  swap_byte_16(x)
#define ntohl(x)  swap_byte_32(x)
#define htonl(x)  swap_byte_32(x)
#else
#define ntohs(x)  (x)
#define htons(x)  (x)
#define ntohl(x)  (x)
#define htonl(x)  (x)
#endif

#endif

#define AK_ERROR 				-1
#define AK_OK					0



/*uart and spi define*/

#define SPI_BUFF_SIZE     		1024

#define UART_BUFF_SIZE     		128
#define BAUD_RATE  		115200


#define CMD_PREAMBLE 			0x55

enum
{
	UNBLOCK = 0,
	BLOCKED = -1,
};


int data_send(char *buf, unsigned short len);
int cmd_uart_init();
int cmd_uart_deinit();
int send_one_cmd(char *buf, unsigned short len);
int read_one_cmd(char *buf, unsigned short len);


