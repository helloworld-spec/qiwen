#ifndef __DRV_IRCUT_H__
#define __DRV_IRCUT_H__

#define IRCUT_CTRL_DAY_LEVEL	(0)	//白天控制ircut电平
#define IRCUT_CTRL_NIGHT_LEVEL	(1-IRCUT_CTRL_DAY_LEVEL)

enum ircut_cmd {
	IRCUT_CMD_SET_STATUS = 0,

	IRCUT_CMD_GET_STATUS
};

enum ircut_mode {
	IRCUT_MODE_CLOSED = 0,	/* in day*/
	IRCUT_MODE_OPENED
};

/**
 * @brief ircut device opened
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
int ircut_open(void);

/**
 * @brief set ircut statue
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] mode  day or night mode
 * @return void
 */
int ircut_ioctl(int cmd, void *arg);

/**
 * @brief ircut device closed
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return void
 */
int ircut_close(void);

#endif
