/**
 * @filename fha_char_interface.c
 * @brief for linux update use
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-07
 * @version 1.0
 * @ref 
 */
 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <mtd/mtd-abi.h>
#include "test_fha_char.h"

static int erased;
/* argument for erase thread */
struct erase_info_thread {
	int fd;
	struct erase_info_user *einfo;
};

/**
 * fha_mac_CheckMTD - check mac partition.   
 * @partition[IN]: partition No    
 * return: 0, sucess; -1, failed
 */
int fha_mac_CheckMTD(int partition)
{
	int fd;
	int ret = -1;
	char device[16] = {0};		
	char *buf;
	
	struct mtd_info_user info;	
	
	if ((partition == 0)||(partition == 0xFF)){
		fprintf(stderr, "can not update mtd%d\n", partition);
		goto out;
	}

	sprintf(device, "/dev/mtd%d", partition);
	/* open mtd */
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "open %s failed\n", device);
		goto out;
	}
	/* get mtd info */
	if (ioctl(fd, MEMGETINFO, &info)) {
		fprintf(stderr, "get mtd infos failed\n");
		goto err;
	}
	
	/* check if it is nandflash */
	if (info.type == MTD_NANDFLASH) {
		fprintf(stderr, "did not support NAND yet\n");
		goto err;
	}

	buf = calloc(1, info.writesize);
	if (!buf) {
		fprintf(stderr, "calloc memory failed\n");
		goto err;
	}
	
	printf("%s  info.writesize:%d \n",__FUNCTION__,info.writesize);
	
	int a;
	putchar('.');
	/* read mac mtd partition */
	if ((ret = read(fd, buf, 21)) < 0) {
		fprintf(stderr, "read mtd%d failed\n", partition);
		goto err2;
	}
	a=*(int *)buf;
	/* output mac info */
	printf("%s len:%d mac:%s \n",__FUNCTION__,a,buf+4);
		
	ret = 0;

err2:
	free(buf);
err:
	close(fd);
out:
	return ret;
}

/**
 * erase_mtd - erase mtd partition.   
 * @data[IN]: erase info    
 * return:  null
 */
static void *erase_mtd(void *data)
{
	struct erase_info_thread *einfot = data;

	/* execute to erase */
	if (ioctl(einfot->fd, MEMERASE, einfot->einfo)) {
		fprintf(stderr, "erase mtd failed\n");
		printf("%s Erase failed\n",__FUNCTION__);
		erased = 0;
		return NULL;
	}
	/* set erase finish flag*/
	erased = 1;
	
	printf("%s Erase done\n",__FUNCTION__);
	return NULL;
}

/**
 * fha_interface_UpdateMTD - write mac data to mtd partition.   
 * @filename[IN]: mac addr file    
 * @partition[IN]: partition No    
 * return:  0, sucess; -1, failed
 */
int fha_interface_UpdateMTD(const char* filename, int partition)
{
	int fd, in_fd;
	int i;
	int ret = -1;
	char device[16] = {0};
	int cycle;
	unsigned long len;
	unsigned long last_len;
	char *buf;
	static pthread_t ptid;

	struct stat stat_buf;
	struct mtd_info_user info;
	struct erase_info_user einfo;
	struct erase_info_thread einfot;
	
	if ((partition == 0)||(partition == 0xFF)){
		fprintf(stderr, "can not update mtd%d\n", partition);
		goto out;
	}

	sprintf(device, "/dev/mtd%d", partition);
	printf("Open %s, file name:%s\n", device, filename); // cdh:test
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "open %s failed\n", device);
		goto out;
	}
	/* get mtd info */
	if (ioctl(fd, MEMGETINFO, &info)) {
		fprintf(stderr, "get mtd infos failed\n");
		goto err;
	}
	
	/* check if it is nandflash */
	if (info.type == MTD_NANDFLASH) {
		fprintf(stderr, "did not support NAND yet\n");
		goto err;
	}

	if ((in_fd = open(filename, O_RDONLY)) < 0) {
		fprintf(stderr, "open %s failed\n", filename);
		goto err;
	}
	if (fstat(in_fd, &stat_buf) == -1) {
		fprintf(stderr, "get fstat failed\n");
		goto err1;
	}
	len = stat_buf.st_size;
	if (len > info.size) {
		fprintf(stderr, "image file large than mtd partition\n");
		goto err1;
	}

	/* warnning */
	printf("Start to erase %s, Don`t halt the system!\n", device);
	einfo.start = 0;
	einfo.length = info.size;

	einfot.fd = fd;
	einfot.einfo = &einfo;

	erased = 0;
	/* create erase thread*/
	ret = pthread_create(&ptid, NULL, erase_mtd, &einfot);
	if (ret) {
		fprintf(stderr, "create erase thread failed\n");
		goto err1;
	}

	/* wait erase finish*/
	while(!erased) {
		fprintf(stderr, ".");
		sleep(1);
	}
	printf("\n");
	printf("Erase done\n");

	buf = calloc(1, info.writesize);
	if (!buf) {
		fprintf(stderr, "calloc memory failed\n");
		goto err1;
	}

	cycle = len / info.writesize;
	last_len = len % info.writesize;
	
	/* 
	 * read data from file,
	 * and then write to mtd partition. 
	 */
	for (i = 0; i < cycle; i++) {
		putchar('.');
		if ((ret = read(in_fd, buf, info.writesize)) < 0) {
			fprintf(stderr, "read %s failed\n", filename);
			goto err2;
		}
		if ((ret = write(fd, buf, info.writesize)) < 0) {
			fprintf(stderr, "write %s failed\n", device);
			goto err2;
		}
	}

	if (last_len) {
		int a;
		putchar('.');
		if ((ret = read(in_fd, buf, last_len)) < 0) {
			fprintf(stderr, "read %s failed\n", filename);
			goto err2;
		}
		a=*(int *)buf;
		printf("len:%d mac:%s \n",a,buf+4);
		memset((buf + last_len), 0xff, (info.writesize - last_len));
		if ((ret = write(fd, buf, info.writesize)) < 0) {
			fprintf(stderr, "write %s failed\n", device);
			goto err2;
		}
	}
	/* 
	 * read info from mtd.
	 * and then put out 
	 */
	fha_mac_CheckMTD(partition);
	ret = 0;

err2:
	free(buf);

err1:
	close(in_fd);
err:
	close(fd);
out:
	return ret;
}


