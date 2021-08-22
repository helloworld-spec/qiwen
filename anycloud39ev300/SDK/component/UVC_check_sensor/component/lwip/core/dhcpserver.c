#define DEBUG


//#include "xrf_debug.h"
#include "inet.h"
#include "err.h"
#include "pbuf.h"
#include "udp.h"
#include "mem.h"

#include "dhcpserver.h"
#if 0
#ifndef LWIP_OPEN_SRC
#include "net80211/ieee80211_var.h"
#endif
#endif


//#define USE_CLASS_B_NET 1
#define DHCPS_DEBUG        0

////////////////////////////////////////////////////////////////////////////////////
static const unsigned char xid[4] = {0xad, 0xde, 0x12, 0x23};
static unsigned char old_xid[4] = {0};
static const unsigned char magic_cookie[4] = {99, 130, 83, 99};
static struct udp_pcb *pcb_dhcps = NULL;
static struct ip_addr broadcast_dhcps;
static struct ip_addr server_address;
static struct ip_addr client_address;//added
static struct ip_addr client_address_plus;
static struct dhcps_msg msg_dhcps;
struct dhcps_state s;

static struct dhcps_lease dhcps_lease;
static int dhcps_lease_flag = 1;
static list_node *plist = NULL;
/******************************************************************************
 * FunctionName : node_insert_to_list
 * Description  : insert the node to the list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void node_insert_to_list(list_node **phead, list_node* pinsert)
{
	list_node *plist = NULL;

	if (*phead == NULL)
		*phead = pinsert;
	else {
		plist = *phead;
		while (plist->pnext != NULL) {
			plist = plist->pnext;
		}
		plist->pnext = pinsert;
	}
	pinsert->pnext = NULL;
}

/******************************************************************************
 * FunctionName : node_delete_from_list
 * Description  : remove the node from list
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
void node_remove_from_list(list_node **phead, list_node* pdelete)
{
	list_node *plist = NULL;

	plist = *phead;
	if (plist == NULL){
		*phead = NULL;
	} else {
		if (plist == pdelete){
			*phead = plist->pnext;
		} else {
			while (plist != NULL) {
				if (plist->pnext == pdelete){
					plist->pnext = pdelete->pnext;
				}
				plist = plist->pnext;
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ��DHCP msg��Ϣ�ṹ����������
 *
 * @param optptr -- DHCP msg��Ϣλ��
 * @param type -- Ҫ��ӵ�����option
 *
 * @return unsigned char* ����DHCP msgƫ�Ƶ�ַ
 */
///////////////////////////////////////////////////////////////////////////////////
static unsigned char* add_msg_type(unsigned char *optptr, unsigned char type)
{

        *optptr++ = DHCP_OPTION_MSG_TYPE;
        *optptr++ = 1;
        *optptr++ = type;
        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ��DHCP msg�ṹ������offerӦ������
 *
 * @param optptr -- DHCP msg��Ϣλ��
 *
 * @return unsigned char* ����DHCP msgƫ�Ƶ�ַ
 */
///////////////////////////////////////////////////////////////////////////////////
static unsigned char* add_offer_options(unsigned char *optptr)
{
        struct ip_addr ipadd;

        ipadd.addr = *( (unsigned int *) &server_address);

#ifdef USE_CLASS_B_NET
        *optptr++ = DHCP_OPTION_SUBNET_MASK;
        *optptr++ = 4;  //length
        *optptr++ = 255;
        *optptr++ = 240;	
        *optptr++ = 0;
        *optptr++ = 0;
#else
        *optptr++ = DHCP_OPTION_SUBNET_MASK;
        *optptr++ = 4;  
        *optptr++ = 255;
        *optptr++ = 255;	
        *optptr++ = 255;
        *optptr++ = 0;
#endif

        *optptr++ = DHCP_OPTION_LEASE_TIME;
        *optptr++ = 4;  
        *optptr++ = 0x00;
        *optptr++ = 0x01;
        *optptr++ = 0x51;
        *optptr++ = 0x80; 	

        *optptr++ = DHCP_OPTION_SERVER_ID;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = ip4_addr2( &ipadd);
        *optptr++ = ip4_addr3( &ipadd);
        *optptr++ = ip4_addr4( &ipadd);

	    *optptr++ = DHCP_OPTION_ROUTER;
	    *optptr++ = 4;  
	    *optptr++ = ip4_addr1( &ipadd);
	    *optptr++ = ip4_addr2( &ipadd);
	    *optptr++ = ip4_addr3( &ipadd);
	    *optptr++ = ip4_addr4( &ipadd);

#ifdef USE_DNS
	    *optptr++ = DHCP_OPTION_DNS_SERVER;
	    *optptr++ = 4;
	    *optptr++ = ip4_addr1( &ipadd);
		*optptr++ = ip4_addr2( &ipadd);
		*optptr++ = ip4_addr3( &ipadd);
		*optptr++ = ip4_addr4( &ipadd);
#endif

#ifdef CLASS_B_NET
        *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = 255;
        *optptr++ = 255;
        *optptr++ = 255;
#else
        *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
        *optptr++ = 4;  
        *optptr++ = ip4_addr1( &ipadd);
        *optptr++ = ip4_addr2( &ipadd);
        *optptr++ = ip4_addr3( &ipadd);
        *optptr++ = 255;
#endif

        *optptr++ = DHCP_OPTION_INTERFACE_MTU;
        *optptr++ = 2;  
#ifdef CLASS_B_NET
        *optptr++ = 0x05;	
        *optptr++ = 0xdc;
#else
        *optptr++ = 0x02;	
        *optptr++ = 0x40;
#endif

        *optptr++ = DHCP_OPTION_PERFORM_ROUTER_DISCOVERY;
        *optptr++ = 1;  
        *optptr++ = 0x00; 

        *optptr++ = 43;	
        *optptr++ = 6;	

        *optptr++ = 0x01;	
        *optptr++ = 4;  
        *optptr++ = 0x00;
        *optptr++ = 0x00;
        *optptr++ = 0x00;
        *optptr++ = 0x02; 	

        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ��DHCP msg�ṹ����ӽ����־����
 *
 * @param optptr -- DHCP msg��Ϣλ��
 *
 * @return unsigned char* ����DHCP msgƫ�Ƶ�ַ
 */
///////////////////////////////////////////////////////////////////////////////////
static unsigned char* add_end(unsigned char *optptr)
{

        *optptr++ = DHCP_OPTION_END;
        return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ����һ��DHCP msg�ṹ��
 *
 * @param -- m ָ�򴴽���DHCP msg�ṹ�����?
 */
///////////////////////////////////////////////////////////////////////////////////
static void create_msg(struct dhcps_msg *m)
{
        struct ip_addr client;

        client.addr = *( (unsigned int *) &client_address);

        m->op = DHCP_REPLY;
        m->htype = DHCP_HTYPE_ETHERNET;
        m->hlen = 6;  
        m->hops = 0;
        memcpy((char *) xid, (char *) m->xid, sizeof(m->xid));
        m->secs = 0;
        m->flags = htons(BOOTP_BROADCAST); 

        memcpy((char *) m->yiaddr, (char *) &client.addr, sizeof(m->yiaddr));

        memset((char *) m->ciaddr, 0, sizeof(m->ciaddr));
        memset((char *) m->siaddr, 0, sizeof(m->siaddr));
        memset((char *) m->giaddr, 0, sizeof(m->giaddr));
        memset((char *) m->sname, 0, sizeof(m->sname));
        memset((char *) m->file, 0, sizeof(m->file));

        memset((char *) m->options, 0, sizeof(m->options));
        memcpy((char *) m->options, (char *) magic_cookie, sizeof(magic_cookie));
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ����һ��OFFER
 *
 * @param -- m ָ����Ҫ���͵�DHCP msg����
 */
///////////////////////////////////////////////////////////////////////////////////
static void send_offer(struct dhcps_msg *m)
{
        unsigned char *end;
	    struct pbuf *p, *q;
	    unsigned char *data;
	    unsigned short cnt=0;
	    unsigned short i;
		err_t SendOffer_err_t;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPOFFER);
        end = add_offer_options(end);
        end = add_end(end);

	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		printf("udhcp: send_offer>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	       
#if DHCPS_DEBUG
	        printf("dhcps: send_offer>>pbuf_alloc succeed\n");
	        printf("dhcps: send_offer>>p->tot_len = %d\n", p->tot_len);
	        printf("dhcps: send_offer>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (unsigned char *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((unsigned char *) m)[cnt++];
#if DHCPS_DEBUG
					printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	        
#if DHCPS_DEBUG
	        printf("dhcps: send_offer>>pbuf_alloc failed\n");
#endif
	        return;
	    }
        SendOffer_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#if DHCPS_DEBUG
	        printf("dhcps: send_offer>>udp_sendto result %x\n",SendOffer_err_t);
#endif
	    if(p->ref != 0){	
#if DHCPS_DEBUG
	        printf("udhcp: send_offer>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ����һ��NAK��Ϣ
 *
 * @param m ָ����Ҫ���͵�DHCP msg����
 */
///////////////////////////////////////////////////////////////////////////////////
static void send_nak(struct dhcps_msg *m)
{

    	unsigned char *end;
	    struct pbuf *p, *q;
	    unsigned char *data;
	    unsigned short cnt=0;
	    unsigned short i;
		err_t SendNak_err_t;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPNAK);
        end = add_end(end);

	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		printf("udhcp: send_nak>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	        
#if DHCPS_DEBUG
	        printf("dhcps: send_nak>>pbuf_alloc succeed\n");
	        printf("dhcps: send_nak>>p->tot_len = %d\n", p->tot_len);
	        printf("dhcps: send_nak>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (unsigned char *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((unsigned char *) m)[cnt++];
#if DHCPS_DEBUG					
					printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	        
#if DHCPS_DEBUG
	        printf("dhcps: send_nak>>pbuf_alloc failed\n");
#endif
	        return;
    	}
        SendNak_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#if DHCPS_DEBUG
	        printf("dhcps: send_nak>>udp_sendto result %x\n",SendNak_err_t);
#endif
 	    if(p->ref != 0){
#if DHCPS_DEBUG			
	        printf("udhcp: send_nak>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ����һ��ACK��DHCP�ͻ���
 *
 * @param m ָ����Ҫ���͵�DHCP msg����
 */
///////////////////////////////////////////////////////////////////////////////////
static void send_ack(struct dhcps_msg *m)
{

		unsigned char *end;
	    struct pbuf *p, *q;
	    unsigned char *data;
	    unsigned short cnt=0;
	    unsigned short i;
		err_t SendAck_err_t;
        create_msg(m);

        end = add_msg_type(&m->options[4], DHCPACK);
        end = add_offer_options(end);
        end = add_end(end);
	    
	    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcps_msg), PBUF_RAM);
#if DHCPS_DEBUG
		printf("udhcp: send_ack>>p->ref = %d\n", p->ref);
#endif
	    if(p != NULL){
	        
#if DHCPS_DEBUG
	        printf("dhcps: send_ack>>pbuf_alloc succeed\n");
	        printf("dhcps: send_ack>>p->tot_len = %d\n", p->tot_len);
	        printf("dhcps: send_ack>>p->len = %d\n", p->len);
#endif
	        q = p;
	        while(q != NULL){
	            data = (unsigned char *)q->payload;
	            for(i=0; i<q->len; i++)
	            {
	                data[i] = ((unsigned char *) m)[cnt++];
#if DHCPS_DEBUG					
					printf("%02x ",data[i]);
					if((i+1)%16 == 0){
						printf("\n");
					}
#endif
	            }

	            q = q->next;
	        }
	    }else{
	    
#if DHCPS_DEBUG
	        printf("dhcps: send_ack>>pbuf_alloc failed\n");
#endif
	        return;
	    }
        SendAck_err_t = udp_sendto( pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT );
#if DHCPS_DEBUG
	        printf("dhcps: send_ack>>udp_sendto result %x\n",SendAck_err_t);
#endif
	    
	    if(p->ref != 0){
#if DHCPS_DEBUG
	        printf("udhcp: send_ack>>free pbuf\n");
#endif
	        pbuf_free(p);
	    }
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * ����DHCP�ͻ��˷�����DHCP����������Ϣ�����Բ�ͬ��DHCP��������������Ӧ��Ӧ��
 *
 * @param optptr DHCP msg�е���������
 * @param len ��������Ĵ��?(byte)
 *
 * @return unsigned char ���ش�����DHCP Server״ֵ̬
 */
///////////////////////////////////////////////////////////////////////////////////
static unsigned char parse_options(unsigned char *optptr, short len)
{
        struct ip_addr client;
    	int is_dhcp_parse_end = 0;

        // Ҫ�����DHCP�ͻ��˵�IP

        unsigned char *end = optptr + len;
        unsigned short type = 0;
	
	client.addr = *( (unsigned int *) &client_address);

        s.state = DHCPS_STATE_IDLE;

        while (optptr < end) {
#if DHCPS_DEBUG
        	printf("dhcps: (short)*optptr = %d\n", (short)*optptr);
#endif
        	switch ((short) *optptr) {

                case DHCP_OPTION_MSG_TYPE:	//53
                        type = *(optptr + 2);
                        break;

                case DHCP_OPTION_REQ_IPADDR://50
                        if( memcmp( (char *) &client.addr, (char *) optptr+2,4)==0 ) {
#if DHCPS_DEBUG
                    		printf("dhcps: DHCP_OPTION_REQ_IPADDR = 0 ok\n");
#endif
                            s.state = DHCPS_STATE_ACK;
                        }else {
#if DHCPS_DEBUG
                    		printf("dhcps: DHCP_OPTION_REQ_IPADDR != 0 err\n");
#endif
                            s.state = DHCPS_STATE_NAK;
                        }
                        break;
                case DHCP_OPTION_END:
			            {
			                is_dhcp_parse_end = 1;
			            }
                        break;
            }

		    if(is_dhcp_parse_end){
		            break;
		    }

            optptr += optptr[1] + 2;
        }

        switch (type){
        
        	case DHCPDISCOVER://1
                s.state = DHCPS_STATE_OFFER;
#if DHCPS_DEBUG
            	printf("dhcps: DHCPD_STATE_OFFER\n");
#endif
                break;

        	case DHCPREQUEST://3
                if ( !(s.state == DHCPS_STATE_ACK || s.state == DHCPS_STATE_NAK) ) {
                        s.state = DHCPS_STATE_NAK;
#if DHCPS_DEBUG
                		printf("dhcps: DHCPD_STATE_NAK\n");
#endif
                }
                break;

			case DHCPDECLINE://4
                s.state = DHCPS_STATE_IDLE;
#if DHCPS_DEBUG
            	printf("dhcps: DHCPD_STATE_IDLE\n");
#endif
                break;


        	case DHCPRELEASE://7
                s.state = DHCPS_STATE_IDLE;
#if DHCPS_DEBUG
            	printf("dhcps: DHCPD_STATE_IDLE\n");
#endif
                break;
        }
#if DHCPS_DEBUG
    	printf("dhcps: return s.state = %d\n", s.state);
#endif
        return s.state;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static short parse_msg(struct dhcps_msg *m, unsigned short len)
{struct ip_addr addr_tmp;
		if(memcmp((char *)m->options,
              (char *)magic_cookie,
              sizeof(magic_cookie)) == 0){
#if DHCPS_DEBUG
        	printf("xuegang dhcps: len = %d\n", len);
#endif
	        /*
         	 * ��¼��ǰ��xid���ﴦ���?
         	 * �˺�ΪDHCP�ͻ����������û�ͳһ��ȡIPʱ��
         	*/
//	        if((old_xid[0] == 0) &&
//	           (old_xid[1] == 0) &&
//	           (old_xid[2] == 0) &&
//	           (old_xid[3] == 0)){
//	            /*
//	             * old_xidδ��¼�κ����?
//	             * �϶��ǵ�һ��ʹ��
//	            */
//	            memcpy((char *)old_xid, (char *)m->xid, sizeof(m->xid));
//	        }else{
//	            /*
//	             * ���δ����DHCP msg��Я���xid���ϴμ�¼�Ĳ�ͬ��
//	             * �϶�Ϊ��ͬ��DHCP�ͻ��˷��ͣ���ʱ����Ҫ����Ŀͻ���IP
//	             * ���� 192.168.4.100(0x6404A8C0) <--> 192.168.4.200(0xC804A8C0)
//	             *
//	            */
//	            if(memcmp((char *)old_xid, (char *)m->xid, sizeof(m->xid)) != 0){
	                /*
                 	 * ��¼���ε�xid�ţ�ͬʱ�����IP����
                 	*/
	                
	                memcpy((char *)old_xid, (char *)m->xid, sizeof(m->xid));

	                {
						struct dhcps_pool *pdhcps_pool = NULL;
						list_node *pnode = NULL;
						list_node *pback_node = NULL;

						POOL_START:
						client_address.addr = client_address_plus.addr;
//							addr_tmp.addr =  htonl(client_address_plus.addr);
//							addr_tmp.addr++;
//							client_address_plus.addr = htonl(addr_tmp.addr);
						for (pback_node = plist; pback_node != NULL;pback_node = pback_node->pnext) {
							pdhcps_pool = pback_node->pnode;
							if (memcmp(pdhcps_pool->mac, m->chaddr, sizeof(pdhcps_pool->mac)) == 0){
//									printf("the same device request ip\n");
								client_address.addr = pdhcps_pool->ip.addr;
								pdhcps_pool->lease_timer = DHCPS_LEASE_TIMER;
								goto POOL_CHECK;
							} else if (pdhcps_pool->ip.addr == client_address_plus.addr){
//									client_address.addr = client_address_plus.addr;
//									printf("the ip addr has been request\n");
								addr_tmp.addr = htonl(client_address_plus.addr);
								addr_tmp.addr++;
								client_address_plus.addr = htonl(addr_tmp.addr);
								client_address.addr = client_address_plus.addr;
							}
						}
						pdhcps_pool = (struct dhcps_pool *)smalloc(sizeof(struct dhcps_pool));
						pdhcps_pool->ip.addr = client_address.addr;
						memcpy(pdhcps_pool->mac, m->chaddr, sizeof(pdhcps_pool->mac));
						pdhcps_pool->lease_timer = DHCPS_LEASE_TIMER;
						pnode = (list_node *)smalloc(sizeof(list_node ));
						pnode->pnode = pdhcps_pool;
						node_insert_to_list(&plist,pnode);

						POOL_CHECK:
						if ((client_address_plus.addr > dhcps_lease.end_ip) || (ip_addr_isany(&client_address))){
							client_address_plus.addr = dhcps_lease.start_ip;
							goto POOL_START;
						}

                        //if (wifi_softap_set_station_info(m->chaddr, &client_address) == 0) {
                        //    printf("return here\n");
						//	return 0;
						//} peng;
					}

#if DHCPS_DEBUG
	                printf("dhcps: xid changed\n");
	                printf("dhcps: client_address.addr = %x\n", client_address.addr);
#endif
	               
//	            }
	            
//	        }
                    
	        return parse_options(&m->options[4], len);
	    }
        return 0;
}
///////////////////////////////////////////////////////////////////////////////////
/*
 * DHCP ��������ݰ���մ���ص�����˺�����LWIP UDPģ������ʱ������
 * ��Ҫ����udp_recv()������LWIP����ע��.
 *
 * @param arg
 * @param pcb ���յ�UDP��Ŀ��ƿ�?
 * @param p ���յ���UDP�е��������?
 * @param addr ���ʹ�UDP���Դ�����IP��ַ
 * @param port ���ʹ�UDP���Դ�����UDPͨ���˿ں�
 */
///////////////////////////////////////////////////////////////////////////////////
static void handle_dhcp(void *arg, 
									struct udp_pcb *pcb, 
									struct pbuf *p, 
									struct ip_addr *addr, 
									unsigned short port)
{
		
		short tlen;
        unsigned short i;
	    unsigned short dhcps_msg_cnt=0;
	    unsigned char *p_dhcps_msg = (unsigned char *)&msg_dhcps;
	    unsigned char *data;

#if DHCPS_DEBUG
    	printf("dhcps: handle_dhcp-> receive a packet\n");
#endif
	    if (p==NULL) return;

		tlen = p->tot_len;
	    data = p->payload;

#if DHCPS_DEBUG
	    printf("dhcps: handle_dhcp-> p->tot_len = %d\n", tlen);
	    printf("dhcps: handle_dhcp-> p->len = %d\n", p->len);
#endif		

	    memset(&msg_dhcps, 0, sizeof(dhcps_msg));
	    for(i=0; i<p->len; i++){
	        p_dhcps_msg[dhcps_msg_cnt++] = data[i];
#if DHCPS_DEBUG					
			printf("%02x ",data[i]);
			if((i+1)%16 == 0){
				printf("\n");
			}
#endif
	    }
		
		if(p->next != NULL) {
#if DHCPS_DEBUG
	        printf("dhcps: handle_dhcp-> p->next != NULL\n");
	        printf("dhcps: handle_dhcp-> p->next->tot_len = %d\n",p->next->tot_len);
	        printf("dhcps: handle_dhcp-> p->next->len = %d\n",p->next->len);
#endif
			
	        data = p->next->payload;
	        for(i=0; i<p->next->len; i++){
	            p_dhcps_msg[dhcps_msg_cnt++] = data[i];
#if DHCPS_DEBUG					
				printf("%02x ",data[i]);
				if((i+1)%16 == 0){
					printf("\n");
				}
#endif
			}
		}

		/*
	     * DHCP �ͻ���������Ϣ����
	    */
#if DHCPS_DEBUG
    	printf("dhcps: handle_dhcp-> parse_msg(p)\n");
#endif
		
        switch(parse_msg(&msg_dhcps, tlen - 240)) {

	        case DHCPS_STATE_OFFER://1
#if DHCPS_DEBUG            
            	 printf("dhcps: handle_dhcp-> DHCPD_STATE_OFFER\n");
#endif			
	             send_offer(&msg_dhcps);
	             break;
	        case DHCPS_STATE_ACK://3
#if DHCPS_DEBUG
            	 printf("dhcps: handle_dhcp-> DHCPD_STATE_ACK\n");
#endif			
	             send_ack(&msg_dhcps);
	             break;
	        case DHCPS_STATE_NAK://4
#if DHCPS_DEBUG            
            	 printf("dhcps: handle_dhcp-> DHCPD_STATE_NAK\n");
#endif
	             send_nak(&msg_dhcps);
	             break;
			default :
				 break;
        }
#if DHCPS_DEBUG
    	printf("dhcps: handle_dhcp-> pbuf_free(p)\n");
#endif
        pbuf_free(p);
}
///////////////////////////////////////////////////////////////////////////////////
static void wifi_softap_init_dhcps_lease(unsigned int ip)
{
	unsigned int softap_ip = 0,local_ip = 0;

	if (dhcps_lease_flag) {
		local_ip = softap_ip = htonl(ip);
		softap_ip &= 0xFFFFFF00;
		local_ip &= 0xFF;
		if (local_ip >= 0x80)
			local_ip -= DHCPS_MAX_LEASE;
		else
			local_ip ++;

		memset(&dhcps_lease,0, sizeof(dhcps_lease));
		dhcps_lease.start_ip = softap_ip | local_ip;
		dhcps_lease.end_ip = softap_ip | (local_ip + DHCPS_MAX_LEASE);
	}
	dhcps_lease.start_ip = htonl(dhcps_lease.start_ip);
	dhcps_lease.end_ip= htonl(dhcps_lease.end_ip);
    #if DHCPS_DEBUG
    printf("start_ip = 0x%x, end_ip = 0x%x\n",dhcps_lease.start_ip, dhcps_lease.end_ip);
    #endif
}
///////////////////////////////////////////////////////////////////////////////////
void dhcps_start(struct ip_addr ip)
{
	memset(&msg_dhcps, 0, sizeof(dhcps_msg));
	pcb_dhcps = udp_new();
	if (pcb_dhcps == NULL ) {
        #if DHCPS_DEBUG
		printf("dhcps_start(): could not obtain pcb\n");
        #endif
	}

	IP4_ADDR(&broadcast_dhcps, 255, 255, 255, 255);

	server_address = ip;
	wifi_softap_init_dhcps_lease(server_address.addr);
	client_address_plus.addr = dhcps_lease.start_ip;
    ip_set_option(pcb_dhcps, SOF_REUSEADDR);
	udp_bind(pcb_dhcps, IP_ADDR_ANY, DHCPS_SERVER_PORT);
	udp_recv(pcb_dhcps, handle_dhcp, NULL);
#if DHCPS_DEBUG
	printf("dhcps:dhcps_start->udp_recv function Set a receive callback handle_dhcp for UDP_PCB pcb_dhcps\n");
#endif
		
}

void dhcps_stop(void)
{
	list_node *pnode = NULL;
	list_node *pback_node = NULL;
	udp_disconnect(pcb_dhcps);
	udp_remove(pcb_dhcps);
	
	pnode = plist;
	while (pnode != NULL) {
		pback_node = pnode;
		pnode = pback_node->pnext;
		node_remove_from_list(&plist, pback_node);
		sfree(pback_node->pnode);
		pback_node->pnode = NULL;
		sfree(pback_node);
		pback_node = NULL;
	}
}

int wifi_softap_set_dhcps_lease(struct dhcps_lease *please)
{
	//struct ip_info info;
	unsigned int softap_ip = 0;
	if (please == NULL)
		return 0;

	//os_bzero(&info, sizeof(struct ip_info));
	//wifi_get_ip_info(SOFTAP_IF, &info);
	//softap_ip = htonl(info.ip.addr);
	please->start_ip = htonl(please->start_ip);
	please->end_ip = htonl(please->end_ip);

	/*config ip information can't contain local ip*/
	if ((please->start_ip <= softap_ip) && (softap_ip <= please->end_ip))
		return 0;

	/*config ip information must be in the same segment as the local ip*/
	softap_ip >>= 8;
	if ((please->start_ip >> 8 != softap_ip)
			|| (please->end_ip >> 8 != softap_ip)) {
		return 0;
	}

	if (please->end_ip - please->start_ip > DHCPS_MAX_LEASE)
		return 0;

	memset(&dhcps_lease,0, sizeof(dhcps_lease));
	dhcps_lease.start_ip = please->start_ip;
	dhcps_lease.end_ip = please->end_ip;
	dhcps_lease_flag = 0;
	return 1;
}

void dhcps_coarse_tmr(void)
{
	list_node *pback_node = NULL;
	list_node *pnode = NULL;
	struct dhcps_pool *pdhcps_pool = NULL;
	pnode = plist;
	while (pnode != NULL) {
		pdhcps_pool = pnode->pnode;
		pdhcps_pool->lease_timer --;
		if (pdhcps_pool->lease_timer == 0){
			pback_node = pnode;
			pnode = pback_node->pnext;
			node_remove_from_list(&plist,pback_node);
			sfree(pback_node->pnode);
			pback_node->pnode = NULL;
			sfree(pback_node);
			pback_node = NULL;
		} else {
			pnode = pnode ->pnext;
		}
	}
}
