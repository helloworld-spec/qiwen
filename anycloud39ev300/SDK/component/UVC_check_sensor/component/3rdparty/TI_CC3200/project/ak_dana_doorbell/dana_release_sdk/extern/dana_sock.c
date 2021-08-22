#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "simplelink.h"

#include "dana_sock.h"
#include "dana_debug.h"

/*
 * select socket
 */
extern int32_t dana_select(dana_socket_handler_t *fd[], uint32_t size, dana_fd_set_t *readfds, dana_fd_set_t *writefds, dana_fd_set_t *exceptfds, uint32_t *timeout_usec)
{
    int32_t max_fd = 0;
    uint32_t count = 0;
    struct timeval delay;
    memset(&delay, 0, sizeof(struct timeval));
    delay.tv_sec = *timeout_usec / 1000000;
    delay.tv_usec = *timeout_usec % 1000000;
    for(count = 0; count < size; ++count) {
        if(max_fd < fd[count]->fd)
            max_fd = fd[count]->fd;
    }
   
    int32_t ret = sl_Select(max_fd + 1, &(readfds->fds), &(writefds->fds), &(exceptfds->fds), &delay);
   // dbg("exit select\r\n");
	 
    *timeout_usec = delay.tv_sec*1000000 + delay.tv_usec;
#if 0
    if (0 == ret) {
      dana_usleep(100*1000);
    }
#endif
    return ret;
}

extern dana_fd_set_t * dana_fd_set_create()
{
    dana_fd_set_t *set = (dana_fd_set_t *)mem_Malloc(sizeof(dana_fd_set_t));
    memset(set, 0, sizeof(dana_fd_set_t));
    
    return set;
}

extern int32_t dana_fd_set_destroy(dana_fd_set_t *set)
{
    if (NULL == set) {
        return -1;
    }

    mem_Free(set);

    return 0;
}


/*
 * FD_CLR
 */
extern void dana_fd_clr(dana_socket_handler_t *fd, dana_fd_set_t *set) 
{
    if (NULL == fd) {
        return ;
    }

    if (NULL == set) {
        return ;
    }

    SL_FD_CLR(fd->fd, &(set->fds));
}

/*
 * FD_ISSET
 */
extern int32_t dana_fd_isset(dana_socket_handler_t *fd, dana_fd_set_t *set) 
{
    if (NULL == fd) {
        return 0;
    }

    if (NULL == set) {
        return 0;
    }

    return SL_FD_ISSET(fd->fd, &(set->fds));
}

/*
 * FD_SET
 */
extern void dana_fd_set(dana_socket_handler_t *fd, dana_fd_set_t *set) 
{
    if (NULL == fd) {
        return ;
    }

    if (NULL == set) {
        return ;
    }

    SL_FD_SET(fd->fd, &(set->fds));
}

/*
 * FD_ZERO
 */
extern void dana_fd_zero(dana_fd_set_t *set) 
{
    if (NULL == set) {
        return ;
    }

    SL_FD_ZERO(&(set->fds));
}

static void ipU32toString(unsigned int ipaddr,char * ipv4)
{

	int index;
	int temp;
	int i,k,f;
	index = 3;
	i = 0;
        f = 0;
	 for(index=4;index>0;index--)
	 {
		 temp =   ( (ipaddr >> ((index-1)*8)) & 0xFF );
                 k = temp/100;
                 if(k!=0)
                 {
                   ipv4[i++] = (temp/100) + '0';
                 }else
                 {
                     f =1;                                      //��λΪ0��־
                 }
                 k = temp/10;
                 if(k!=0)
                 {
                    ipv4[i++] = (temp/10)%10 +'0';
                 }else if(f == 0)                               //��λ��Ϊ��
                 {
                    ipv4[i++] = (temp/10)%10 +'0';
                 }
                 
		 ipv4[i++] = (temp%10)   +'0';
		 ipv4[i++] = '.';
	 }
         for(--i;i<16;i++)
         {
           ipv4[i] = 0;
         }
}

extern char* dana_gethostbyname(const char *name, char *ip, const size_t ip_size)
{
       int destipaddr;
	   char ip_str[16];
	   sl_NetAppDnsGetHostByName((unsigned char *)name,strlen(name),&destipaddr,SL_AF_INET);

	   mem_set(ip_str,0,16);
	   ipU32toString(destipaddr,ip_str);
	   if(strlen(ip_str)>ip_size)
	   	{
			return NULL;
	   	}
       strncpy(ip,ip_str,strlen(ip_str));
     return ip;
}







extern char* dana_inet_ntoa(const uint32_t ip, char *str_ip, uint32_t str_ip_size)
{
    struct in_addr addr;
    mem_set(&addr, 0, sizeof(addr));
    addr.s_addr = htonl(ip);

    ipU32toString(htonl(addr.s_addr),str_ip);
    return str_ip;
}

extern uint32_t dana_inet_addr(const char* ip)
{
	long a, b, c, d;
	long address = 0;
    sscanf(ip, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
    address |= d<<24;
    address |= c<<16;
    address |= b<<8;
    address |= a;
    return address;
}

extern uint32_t dana_htonl(uint32_t hostlong)
{
    return htonl(hostlong);
}

extern uint16_t dana_htons(uint16_t hostlong)
{
    return htons(hostlong);
}

extern uint32_t dana_ntohl(uint32_t netlong)
{
    return ntohl(netlong);
}

extern uint16_t dana_ntohs(uint16_t netlong)
{
    return ntohs(netlong);
}


/*
 * bind socket
 */
extern int32_t dana_bind(dana_socket_handler_t *sock, uint16_t port)
{
    struct sockaddr_in local_addr;
    mem_set(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port);

    if (sl_Bind(sock->fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        return -1;
    }

    return 0;
}

/*
 * listen socket
 */
extern int32_t dana_listen(dana_socket_handler_t *sock, uint32_t backlog)
{
    if (sl_Listen(sock->fd, backlog) < 0) { 
        return -1;
    }

    return 0;
}


extern dana_socket_handler_t *dana_accept(dana_socket_handler_t *sock)
{
    char str_ip[16];

	memset(str_ip, 0,16);
    dana_socket_handler_t *new_sock = (dana_socket_handler_t *) mem_Malloc(sizeof(dana_socket_handler_t));
    if (NULL == new_sock) {
        return NULL; 
    }
    memset(new_sock, 0, sizeof(dana_socket_handler_t));

    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len;
    new_sock->fd = sl_Accept(sock->fd, (struct sockaddr *)&peeraddr, &peeraddr_len);
    if (0 > new_sock->fd) {
        mem_Free(new_sock); 
        return NULL;
    }

    strncpy(new_sock->sock_info.peer_ip, dana_inet_ntoa(peeraddr.sin_addr.s_addr,str_ip,16), sizeof(new_sock->sock_info.peer_ip) -1);
    new_sock->sock_info.peer_port = ntohs(peeraddr.sin_port);

    return new_sock;
}


extern int32_t dana_getsock_info(dana_socket_handler_t *sock, dana_sock_info_t *sock_info)
{
    mem_copy(sock_info, &sock->sock_info, sizeof(dana_sock_info_t));
    return 0;
}


//tcp

/*
 * 创建tcp socket
 */
extern dana_socket_handler_t* dana_tcp_socket_create()
{
    dana_socket_handler_t *sock = (dana_socket_handler_t *) mem_Malloc(sizeof(dana_socket_handler_t));
    if (NULL == sock) {
        return NULL; 
    }

    int32_t fd = -1;

    fd = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, 0);
    if (0 > fd) {
        sl_Close(fd);
        mem_Free(sock);
        return NULL; 
    }

    SlSockNonblocking_t enableOption;
    enableOption.NonblockingEnabled = 1;
    if(sl_SetSockOpt(fd,SOL_SOCKET,SL_SO_NONBLOCKING,&enableOption,sizeof(enableOption)) < 0){
			mem_Free(sock);
			sl_Close(fd);
			return NULL;
    	}
	

    sock->fd = fd;
    sock->sock_type = SOCK_STREAM;

    return sock;
}



/*
 * socket连接到服务器(ip+port，ip是以'\0'结尾的字符串)，超时时间为timeout_usec毫秒
 * 返回值为0：表示连接成�?
 * 返回值为-1：表示连接失�?
 */
extern int32_t dana_tcp_connect(dana_socket_handler_t *sock, char *ip, uint16_t port, uint32_t timeout_usec)
{
    int32_t fd = sock->fd;
    struct sockaddr_in peer_addr;
	int val,timeout_ms;
	val = 0;
	timeout_ms = timeout_usec/1000;
    mem_set(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = dana_inet_addr(ip);
    peer_addr.sin_port = htons(port);
    
    while (1) {
        val = sl_Connect(fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
		if((SL_EALREADY == val)||(SL_POOL_IS_EMPTY == val))	{
			if(timeout_ms > 0){
					timeout_ms -=5;
					osi_Sleep(5);
					continue;
			}else{
				return -1;
			}
		}else if(val < 0)
		{
			return (-1);
		}else
		{
			 break;
		}
    }
    return val;
}

/*
 * 从socket中接收数据到buf[0:n)中，超时时间为timeout_usec毫秒
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_usec时间内没有收到数�?
 * 返回值为正数：表示读收到的字节数
 */
extern int32_t dana_tcp_recv(dana_socket_handler_t *sock, void *buf, uint32_t n, uint32_t timeout_usec)
{
    int32_t fd = sock->fd;
    int32_t ret;
    uint32_t len = n;
    uint8_t * recv_buf;
    recv_buf = buf;

    if (0 == timeout_usec) {
        return sl_Recv(fd, recv_buf, len, SL_SO_NONBLOCKING);
    }

    uint32_t timeout_left = timeout_usec;
    struct timeval delay;
    while(len > 0) {
        mem_set(&delay, 0, sizeof(struct timeval));
        delay.tv_sec = timeout_left / 1000000;
        delay.tv_usec = timeout_left % 1000000;
        fd_set fds;
        SL_FD_ZERO(&fds);
		SL_FD_SET(fd, &fds);
        ret = sl_Select(fd + 1, &fds, NULL, NULL, &delay);
        if(ret < 0) {
            return -1;
        } else if(ret == 0){
			return 0;
        }else{
            if(SL_FD_ISSET(fd, &fds)) {
                timeout_left = delay.tv_sec * 1000000 + delay.tv_usec;
                ret = sl_Recv(fd, recv_buf, len, 0);
                if(SL_EAGAIN == ret) {
					osi_Sleep(100);
					continue;	
                } else if(ret<0){
					return -1;
                }else{
					return ret;	
                }
                
            } else {
                return -1;
            }
        }
    }
    return 0;
}



/*
 * 发送数据buf[0:n)，超时时间为timeout_us微秒
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_usec时间内没有发送数�?
 * 返回值为正数：表示发送的字节�?
 */
extern int32_t dana_tcp_send(dana_socket_handler_t *sock, void *buf, uint32_t n, uint32_t timeout_usec)
{
    uint32_t send_len ;
    uint8_t *send_buf;
    uint32_t ___timeout;
    int32_t fd ;
    int32_t ret=0;
        
	send_len = n;
	send_buf = buf;
	___timeout = timeout_usec;
	fd = sock->fd;
	//dbg("FREE STACK SPACE:%d\r\n",uxTaskGetStackHighWaterMark());
	
    //TODO 如果timeout_usec �? 则直接进行发送一�?
    if (0 == timeout_usec) {    
          ret = sl_Send(fd, send_buf, send_len, 0);
          return ret;
        
    }

    while (0 != send_len) {
        //判断是否可写
        fd_set wset;
        struct timeval timeout;
        SL_FD_ZERO(&wset);
        SL_FD_SET(fd, &wset);
        timeout.tv_sec = (___timeout)/1000000;
        timeout.tv_usec = (___timeout)%1000000;
        ret = sl_Select(fd+1, NULL, &wset, NULL, &timeout);
        if (0 > ret) {
            return -1; 
        } else if(0 == ret){
			return 0;
		}else {
            if (SL_FD_ISSET(fd, &wset)) {
                ___timeout = timeout.tv_sec*1000000 + timeout.tv_usec;
            } else {
                return -1; //sock 错误�?
            }
            //send
            ret = sl_Send(fd, send_buf, send_len, 0);
            if (SL_EAGAIN == ret) {
               osi_Sleep(100);
			   continue;
            }else if(ret < 0){
				return (-1);	
            }
			return ret;	  
        }
    }
    return 0;
}


/*************************** udp 接口 *********************************/
/*
 * 创建udp socket，绑定端口到port，（为了加快近场搜索，建议设置socket支持发送广播包�?
 * 注意port为本机字节序
 */
extern dana_socket_handler_t* dana_udp_socket_create()
{
    dana_socket_handler_t *sock = (dana_socket_handler_t *) mem_Malloc( sizeof(dana_socket_handler_t));
    if (NULL == sock) {
        return NULL; 
    }

    int32_t fd = -1;

    fd = sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, 0);
    if (0 > fd) {
        sl_Close(fd);
        mem_Free(sock);
        return NULL; 
    }

       SlSockNonblocking_t enableOption ;
        enableOption.NonblockingEnabled = 1;
      if(sl_SetSockOpt(fd,SOL_SOCKET,SL_SO_NONBLOCKING,&enableOption,sizeof(enableOption)) < 0){
			mem_Free(sock);
			sl_Close(fd);
			return NULL;
    	}

    sock->fd = fd;
    sock->sock_type = SOCK_DGRAM;

    return sock;
}

extern int32_t dana_udp_socket_join_multicast(dana_socket_handler_t *sock, char *ip)
{
    int32_t fd = sock->fd;

	SlSockIpMreq mreq;
	mem_set((void *)&mreq,0,sizeof(mreq));
	mreq.imr_multiaddr.s_addr = dana_inet_addr(ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if(sl_SetSockOpt(fd, SL_IPPROTO_IP, SL_IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))<0){
			return -1;
	};

	return 0;
}


/*
 * 接收数据，超时时间为timeout_usec毫秒
 * ip: 用于保存对端ip, ip_len为ip缓冲区大小，ip为大端模式的字符�?
 * port: 用于保存对端port，注意port为本机字节序
 * 接收到的数据写入到buf[0,n)�?
 *
 * 返回值为-1：表示出�?
 * 返回值为0：表示在timeout_usec时间内没有接收到数据
 * 返回值为正数：表示接收到的字节数
 */

static char * my_inet_ntop(int family,const void *addrptr,char *strptr,size_t len )
{

    unsigned char  *p = (unsigned char *)addrptr;   
    if(family == AF_INET)   
    {   
        char temp[16];   
        snprintf(temp,sizeof(temp),"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);   
        if(strlen(temp) >= len)   
        {   
            return NULL;   
        }   
        strcpy(strptr,temp);   
        return strptr;   
    }   
    return NULL;   

}


extern int32_t dana_udp_recvfrom(dana_socket_handler_t *sock, char *ip, uint32_t ip_len, uint16_t *port, void *buf, uint32_t n, uint32_t timeout_usec)
{

    struct sockaddr_in peer_addr;
    socklen_t peeraddr_len = sizeof(peer_addr);
    int32_t __fd = sock->fd;


    if (0 == timeout_usec) {
        int32_t ret = sl_RecvFrom(__fd, buf, n, 0, (struct sockaddr*)&peer_addr, &peeraddr_len);
        if (ret > 0) {
            *port = ntohs(peer_addr.sin_port);
            my_inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, ip_len - 1); 
        }

        return ret;
    }

    struct timeval __timeout;
    mem_set(&__timeout, 0, sizeof(struct timeval));
    __timeout.tv_sec = timeout_usec / 1000000;
    __timeout.tv_usec = timeout_usec % 1000000;
    fd_set rwet;
    SL_FD_ZERO(&rwet);
    SL_FD_SET(__fd, &rwet);
    int32_t __ret = sl_Select(__fd + 1, &rwet, NULL, NULL, &__timeout);
    if(__ret < 0) {
        return -1;
    } else if(0 == __ret){
		return 0;
	}else {
        if(SL_FD_ISSET(__fd, &rwet)){
            __ret = sl_RecvFrom(__fd, buf, n, 0, (struct sockaddr*)&peer_addr, &peeraddr_len);
            if (__ret > 0) {
                *port = ntohs(peer_addr.sin_port);
                my_inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, ip_len - 1); 
            }else if(SL_EAGAIN == __ret){
					return 0;	
            }
            return __ret;
        } else {
            return -1;
        }
    }
}

/*
 * 向ip:port发送数据：buf[0,n)�?超时时间为timeout_usec毫秒
 * 注意port为本机字节序
 *
 * 返回值为-1：表示表示出�?
 * 返回值为0：表示在timeout_usec时间内没有写入数�?
 * 返回值为正数：表示发送的字节�?
 */
extern int32_t dana_udp_sendto(dana_socket_handler_t *sock, char *ip, uint16_t port, void *buf, uint32_t n, uint32_t timeout_usec)
{
    uint32_t send_len = n;
    uint8_t *send_buf = buf;

    uint32_t ___timeout = timeout_usec;
    int32_t fd = sock->fd;

    struct sockaddr_in peer_addr;
    mem_set(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = dana_inet_addr(ip);
    peer_addr.sin_port = htons(port);

	
    if (0 == timeout_usec) {
        return sl_SendTo(fd, send_buf, send_len, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    }

    while (0 != send_len) {
        //判断是否可写
        fd_set wset;
        struct timeval timeout;
        SL_FD_ZERO(&wset);
        SL_FD_SET(fd, &wset);
        timeout.tv_sec = (___timeout)/1000000;
        timeout.tv_usec = (___timeout)%1000000;
        int32_t ret = sl_Select(fd+1, NULL, &wset, NULL, &timeout);
        if (0 > ret) {
            return -1; 
        }else if(0 == ret){
			return 0;
        }else {
            if (SL_FD_ISSET(fd, &wset)) {
                ___timeout = timeout.tv_sec*1000000 + timeout.tv_usec;
            } else {
                return -1; //sock 错误�?
            }
            //send
            ret = sl_SendTo(fd, send_buf, send_len, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
            if (SL_EAGAIN == ret) {
				osi_Sleep(10);
				continue;
            }else if(ret<0){
				return -1;
            }

			return ret;
      }
    }
    return 0;
}

/*
 * 销�?socket
 */
extern int32_t dana_socket_destroy(dana_socket_handler_t  *sock)
{
    sl_Close(sock->fd);
    mem_Free(sock);
    return 0;
}

