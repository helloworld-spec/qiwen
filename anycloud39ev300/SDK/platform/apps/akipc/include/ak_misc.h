#ifndef _AK_MISC_H_
#define _AK_MISC_H_

#include "ak_global.h"

enum day_ctrl_level {
	DAY_LEVEL_RESERVED = 0x00,
	DAY_LEVEL_HH,		//high-high
	DAY_LEVEL_HL,		//high-low
	DAY_LEVEL_LH,		//low-high
	DAY_LEVEL_LL,		//low-low
	DAY_LEVEL_MAX
};

struct ak_ir_check_param {
	int switch_by_rffeed; 	// switch ir cut by rf feed
	int ir_set_mode;		// 0:set night,1:set day,2:auto change day and night
};

enum ak_photosensitive_mode {
	HARDWARE_PHOTOSENSITIVE,	// hardware photosensitive
	AUTO_PHOTOSENSITIVE		// auto ir switch
};

/* not suggest to use */
struct auto_day_night_switch {
    int day_night_mode;				// 0 night,1 day,2 auto
    int day_to_night_lum;			// DAY_TO_NIGHT_LUM_FACTOR
    int night_to_day_lum;			// NIGHT_TO_DAY_LUM_FACTOR
	int night_cnt[5];	// awb night cnt array
	int day_cnt[10];	// awb day cnt array
	int lock_time;						// AWB_TOTAL_CNT_MAX
	int quick_switch_mode;			// quick switch mode
};


enum day_night_switch_mode {
	SET_NIGHT_MODE,
	SET_DAY_MODE,
	SET_AUTO_MODE,
	SET_CLOSE_MODE
};


/**
 * ak_misc_get_version - get misc version
 * return: version string
 */
const char* ak_misc_get_version(void);

/**
 * ak_misc_ipc_first_run: ipc is first run or not
 * return: 1 first run, 0 not first run
 */
int ak_misc_ipc_first_run(void);

/**
 * ak_misc_set_video_day_night: set video day or night mode, according to IR value
 * @vi_handle: opened vi handle
 * @ir_val: IR value, [0, 1]
 * @day_level: day control level, [1, 4]
 * return: 0 success, -1 failed
 */
int ak_misc_set_video_day_night(void *vi_handle, int ir_val, int day_level);

/**
 * ak_misc_start_photosensitive_switch: start photosensitive switch
 * @day_ctrl: day control level, [1, 4]
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
int ak_misc_start_photosensitive_switch(int day_ctrl);

/**
 * ak_misc_switch_photosensitive_ircut - photosensitive and ircut switch
 * @enable[IN]: 0 disable, 1 enable
 * return: 0 success, -1 failed
 * notes:
 */
void ak_misc_switch_photosensitive_ircut(int enable);

/**
 * ak_misc_stop_photosensitive_switch: stop photosensitive switch
 * return: void
 * notice: not suggest to use
 */
void ak_misc_stop_photosensitive_switch(void);

/**
 * ak_misc_init_voice_tips - init voice tips play module
 * @ao_handle[IN]: opened ao handle
 * return: 0 on seccuss, -1 failed
 */
int ak_misc_init_voice_tips(void *ao_handle);

/*
 * ak_misc_add_voice_tips - add voice tips file to be played
 * file_name[IN]: file name include absolutely path
 * file_param[IN]: voice tips file param
 * return: 0 on success, -1 failed
 */
int ak_misc_add_voice_tips(const char *file_name,
                            struct audio_param *file_param);

/*
 * ak_misc_exit_voice_tips - exit play voice tips
 * return: 0 on success, -1 failed
 */
int ak_misc_exit_voice_tips(void);

void ak_misc_sys_ipc_register(void);
void ak_misc_sys_ipc_unregister(void);

/**
 * ak_misc_start_auto_ir_switch: start ircut auto switch
 * @day_ctrl: day control level, [1, 4]
 * @config: auto day night switch config
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
int ak_misc_start_auto_ir_switch(int day_ctrl, 
								struct auto_day_night_switch *config);
									
/**
 * ak_misc_auto_ir_set_status: set ircut switch mode
 * @mode: day/night/auto mode
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
int ak_misc_auto_ir_set_status(enum day_night_switch_mode mode);

/**
 * ak_misc_stop_auto_ir_switch - stop ircut auto switch
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
void ak_misc_stop_auto_ir_switch(void);

/**
 * ak_misc_start_photosensitive_switch_ex: start photosensitive switch
 * @ps_mode: photosensitive mode
 * @day_ctrl: day control level, [1, 4]
 * @day_night_mode: day night switch mode
 * return: 0 success, -1 failed
 */
int ak_misc_start_photosensitive_switch_ex(enum ak_photosensitive_mode ps_mode, 
									int day_ctrl, enum day_night_switch_mode day_night_mode);

/**
 * ak_misc_photosensitive_set_status_ex: set ircut switch mode
 * @mode: day/night/auto mode
 * return: 0 success, -1 failed
 */
int ak_misc_photosensitive_set_status_ex(enum day_night_switch_mode mode);

/**
 * ak_misc_stop_photosensitive_switch_ex - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_misc_stop_photosensitive_switch_ex(void);

#endif
