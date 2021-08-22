#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "isp_basic.h"
#include "isp_vi.h"
#include "list.h"
#include "akuio.h"

#include "ak_thread.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define BUFF_NUM 				3
#define DEFAULT_DROP_FRAME_NUM 	3
#define ENOUGH_MEM(min_type, max_type) assert(sizeof(min_type)<=sizeof(max_type))
#define IMAGE_BUFFER_SIZE		4096

#define CHECK_V4L2_DATA_HEAD 	(0)
#define CALC_VI_CAP_TIME 		(0)
#define ISP_FRAME_DEBUG			(0)
#define ISP_CHECK_FRAME_TS		(0)
#define CALC_GET_V4L2_TIME		(0)

/* frame status */
enum frame_status {
	FRAME_STATE_RESERVED,	//reserved flag
	FRAME_STATE_USING,		//usging flag, cann't release
	FRAME_STATE_USE_OK		//when use ok, can release
};

/* frame buffer information */
struct buffer {
	void   *start;
	size_t  length;
};

/* frame buf node */
struct frame_buf {
	struct v4l2_buffer *buf;
	struct list_head buf_list;
	enum frame_status state;
};

enum AK_ISP_RES_TYPE {
	MAIN_CHN_RES,	//main channel resolution type
	SUB_CHN_RES		//sub channel resolution type
};

static struct v4l2_buffer v4l2_buf;
static struct buffer *buffers = NULL;

LIST_HEAD(v4l2_frame_list);	//frame list head

/* 2 frame use time calc value */
static struct ak_timeval frame_tv_start, frame_tv_end;

/* global frame sequence number */
static unsigned long frame_ptr_seq_no = 0;
static unsigned int drop_isp_frame = (BUFF_NUM - 1);
/* isp operate lock and frame list operate lock */
static pthread_mutex_t isp_manipulate_mutex_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t isp_frame_list_lock = PTHREAD_MUTEX_INITIALIZER;
/* buffer memory address */
static void *ion_mem = NULL;
/* video device name */
static char *dev_name = "/dev/video0";
/* drop some of first frame */
static int camera_lost_frame = DEFAULT_DROP_FRAME_NUM;
/* global video device handle */
static int video0_fd = -1;

/* ioctl api */
static int xioctl(int fd, int request, void *arg)
{
	int ret = AK_FAILED;
	
	do {
		ret = ioctl(fd, request, arg);
	} while ((AK_FAILED == ret) && (EINTR == errno));
	
	return ret;
}

static int malloc_capture_buffers(int align_buf_size)
{
	/* allocate buffers memory */
	buffers = calloc(BUFF_NUM, sizeof(struct buffer));
	if (!buffers) {
		ak_print_normal_ex("Out of memory\n");
		return AK_FAILED;
	}

	/* init frame buf's dma buf */
	int ion_size = align_buf_size * BUFF_NUM;
	ion_mem = akuio_alloc_pmem(ion_size);
	if (!ion_mem) {
		ak_print_normal_ex("Allocate %d bytes failed\n", ion_size);
		free(buffers);
		buffers = NULL;
		return AK_FAILED;
	}

	int n_buffers = 0;
	unsigned long temp = akuio_vaddr2paddr(ion_mem) & 7;
	/* buffer begin address must be 8-byte aligned */
	unsigned char *ptemp = ((unsigned char *)ion_mem) + ((8-temp)&7);
	ak_print_notice_ex("ion_mem: %p, temp: 0x%lx\n", 
		ion_mem, akuio_vaddr2paddr(ion_mem));
	
	/* init buffer's data */
	for (n_buffers = 0; n_buffers < BUFF_NUM; ++n_buffers) {
		buffers[n_buffers].length = align_buf_size;
		buffers[n_buffers].start = ptemp + align_buf_size * n_buffers;
		ak_print_notice_ex("n_buffers: %d, start: %p, length: %d\n", 
			n_buffers, buffers[n_buffers].start, buffers[n_buffers].length);
		if (!buffers[n_buffers].start) {
			ak_print_normal_ex("Out of memory\n");
			return -1;
		}
	}
	ak_print_normal_ex("set buffer size: %d ok\n", align_buf_size);

	return AK_SUCCESS;
}

/**
 * set_buffer_size: calculate the size for isp drv buffer and set buffer size
 * @fd[IN]: the handle of video0
 * @buffer_size[IN]: size for the isp drv buffer
 * void
 * notes:
 */
static int set_buffer_size(int fd, enum isp_vi_status status, 
						const int buffer_size)
{
	ak_print_info_ex("enter...\n");
	struct v4l2_requestbuffers req = {0};
	int align_buf_size = (buffer_size + IMAGE_BUFFER_SIZE - 1) /
		IMAGE_BUFFER_SIZE * IMAGE_BUFFER_SIZE;

	req.count  = BUFF_NUM;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	/* request bufs */
	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		ak_print_normal_ex("REQBUFS failed!\n");
		if (EINVAL == errno) {
			ak_print_normal("%s does not support user pointer i/o\n", dev_name);
		} else {
			ak_print_error_ex("VIDIOC_REQBUFS, %s\n", strerror(errno));
		}
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if (ISP_VI_STATUS_OPEN == status) {
		ret = malloc_capture_buffers(align_buf_size);
	}
	ak_print_info_ex("leave...\n");
	
	return ret;
}

/**
 * read_frame: read the frame buffer,if the buffer has data,return true
 * @fd[IN]: the handle of video0
 * return: 0 on success, -1 failed.
 * notes:
 */
static int read_frame(int fd)
{
	memset(&v4l2_buf, 0x00, sizeof(v4l2_buf));
	v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_buf.memory = V4L2_MEMORY_USERPTR;
	v4l2_buf.m.userptr = 0;

	/* dqbuf, its to call driver's get frame interface */
	if (-1 == xioctl(fd, VIDIOC_DQBUF, &v4l2_buf)) {
		switch (errno) {
		case EAGAIN:
			ak_print_normal_ex("interrupt\n");
			return -1;

		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */

		default:
			ak_print_error_ex("VIDIOC_DQBUF, %s\n", strerror(errno));
			return -1;
		}
	}

	unsigned int i = 0;
	for (i = 0; i < BUFF_NUM; ++i) {
		if (v4l2_buf.m.userptr == (unsigned long)buffers[i].start) {
			/* debug code */
#if CHECK_V4L2_DATA_HEAD
			unsigned char *data = (void *)v4l2_buf.m.userptr;
			for (i=0;i<32;i++) {
				if (0==(i%16))
					ak_print_normal("\n");
				ak_print_normal("%02x ", data[0]);
			}
			ak_print_normal("\n");
#endif
			return 0;
		}
	}

	return -1;
}

/**
 * get_capture_cap: get the capability of capture
 * @fd[IN]: the handle of video0
 * return: 0 not has capability of capture, 1 has capability of capture
 * notes:
 */
static int get_capture_cap(int fd)
{
	int ret_cap = AK_TRUE;
	struct v4l2_capability cap;

	/* do ioctl to get querycap */
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		ret_cap = AK_FALSE;
		if (EINVAL == errno) {
			ak_print_normal_ex("%s is no V4L2 device\n", dev_name);
			return -1;
		} else {
			ak_print_normal_ex("VIDIOC_QUERYCAP, %s\n", strerror(errno));
			return -1;
		}
	}

	/* return valure check */
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		ret_cap = AK_FALSE;
		ak_print_normal_ex("%s is no video capture device\n", dev_name);
		return -1;
	}

	return ret_cap;
}

/* api use to alloc frame buf struct */
static struct frame_buf* alloc_frame_buf(void)
{
	/* alloc parent struct */
	struct frame_buf *n = (struct frame_buf *)calloc(1, sizeof(struct frame_buf));
	if (n) {
		/* alloc child struct */
		n->buf = (struct v4l2_buffer *)calloc(1, sizeof(struct v4l2_buffer));
		if (NULL == n->buf) {
			free(n);
			n = NULL;
		}
	}

	if (!n) {
		ak_print_warning_ex("calloc failed\n");
	}
	
	return n;
}

/* free frame buf struct memory */
static void free_frame_buf(struct frame_buf *f)
{
	if (f) {
		if (f->buf) {
			free(f->buf);
			f->buf = NULL;
		}
		
		free(f);
		f = NULL;
	}
}

/**
 * release_data_buf: release the frame buffer
 * @fd[IN]: the handle of video0
 * @pbuf[IN]: buffer address
 * return: 0 success, otherwise failed
 * notes:
 */
static int release_data_buf(int fd, void *pbuf)
{
	struct v4l2_buffer *v4l2_buf = pbuf;
	memset((void *)v4l2_buf->m.userptr, 1, 31*1024);

	/* set specific frame's state */
	struct frame_buf *first = NULL;
	list_for_each_entry(first, &v4l2_frame_list, buf_list) {
		if (first && (first->buf == v4l2_buf))
			first->state = FRAME_STATE_USE_OK;
	}

	/* release all first frame which state is use ok */
	struct frame_buf *tmp = NULL;

	ak_thread_mutex_lock(&isp_frame_list_lock);
	/* only when frame state is use-ok it can do true release */
	list_for_each_entry_safe(first, tmp, &v4l2_frame_list, buf_list) {
		if (first->state == FRAME_STATE_USE_OK) {
			ak_print_debug_ex("release, i = %d\n", first->buf->index);
			if (-1 == xioctl(fd, VIDIOC_QBUF, first->buf))
				ak_print_error_ex("VIDIOC_QBUF, %s\n", strerror(errno));
			/* delete from list */
			list_del(&first->buf_list);
			free_frame_buf(first);
		} else
			break;	//when some one who using this frame, do release late
	}
	ak_thread_mutex_unlock(&isp_frame_list_lock);

	return AK_SUCCESS;
}

/**
 * set_v4l2_qbuf - set V4L2 query buffer
 * @fd[IN]: the handle of video0
 * return: 0 success, -1 failed
 * notes: the isp dev use 4 buffer to save the frame data,but when we get
 * 		frame data,just can get one buffer data
 */
static int set_v4l2_qbuf(int fd)
{
	int ret = AK_SUCCESS;
	unsigned int i = 0;
	struct v4l2_buffer buf;

	for (i = 0; i < BUFF_NUM; ++i) {
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.m.userptr = (unsigned long)buffers[i].start;
		buf.length = buffers[i].length;
#if 0
		ak_print_normal_ex("i:%d, userptr:%lu, length:%d\n",
				i, buf.m.userptr, buf.length);
#endif
		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
			ret = AK_FAILED;
			ak_print_error_ex("VIDIOC_QBUF, %s, i: %d\n", strerror(errno), i);
			break;
		}
	}

	return ret;
}

/**
 * set_data_buffer: set buffer for ISP drv
 * @fd[IN]: the handle of video0
 * @status[IN]: isp vi operate status
 * @buffer_size[IN]: buffer_size = (main channel width *main channel)*3/2 +
 * (sub channel width *sub channel)*3/2
 * return: 0 success, otherwise failed
 * notes:
 */
static int set_data_buffer(int fd, enum isp_vi_status status,
						const int buffer_size)
{
	set_buffer_size(fd, status, buffer_size);

	return set_v4l2_qbuf(fd);
}

/* get frame from driver */
static struct v4l2_buffer* get_v4l2_ptr(void)
{
#if CALC_VI_CAP_TIME
	struct ak_timeval tv_start;
	struct ak_timeval tv_end;
	ak_get_ostime(&tv_start);
#endif

	/* timeout dealwith */
	fd_set fds;
	struct timeval tv;
	struct v4l2_buffer *tmp_buf = NULL;
	struct frame_buf *frame = NULL;
	
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(video0_fd, &fds);

	int ret = select(video0_fd + 1, &fds, NULL, NULL, &tv);
	switch (ret) {
	case -1:
		ak_print_error_ex("select error\n");
		goto get_v4l2_end;
	case 0:
		/* when timeout,return failed to user */
		ak_print_error_ex("select timeout!\n");
#if CALC_VI_CAP_TIME
		ak_get_ostime(&tv_end);
		ak_print_notice_ex("timeout use: %ld(ms), ret: %ld(s), %ld(us)\n",
			ak_diff_ms_time(&tv_end, &tv_start), tv.tv_sec, tv.tv_usec);
#endif
		goto get_v4l2_end;
	default:
		if (read_frame(video0_fd)) {	//we should almost run to here
			ak_print_notice_ex("read frame failed\n");
			goto get_v4l2_end;
		}

		/* get frame ok, store to frame list */
		frame = alloc_frame_buf();
		if (frame) {
			memcpy(frame->buf, &v4l2_buf, sizeof(struct v4l2_buffer));
			frame->state = FRAME_STATE_USING;
			ak_thread_mutex_lock(&isp_frame_list_lock);
			list_add_tail(&frame->buf_list, &v4l2_frame_list);
			ak_thread_mutex_unlock(&isp_frame_list_lock);
			tmp_buf = frame->buf;
		}
		break;
	}

get_v4l2_end:
	return tmp_buf;
}

#if ISP_CHECK_FRAME_TS
/* check frame ts, incase of ts = 0 or ts rollback */
static void check_frame_ts(struct isp_frame *frame)
{
	static unsigned long long pre_ts = 0;
	int cur_fps = isp_get_sensor_fps();
	int interval = (1000 / cur_fps);

	if (1000 % cur_fps) {
		interval += (10 - (interval % 10));
	}

	/* rollback check */
	if (pre_ts > 0) {
		if (pre_ts > frame->ts) {
			ak_print_info_ex("frame rollback, pre_ts=%llu, frame->ts=%llu, diff=%llu\n",
				pre_ts, frame->ts, (pre_ts - frame->ts));
		} else if ((frame->ts - pre_ts) > interval) {
			ak_print_info_ex("over frame interval, cur_fps=%d, interval=%d\n",
				cur_fps, interval);
			ak_print_info_ex("seq_no=%ld, pre_ts=%llu, frame->ts=%llu, diff=%lld\n",
				frame->seq_no, pre_ts, frame->ts, (frame->ts - pre_ts));
		}
	}	

	/* restore to previous value for next cpmpare */
	if (frame->ts > 0) {
		pre_ts = frame->ts;
	}
}
#endif

/**
 * isp_vi_open: open video0 handle
 * @void
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_open(void)
{
	/* check device node */
	struct stat st;
	if (-1 == stat(dev_name, &st)) {
		ak_print_error_ex("Cannot identify '%s': %s\n", dev_name, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		ak_print_notice_ex("%s is no device\n", dev_name);
		return -1;
	}

	/* open device */
	video0_fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == video0_fd) {
		ak_print_error_ex("Cannot open '%s': %s\n", dev_name, strerror(errno));
		return -1;
	}

	/* set fd attrubute incase of inherited by others */
	if (fcntl(video0_fd, F_SETFD, FD_CLOEXEC) == -1) {
		ak_print_notice_ex("fcntl: %s\n", strerror(errno));
		return -1;
	}

	return video0_fd;
}

/**
 * isp_vi_close: close video0 handle
 * @void
 * return: void
 * notes:
 */
void isp_vi_close(void)
{
	if (-1 != video0_fd) {
		close(video0_fd);
		video0_fd = -1;
	}
}

/**
 * isp_vi_get_sensor_attr: get the resolution of sensor support
 * @width[OUT]: the width of sensor resolution
 * @height[OUT]: the height of sensor resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_sensor_attr(int *width, int *height)
{
	int ret = AK_FAILED;
	struct v4l2_cropcap cropcap;

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(video0_fd, VIDIOC_CROPCAP, &cropcap)) {
		if(0 < cropcap.bounds.height) {
			*width = cropcap.bounds.width;
			*height = cropcap.bounds.height;
			ret = AK_SUCCESS;
		}
	} else {
		ret = AK_FAILED;
	}

	return ret;
}

/**
 * isp_vi_set_main_attr: set the property of channel main resolution
 * @width[IN]: the width of the resolution
 * @height[IN]: the height of the resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_main_attr(const int width, const int height)
{
	struct v4l2_format format;
	int m_width = width;
	int m_height = height;

	CLEAR(format);
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (320 == width) {
		m_width = 640;
		m_height = 320;
	}

	format.fmt.pix.width		= m_width;
	format.fmt.pix.height		= m_height;
	format.fmt.pix.pixelformat 	= V4L2_PIX_FMT_YUYV;
	format.fmt.pix.field		= V4L2_FIELD_INTERLACED;

	/* set the fmt */
	int ret = xioctl(video0_fd, VIDIOC_S_FMT, &format);
	if (AK_FAILED == ret) {
		ak_print_error_ex("VIDIOC_S_FMT, %s\n", strerror(errno));
	}

	return ret;
}

/**
 * isp_vi_set_sub_attr: set the property of channel sub resolution
 * @width[IN]: the width of the resolution
 * @height[IN]: the height of the resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_sub_attr(const int width, const int height)
{
	/* camera private cid & data defined*/
	/***********************************/
	

	struct pcid_ch2_output_fmt_data {
		int type;
		int width;
		int height;
	};
	/***********************************/

	int ret = AK_SUCCESS;
	struct pcid_ch2_output_fmt_data *ch2_fmt;
	struct v4l2_streamparm parm;

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ch2_fmt = (struct pcid_ch2_output_fmt_data *)parm.parm.raw_data;
	ch2_fmt->type = PCID_CH2_OUTPUT_FMT;
	ch2_fmt->width = width;
	ch2_fmt->height = height;

	if ( 0 != xioctl(video0_fd, VIDIOC_S_PARM, &parm)) {
		ret = AK_FAILED;
		ak_print_error_ex("VIDIOC_S_PARM fail!!!\n");
	}

	return ret;
}

/**
 * isp_vi_get_raw_data: send cmd to get raw data
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_raw_data(void)
{
	/* camera private cid & data defined*/
	/***********************************/
	struct pcid_raw_data {
		int type;
	};
	/***********************************/

	int ret = AK_SUCCESS;
	struct pcid_raw_data *raw_data;
	struct v4l2_streamparm parm;

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	raw_data = (struct pcid_raw_data *)parm.parm.raw_data;
	raw_data->type = PCID_A_FRAME_RAW;


	if ( 0 != xioctl(video0_fd, VIDIOC_S_PARM, &parm)) {
		ret = AK_FAILED;
		ak_print_error_ex("VIDIOC_S_PARM fail!!!\n");
	}

	return ret;
}


/**
 * isp_vi_set_crop_attr: set the position and resolution of crop
 * @left[IN]: the left position of the crop
 * @top[IN]: the top position of the crop
 * @width[IN]: the width of the crop resolution
 * @height[IN]: the height of the crop resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_crop_attr(const int left, const int top,
		const int width, const int height)
{
	/* get capture capability */
	if (AK_FALSE == get_capture_cap(video0_fd)) {
		ak_print_error_ex("get capture capability failed!\n");
		return AK_FAILED;
	}

	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	int ret = AK_SUCCESS;

	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(video0_fd, VIDIOC_CROPCAP, &cropcap)) {
		/* set the crop */
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c.left = left;
		crop.c.top = top;
		crop.c.width = width;
		crop.c.height = height;

		if (-1 == xioctl(video0_fd, VIDIOC_S_CROP, &crop)) {
			ak_print_error_ex("init CROP failed!\n");
			ret = AK_FAILED;
		} else {
			ak_print_normal("init CROP succeedded! reset to CROP[%d, %d]\n",
				crop.c.width, crop.c.height);
		}
	}

	return ret;
}

/**
 * isp_vi_get_fps: get fps
 * @void
 * return: fps value
 * notes:
 */
int isp_vi_get_fps(void)
{
	return 0;
}

/**
 * isp_vi_set_fps: set fps
 * @fps[IN]: the fps value to be set
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_set_fps(const int fps)
{
	return 0;
}

/**
 * isp_vi_capture_on - open isp capture and set 4 buffer for isp driver
 * @status[IN]: isp vi operate status
 * @buffer_size[IN]: buffer_size = (main channel width *main channel)*3/2 
 *						+ (sub channel width *sub channel)*3/2
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_capture_on(enum isp_vi_status status, const int buffer_size)
{
	ak_print_info_ex("enter..., camera_lost_frame=%d\n", camera_lost_frame);
	/* on and off should only call once */
	if (!camera_lost_frame) {
		return AK_SUCCESS;
	}

	/* set 4 buffer for isp driver */
	set_data_buffer(video0_fd, status, buffer_size);

	/* open capture */
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(video0_fd, VIDIOC_STREAMON, &type)) {
		ak_print_error_ex("STREAMON failed, %s\n", strerror(errno));
		return AK_FAILED;
	}

	/* drop first n frames */
	ak_print_notice_ex("stream on, drop first %d frames\n", camera_lost_frame);
	
	while (camera_lost_frame > 0) {
		ak_thread_mutex_lock(&isp_manipulate_mutex_lock);
		struct v4l2_buffer *frame_ptr = get_v4l2_ptr();
		if (NULL == frame_ptr) {
			ak_print_error_ex("get v4l2 frame ptr failed!\n");
			ak_thread_mutex_unlock(&isp_manipulate_mutex_lock);
			return AK_FAILED;
		}

#if ISP_FRAME_DEBUG
		unsigned long long ts = frame_ptr->timestamp.tv_sec * 1000ULL
			+ frame_ptr->timestamp.tv_usec / 1000ULL;
		ak_print_info_ex("drop frame %d, ts=%llu\n", camera_lost_frame, ts);
#endif

		release_data_buf(video0_fd, frame_ptr);
		camera_lost_frame--;
		ak_thread_mutex_unlock(&isp_manipulate_mutex_lock);
	}
	ak_get_ostime(&frame_tv_start);

	ak_print_info_ex("leave...\n");
	return AK_SUCCESS;
}

/**
 * isp_vi_capture_off: close isp capture
 * @status[IN]: isp vi operate status
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_capture_off(enum isp_vi_status status)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl(video0_fd, VIDIOC_STREAMOFF, &type)) {
		ak_print_normal("STREAMOFF failed\n");
		return AK_FAILED;
	} else {
		ak_print_normal_ex("STREAMOFF succeedded\n");
		camera_lost_frame = BUFF_NUM + 1;
	}

	if (ISP_VI_STATUS_CLOSE == status) {
		if (buffers) {
			free(buffers);
			buffers = NULL;
		}
		if (ion_mem) {
			ak_print_notice_ex("free pmem=%p\n", ion_mem);
			akuio_free_pmem(ion_mem);
			ion_mem = NULL;
		}
	}

	return AK_SUCCESS;
}

static void raw_data_save(unsigned char* buf, unsigned int size)
{
    FILE *fd = NULL;
    int len = size;

    fd = fopen(RAW_SAVE_PATH, "w+b");
   
  	if (NULL == fd)
  	{
  		ak_print_error("raw file create failed.\n");
      	return;
  	}

    do {
      	len -= fwrite(buf + size - len, 1, len, fd);
    }
    while (len > 0);

	fclose(fd);
}


/**
 * isp_vi_get_frame: get frame information
 * @frame[IN]: the frame information
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_frame(struct isp_frame *frame)
{
	int ret = AK_FAILED;
	if (!frame) {
		ak_print_normal_ex("pointer = NULL !\n");
		return ret;
	}	

	/* debug code */
#if CALC_GET_V4L2_TIME
	struct ak_timeval tv_start;
	struct ak_timeval tv_end;
	static struct ak_timeval pre_v4l2_time = {0};
	struct ak_timeval cur_v4l2_time;

	if (0 == pre_v4l2_time.sec) {
		ak_get_ostime(&pre_v4l2_time);
	}
	ak_get_ostime(&tv_start);
#endif	

	ak_thread_mutex_lock(&isp_manipulate_mutex_lock);
	struct v4l2_buffer *frame_ptr = get_v4l2_ptr();
	++frame_ptr_seq_no;
	ak_thread_mutex_unlock(&isp_manipulate_mutex_lock);
	
	/* debug code */
#if CALC_GET_V4L2_TIME
	ak_get_ostime(&tv_end);
	long get_time = ak_diff_ms_time(&tv_end, &tv_start);
	ak_get_ostime(&cur_v4l2_time);
	long diff_time = ak_diff_ms_time(&cur_v4l2_time, &pre_v4l2_time);
	if (diff_time > 80) {
		ak_print_notice_ex("get v4l2 diff time=%ld(ms), get_time=%ld, seq_no=%lu\n", 
			diff_time, get_time, frame_ptr_seq_no);
	}
	ak_get_ostime(&pre_v4l2_time);
#endif

	if (!frame_ptr) {
		ak_print_error_ex("get frame fail\n");
		return ret;
	}

	/* assignment */
	frame->private_data = frame_ptr;
	frame->buf = (void *)frame_ptr->m.userptr;
	frame->ts = frame_ptr->timestamp.tv_sec * 1000ULL
		+ frame_ptr->timestamp.tv_usec / 1000ULL;
	frame->seq_no = frame_ptr_seq_no;
	ak_get_ostime(&(frame->get_time));

	/* drop some useless frames */
	if (drop_isp_frame > 0) {
		/* debug code */
		ak_get_ostime(&frame_tv_end);
		ak_print_notice_ex("capture on --> real get frame time=%ld\n", 
			ak_diff_ms_time(&frame_tv_end, &frame_tv_start));
		ak_print_info_ex("seq_no=%ld, frame->ts=%llu, drop_isp_frame=%d\n",
			frame->seq_no, frame->ts, drop_isp_frame);
		/* true useful code  */
		--drop_isp_frame;
		isp_vi_release_frame(frame->private_data);
		return AK_FAILED;
	}

    if (0 == strncmp((const char *)frame->buf, "AK-RAW", 6))
    {
        int headsize = 0;
        int bitsw = 0;
        int datasize = 0;
        int width = 0;
        int height = 0;
        
        sscanf((const char *)frame->buf, "AK-RAW,headsize:0x%x,bitsw:0x%x,size:0x%x,w:0x%x,h:0x%x", 
            &headsize, &bitsw, &datasize, &width, &height);
        ak_print_notice_ex("get a raw frame, headsize:0x%x,bitsw:0x%x,size:0x%x,w:0x%x,h:0x%x",
            headsize, bitsw, datasize, width, height);

        raw_data_save(frame->buf, datasize + headsize);
        isp_vi_release_frame(frame->private_data);
        return AK_FAILED;
    }

	if (frame->ts > 0) {
		/* debug code */
#if ISP_FRAME_DEBUG	
		ak_print_info_ex("seq_no=%ld, frame->ts=%llu\n",
			frame->seq_no, frame->ts);
#endif			
		ret = AK_SUCCESS;
	} else {
		/* drop some useless frame because its ts=0  */
		ak_print_notice_ex("seq_no=%ld, frame->ts=%llu, we had dropped it!\n",
			frame->seq_no, frame->ts);
		isp_vi_release_frame(frame->private_data);
	}

	/* debug code */
#if ISP_CHECK_FRAME_TS
	check_frame_ts(frame);
#endif	

	return ret;
}

/**
 * isp_vi_reset_drop_frame: reset vi isp drop frame count
 * @void
 * return: void
 * notes: reset drop frame before get frame to encode again.
 */
void isp_vi_reset_drop_frame(void)
{
	drop_isp_frame = (BUFF_NUM - 1);
}

/**
 * isp_vi_release_frame: release the frame buffer
 * @pbuf[IN]: buffer address
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_release_frame(void *pbuf)
{
	return release_data_buf(video0_fd, pbuf);
}

/**
 * isp_vi_stream_ctrl_on: open capture
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_stream_ctrl_on(void)
{
	struct v4l2_requestbuffers req;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fflush(stdout);
	CLEAR(req);
	req.count  = BUFF_NUM;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	/* request buf */
	if (-1 == xioctl(video0_fd, VIDIOC_REQBUFS, &req)) {
		ak_print_normal_ex("REQBUFS failed!\n");
		if (EINVAL == errno) {
			ak_print_normal("%s does not support user pointer i/o\n", dev_name);
			return -1;
		} else {
			ak_print_error_ex("VIDIOC_REQBUFS, %s\n", strerror(errno));
			return -1;
		}
	}

	if (set_v4l2_qbuf(video0_fd)) {
		return AK_FAILED;
	}

	/* open capture */
	if (-1 == xioctl(video0_fd, VIDIOC_STREAMON, &type)) {
		ak_print_error_ex("STREAMON failed, %s\n", strerror(errno));
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * isp_vi_stream_ctrl_off: close capture
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_stream_ctrl_off(void)
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fflush(stdout);
	if (-1 == xioctl(video0_fd, VIDIOC_STREAMOFF, &type)) {
		ak_print_normal("STREAMOFF failed!\n");
		return AK_FAILED;
	} else {
		ak_print_normal("STREAMOFF succeedded!\n");
		camera_lost_frame = BUFF_NUM + 1;
	}

	return AK_SUCCESS;
}
