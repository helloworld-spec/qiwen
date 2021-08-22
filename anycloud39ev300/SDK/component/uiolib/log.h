/*
 * @FILENAME log.h
 * @BEIEF This file provide some macro for logging.
 * Copyright (C) 2011 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
 * @AUTHOR Jacky Lau
 * @DATE 2011-08-26
 * @VERSION 0.1
 * @REF Please refer to the log function of Android
 */

#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

/* error message */
#define ERROR_FD  stderr
#define LOG_ERROR "E/"
#define LOGE(...) fprintf(ERROR_FD, LOG_ERROR LOG_TAG ": " __VA_ARGS__)

/* warnning message */
#define WARN_FD   stderr
#define LOG_WARN  "W/"
#define LOGW(...) fprintf(WARN_FD, LOG_WARN LOG_TAG ": " __VA_ARGS__)

/* information message */
#define INFO_FD   stdout
#define LOG_INFO  "I/"
#define LOGI(...) //fprintf(INFO_FD, LOG_INFO LOG_TAG ": " __VA_ARGS__)

/* debug message */
#define DEBUG_FD  stdout
#define LOG_DEBUG "D/"
#define LOGD(...) //fprintf(DEBUG_FD, LOG_DEBUG LOG_TAG ": " __VA_ARGS__)

/* verbose message */
#define VERBOSE_FD  stdout
#define LOG_VERBOSE "V/"
#define LOGV(...) //fprintf(VERBOSE_FD, LOG_VERBOSE LOG_TAG ": " __VA_ARGS__)

#endif /* __LOG_H */
