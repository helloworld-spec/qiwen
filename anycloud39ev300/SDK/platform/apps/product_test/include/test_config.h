#ifndef _TEST_CONFIG_H_
#define _TEST_CONFIG_H_

struct config_handle {
	void *handle;
	char osd_name[50];
	char dev_name[100];
	char uid_name[100];	
	int dhcp;
	unsigned int soft_version;
	int sample_rate;	//audio sample rate

	/*camera config*/
	int main_width;
    int main_height;
    int sub_width;
    int sub_height;
	int main_max_width;
    int main_max_height;
    int sub_max_width;
    int sub_max_height;
	int day_ctrl;
	int irled_mode;

	/*video config*/
	int main_min_qp;
    int main_max_qp;
	int sub_min_qp;
    int sub_max_qp;
	
    int main_fps;
    int main_min_kbps;
    int main_max_kbps;

    int sub_fps;
    int sub_min_kbps;
    int sub_max_kbps;

    int main_gop_len;
	int sub_gop_len;
	int main_video_mode;
	int sub_video_mode;	
	int main_enc_type;
	int sub_enc_type;

	/* ratio of target bitrate and max bitrate */
	int main_target_ratio_264;
	int sub_target_ratio_264;
	int main_target_ratio_265;
	int sub_target_ratio_265;

	/* smart encode model configurations */
	int main_smart_goplen;
	int sub_smart_goplen;
	int main_smart_mode;
	int sub_smart_mode;
	int main_smart_quality_264;
	int main_smart_quality_265;
	int sub_smart_quality_264;
	int sub_smart_quality_265;

	/* ratio of target bitrate and max bitrate on smart mode */
	int main_smart_target_ratio_264;
	int sub_smart_target_ratio_264;
	int main_smart_target_ratio_265;
	int sub_smart_target_ratio_265;

	int main_smart_static_value;
	int sub_smart_static_value;
};

/**
 * test_init_ini: init ini module
 * return: void
 */
void test_init_ini(void);

/**
 * test_config_get_value: init config value
 * return: config_handle
 */
struct config_handle* test_config_get_value(void);

/**
 * test_exit_ini: exit ini module
 * return: void
 */
void test_exit_ini(void);
#endif
