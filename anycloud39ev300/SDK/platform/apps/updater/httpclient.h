#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_
/*****************************************************************
 *@brief:download http file
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :url:the http url
 					filenane:file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int HttpDownload(char* url, char* filename);
#endif
