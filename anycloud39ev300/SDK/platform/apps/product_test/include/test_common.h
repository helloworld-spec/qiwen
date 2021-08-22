#ifndef _TEST_PRODUCT_H_
#define _TEST_PRODUCT_H_

#include <stdint.h>


struct ssid_info
{
	char wify_ssid_name[256];
	int quality;
};

struct wifi_list_info
{
	int cnt;
	struct ssid_info info[100];
};


/**
 * test_audio_start - test audio function.  
 * @ai[IN]: opened ai handle   
 * return: void
 */
void test_audio_start(void *ai);

/**
 * test_ircut_on_off - test ircut function.  
 * @vi_handle[IN]: opened vi handle   
 * @day_ctrl[IN]: day control level, [1, 4]   
 * return:  0, sucess; -1, failed
 */
int test_ircut_on_off(void *vi_handle, int day_ctrl);

/**
 * test_recover_dev - test recover key function.  
 * @sec[IN]: time limit wait key pressed, second      
 * return:  0, sucess; -1, failed
 */
int test_recover_dev(int sec);
int test_get_press_key(void);
void test_reset_press_key(void);

int test_get_conn_state(void);

/**
 * test_mmc - test mmc function.    
 * return:  0, sucess; -1, failed
 */
int test_mmc(void);

uint64_t test_mmc_get_total_size(void);


/**
 * test_wifi - test wifi function.    
 * return:  0, sucess; -1, failed
 */
int test_wifi(void);

int get_wifi_info_list(struct wifi_list_info *wifi_info);
int is_test_wifi_finish(void);
void test_wifi_clean_flag(void);



/**
 * test_ptz_init - int ptz for test  
 * return:   0 , success ;  -1 , failed;
 */
int test_ptz_init(void);

/**
 * test_ptz_exit - exit ptz for test  
 * return:   void;
 */
void test_ptz_exit(void);

/**
 * test_set_ptz_cmd - set cmd to ptz  and  ptz run    
 * @code[IN]: cmd    
 * return:   0 , success ;  -1 , failed;
 */
int test_set_ptz_cmd(int code);

/**
 * test_mac - set mac addr    
 * @macstr[IN]: mac addr string    
 * return:   0 , success ;  -1 , failed;
 */
int test_mac(char *macstr);

/**
 * test_pushid - set  uid  file
 * @uid[IN]: uid str 
 * @data[IN]: uid file data
 * @datalen[IN]: uid file data len
 * @dest_file[IN]: uid dest file path
 * return:   0 , success ;  -1 , failed;
 */
int test_pushid(char *uid, char *data, int datalen, char *dest_file);

#endif
