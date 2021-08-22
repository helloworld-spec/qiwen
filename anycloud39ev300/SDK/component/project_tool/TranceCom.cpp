// TranceCom.cpp: implementation of the CTranceCom class.
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include "stdafx.h"
#include "TranceCom.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



#define MAX_BLOCK_SIZE (4096)

#define IS_OVERLAPPED_IO    (TRUE)


HANDLE Com_handle;
OVERLAPPED osWrite,osRead;
volatile char *txBuf;
volatile DWORD iTxBuf;
DWORD txBufSize;


volatile int txEmpty=TRUE;
volatile int isConnected=FALSE;
//CAnyka_BlueTestDlg buletest_dlg;

char rxBuf[MAX_BLOCK_SIZE] = {0};

TCHAR cur_play_name[MAX_PATH] = {0};

//CEQ_ToolDlg EQ_Tool;
BOOL first_pack_flag = TRUE;
UINT g_cur_poit = 0;
UINT g_file_idex = 0;

T_PLAY_LIST m_play_param = {0};
T_FM_FREQUENCY m_FM_frequecy_param = {0};
unsigned char m_volume_size = 8;

TCHAR g_buf_value[6] = {0};
BOOL g_open_com_flag = TRUE;


char g_current_serial_number[MAX_PATH] = {0};
UINT g_serial_num_len = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTranceCom::CTranceCom()
{

}

CTranceCom::~CTranceCom()
{

}

short Trancecom_hex2int(const char *str)
{
    int i;
    short number=0; 
    int order=1;
    TCHAR ch;
	
    for(i=strlen(str)-1;i>=0;i--)
    {
		ch=str[i];
		if(ch=='x' || ch=='X')break;
		
		if(ch>='0' && ch<='9')
		{
			number+=order*(ch-'0');
			order*=16;
		}
		if(ch>='A' && ch<='F')
		{
			number+=order*(ch-'A'+10);
			order*=16;
		}
		if(ch>='a' && ch<='f')
		{
			number+=order*(ch-'a'+10);
			order*=16;
		}
    }
    return number;
}

UINT DecodePacket(char *rxBuf, UINT rxBuf_Len)
{
	UINT ret=1;

	memset(g_current_serial_number, 0, MAX_PATH);
	memcpy(g_current_serial_number, rxBuf, rxBuf_Len);
	g_serial_num_len = rxBuf_Len;
	return ret;
}

int ReadCommBlock(char *buf,int maxLen)
{
    BOOL fReadStat;
    COMSTAT comStat;
    DWORD dwErrorFlags;
    DWORD dwLength;
	
    ClearCommError(Com_handle,&dwErrorFlags,&comStat);
    dwLength=min((DWORD)maxLen,comStat.cbInQue);
    if(dwLength>0)
    {
#if IS_OVERLAPPED_IO
        fReadStat=ReadFile(Com_handle,buf,dwLength,&dwLength,&osRead);
        if(!fReadStat)   //Apr.28.2003:GetOverlappedResult() may be needed.
        {
            //By experiment, fReadStat was always TRUE,of course, and the following was never executed.
            //EB_Printf(TEXT("\n[RX_RD_WAIT]\n") ); 
            if(GetLastError()==ERROR_IO_PENDING)
            {
                GetOverlappedResult(Com_handle,&osRead,&dwLength,TRUE); 
            }
            else
            {
                //EB_Printf(TEXT("[RXERR]") );
            }
        }
#else
        fReadStat=ReadFile(Com_handle,buf,dwLength,&dwLength,NULL);
        if(!fReadStat)
        {
            //EB_Printf(TEXT("[RXERR]") );
        }
		
#endif
    }
    return dwLength;
}


void DoRxTx(void *args)
{
    OVERLAPPED os;
    DWORD dwEvtMask;
    int nLength;
    BOOL fStat;
    DWORD temp;
	
    memset(&os,0,sizeof(OVERLAPPED));
    os.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(os.hEvent==NULL)
    {
		//EB_Printf(TEXT("[ERROR:DoRxTx os.hEvent]\n"));
		return;
    }
	
    if(!SetCommMask(Com_handle,EV_RXCHAR|EV_TXEMPTY))
    {
		//EB_Printf(TEXT("[ERROR:SetCommMask()]\n"));
		CloseHandle(os.hEvent);
		return;
    }
	
    while(isConnected)
    {
		
        //while(isDumping==TRUE);
		
		dwEvtMask=0;
		
#if IS_OVERLAPPED_IO
        fStat=WaitCommEvent(Com_handle,&dwEvtMask,&os);   
        //Apr.28.2003: fStat should be checked for the cpu time saving. 
        if(!fStat)   //Apr.28.2003:GetOverlappedResult() is needed.  
        {
            //By experiment. Only when there was no signalled event, the following was executed.
            //EB_Printf(TEXT("\n[WaitCommEvent=false]\n") ); 
            if(GetLastError()==ERROR_IO_PENDING)
            {
                GetOverlappedResult(Com_handle,&os,&temp,TRUE); 
            }
            else
            {
                //EB_Printf(TEXT("[RXTX_THREAD_ERR]") );
            }
        }
#else
        WaitCommEvent(Com_handle,&dwEvtMask,NULL);  //wait until any event is occurred.
#endif		
		if( (dwEvtMask & EV_TXEMPTY) == EV_TXEMPTY )
			txEmpty=TRUE;
		
		//if((dwEvtMask & EV_RXCHAR) == EV_RXCHAR && isDumping!=TRUE)
		if((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
		{
			UINT read_times = 0;
			UINT ret = DATA_OK;
//RET_READ:
			do   //Apr.28.2003:The caveat on MSDN,"Serial Communications in Win32" recommends while();
            {
				nLength = ReadCommBlock(rxBuf,MAX_BLOCK_SIZE);
				if( nLength > 0 )
				{
					rxBuf[nLength]='\0';

					//***************************************************//
					//获取rxbuf,进行解释
					ret = DecodePacket(rxBuf, nLength);
					/*
					if (ret == DATA_ERR)
					{
						if (read_times < 3)
						{
							goto RET_READ;
						}
						read_times++;
					}
					*/
					
					//***************************************************//

					//EB_Printf(rxBuf);
				}
				else if (nLength == -1)
				{
					break;
				}
			}while(nLength);  
		}
		
		// Clear OVERRUN condition.
		// If OVERRUN error is occurred,the tx/rx will be locked.
		if(dwEvtMask & EV_ERR)
		{
			COMSTAT comStat;
			DWORD dwErrorFlags;
			ClearCommError(Com_handle,&dwErrorFlags,&comStat);
			//EB_Printf(TEXT("[DBG:EV_ERR]\n"));
		}
    }
    /*
    if (nLength == -1)
    {
        SendMessage(_hwnd, WM_USER+2, 0, 0);
    }
	*/
    CloseHandle(os.hEvent);
    return;
}

//打开串口
BOOL CTranceCom::trancecom_Open(TCHAR *com, UINT userBaudRate)
{	
    DCB dcb;
    COMMTIMEOUTS commTimeOuts;
    
//====================================================
    osRead.Offset=0;
    osRead.OffsetHigh=0;
    osWrite.Offset=0;
    osWrite.OffsetHigh=0;

    osRead.hEvent = CreateEvent(NULL,TRUE/*bManualReset*/,FALSE,NULL);
     //manual reset event object should be used. 
     //So, system can make the event objecte nonsignalled.
	//osRead.hEvent & osWrite.hEvent may be used to check the completion of 
	// WriteFile() & ReadFile(). But, the DNW doesn't use this feature.
    if(osRead.hEvent==NULL)
    {
		//EB_Printf(TEXT("[ERROR:CreateEvent for osRead.]\n"));
		return FALSE;
    }
	
    osWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(osWrite.hEvent==NULL)
    {
		//EB_Printf(TEXT("[ERROR:CreateEvent for osWrite.]\n"));
		return FALSE;
    }
	
	Com_handle =CreateFile(com,
		GENERIC_READ|GENERIC_WRITE,
		0, //exclusive access
		//FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,	
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED, //FILE_ATTRIBUTE_NORMAL|
		NULL);
	//hCom = CreateFile("COM6", GENERIC_WRITE|GENERIC_READ, 0, NULL, 
		//	hCom = CreateFile("\\\\.\\COM22", GENERIC_WRITE|GENERIC_READ, 0, NULL, 
	//	OPEN_EXISTING, 0, NULL);
	
    if(Com_handle==INVALID_HANDLE_VALUE)
    {
		DWORD  ret = GetLastError();
		//EB_Printf(TEXT("[ERROR:CreateFile for opening COM port.]\n") );
		return FALSE;
    }
	
    SetCommMask(Com_handle,EV_RXCHAR);
    SetupComm(Com_handle,4096,4096);
    PurgeComm(Com_handle,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    
    commTimeOuts.ReadIntervalTimeout=0xffffffff;
    commTimeOuts.ReadTotalTimeoutMultiplier=0;
    commTimeOuts.ReadTotalTimeoutConstant=1000;
    commTimeOuts.WriteTotalTimeoutMultiplier=0;
    commTimeOuts.WriteTotalTimeoutConstant=1000;
    SetCommTimeouts(Com_handle,&commTimeOuts);
	
	//====================================================
    dcb.DCBlength=sizeof(DCB);
    GetCommState(Com_handle,&dcb);
	
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.BaudRate = userBaudRate;
	//dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;  //7个
    dcb.Parity = 0;    //ODD
    dcb.StopBits = 0;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow = 0;
    dcb.fOutxDsrFlow = 0;
    
    UINT dcb_size = sizeof(dcb);
    if(SetCommState(Com_handle,&dcb)==TRUE)
    {	
		isConnected=TRUE;
		
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE )DoRxTx, NULL, 0, NULL);
		g_open_com_flag = TRUE;
		return TRUE;
    }
    else
    {
        DWORD  errCode;
        
        errCode = GetLastError();
        if (errCode != 0)
        {
            //EB_Printf("Open Fail: [%d]\r\n", errCode);
        }
		isConnected=FALSE;
		CloseHandle(Com_handle);
		return FALSE;
    }
}

void CTranceCom::trancecom_Close(void)
{
	if(isConnected)
    {
		isConnected=FALSE;
		SetCommMask(Com_handle,0);
		//disable event notification and wait for thread to halt
		EscapeCommFunction(Com_handle,CLRDTR);
		PurgeComm(Com_handle,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		CloseHandle(Com_handle);
		
        CloseHandle(osRead.hEvent);
		CloseHandle(osWrite.hEvent);
    }
    
    Sleep(100); 
}

//读串口
UINT CTranceCom::trancecom_Read(char *buf, UINT buf_len)
{
	unsigned long wlen = 0;
	BOOL ret = FALSE;

	if (Com_handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	ret = ReadFile(Com_handle, buf, buf_len, &wlen, NULL);
	if (ret == FALSE)
	{
		return 0;
	}
	
	return wlen;
}

//写串口
UINT CTranceCom::trancecom_Write(char *buf, UINT buf_len)
{
	unsigned long wlen = 0;
	DWORD ret = FALSE;
	DWORD temp;

	if (Com_handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
#if IS_OVERLAPPED_IO
	ret = WriteFile(Com_handle, buf, buf_len, &wlen, &osWrite);

	if(!ret)   //Apr.28.2003:GetOverlappedResult() may be needed.
	{
		
		if(GetLastError()==ERROR_IO_PENDING)
		{
			GetOverlappedResult(Com_handle,&osWrite,&temp,TRUE);
#if 0
			while(GetOverlappedResult(Com_handle,&osWrite,&temp,TRUE))
			{
				
				DWORD dwError = GetLastError();
				
				if(dwError == ERROR_IO_INCOMPLETE)
				{
					//dwBytesSent += dwBytesWritten; 
					continue;
				}

			}
#endif
			//more efficient in order to save the cpu time
		}
		else
		{
			//EB_Printf(TEXT("[TXERR]") );
		}
	}
	else
	{
		//By experiment, never executed.
		//EB_Printf(TEXT("\n[TX_NO_WR_WAIT]\n") ); 
	}
#else
	ret = WriteFile(Com_handle, buf, buf_len, &wlen, NULL);
#endif
	/*
	if (ret == FALSE && buf_len != wlen)
	{
		return 0;
	}
	return wlen;
	*/
	return 1;
}



//发送命令和数据
BOOL CTranceCom::trancecom_out(char cmd, char *buf, UINT buf_len)
{
	char *temp_buf = NULL;
	char *buffer = NULL;  //转义BUF
	char buffer_pack_len[5] = {0};  //转义BUF
	char buffer_pack_len_temp[5] = {0};  //转义BUF
	char buffer_check_sum_tem[5] = {0};  //转义BUF
	char buffer_check_sum[5] = {0};  //转义BUF
	UINT temp_buf_len = 0;
	char   pack_start = PACK_START_FLAG;			    //0x01	
	SHORT  pack_len;	            //是命令,数据,以及check sum的字节数
	char   *data_buffer = NULL;			//data buffer
	SHORT  check_sum;               //包长度,命令类型,命令数据,三部分的累加和.
	char   pack_end = PACK_END_FLAG;			    //0x02
	UINT i = 0, n = 0, pl_num = 0, cs_num = 0;

	pack_len   = 1 + buf_len + 2;

	//包长度,命令类型,命令数据,三部分的累加和.
	check_sum  = pack_len;
	check_sum += cmd;

	buffer = (char *)malloc(buf_len*2 +1);
	memset(buffer, 0, buf_len*2+1);
	for (i = 0; i < buf_len; i++)
	{
		check_sum += (unsigned char)buf[i];
		if (buf[i] == PACK_START_FLAG)
		{
			buffer[n++] = PACKET_TRAN_FLAG;
			buffer[n++] = PACKET_TRAN_FLAG1;
		}
		else if (buf[i] == PACK_END_FLAG)
		{
			buffer[n++] = PACKET_TRAN_FLAG;
			buffer[n++] = PACKET_TRAN_FLAG2;
		}
		else if (buf[i] == PACKET_TRAN_FLAG)
		{
			buffer[n++] = PACKET_TRAN_FLAG;
			buffer[n++] = PACKET_TRAN_FLAG3;
		}
		else
		{
			memcpy(&buffer[n++], &buf[i], 1);
		}
	}
	
	memcpy(buffer_pack_len_temp, &pack_len, 2);
	for (i = 0; i < 2; i++)
	{
		if (buffer_pack_len_temp[i] == PACK_START_FLAG)
		{
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG;
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG1;
		}
		else if (buffer_pack_len_temp[i] == PACK_END_FLAG)
		{
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG;
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG2;
		}
		else if (buffer_pack_len_temp[i] == PACKET_TRAN_FLAG)
		{
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG;
			buffer_pack_len[pl_num++] = PACKET_TRAN_FLAG3;
		}
		else
		{
			memcpy(&buffer_pack_len[pl_num++], &buffer_pack_len_temp[i], 1);
		}
	}

	memcpy(buffer_check_sum_tem, &check_sum, 2);
	for (i = 0; i < 2; i++)
	{
		if (buffer_check_sum_tem[i] == PACK_START_FLAG)
		{
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG;
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG1;
		}
		else if (buffer_check_sum_tem[i] == PACK_END_FLAG)
		{
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG;
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG2;
		}
		else if (buffer_check_sum_tem[i] == PACKET_TRAN_FLAG)
		{
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG;
			buffer_check_sum[cs_num++] = PACKET_TRAN_FLAG3;
		}
		else
		{
			memcpy(&buffer_check_sum[cs_num++], &buffer_check_sum_tem[i], 1);
		}
	}
	
	
	temp_buf_len = 1 + 1 + n + pl_num + cs_num + 1; //转义后包长
	temp_buf = (char *)malloc(temp_buf_len+1);
	if (NULL == temp_buf)
	{
		return FALSE;
	}
	memset(temp_buf, 0, temp_buf_len+1);


	memcpy(&temp_buf[0], &pack_start, 1);
	memcpy(&temp_buf[1], buffer_pack_len, pl_num);
	memcpy(&temp_buf[1+pl_num], &cmd, 1);
	memcpy(&temp_buf[1+pl_num+1], buffer, n);
	memcpy(&temp_buf[1+pl_num+1+n], buffer_check_sum, cs_num);
	memcpy(&temp_buf[1+pl_num+1+n+cs_num], &pack_end, 1);

	if(0 == trancecom_Write(temp_buf, temp_buf_len))
	{
		free(temp_buf);
		return FALSE;
	}
	
	free(temp_buf);
	return TRUE;
}
