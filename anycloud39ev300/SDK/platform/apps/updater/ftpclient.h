#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_
/*****************************************************************
 *@brief:download ftp file
 *@author:zhangshenglin
 *@date:2013-3-07
 *@param :host:the  ftp url
 					port:file name
 					user:ftp server username
 					pass:ftp server password
 					filename:file name in ftp server
 					pcSaveFile:file name in local
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int FtpGet(char *host,unsigned short port, char *user,char *pass,char *filename,char *pcSaveFile);
#endif
