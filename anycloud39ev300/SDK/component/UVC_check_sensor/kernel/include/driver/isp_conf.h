#ifndef __ISP_CONF_H__
#define __ISP_CONF_H__

#include "anyka_types.h"

typedef enum {
	MODE_DAYTIME,
	MODE_NIGHTTIME,
	MODE_NUM
} T_MODULE_MODE;

typedef enum {
	LOAD_MODE_DAY_ISP = MODE_DAYTIME,
	LOAD_MODE_NIGHT_ISP = MODE_NIGHTTIME,
	LOAD_MODE_WHOLE_FILE,
	LOAD_MODE_NUM
} T_LOAD_MODE;

typedef struct cfgfile_headinfo               
{
    unsigned int   main_version;
    char    file_version[16];

    unsigned int   sensorId;
    
    unsigned short   year;
    unsigned char    month;
    unsigned char    day;
    unsigned char    hour;
    unsigned char    minute; 
    unsigned char    second;
    unsigned char    subFileId;
    
    unsigned char    styleId;
    unsigned char    reserve1;
    unsigned short   reserve2;
    
    unsigned int		subFileLen;    
    unsigned char    reserve3[88];
    
    char    notes[384];	
}CFGFILE_HEADINFO;



/**
 * @brief initiation for isp conf module
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] isp_conf_buf basic memory address to isp_conf file
 * @param[in] len memory length
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_init(unsigned char *isp_conf_buf, unsigned long len);

/**
 * @brief deinitiation for isp conf module
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_deinit(void);

/**
 * @brief set sensor initial configuration
 * @author ye_guohong   
 * @date 2017-03-14
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_sensor_init_conf(void);

/**
 * @brief set day configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_day(void);

/**
 * @brief set night configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_set_night(void);

/**
 * @brief start fvpr
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_fvpr_start(void);

/**
 * @brief finish fvpr
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true if successfully
 * @retval false if unsuccessfully
 */
bool isp_conf_fvpr_finish(void);

#endif
