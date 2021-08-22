

#ifndef __AK_UART_H__
#define __AK_UART_H__




extern volatile unsigned long wakeup_type;


#define CMD_HEAD_LEN (4)


/*cmd define 
*  H240 -> CC3200  odd number
*  CC3200 -> H240 oven number
*/
#define	CMD_WAKEUP_SRC_REQ   		0x01
#define	CMD_WAKEUP_SRC_RESP 		0x02

/*reset on CC3200 tell H240 to give out a audio prompt*/
#define CMD_SYS_CONFIG				0x04
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
//#define BATTERY_ALARM_THRESHOLD 	5


/*sys alarm event define*/
#define ALARM_BATTERY_TOO_LOW 		0x01
#define ALARM_MOVE_DETECT			0x02

#define CMD_PREAMBLE 			0x55

/*
*
*akipc.c first app statarted
*author:wmj
*date: 20161214
*/

typedef struct _cmd_info
{
	unsigned char preamble;
	unsigned char cmd_id;
	unsigned short cmd_len;
	unsigned short cmd_seq;
	unsigned short cmd_result;
	union
	{
		unsigned int event; /*event type param*/
		struct _video_param
		{
			unsigned char  resolution;
    		unsigned char  fps;
    		unsigned short kbps;
		}video_param;
		/*other cmd param*/
	} param;
}CMD_INFO;

extern void send_cmd(CMD_INFO *cmd);


//extern void ak_uart_write(unsigned char* buf,unsigned long len);

//extern unsigned long ak_uart_read(unsigned char* buf,unsigned long len);

extern void ak_uart1_init(void);


#endif //#ifndef __AK_UART_H__

