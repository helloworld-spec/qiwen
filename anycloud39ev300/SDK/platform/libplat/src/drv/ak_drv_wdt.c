#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "internal_error.h"
#include "ak_common.h"
#include "ak_drv_wdt.h"

#define WATCHDOG_FILE_PATH 	"/dev/watchdog"
#define WATCHDOG_IOCTL_BASE 'W'
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE, 7, int)

static const char drv_wdt_version[] = "libplat_drv_wdt V1.0.00";
static int wdt_fd = -1; /* device fd define */
static int wdt_feed_time = 0; /* watch dog feed time define */

/**
 * ak_drv_wdt_get_version - get current module's version string
 * return: pointer to current module's version string
 * note:
 */
const char* ak_drv_wdt_get_version(void)
{
	return drv_wdt_version;
}

/**
 * ak_drv_wdt_open - open watch dog and watch dog start work.
 * @feed_time: [in] second
 * return: 0 - success; otherwise -1;
 * note: driver limit feed_time max to 8 second.
 */
int ak_drv_wdt_open(unsigned int feed_time)
{
	/* arguments check */
	if(wdt_fd >= 0) {
		ak_print_notice("watch dog have been opened.\n");
		return -1;
	}
	/* feed time should > 0 */
	if(feed_time < 1){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("invalid arg.\n");
		return -1;
	}
	/* open watch dog device */
	wdt_fd = open(WATCHDOG_FILE_PATH, O_RDONLY);
	if(wdt_fd < 0) {
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		ak_print_error("open watch dog  fail.\n");
		return -1;
	}
	/*
	 * set timeout
	 * set close-on-exec flag
	 */
	wdt_feed_time = feed_time;
	if(fcntl(wdt_fd, F_SETFD, FD_CLOEXEC)< 0){
		set_error_no(ERROR_TYPE_DEV_CTRL_FAILED);
		ak_print_error_ex("fcntl fail.\n");
		return -1;
	}

	return ioctl(wdt_fd, WDIOC_SETTIMEOUT, &wdt_feed_time);
}

/**
 * ak_drv_wdt_feed - feed watch dog.
 * return:0 - success; otherwise -1;
 */
int ak_drv_wdt_feed(void)
{
	/* not open watch dog */
	if(wdt_fd < 0) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("watch dog not opened.\n");
		return -1;
	}
	 return ioctl(wdt_fd, WDIOC_SETTIMEOUT, &wdt_feed_time);
}

/**
 * ak_drv_wdt_close - close watch dog.
 * return:0 - success; otherwise -1;
 */
int ak_drv_wdt_close(void)
{
	if(wdt_fd < 0) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("watch dog not opened.\n");
		return -1;
	}
	/*
	 * set timeout -1 is to close watch dog,
	 * its difference with close device.
	 * if close device directly, system will reboot
	 */
	wdt_feed_time = -1;
	/*
	 * before we close watch dog,
	 * set watch dog alive .
	 */
	if(ioctl(wdt_fd, WDIOC_SETTIMEOUT, &wdt_feed_time) < 0){
		set_error_no(ERROR_TYPE_DEV_CTRL_FAILED);
		ak_print_error_ex("ioctl fail.\n");
		return -1;
	}
	/* close device */
	if(close(wdt_fd) < 0){
		ak_print_error_ex("close watch_dog fail.\n");
		return -1;
	}
	wdt_fd = -1;

	return 0;
}
