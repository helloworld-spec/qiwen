#ifndef _DANA_SOCK_H_
#define _DANA_SOCK_H_

#include "dana_base.h"
#include "socket.h"

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef ETH_ALEN
#define ETH_ALEN 6
struct ether_addr {
    uint8_t ether_addr_octet[6];
};
#endif

typedef struct dana_sock_info_s {
    char local_ip[16];
    char peer_ip[16];
    uint16_t local_port;
    uint16_t peer_port;
    uint32_t sock_type;
}dana_sock_info_t;

struct dana_socket_handler {
    int32_t fd;
    int32_t sock_type;
    dana_sock_info_t sock_info;
};

struct dana_fd_set {
    fd_set fds;
};
/*
 * TCP socket，这里read与write函数，开发者可以实现为阻塞或非阻塞模式，SDK并不关心
 * 内部维系了read与write，不关心具体实现是否阻塞
 */

typedef struct dana_socket_handler dana_socket_handler_t;

typedef struct dana_fd_set  dana_fd_set_t;


extern int32_t dana_select(dana_socket_handler_t *fd[], uint32_t size, dana_fd_set_t *readfds, dana_fd_set_t *writefds, dana_fd_set_t *exceptfds, uint32_t *timeout_usec);

extern dana_fd_set_t * dana_fd_set_create();

extern int32_t dana_fd_set_destroy(dana_fd_set_t *set);

extern void dana_fd_clr(dana_socket_handler_t *fd, dana_fd_set_t *set);

extern int32_t dana_fd_isset(dana_socket_handler_t *fd, dana_fd_set_t *set);

extern void dana_fd_set(dana_socket_handler_t *fd, dana_fd_set_t *set);

extern void dana_fd_zero(dana_fd_set_t *set);

extern int32_t dana_bind(dana_socket_handler_t *sock, uint16_t port);

extern int32_t dana_listen(dana_socket_handler_t *sock, uint32_t backlog);

extern dana_socket_handler_t *dana_accept(dana_socket_handler_t *sock);

extern int32_t dana_getsock_info(dana_socket_handler_t *sock, dana_sock_info_t *sock_info);

extern char* dana_gethostbyname(const char *name, char *ip, const size_t ip_size);

extern char* dana_inet_ntoa(const uint32_t ip, char *str_ip, uint32_t str_ip_size);

extern uint32_t dana_inet_addr(const char* ip);

extern uint32_t dana_htonl(uint32_t hostlong);
extern uint16_t dana_htons(uint16_t hostlong);
extern uint32_t dana_ntohl(uint32_t netlong);
extern uint16_t dana_ntohs(uint16_t netlong);

//#define SOCK_STREAM 1
//#define SOCK_DGRAM  0

/*************************** tcp 接口 *********************************/

/*
 * 创建tcp socket
 */
extern dana_socket_handler_t* dana_tcp_socket_create();


/*
 * socket连接到服务器(ip+port，ip是以'\0'结尾的字符串)，超时时间为timeout_usec微秒
 * 返回值为0：表示连接成功
 * 返回值为-1：表示连接失败
 */
extern int32_t dana_tcp_connect(dana_socket_handler_t *sock, char *ip, uint16_t port, uint32_t timeout_usec);


/*
 * 从socket中接收数据到buf[0:n)中，超时时间为timeout_usec微秒
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_usec时间内没有收到数据
 * 返回值为正数：表示读收到的字节数
 */
extern int32_t dana_tcp_recv(dana_socket_handler_t *sock, void *buf, uint32_t n, uint32_t timeout_usec);

/*
 * 发送数据buf[0:n)，超时时间为timeout_usec微秒
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_usec时间内没有发送数据
 * 返回值为正数：表示发送的字节数
 */
extern int32_t dana_tcp_send(dana_socket_handler_t *sock, void *buf, uint32_t n, uint32_t timeout_usec);

#if 0
/*
 * 销毁 tcp socket
 */
extern int32_t dana_tcp_socket_destroy(dana_socket_handler_t *sock);
#endif



/*************************** udp 接口 *********************************/

/*
 * 创建udp socket，绑定端口到port，（为了加快近场搜索，建议设置socket支持发送广播包, 组播包）
 * 注意port为本机字节序
 */
extern dana_socket_handler_t* dana_udp_socket_create();

/*
 * 加入组播组
 */
extern int32_t dana_udp_socket_join_multicast(dana_socket_handler_t *sock, char *ip);

/*
 * 接收数据，超时时间为timeout_usec微秒
 * ip: 用于保存对端ip, ip_len为ip缓冲区大小，ip为大端模式的字符串
 * port: 用于保存对端port，注意port为本机字节序
 * 接收到的数据写入到buf[0,n)中
 *
 * 返回值为-1：表示出错
 * 返回值为0：表示在timeout_usec时间内没有接收到数据
 * 返回值为正数：表示接收到的字节数
 */
extern int32_t dana_udp_recvfrom(dana_socket_handler_t *sock, char *ip, uint32_t ip_len, uint16_t *port, void *buf, uint32_t n, uint32_t timeout_usec);

/*
 * 向ip:port发送数据：buf[0,n)， 超时时间为timeout_usec微秒
 * 注意port为本机字节序
 *
 * 返回值为-1：表示表示出错
 * 返回值为0：表示在timeout_usec时间内没有写入数据
 * 返回值为正数：表示发送的字节数
 */
extern int32_t dana_udp_sendto(dana_socket_handler_t *sock, char *ip, uint16_t port, void *buf, uint32_t n, uint32_t timeout_usec);

#if 0
/*
 * 销毁 udp socket
 */
extern int32_t dana_udp_socket_destroy(dana_socket_handler_t *sock);
#endif

/*
 * 销毁 socket
 */
extern int32_t dana_socket_destroy(dana_socket_handler_t *sock);

//ntohl/ntohs htons htonl


//inet_ntop/inet_ntoa 


#ifdef __cplusplus
extern "C"
{
#endif
#endif
