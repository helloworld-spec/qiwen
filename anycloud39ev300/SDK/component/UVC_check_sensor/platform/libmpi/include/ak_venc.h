#ifndef _AK_VIDEO_ENCODE_H_
#define _AK_VIDEO_ENCODE_H_

/* encode group define */
enum encode_group_type {
	ENCODE_RECORD,   	//local record
	ENCODE_MAINCHN_NET, //main-chn net
	ENCODE_SUBCHN_NET,  //sub-chn net
	ENCODE_PICTURE,  	//picture
	ENCODE_GRP_NUM 		//total
};

/* encode use channel define */
enum encode_use_chn {
	ENCODE_MAIN_CHN,
	ENCODE_SUB_CHN
};

/* h.264 encode control define */
enum bitrate_ctrl_mode {
	BR_MODE_CBR,
	BR_MODE_VBR
};

/* video encode output type define */
enum encode_output_type {
	H264_ENC_TYPE,
	MJPEG_ENC_TYPE
};

enum profile_mode {
	PROFILE_MAIN,
	PROFILE_HIGH,
	PROFILE_BASE
};

struct venc_roi_param {
	int enable;	//1 enable, 0 disable
	long top;
	long bottom;
	long left;
	long right;
	long delta_qp;
};

struct encode_param {
	unsigned long width;		//real encode width, to be divisible by 4
	unsigned long height;		//real encode height, to be divisible by 4
	signed long   minqp;		//Dynamic bit rate parameter[20,25]
	signed long   maxqp;		//Dynamic bit rate parameter[45,50]
	signed long   fps;          //frame rate
	signed long   goplen;       //IP FRAME interval
	signed long   bps;	        //target bps
	enum profile_mode profile; 			//profile mode
	enum encode_use_chn use_chn;		//encode channel, 0: main, 1 sub
	enum encode_group_type enc_grp;		//encode group
	enum bitrate_ctrl_mode br_mode; 	//bitrate control mode, vbr or cbr
	enum encode_output_type enc_out_type;	//encode output type, h264 or jpeg
};


/**
 * ak_venc_get_version - get venc version
 * return: version string
 */
const char* ak_venc_get_version(void);

/**
 * ak_venc_open - open encoder and set encode param
 * @param[IN]: encode param
 * return: on success return encode handle, failed return NULL.
 */
void *ak_venc_open(const struct encode_param *param);

/**
 * ak_venc_send_frame - encode single frame
 * @enc_handle[IN]: encode handle
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @stream[OUT]: encode output buffer address
 * return: on success, return the length of output,
 *         on failed, -1 is return.
 * notes: when working by single mode, the memory of stream->data should
 * 		  free by user 
 */
int ak_venc_send_frame(void *enc_handle, const unsigned char *frame,
		unsigned int frame_len, struct video_stream *stream);

/*
 * ak_venc_send_frame_ex - encode single frame
 * @enc_handle[IN]: encode handle
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @stream[OUT]: encode output buffer address
 * return: 0 success; -1 failed
 * note: IMPORTANT-make sure your stream->data can contain encoded data
 */
int ak_venc_send_frame_ex(void *enc_handle, const unsigned char *frame,
		unsigned int frame_len, struct video_stream *stream);

/**
 * ak_venc_get_fps - get encode frames per second
 * @enc_handle[IN]: enc_handle return by 'ak_venc_open'
 * return: on success the fps, failed 0
 */
int ak_venc_get_fps(void *enc_handle);

/**
 * ak_venc_set_fps-reset encode fps
 * @enc_handle[IN]: encode handle return by 'ak_venc_open'
 * @fps[IN]: fps you want to set
 * return: 0 on success, -1 no effect
 * notes: the 'fps' should be careful do not out of range[1, sensor max fps]
 */
int ak_venc_set_fps(void *enc_handle, int fps);

/**
 * ak_venc_get_kbps - get encode kbps
 * @enc_handle[IN]:  enc_handle return by 'ak_venc_open'
 * return: on success return actually kbps ;-1:failed
 */
int ak_venc_get_kbps(void *enc_handle);

/**
 * ak_venc_set_rc - reset encode bitpersecond
 * @enc_handle[IN]: encode handle return by 'ak_venc_open'
 * @bps[IN]: bps you want to set
 * return: 0 on success, -1 faield
 */
int ak_venc_set_rc(void *enc_handle, int bps);

/**
 * ak_venc_set_iframe - set next encode frame to I frame
 * @enc_handle[IN], encode handle, return by 'ak_venc_open'
 * return: 0 on success, -1 failed
 */
int ak_venc_set_iframe(void *enc_handle);

/**
 * ak_venc_get_gop_len - get encode gop len
 * @enc_handle[IN]: encode handle
 * return: value of GOP len, -1 failed
 * notes: 
 */
int ak_venc_get_gop_len(void *enc_handle);

/**
 * ak_venc_set_gop_len - set encode gop len
 * @enc_handle[IN]: encode handle
 * @gop_len[IN]: value of GOP len
 * return: 0 success, -1 failed
 * notes: set new gop len after you change encode fps
 */
int ak_venc_set_gop_len(void *enc_handle, int gop_len);

/**
 * ak_venc_set_roi - set ROI param
 * @enc_handle[IN]: encode handle
 * @roi[IN]: ROI param
 * return: 0 success, -1 failed
 */
int ak_venc_set_roi(void *enc_handle, struct venc_roi_param *roi);

/**
 * ak_venc_set_br - set video bitrate control mode, cbr or vbr
 * @enc_handle[IN]: encode handle
 * @mode[IN]: see define of enum bitrate_ctrl_mode
 * return: 0 success, -1 failed
 */
int ak_venc_set_br(void *handle, enum bitrate_ctrl_mode mode);

/**
 * ak_venc_set_profile - set video profile, main/high/base
 * @enc_handle[IN]: encode handle
 * @profile[IN]: see define of enum profile
 * return: 0 success, -1 failed
 */
int ak_venc_set_profile(void *handle, enum profile_mode profile);

/**
 * ak_venc_set_rc_weight - set video rate control weight
 * @enc_handle[IN]: encode handle
 * @weight[IN]: quality weight, [0, 100]
 * return: 0 success, -1 failed
 * notes:  quality weight 0 is the best quality, 100 is the lowest bitrate.
 */
int ak_venc_set_rc_weight(void *enc_handle, int weight);

/**
 * ak_venc_set_mjpeg_qlevel - set mjpeg quality level
 * @handle[IN]: encode handle return by ak_venc_open()
 * @level[IN]:quality level, 0-9
 * return: 0 success, specifically error number
 */
int ak_venc_set_mjpeg_qlevel(void *handle, int level);

/**
 * ak_venc_close - close video encode
 * @enc_handle[IN]: encode handle return by ak_venc_open()
 * return: 0 success, -1 failed
 * notes: if you request a stream, you should can this function
 *        after you call 'ak_venc_cancel_stream()' success.
 */
int ak_venc_close(void *enc_handle);

/**
 * ak_venc_request_stream - bind vi and venc handle start capture
 * @vi_handle[IN]: video input handle, return by ak_vi_open()
 * @enc_handle[IN]: encode handle, return by ak_venc_open() 
 * return: on success return stream_handle, failed return NULL
 * notes:
 */
void* ak_venc_request_stream(void *vi_handle, void *enc_handle);

/**
 * ak_venc_get_stream - on stream-encode, get encode output stream
 * @stream_handle[IN]: stream handle
 * @stream[IN]: stream data
 * return: 0 success, failed return specifically error
 * notes: if you got a error, you can use 'ak_get_error_no()' or 
 *        'ak_get_error_str()' to get more detail.
 */
int ak_venc_get_stream(void *stream_handle, struct video_stream *stream);

/**
 * ak_venc_release_stream - release stream resource
 * @stream_handle[IN]: stream handle
 * @stream[IN]: stream return by get()
 * return: 0 success, -1 failed
 * notes:
 */
int ak_venc_release_stream(void *stream_handle, struct video_stream *stream);

/**
 * ak_venc_cancel_stream - cancel stream encode
 * @stream_handle[IN]: encode handle return by ak_venc_request_stream()
 * return: 0 success, specifically error number
 * notes: if you got a error, you can use 'ak_get_error_no()' or 
 *       'ak_get_error_str()' to get more detail.
 */
int ak_venc_cancel_stream(void *stream_handle);

#endif
