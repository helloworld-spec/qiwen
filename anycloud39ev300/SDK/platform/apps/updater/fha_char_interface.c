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
#include "fha_char_interface.h"


static int fha_fd = 0;//fha_char接口文件描述符
static char *file;
static int erased;

extern char g_nand_flash_flag;


struct erase_info_thread {//erase线程传入参数
	int fd;
	struct erase_info_user *einfo;
};

/*****************************************************************
 *@brief:the init function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param :void
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_init(void)
{
	file = AK_FHA_CHAR_NODE;
	fha_fd = open(file, O_RDWR);
	if(fha_fd != -1)
	{
		printf("fha char device interface initialize successed!\n");
		return 0;
	}
	else
	{
		printf("fha char device interface initialize failed!\n");
		return -1;
	}
}

/*****************************************************************
 *@brief:the destroy function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param: void
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_destroy(void)
{
	if(fha_fd)
	{
		close(fha_fd);
		return 0;
	}
	else
	{
		return -1;
	}
}


/*****************************************************************
 *@brief:update boot function
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param filename:boot file name
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBoot(const char* filename, const char* ddrparfilename)
{
	int fd = -1;
	T_BufInfo bufInfo;
	memset(&bufInfo, 0, sizeof(bufInfo));
	FILE* f;
	char reg[32], val[32];
	char buff[1024];
	unsigned int nreg, nval;
	int i;
	f = fopen(ddrparfilename, "r");
	if(f != NULL)
	{
		while(NULL != fgets(buff, 1024, f))
		{
			if(strncmp(buff, "[CONFIG_DATA]", strlen("[CONFIG_DATA]")) == 0)
			{
				printf("config data start\n");
				while(NULL != fgets(buff, 1024, f))
				{
					if(strncmp(buff, "[END]", strlen("[END]")) == 0)
					{
						printf("config data end\n");
						break;
					}
					else
					{
						memset(reg, 0, 32);
						memset(val, 0, 32);
						for(i = 0; i < strlen(buff); i ++)
						{

							if(buff[i] == '0' && buff[i + 1] == 'x' && buff[i+10] == ',')
							{
								memcpy(reg, buff + i + 2, 8);
								break;
							}

						}

						for(i = i + 10; i < strlen(buff); i ++)
						{
							if(buff[i] == '0' && buff[i + 1] == 'x' && buff[i+10] == ',')
							{
								memcpy(val, buff + i + 2, 8);
								break;
							}
					
						}
						nreg = strtoll(reg, (char**)NULL, 16);
						nval = strtoll(val, (char**)NULL, 16);
						bufInfo.ddrpar[bufInfo.ddrparcnt][0] = nreg;
						bufInfo.ddrpar[bufInfo.ddrparcnt][1] = nval;
						bufInfo.ddrparcnt ++;
					}
				}
			}
		}
		fclose(f);
		f = NULL;
	}	

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		printf("open file %s error\n", filename);
		return -1;	
	}

	struct stat statbuf;
	fstat(fd, &statbuf);

	int nLen = 0;
	while((nLen = read(fd, bufInfo.buff, MAX_BUF_LEN)) > 0)
	{
		bufInfo.buflen = nLen;
		//printf("bufInfo.buflen=%ld,, nLen=%ld\n", bufInfo.buflen, nLen);
		//调用升级boot接口
		if(ioctl(fha_fd, AK_FHA_UPDATE_BOOT, &bufInfo) != 0)
		{
			printf("update boot error\n");
			close(fd);
			return -1;
		}
	}

	close(fd);
	return 0;
}

/*****************************************************************
 *@brief:update the bin file
 *@author:zhangshenglin
 *@date:2012-12-07
 *@param filename:bin file name
 *@param BinFileName:bin file name in flash
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBin(const char* filename, const char* BinFileName)
{
	int fd = -1;

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		printf("open file %s error\n", filename);
		return -1;	
	}

	T_BinInfo binInfo;
	memset(&binInfo, 0, sizeof(binInfo));
	struct stat statbuf;
	fstat(fd, &statbuf);
	strcpy(binInfo.filename, BinFileName);
	binInfo.filelen = statbuf.st_size;

	if(ioctl(fha_fd, AK_FHA_UPDATE_BIN_BEGIN, &binInfo) != 0)
	{
		printf("update bin begin begin error\n");
		close(fd);
		return -1;
	}

	T_BufInfo bufInfo;
	int nLen = 0;
	long nTmp = 0;
	printf("Start To Update %s\n", binInfo.filename);
	while((nLen = read(fd, bufInfo.buff, MAX_BUF_LEN)) > 0)
	{
		//usleep(100 * 1000);
		bufInfo.buflen = nLen;
		//printf("bufInfo.buflen=%ld,, nLen=%ld\n", bufInfo.buflen, nLen);
		//调用升级bin文件接口
		if(ioctl(fha_fd, AK_FHA_UPDATE_BIN, &bufInfo) != 0)
		{
			printf("update bin error\n");
			close(fd);
			return -1;
		}

		nTmp += nLen;
		printf("progress[%ld/%ld]  %.1f\n", nTmp, binInfo.filelen, (nTmp * 1.0 / binInfo.filelen) * 100.0);
	}
	printf("Finish Update %s\n", binInfo.filename);
	close(fd);
	return 0;
}


//擦除flash线程
static void *erase_mtd(void *data)
{
	struct erase_info_thread *einfot = data;

	//调用mtd层的擦除接口
	if (ioctl(einfot->fd, MEMERASE, einfot->einfo)) {
		fprintf(stderr, "erase mtd failed\n");
		erased = 0;
		return NULL;
	}
	erased = 1;//设置擦除完毕标志

	return NULL;
}

/*****************************************************************
 *@brief:update the mtd
 *@author:zhongjunchao
 *@date:2013-12-19
 *@param filename:the root file name
 *@param partition:partition number
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
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
	//获取mtd信息
	if (ioctl(fd, MEMGETINFO, &info)) {
		fprintf(stderr, "get mtd infos failed\n");
		goto err;
	}
	
	if (info.type == MTD_NANDFLASH)
	{
	    printf("info.type: %d\n", info.type);
	    printf("info.size: %d\n", info.size);
	    printf("info.erasesize: %d\n", info.erasesize);
	    printf("info.writesize: %d\n", info.writesize);
	    printf("info.oobsize: %d\n", info.oobsize);

	    info.writesize = 32*(info.writesize + info.oobsize);
	    printf("writesize: %d\n", info.writesize);
		g_nand_flash_flag = 1;
	}

	//判断是否nandflash，这种方式暂不支持nandflash
#if 0
	if (info.type == MTD_NANDFLASH) {
		fprintf(stderr, "did not support NAND yet\n");
		goto err;
	}
#endif

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

	//创建擦除线程
	ret = pthread_create(&ptid, NULL, erase_mtd, &einfot);
	if (ret) {
		fprintf(stderr, "create erase thread failed\n");
		goto err1;
	}

	//等待擦除完毕
	while(!erased) {
		fprintf(stderr, ".");
		sleep(1);
	}
	erased = 0;
	printf("\n");
	printf("Erase done\n");

	buf = calloc(1, info.writesize);
	if (!buf) {
		fprintf(stderr, "calloc memory failed\n");
		goto err1;
	}

	cycle = len / info.writesize;
	last_len = len % info.writesize;
	//循环从文件中读取，然后写入到mtd
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
		putchar('.');
		if ((ret = read(in_fd, buf, last_len)) < 0) {
			fprintf(stderr, "read %s failed\n", filename);
			goto err2;
		}
		if (info.type == MTD_NANDFLASH)
		{
			//memset((buf + last_len), 0xff, (info.writesize - last_len));
			if ((ret = write(fd, buf, last_len)) < 0) {
				fprintf(stderr, "write %s failed\n", device);
				goto err2;
			}
		}
		else
		{
			memset((buf + last_len), 0xff, (info.writesize - last_len));
			if ((ret = write(fd, buf, info.writesize)) < 0) {
				fprintf(stderr, "write %s failed\n", device);
				goto err2;
			}
		}
	}

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

/*****************************************************************
 *@brief:update the mac addr in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param mac:mac addr, eg:AA:BB:CC:DD:EE:FF
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateMac(const char* mac)
{
	T_BufInfo bufInfo;
	
	memset(&bufInfo, 0, sizeof(bufInfo));
	strcpy(bufInfo.buff, mac);
	bufInfo.buflen = strlen(mac);
	printf("Start To Update mac\n");
	
	//调用升级mac地址接口
	if(ioctl(fha_fd, AK_FHA_UPDATE_MAC, &bufInfo) != 0)
	{
		printf("update mac error\n");
		return -1;
	}

	printf("End To Update mac\n");
	return 0;
}

/*****************************************************************
 *@brief:update the serial number in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param sn:serial number
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateSn(const char* sn)
{
	T_BufInfo bufInfo;
	
	memset(&bufInfo, 0, sizeof(bufInfo));
	strcpy(bufInfo.buff, sn);
	bufInfo.buflen = strlen(sn);
	printf("Start To Update sn\n");
	
	//调用升级序列号接口
	if(ioctl(fha_fd, AK_FHA_UPDATE_SER, &bufInfo) != 0)
	{
		printf("update sn error\n");
		return -1;
	}

	printf("End To Update sn\n");
	return 0;
}

int fha_interface_set_protect(int protect)
{
	int fd;
	char device[16] = {0};
	
	printf("Set Spinand Protect: %d\n", protect);

	sprintf(device, "/dev/mtd%d", 9);
	
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "open %s failed\n", device);
		return -1;
	}

    if (ioctl(fd, AK_PROTECT_CTL, &protect)) {
        fprintf(stderr, "set_protect failed\n");
        return -1;
    }

	close(fd);

	return 0;
}


int fha_interface_Update_ASA_data(const char* asa_data)
{
	int fd;
	char device[16] = {0};
	
	printf("Start To Update ASA\n");

	sprintf(device, "/dev/mtd%d", 9);
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "open %s failed\n", device);
		return -1;
	}

	if (ioctl(fd, AK_UPDATE_PART_NAME, asa_data)) {
		fprintf(stderr, "get mtd infos failed\n");
		return -1;
	}

	close(fd);


	printf("End To Update ASA\n");
	return 0;
}


int fha_interface_get_ASA_data(const char* asa_data)
{
	int fd;
	char device[16] = {0};
	
	printf("Start To get ASA\n");

	sprintf(device, "/dev/mtd%d", 9);
	if ((fd = open(device, O_RDWR)) < 0) {
		fprintf(stderr, "open %s failed\n", device);
		return -1;
	}

	if (ioctl(fd, AK_GET_PART_NAME, asa_data)) {
		fprintf(stderr, "get asa failed\n");
		return -1;
	}

	close(fd);


	printf("End To get ASA\n");
	return 0;
}



/*****************************************************************
 *@brief:update the bar code in asa area
 *@author:zhangshenglin
 *@date:2013-12-19
 *@param bsn:bar code
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int fha_interface_UpdateBsn(const char* bsn)
{
	T_BufInfo bufInfo;
	
	memset(&bufInfo, 0, sizeof(bufInfo));
	strcpy(bufInfo.buff, bsn);
	bufInfo.buflen = strlen(bsn);
	printf("Start To Update bsn\n");
	
	//调用升级条码接口
	if(ioctl(fha_fd, AK_FHA_UPDATE_BSER, &bufInfo) != 0)
	{
		printf("update bsn error\n");
		return -1;
	}

	printf("End To Update bsn\n");
	return 0;
}
