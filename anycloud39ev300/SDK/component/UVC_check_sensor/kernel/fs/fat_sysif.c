#include "fs.h"
#include "print.h"
#include "drv_api.h"
#include "akos_api.h"
#include "string.h"
#include "os_malloc.h"



static unsigned long FsApi_GetChipId(void)
{
    return FS_AK39XX_H240;
}

static void FsApi_OutStream(unsigned short ch)
{
	printf("%c",ch);
}

static unsigned char FsApi_InStream(void)
{
    unsigned char ch = 'c';
    ch = getch() & 0xff;
    
    return ch;
}



static const union cptable  * g_cptable;

static signed long Eng_Eng2Unic(const signed char *src, unsigned long srcLen, unsigned short *ucBuf, unsigned long ucBufLen)
{
    unsigned long i;

    if (NULL != ucBuf && ucBufLen > 0)
    {
        for(i=0; (i<srcLen && i<(ucBufLen-1) && 0 != src[i]); i++)
            ucBuf[i] = ((unsigned short)(src[i])) & 0xff;

        ucBuf[i] = 0;

        if (i == ucBufLen-1 && i != srcLen)
            i = 0xffffffff;
    }
    else
    {
        i = srcLen;
    }

    return (signed long)i;
}


static signed long Eng_Unic2Eng(const unsigned short *unicode, unsigned long ucLen, signed char *pDestBuf, unsigned long destLen)
{
    unsigned long i;

    if (NULL != pDestBuf && destLen > 0)
    {
        for(i=0; (i<ucLen && i<(destLen-1) && 0 != unicode[0]); i++)
            pDestBuf[i] = (unsigned char)(unicode[i]);


        pDestBuf[i] = 0;
        if (i == destLen-1 && i < ucLen)
        i = 0xffffffff;
    }
    else
    {
        i = ucLen;
    }

    return (signed long)i;
}

//暂时只支持英文
static signed long FsApi_AnsiStr2UniStr(const char * pAnsiStr, unsigned long AnsiStrLen,unsigned short *pUniBuf, unsigned long UniBufLen, unsigned long code)
{
    signed long ret = -1;

	ret = Eng_Eng2Unic(pAnsiStr,AnsiStrLen,pUniBuf, UniBufLen);

    return ret;
}

//暂时只支持英文
static signed long FsApi_UniStr2AnsiStr(const unsigned short *pUniStr, unsigned long UniStrLen,char * pAnsibuf, unsigned long AnsiBufLen, unsigned long code)
{
    signed long ret = -1;


   	ret = Eng_Unic2Eng(pUniStr,UniStrLen, pAnsibuf, AnsiBufLen);
    return ret;
}

static unsigned long FsApi_GetSecond(void)
{
    unsigned long current_time = 0;
    unsigned short year         = 0;
    unsigned short TempYear     = 0;
    unsigned short month        = 0;
    unsigned short MonthToDays  = 0;
    unsigned short day          = 0;
    unsigned short hour         = 0;
    unsigned long minute       = 0;
    unsigned long second       = 0;
      unsigned short i            = 0;
#ifdef OS_ANYKA
    T_SYSTIME systime;
	unsigned long sys_curr_seconds;
#endif

    unsigned short std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned short leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
#ifdef OS_ANYKA
//    systime = GetSysTime();

	systime = rtc_get_systime();
	
    year = systime.year ;
    month = systime.month ;
    day = systime.day ;
    hour = systime.hour ;
    minute = systime.minute ;
    second = systime.second ;

    if (year < 1980)
    {
        /* Typho2257, Jan.2,07 - Modified the default year to 2007 */
        year = 2007;                                //default
    }
        

    /* BEGIN Typho5369, Dec24,06 - Modified the calculating: It is not true that
       each 4 years has one leap year! */
    for (TempYear = year - 1; TempYear >= 1980; TempYear--)
    {
        /* the case of leap year */
        if ( TempYear % 4 == 0 && (TempYear % 100 != 0 || TempYear % 400 == 0))
        {
            current_time += 31622400; // the seconds of a leap year
        }
        else
        {
            /* not a leap year */
            current_time += 31536000;  // the seconds of a common year(not a leap one)
        }
    }

    /* calculate the current year's seconds */

    if ((month < 1) || (month > 12))
    {
        /* get the default value. */
        month = 1;
    }

    /* the current year is a leap one */
    if ( year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
    {
        for (i = 1; i < month; i++)
        {
            MonthToDays += leap_month_days[i];
        }
    }
    else
    {
        /* the current year is not a leap one */
        for (i = 1; i < month; i++)
        {
            MonthToDays += std_month_days[i];
        }
    }

    if ((day < 1) || (day > 31))
    {
        /* get the default value */
        day = 1;
    }
    MonthToDays += (day - 1);

    /* added the past days of this year(change to seconds) */
    current_time += MonthToDays * 24 * 3600;

    /* added the current day's time(seconds) */
    current_time += hour * 3600 + minute * 60 + second;
#endif // #ifdef OS_ANYKA
	 //printf("year = %d,month = %d,day = %d,hour = %d,minute = %d,second = %d\n",
	 //year,month,day,hour,minute,second);
    return current_time;
}


static void FsApi_SetSecond(unsigned long seconds)
{
    return;
}

static void *    FsApi_Alloc(unsigned long size, signed char *filename, unsigned long fileline)
{
    void * ptr = Fwl_MallocAndTrace(size, (char *)filename, fileline);
    if(NULL == ptr)
        printf("FsApi_Alloc : malloc error");
    return ptr;
}


static void * FsApi_Realloc(void * var, unsigned long size, signed char *filename, unsigned long fileline)
{
    void * ptr = Fwl_ReMallocAndTrace(var, size, (char *)filename, fileline);
    if(NULL == ptr)
        printf("FsApi_Realloc : malloc error");
    return ptr;
}

static void *  FsApi_Free(void * var, signed char *filename, unsigned long fileline)
{
    return Fwl_FreeAndTrace(var, (char *)filename, fileline);
}

static signed long FsApi_Create_Semaphore(unsigned long initial_count, unsigned char suspend_type, signed char *filename, unsigned long fileline)
{
    return AK_Create_Semaphore(initial_count, suspend_type);
}

static signed long FsApi_Delete_Semaphore(signed long semaphore, signed char *filename, unsigned long fileline)
{
    return AK_Delete_Semaphore(semaphore);
}
static signed long FsApi_Obtain_Semaphore(signed long semaphore, unsigned long suspend, signed char *filename, unsigned long fileline)
{
    return AK_Obtain_Semaphore(semaphore, suspend);
}
static signed long FsApi_Release_Semaphore(signed long semaphore, signed char *filename, unsigned long fileline)
{
    return AK_Release_Semaphore(semaphore);
}

static     void FsApi_SysReset(void)
{
}

static void FsApi_RandSeed(void)
{
    srand(get_tick_count());
}

static unsigned long FsApi_Rand(unsigned long MaxVal)
{
    unsigned long val = 0;
    
    val = (unsigned long)rand() % MaxVal;
    return val;
}

#define MOUNT_THRD_STACK_SIZE   (10*1024)
typedef struct
{
    T_hTask thread;
    unsigned char Stack[MOUNT_THRD_STACK_SIZE];
} T_MNT_CTRL,*T_PMNT_CTRL;

static int g_asyn_thread=0;
static unsigned long FsApi_MountThread(ThreadFunPTR Fun, void * pData, unsigned long priority)
{
    T_PMNT_CTRL thrdCtrl = NULL;
        
    thrdCtrl = Fwl_Malloc(sizeof(T_MNT_CTRL));

    if (NULL == thrdCtrl)
    {
        printk("FsApi_MountThread Malloc Error\n");
        return 0;
    }

    if (50 == priority)
    {
        thrdCtrl->thread = AK_Create_Task((void*)Fun, "AsynThread",
                        1, pData, 
                       thrdCtrl->Stack, MOUNT_THRD_STACK_SIZE,
                       150, 5,
                       AK_PREEMPT,AK_START);
        g_asyn_thread =   thrdCtrl->thread;                     
    }
    else
    {
    thrdCtrl->thread = AK_Create_Task((void*)Fun, "MountThread",
                        1, pData, 
                       thrdCtrl->Stack, MOUNT_THRD_STACK_SIZE,
                       150, 1,
                       AK_PREEMPT,AK_START);
    }
    if (AK_IS_INVALIDHANDLE(thrdCtrl->thread))
    {
        printk("FsApi_MountThread Create_Task Error\n");
        Fwl_Free(thrdCtrl);
        thrdCtrl = NULL;
        return 0;
    }

    return (unsigned long)thrdCtrl;
}
static void FsApi_KillThead(unsigned long ThreadHandle)
{   
    T_PMNT_CTRL thrdCtrl = (T_PMNT_CTRL)ThreadHandle;

    if (NULL != thrdCtrl)
    {
        AK_Terminate_Task(thrdCtrl->thread);
        AK_Delete_Task(thrdCtrl->thread);
        Fwl_Free(thrdCtrl);
        thrdCtrl = NULL;
    }
}


static void FsApi_SleepMs(unsigned long ms)
{
    AK_Sleep((ms + 4)/5);
}


bool   init_fs()
{
    T_FSCallback    fsGlbCb = {0};

    fsGlbCb.out = FsApi_OutStream;
    fsGlbCb.in = FsApi_InStream; 
    fsGlbCb.fGetSecond = FsApi_GetSecond;
    fsGlbCb.fSetSecond = FsApi_SetSecond;
    
    fsGlbCb.fUniToAsc = (F_UniToAsc)FsApi_UniStr2AnsiStr;
    fsGlbCb.fAscToUni = (F_AscToUni)FsApi_AnsiStr2UniStr;

    fsGlbCb.fRamAlloc = FsApi_Alloc;
    fsGlbCb.fRamRealloc = FsApi_Realloc;
    fsGlbCb.fRamFree = FsApi_Free;
    
    fsGlbCb.fCrtSem    = FsApi_Create_Semaphore;
    fsGlbCb.fDelSem    = FsApi_Delete_Semaphore;
    fsGlbCb.fObtSem    = FsApi_Obtain_Semaphore;
    fsGlbCb.fRelSem    = FsApi_Release_Semaphore;
    
    fsGlbCb.fMemCpy = (void *)memcpy;
    fsGlbCb.fMemSet = (void *)memset;
    fsGlbCb.fMemMov = (void *)memmove;
    fsGlbCb.fMemCmp = (void *)memcmp;

    fsGlbCb.fPrintf = (void *)printk;
    fsGlbCb.fGetChipId  = FsApi_GetChipId;


    fsGlbCb.fSysRst= FsApi_SysReset;
    fsGlbCb.fRandSeed    = FsApi_RandSeed;
    fsGlbCb.fGetRand = FsApi_Rand;

    fsGlbCb.fMountThead = FsApi_MountThread;
    fsGlbCb.fKillThead  = FsApi_KillThead;
    
    fsGlbCb.fSystemSleep = FsApi_SleepMs;

    return FS_InitCallBack(&fsGlbCb, 128);
}

int change_fs_thread_priority(int priority)
{
    int old_priority=-1;

	if (g_asyn_thread !=0)
	{
        old_priority = AK_Change_Priority(g_asyn_thread,priority);
	}
    
    return old_priority;
}
