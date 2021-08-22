/**
 * @FILENAME: command.c
 * @BRIEF 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2007-10-23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_types.h"
#include "command.h"
#include "drv_api.h"
#include "akos_api.h"


/* define the current command id */
#define CMD_TOTAL_AMOUNT 100
#define MAX_ARGS_NUM	16
#define MAX_ARG_LEN		32
#define MAX_CMD_LEN     128

typedef struct _tagSigFunc
{
    T_pfSigfunc term;       //terminate callback function
}
T_SIGNAL_FUNC;

/* Command info struct */
typedef struct _tagCmd{
    signed char        name[MAX_CMD_LEN];
    T_pfCMD cmd;
    T_SIGNAL_FUNC sigfunc;
    struct _tagCmd *parent;
    struct _tagCmd *head;
    char       * help[2];
}T_CMD_INFO;

static T_CMD_INFO *cur_cmd;
static T_CMD_INFO main_cmd[CMD_TOTAL_AMOUNT];
static unsigned char index=0;

#define MAX_BG_COUNT            8
#define COMMAND_STACK_SIZE      (16*1024)

/*  Backgound task status    */
#define BG_TASK_NULL            -1  //not inital
#define BG_TASK_IDLE            0   //not running
#define BG_TASK_RUNNING         1   //running
#define BG_TASK_END             2   //run finished

/*  Backgound task struct    */
typedef struct 
{
    volatile int status;            //status   
    T_CMD_INFO *cmd;                //which cmd bg task run
    T_hTask handle;                 //thread handle
    char argc;                      //argc
    char *argv[MAX_ARGS_NUM];       //argv
    char *stack;                    //thread stack
}
T_BG_TASK;

static T_BG_TASK s_bg_task[MAX_BG_COUNT];

bool test_falg = false;

#ifdef __GNUC__
extern int __cmd_initcall_start, __cmd_initcall_end;
static int *initcall_start=&__cmd_initcall_start, *initcall_end=&__cmd_initcall_end;
#endif

const char* ak_kernel_common_get_version(void)
{
    return LIBKERNEL_COMMON_VERSION;
}

static int strfnd(const char *s1, const char *s2)
{
    int i,j;

    for (i=0; s1[i]!=0; i++)
    {
        j = 0;
        while(s2[j] && s1[i+j])
        {
            if (s2[j] != s1[i+j])
                break;
            j++;
        }
        if (s2[j] == 0)
            return i;
    }
    
    return -1;
}

//执行实际的命令注册
/**
 * @brief  do the functions defined by macro cmd_module_init 
 * 
 * @param   none
 * @return  none
 */
void cmd_do_register(void)
{
    cmd_initcall_t *call;
    int *addr;

    for (addr = initcall_start; addr < initcall_end; addr++) 
    {
        call = (cmd_initcall_t *)addr;
        (*call)();    
    }
}

/**
 * @brief  command init
 * 
 * @param   cmd_help  help function to cmd 'help'
 * @return  none
 */
void cmd_init(T_pfCMD cmd_help)
{
    unsigned short i,j;
    T_CMD_INFO *cmd = &main_cmd[0];

    strcpy(main_cmd[index].name, "help");
//    strcpy(main_cmd[index].help, ":)");
    main_cmd[index].help[0]=":)";
    main_cmd[index].cmd    = cmd_help;
    main_cmd[index].sigfunc.term = CMD_SIG_DFL;
    main_cmd[index].head   = NULL;
    main_cmd[index].parent = NULL;
    index++;

    for (i=0; i<index; i++)
        cmd[i].head = cmd;

    cur_cmd = cmd;

    //init background task struct
    for(i = 0; i < MAX_BG_COUNT;i++)
    {
        s_bg_task[i].status = BG_TASK_IDLE;
        s_bg_task[i].cmd = NULL;
        s_bg_task[i].handle = AK_INVALID_TASK;

        //alloc memory for stack and args
        s_bg_task[i].stack = (char *)drv_malloc(COMMAND_STACK_SIZE);
        s_bg_task[i].argv[0]= (char *)drv_malloc(MAX_ARGS_NUM*MAX_ARG_LEN);

        if(s_bg_task[i].stack == NULL || s_bg_task[i].argv[0] == NULL){
            s_bg_task[i].status = BG_TASK_NULL;
            break;
        }

        //set argv address
        for(j=1; j<MAX_ARGS_NUM; j++)
            s_bg_task[i].argv[j] = s_bg_task[i].argv[j-1] + MAX_ARG_LEN;
    }
}

/**
 * @brief  register a command 
 * 
 * @param   name       command name
 * @param   cmd_func   command function
 * @param   help       the first element to display in command list to 'help' command
                       the second element to display in '[command] help' command
 * @return  none
 */
void cmd_register(signed char name[], T_pfCMD cmd_func,  char * help[])
{
    if (index <CMD_TOTAL_AMOUNT -1)
    {
        strcpy(main_cmd[index].name, name);
//        strcpy(main_cmd[index].help, help);
        main_cmd[index].help[0] = help[0];
        main_cmd[index].help[1] = help[1];
        main_cmd[index].cmd    = cmd_func;
        main_cmd[index].sigfunc.term = CMD_SIG_DFL;
        main_cmd[index].head   = NULL;
        main_cmd[index].parent = NULL;
        index++;
    }
	else
	{
		printk("command is out of range\n");
	}
}

void cmd_enter(T_CMD_INFO *cmd)
{
    unsigned short i;
#if 0    
    if (cmd==NULL)
    {
        printk("command is null\n");
        return;
    }
    if (cmd == cur_cmd)
    {
        //printk("command no enter\n");
        return;
    }
    if (cur_cmd)
    {
        for (i=0; cmd[i].cmd; i++)
        {
            cmd[i].parent = cur_cmd;
            cmd[i].head = cmd;
        }
    }
    cur_cmd = cmd;
#endif    
}

bool cmd_exit(void)
{
#if 0    
    if (cur_cmd->parent)
    {
        cur_cmd = cur_cmd->parent;
        return true;
    }
    else
    {
        printk("command exit null\n");
        return false;
    }
#else
	return true;
#endif
}

/**
 * @brief  get current command running
 */
static T_CMD_INFO *get_cur_cmd(void)
{
    T_hTask cur_task = AK_GetCurrent_Task();
    int i;

    //loop to find which bg task this function is called in
    for(i = 0; i < MAX_BG_COUNT; i++) {
        if((s_bg_task[i].status == BG_TASK_RUNNING) && (s_bg_task[i].handle == cur_task))
            return s_bg_task[i].cmd;
    }

    return cur_cmd;
}

/**
 * @brief  background task function
 */
static void cmd_bg_task(unsigned long argc, void *argv)
{	
    if(argc >= MAX_BG_COUNT)
        return;

    T_BG_TASK *pBgTask = (T_BG_TASK *)&s_bg_task[argc];

    printk("[%d]: %s\n", argc, pBgTask->cmd->name);

    //call background task
    pBgTask->cmd->cmd(pBgTask->argc, (char **)(pBgTask->argv));

    printk("[%d]: %s stopped\n", argc, pBgTask->cmd->name);
    
    pBgTask->status = BG_TASK_END;
    pBgTask->cmd = NULL;
}

/**
 * @brief  call jobs function
 */
static void cmd_call_job(T_CMD_INFO *cmd, unsigned long argc, void *argp)
{
    T_hTask handle;
    unsigned long *Command_Task_Stack;
    int i, j;
    char **argv = (char **)argp;
    char name[MAX_CMD_LEN+8];
    
    //find a suitable bg task id
    for(i = 0; i < MAX_BG_COUNT; i++) 
    {
        if(s_bg_task[i].status == BG_TASK_IDLE){
            break;
        }
        else if(s_bg_task[i].status == BG_TASK_END)
        {
            AK_Delete_Task(s_bg_task[i].handle);
            s_bg_task[i].handle = AK_INVALID_TASK;
            s_bg_task[i].status = BG_TASK_IDLE;
            break;
        }
    }

    //bg task is full
    if(i >= MAX_BG_COUNT)
    {
        printk("[jobs]: full\n");
        return;
    }

    //save args
    s_bg_task[i].cmd = cmd;
    s_bg_task[i].status = BG_TASK_RUNNING;
    s_bg_task[i].argc = argc;

    for(j = 0; j < argc; j++)
    {
        strcpy(s_bg_task[i].argv[j], argv[j]);
    }

    //start task
    sprintf(name, "%s_%d", cmd->name, i);
    s_bg_task[i].handle = AK_Create_Task(cmd_bg_task, name, i, NULL,
            s_bg_task[i].stack, COMMAND_STACK_SIZE,
            100, 2,
            AK_PREEMPT, AK_START);

}

/**
 * @brief  show all background task
 * 
 * @param   none
 * @return  none
 */
void cmd_show_jobs(void)
{
    int i, count = 0;

    for(i = 0; i < MAX_BG_COUNT; i++)
    {
        if(s_bg_task[i].status == BG_TASK_RUNNING)
        {
            count++;
            printk("[%d]: %s\n", i, s_bg_task[i].cmd->name);
        }
    }

    if(count == 0) {
        printk("[jobs]: none\n");
    }
}

/**
 * @brief  show one background task
 * 
 * @param   id: job id
 * @return  none
 */
void cmd_kill_job(int id)
{
    if(id < 0 || id >= MAX_BG_COUNT ||s_bg_task[id].status != BG_TASK_RUNNING)
    {
        printk("[%d]: not running\n", id);
        return;
    }
    
    if(s_bg_task[id].cmd->sigfunc.term!= NULL)
    {
        s_bg_task[id].cmd->sigfunc.term(CMD_SIGTERM);
    }
    else
    {
        printk("[%d]: ignored\n", id);
    }
}

/**
 * @brief  register signal handle function
 * 
 * @param   signo: signal number
 * @param   func: signal handle function
 * @return  signal handle function, 
            CMD_SIG_DFL: register fail 
 */
T_pfSigfunc cmd_signal(int signo, T_pfSigfunc func)
{
    if(signo == CMD_SIGTERM)
    {
        T_CMD_INFO *cmd = get_cur_cmd();
        if(cmd != NULL)
        {
            cmd->sigfunc.term = func;
            return cmd->sigfunc.term;
        }
    }

    return CMD_SIG_DFL;
}

static bool cmd_handle(signed char *name)
{
    T_CMD_INFO *tmp;
    unsigned short i,j;
    char *para;
	char args[MAX_ARGS_NUM][MAX_ARG_LEN];
	char *argp[MAX_ARGS_NUM];
	int argc;

	for (i=0; i<MAX_ARGS_NUM; i++)
		argp[i] = (char *)args[i];
	
	memset(args, 0, sizeof(args));
	argc = 0;
	
    if (cur_cmd == NULL)
    {
        printk("command is null\n");
        return false;
    }

	para = (char *)strchr(name, ' ');
	if (para !=NULL)
	{
		*para = '\0';
		para ++;

		while(*para)
		{
			j = 0;
			while(para[j] != 0 && para[j] != ' ')
			{
				if(j >= MAX_ARG_LEN - 1)
				{
					printk("command args %d len should less than %d\n",argc, MAX_ARG_LEN);
					return false;
				}
				args[argc][j] = para[j];
				j++;
			}
			args[argc][j] = '\0';
			argc++;
		
			para = (char*)strchr(para, ' ');
			if (para == NULL)
				break;
			para++;
		}

	}


#if 0
	for (i=0; i<argc; i++)		
		printk("arg[%d]=%s\n", i, args[i]); 
#endif

    tmp = cur_cmd->head;
    for (i=0; tmp[i].cmd; i++)
    {
        //printk("%s\n", tmp[i].name);
        if (strcmp(name, tmp[i].name) == 0)
        {
            cur_cmd = &tmp[i];

            if (1 == argc )
            {
            	if (strcmp(args[0],"help") ==0) //show   usage 
            	{	
            		printk("%s",tmp[i].help[1]);
            		return true;
            	}
            }
            if (strlen(tmp[i].help) > 0)
            {
                //printk("================= ENTER %s ===================\n", tmp[i].name);
                //printk("TIPS: %s\n", tmp[i].help);
            }

            //check run in back ground or fore ground
            if((argc > 0) && (strcmp(argp[argc-1], "&") == 0)) 
            {
                cmd_call_job(cur_cmd, argc-1, argp);
            }
            else 
            {
                cur_cmd->cmd(argc, argp);
            }
            
            if (strlen(tmp[i].help) > 0)
            {
                //printk("================= EXIT  %s ===================\n", tmp[i].name);
            }
            return true;
        }
    }
    
    //printk("command error!!\n");  
    return false;
}

static T_CMD_INFO *cmd_current(void)
{
    return cur_cmd->parent;
}


/**
 * @brief  show command list
 * 
 * @param   cmd    common is NULL
 * @return  none
 */
void cmd_show_help(void *cmd)
{
    int i;
    T_CMD_INFO *p = (T_CMD_INFO *)cmd;

    if (p == NULL)
        p = &main_cmd[0];

    //printk("\ncommand list:\n");
    for (i=0; p[i].cmd; i++)
    {
        if (strlen(p[i].help[0]) > 0)    
        {
            if(strlen(p[i].name) >= 8)
                printk("%s\t: %s\n", p[i].name, p[i].help[0]);
            else
                printk("%s\t\t: %s\n", p[i].name, p[i].help[0]);
        }
    }
    printk("\n");
}


static void show_command_line(signed char *name)
{
    if (name)
    {
        while(*name)
        {
            putch(*name);
            name++;
        }
    }
    else
    {
        puts("root");    
    }
    putch('>');
    putch(' ');
}


/**
 * @brief  command loop
 * 
 * @param   none
 * @return  none
 */
void cmd_loop(void)
{
    signed char name[MAX_CMD_LEN];
    T_CMD_INFO *cmd;
    
    while(1)
    {
        cmd = (T_CMD_INFO *)cmd_current();
        if (cmd)
         {
            show_command_line(cmd->name);
         }
        else
        {
            show_command_line(NULL);
        }

        ak_gets(name, MAX_CMD_LEN);   
        cmd_handle(name);   
    }
}

