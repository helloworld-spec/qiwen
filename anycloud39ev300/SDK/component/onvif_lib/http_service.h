/*************************************************

httpd.h
used by onvif

**************************************************/

#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H
#ifdef __cplusplus
extern "C"
{
#endif

#define HTTP_TEMP_STR_LEN            10240
#define HTTP_CMDSIZE                 10240
#define HTTP_SEND_BUF_SIZE           10240
#define HTTP_RECV_BUF_SIZE           10240
#define MAX_SOAP_SIZE                20480
#define HTTP_LISTEN_PORT             80 




int http_init_client (client_t * client);
void *http_client_recv (void *data);
void *http_client_listen (void *data);

#ifdef __cplusplus
}
#endif
#endif
