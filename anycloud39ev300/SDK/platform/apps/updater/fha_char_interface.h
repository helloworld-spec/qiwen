/**
 * @filename fha_char_interface.h
 * @brief for linux update use
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-07
 * @version 1.0
 * @ref 
 */

#ifndef _FHA_CHAR_INTERFACE_H_
#define _FHA_CHAR_INTERFACE_H_

#define AK_FHA_UPDATE_BOOT_BEGIN	0xb1
#define AK_FHA_UPDATE_BOOT		0xb2
#define AK_FHA_UPDATE_BIN_BEGIN		0xb3
#define AK_FHA_UPDATE_BIN		0xb4
#define AK_FHA_UPDATE_MAC	0xb5
#define AK_FHA_UPDATE_SER	0xb6
#define AK_FHA_UPDATE_BSER	0xb7

#define AK_UPDATE_PART_NAME	0x85
#define AK_GET_PART_NAME	0x86
#define AK_PROTECT_CTL      0x87
#define AK_FHA_CHAR_NODE               "/dev/akfha_char"
#define MAX_BUF_LEN			64*1024
typedef struct
{
	long filelen;
	char filename[256];//the bin name in flash
}T_BinInfo;

typedef struct
{
	long buflen;
	char buff[MAX_BUF_LEN]; 
	long ddrparcnt;
	unsigned int ddrpar[64][2];
}T_BufInfo;

/*****************************************************************
 *@brief:the init function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param :void
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_init(void);

/*****************************************************************
 *@brief:the destroy function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param: void
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_destroy(void);

/*****************************************************************
 *@brief:update boot function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param filename:boot file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBoot(const char* filename, const char* ddrparfilename);

/*****************************************************************
 *@brief:update the bin file
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param filename:bin file name
 *@param BinFileName:bin file name in flash
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBin(const char* filename, const char* BinFileName);

/*****************************************************************
 *@brief:update the mtd
 *@author:zhongjunchao
 *@date:2013-12-19
 *@param filename:the root file name
 *@param partition:partition number
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateMTD(const char* filename, int partition);

/*****************************************************************
 *@brief:update the mac addr in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param mac:mac addr, eg:AA:BB:CC:DD:EE:FF
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateMac(const char* mac);

/*****************************************************************
 *@brief:update the serial number in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param sn:serial number
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateSn(const char* sn);

/*****************************************************************
 *@brief:update the bar code in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param bsn:bar code
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBsn(const char* bsn);
int fha_interface_set_protect(int protect);

int fha_interface_Update_ASA_data(const char* asa_data);

int fha_interface_get_ASA_data(const char* asa_data);

#endif
