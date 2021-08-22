/**
 * @filename updater.c
 * @brief for linux update use
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-07
 * @version 1.0
 * @ref 
 */
/*how to use*/
/*
 *			local K=./zImage B=./nandboot.bin L=./logo.bmp D=./ddrpar.txt
 *./updater http K=http://www.a.com/zI B=http://www.a.com/nb.bin L=http://www.a.com/l.bmp X=1 D=./ddrpar.txt
 *			ftp K=/path/file1 B=/path/file2 L=/path/file3 A=a.b.c.d P=port U=aaa C=xxx X=1 D=./ddrpar.txt
 *
 * local:the file is on local
 * http:the file is on http server
 * ftp:the file is on ftp server
 *
 * K:kernel
 * B:nandboot
 * L:logo
 * A:ip addr
 * P:port
 * U:username
 * C:password
 * MTD[x]:update mtd[x]
 * */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/fcntl.h>

#include <sys/types.h>  // cdh:add for check partition table under the sys directory
#include <dirent.h>     // cdh:add for check partition table under the sys directory

#include "test_fha_char.h"

int bNeedCheckFile = 1;// wheteher need check or not


#define CAMERA			"/etc/init.d/rcS"

#define SEM_PROJ_ID	0x23
#define SEM_NUMS		12
#define PARTITION_NAME_LEN                  	6  // cdh:add for part name len
#define KERNEL_IMG_NAME_LEN                  	7  // cdh:add for kernel image name len

#define PATH_LEN 								256


#define KERNEL_UPDATE_OP						0
#define KERNEL_UPDATE_OP_DOWNLOAD				1
#define KERNEL_UPDATE_SD						2
#define KERNEL_UPDATE_DOWNLOAD					3
#define KERNEL_UPDATE_CAN_BURN_KER				4
#define KERNEL_UPDATE_FILE						5

#define APP_UPDATE_OP							6
#define APP_UPDATE_OP_DOWNLOAD					7
#define APP_UPDATE_OP_SD						8
#define APP_UPDATE_DOWNLOAD						9
#define APP_UPDATE_CAN_BURN_APP					10
#define APP_UPDATE_FILE							11

key_t sem_key;
int sem_id;

union semun{
	int			val;			//semctl SETVAL
	struct semid_ds *	buf;	//semctl IPC_STAT, IPC_SET
	unsigned short	*	array; 	//Array for GETALL, SETALL
	struct seminfo	*	 __buf;	//IPC_INFO
};

int g_bIsSem = 0;


/**
 * *  @brief       init the system V semaphore
 * *  @author      
 * *  @date        2013-5-9
 * *  @param[in]   none
 * *  @return	   	none
 * */
static int init_systemv_sem(void)
{
	union semun seminfo;
	unsigned short array[SEM_NUMS] = {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1};
	
	sem_key = ftok(CAMERA, SEM_PROJ_ID); //use camera.ini to generate the key
	if (sem_key < 0) {
		printf("[%s:%d] ftok fail!", __func__, __LINE__);
		return -1;
	}
	
	if ((sem_id = semget(sem_key, 0, 0)) < 0) {
		if ((sem_id = semget(sem_key, SEM_NUMS, IPC_CREAT | 0666)) < 0) {
			printf("[%s:%d] semget fail, error %d!", __func__, __LINE__, errno);
			return -1;
		}
		
		seminfo.array = array;
		if (semctl(sem_id, 0, SETALL, seminfo) < 0) {
			printf("[%s:%d] semctl fail, error %d!", __func__, __LINE__, errno);
			return -1;
		}
	}
	
	return 0;
}


/*****************************************************************
 *@brief:get part mtd index accord from part name
 *@author:cao_donghua
 *@date:2017-02-22
 *@param *spartname:part name
 *@param *mtd_block_index: geted part mtd index
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
static int get_partition_mtdblock_index(char *spartname, int *mtd_block_index) 
{
	int ret = 0;
	char partname[100];
	FILE *p = NULL;
	int number = 0;
	char *buffer = NULL;

	buffer = (char *)calloc(1, sizeof(char)*2);
	memset(buffer, 0, 2);
	memset(partname, 0, 100);
	
	/*
	* first step: judge partition name whether is null or not
	*/
	if (spartname == NULL) {
		printf("err:no this partition name\n");
		return -1;
	}

	/*
	* second step: judge partition table whether is exist or not,
	* search part name directory from  partition table 
	*/
	sprintf(partname, "/sys/kernel/partition_table/%s", spartname);
	//printf("partition:%s\n", partname);
	if (opendir(partname) == NULL) {
		printf("err:no this partition directory, partname: %s\n",
				partname);
		return -1;	
	}

	/*
	* third step: get the correspond mtd block index from this partition directory mtd_index file 
	*/
	sprintf(partname, "/sys/kernel/partition_table/%s/%s", spartname, "mtd_index");
	//printf("partition:%s\n", partname);
	p = fopen(partname, "r");
	if (p == NULL){
		printf("open file failed!\n");
		fclose(p);
		return -1;
	}

	/*
	* fourth step:read out mtd_index, but is string ,the function same cat cmd
	*/
	number = fread(buffer, 1, 1, p);
	if (number < 0) {
		printf("read file failed!\n");
		fclose(p);
		return -1;
	}

	/*
	* fifth step:transmit string to int
	*/
	sscanf(buffer, "%x", mtd_block_index);
	//printf("%s, mtd_block_index:%d\n", buffer, *mtd_block_index);

	/*
	* sixth step:free buffer
	*/
	free(buffer);
	buffer = NULL;
	
	fclose(p);

	return ret;
}


/*****************************************************************
 *@brief:update main entry
 *@author:cao_donghua
 *@date:2017-02-22
 *@param argc:parameter count
 *@param *argv[]: parameter array
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int test_update_mac(char *mac_str)
{
	
	char MLinkPath[PATH_LEN];// mac path
	int  MLpartition = 0;

	printf("%s param:%s\n",__FUNCTION__,mac_str);
	char partname[PARTITION_NAME_LEN]; // cdh:add for save part name
		
	memset(partname, 0, PARTITION_NAME_LEN); // cdh:add
	memset(MLinkPath, 0, PATH_LEN);
	
	/* update MAC=/tmp/mac.txt */ 
	strncpy(partname, mac_str + 0, 3);
	strcpy(MLinkPath, mac_str + 4);
						
	if (get_partition_mtdblock_index(partname, &MLpartition)) {
		MLpartition = 0xFF; //get mtd block index failed
		printf("update MAC failed:get_partition_mtdblock_index failed\n");
		return -1;
	}
	
	
	if (!init_systemv_sem()) g_bIsSem = 1;  // cdh: I don't know why do this???

	
	if(MLinkPath[0] != 0) 
	{
		/* update MAC file */ 
		if(fha_interface_UpdateMTD(MLinkPath, MLpartition) == 0)
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 1);
			printf("update MAC success\n");
		}
		else
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
			printf("update MAC failed\n");
		}
	}
	
	printf("# Update mac End! !\n");
	return 0;
}



