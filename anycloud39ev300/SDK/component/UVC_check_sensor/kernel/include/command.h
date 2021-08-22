/**
 * @FILENAME: command.h
 * @BRIEF 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2007-10-23
 * @VERSION 1.0
 * @REF
 */
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "anyka_types.h"

#define LIBKERNEL_COMMON_VERSION "libkernel_common_1.0.01"

#define CMD_SIGTERM         1
#define CMD_SIG_DFL         (void (*)())0

typedef void(*T_pfCMD)(int argc, char **args);

typedef void(*T_pfSigfunc)(int);

const char* ak_kernel_common_get_version(void);

/**
 * @brief  do the functions defined by macro cmd_module_init 
 * 
 * @param   none
 * @return  none
 */
void cmd_do_register(void);

/**
 * @brief  command init
 * 
 * @param   cmd_help  help function to cmd 'help'
 * @return  none
 */
void cmd_init(T_pfCMD cmd_help);


/**
 * @brief  register a command 
 * 
 * @param   name       command name
 * @param   cmd_func   command function
 * @param   help       the first element to display in command list to 'help' command
                       the second element to display in '[command] help' command
 * @return  none
 */
void cmd_register(signed char name[], T_pfCMD cmd_func,  char * help[]);

/**
 * @brief  show command list
 * 
 * @param   cmd    common is NULL
 * @return  none
 */
void cmd_show_help(void *cmd);

/**
 * @brief  command loop
 * 
 * @param   none
 * @return  none
 */
void cmd_loop(void);

/**
 * @brief  show all background task
 * 
 * @param   none
 * @return  none
 */
void cmd_show_jobs(void);

/**
 * @brief  show one background task
 * 
 * @param   id: job id
 * @return  none
 */
void cmd_kill_job(int id);

/**
 * @brief  register signal handle function
 * 
 * @param   signo: signal number
 * @param   func: signal handle function
 * @return  signal handle function, 
                SIG_DFL: register fail 
 */
T_pfSigfunc cmd_signal(int signo, T_pfSigfunc func);

/*
 * Used for initialization calls..
 */
typedef int (*cmd_initcall_t)(void);
typedef void (*cmd_exitcall_t)(void);


#define __cmd_initcall(fn) \
cmd_initcall_t __cmd_initcall_##fn \
__attribute__((__section__(".cmd_initcall"))) = fn;

/**
 *  module init
 */
#define cmd_module_init(x)  __cmd_initcall(x)





#endif

