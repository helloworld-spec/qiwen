#ifndef _PLAT_PCM_H_
#define _PLAT_PCM_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "akpcm.h"
#include "ak_common.h"
#include "ak_ai.h"
#include "kernel.h"

typedef struct pcm_user {
	void *adc_in;
    int interval;
	int l2_buf_interval;//record l2_buf_interval,drop first pcm data
	struct akpcm_pars param;	
	enum ai_source source;

} pcm_user_t;
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
