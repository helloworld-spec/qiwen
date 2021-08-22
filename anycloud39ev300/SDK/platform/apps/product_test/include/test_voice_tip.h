#ifndef _TEST_VOICE_TIP_H_
#define _TEST_VOICE_TIP_H_


/**
 * test_init_voice_tips - init voice tips play module
 * @ao_handle[IN]: opened ao handle
 * return: 0 on seccuss, -1 failed
 */
int test_init_voice_tips(void *ao_handle);

/*
 * test_add_voice_tips - add voice tips file to be played
 * file_name[IN]: file name include absolutely path
 * file_param[IN]: voice tips file param
 * return: 0 on success, -1 failed
 */
int test_add_voice_tips(const char *file_name, struct audio_param *file_param);

/*
 * test_exit_voice_tips - exit play voice tips
 * return: 0 on success, -1 failed
 */
int test_exit_voice_tips(void);
#endif
