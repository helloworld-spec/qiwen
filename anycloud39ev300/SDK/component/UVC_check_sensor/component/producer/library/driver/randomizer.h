/**
 * @filename randomizer.h
 * @brief  the arithmetic  to randomize or derandomize the data
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */

#ifndef __RANDOMIZER_H_
#define __RANDOMIZER_H_

#define MALLOC drv_malloc

/**
*@brief     initial the randomizer
*
*@author    yangyiming
*@date       2012-05-15
*@param     PageSize[in]  the pagesize of  the nand
*@return    bool true for success, false for failure
*/
bool randomizer_init(unsigned long PageSize);

/**
*@brief     randomize the data or derandomize the randomized data
*
*@author    yangyiming
*@date       2012-05-15
*@param     pageIndex[in]   the offset of the page where the data got from /will be stored in the nand
*@param     EccFrameIndex[in] which ecc frame in a page
*@param     destBuff[out]  the  buffer to store the data to be converted 
*@param     srcBuff[in]      the buffer stored the data to be converted
*@param     datalen[in]      the data length ,unit byte
*@return void
*/
void randomize(unsigned long pageIndex, unsigned long EccFrameIndex, unsigned char *destBuff,unsigned char *srcBuff, unsigned long datalen);

#endif

