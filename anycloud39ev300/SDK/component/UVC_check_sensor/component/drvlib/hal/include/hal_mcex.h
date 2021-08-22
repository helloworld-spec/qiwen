/**
 * @FILENAME: hal_mcex.h
 * @BRIEF mcex implement file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liaozhijun
 * @DATE 2008-04-29
 * @VERSION 1.0
 * @REF
 */
#ifndef __HAL_MCEX_H__
#define __HAL_MCEX_H__

#define MCEX_WRITE_SEC_CMD	        35	/* adtc 		R1 */
#define MCEX_READ_SEC_CMD	        34	/* adtc			R1 */
#define MCEX_SEND_PSI		        36	/* adtc			R1 */
#define MCEX_CONTROL_TRM            37  /* ac           R1b */
#define MCEX_DIRECT_SECURE_READ     50  /* adtc         R1 */
#define MCEX_DIRECT_SECURE_WRITE    57  /* adtc         R1 */

#define MCEX_PSI_SR         0
#define MCEX_PSI_PR         4
#define MCEX_PSI_RNR        6

typedef enum tagMCEX_STATUS
{
    eMCEX_status_idle,
    eMCEX_status_cmd_progess,
    eMCEX_status_cmd_complete,
    eMCEX_statu_cmd_abort
}MCEX_STATUS;

typedef enum tagMCEX_ERROR
{
    eMCEX_error_none,
    eMCEX_error_auth,
    eMCEX_error_area_not_found,
    eMCEX_error_range_over,
    eMCEX_condition
}
MCEX_ERROR;

/**
 * @brief mcex init
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true init success
 * @retval false fail to init
 */
bool mcex_init(void);

/**
 * @brief mcex close
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return void
 */
 void mcex_close(void);

/**
 * @brief check if the present card support mcex function or not
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true the present card support mcex
 * @retval false the present card doesn't support mcex
 */
bool mcex_check(void);

/**
 * @brief open mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true open success
 * @retval false fail to open
 */
bool mcex_open(void);

/**
 * @brief reset mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return bool
 * @retval true reset success
 * @retval false fail to reset
 */
bool mcex_reset(void);

/**
 * @brief get psi
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param type [in]: psi type
 * @param data [out]: data buffer
 * @return bool
 * @retval true get psi success
 * @retval false fail to get psi
 */
bool mcex_get_psi(unsigned long type, unsigned char *data);

/**
 * @brief get timeout for mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param read_timeout [in]: timeout for read operation
 * @param write_timeout [in]: timeout for write operation
 * @return bool
 * @retval true reset success
 * @retval false fail to reset
 */
bool mcex_get_timeout(unsigned long *read_timeout, unsigned long *write_timeout);

/**
 * @brief write data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param mode [in]: 
 * @param data [in]: data to be written
 * @param blk_size [in]: block size
 * @return bool
 * @retval true write success
 * @retval false fail to write
 */
 bool mcex_write(unsigned long mode, unsigned char *data, unsigned long blk_size);
 
/**
 * @brief read data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param data [out]: data to be read
 * @param blk_size [in]: block size
 * @return bool
 * @retval true read success
 * @retval false fail to read
 */
 bool mcex_read(unsigned char *data, unsigned long blk_size);


/** @} */

#endif

