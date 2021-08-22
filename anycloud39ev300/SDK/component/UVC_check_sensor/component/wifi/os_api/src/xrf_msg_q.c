#include "xrf_api.h"

#define MSG_QUEUE_MAX 20

struct _msg_q_manage
{
	msg_q_t msg;
	void **q_start;
};

struct _msg_q_manage q_man[MSG_QUEUE_MAX];

int msgget(msg_q_t *msg_q, int q_size)
{
	uint32_t *q_start;
	int i, msg;

	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		if (q_man[i].msg == 0)
			break;
	} 
	if (i >= MSG_QUEUE_MAX)
	{
		p_err("msgget: no more msg queen");
		return  - 1;
	}
	
	q_start = (uint32_t*)mem_calloc(q_size, sizeof(uint32_t*));
	if(!q_start){
		p_err("no mem:%d, ", q_size);
		return -1;
	}
	
	msg = AK_Create_Queue((T_VOID *)q_start,
                            q_size*sizeof(uint32_t*),
                            AK_FIXED_SIZE,
                            sizeof(uint32_t*),
                            AK_FIFO);

	if(AK_IS_INVALIDHANDLE(msg))
	{
		p_err("create queue err:%d", msg);
		mem_free(q_start);
		return -1;
	}

	//wmj-
	//q_man[i].msg = (msg_q_t)*msg_q = msg;;
	q_man[i].msg = msg;
	*msg_q = msg;;

	q_man[i].q_start = q_start;
	
	return 0;
}
#pragma arm section code ="_video_server_"
int msgsnd(msg_q_t msgid, void *msgbuf)
{

	int status = AK_Send_To_Queue(msgid, &msgbuf,
                sizeof(uint32_t*), AK_SUSPEND/*AK_NO_SUSPEND*/);
	if(status != AK_SUCCESS)
        {
        	if(status == AK_QUEUE_FULL)
			return 1;
			
		p_err("msgsnd: err %d", status);
		return -1;
	}
	return  0;
}

int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout)
{
	int status;
	uint32_t actual_size;

	assert(msgid);
	timeout = ms2ticks(timeout);

	status = AK_Receive_From_Queue(msgid, msgbuf, sizeof(uint32_t*), &actual_size, timeout);
        if(status != AK_SUCCESS)
        {
		if(status == AK_TIMEOUT)
			return 1;
		
		p_err("msgrcv: err %d", status);
		return  - 1;
        }
	 return 0;
}

#pragma arm section code 

int msgfree(msg_q_t msgid)
{

	int status, i;

	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		if (q_man[i].msg == msgid)
			break;
	}
	if (i >= MSG_QUEUE_MAX)
	{
		p_err("msgfree: err no match msg_q %x", msgid);
		return  - 1;
	}

	if (!q_man[i].q_start)
	{
		assert(0);
	}

	mem_free(q_man[i].q_start);
	q_man[i].q_start = 0;
	q_man[i].msg = 0;
	
	status = AK_Delete_Queue(msgid);
        if(status != AK_SUCCESS)
        {
        	p_err("del: err %d", status);
		return  - 1;
        }
	
	return 0;
}

void msg_q_init(void)
{
	int i;
	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		q_man[i].msg = 0;
		q_man[i].q_start = 0;
	}
}
