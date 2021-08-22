#ifndef _MD5_H_
#define _MD5_H_
/*****************************************************************
 *@brief:set md5 to file
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :filename:file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int SetFileMd5(const char* filename);
/*****************************************************************
 *@brief:check file md5
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :filename:file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int CheckFileMd5(const char* filename);
#endif