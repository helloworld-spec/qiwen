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
 *@brief:update the mtd
 *@author:zhongjunchao
 *@date:2013-12-19
 *@param filename:the root file name
 *@param partition:partition number
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateMTD(const char* filename, int partition);
int test_update_mac(char *mac_str);

#endif
