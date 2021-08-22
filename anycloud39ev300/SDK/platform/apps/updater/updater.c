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
#include "fha_char_interface.h"
#include "httpclient.h"
#include "ftpclient.h"
#include <sys/types.h>  // cdh:add for check partition table under the sys directory
#include <dirent.h>     // cdh:add for check partition table under the sys directory

#define KERNEL_FILE "uImage" // cdh:for uboot format zImage
#define BOOT_FILE "nandboot.bin"
#define LOGO_FILE "logo.bmp"

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



#define PARTITION_CNT  				3

typedef struct
{
    unsigned char partition_name[6];
}T_PARTITION_NAME_INFO;

typedef struct
{
    unsigned long partition_cnt;
    T_PARTITION_NAME_INFO partition_name_info[PARTITION_CNT];
}T_PARTITION_INFO;


#define UPDATE_NULL  ""       // NO UPDATA
#define UPDATE_ING   "U_ING"  //UPDATE ING
#define UPDATE_END   "U_END"  //UPDATE FINISH
#define UPDATE_ERR   "U_ERR"  //UPDATE ERROR, is not to update





int g_bIsSem = 0;

char g_nand_flash_flag = 0;


/*****************************************************************
 *@brief:update the kernel mtd part
 *@author:cao_donghua
 *@date:2017-02-22
 *@param filename:kernel file name
 *@param partition:part mtd index
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
static int UpdateKernelMTD(const char* filename, int partition)
{
	return fha_interface_UpdateMTD(filename, partition);
}



/*****************************************************************
 *@brief:update the file system mtd part
 *@author:cao_donghua
 *@date:2017-02-22
 *@param filename:fs file name
 *@param partition: part mtd index
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
static int UpdateFsMTD(const char* filename, int partition)
{
	return fha_interface_UpdateMTD(filename, partition);
}


/*****************************************************************
 *@brief:update the file system mtd part
 *@author:cao_donghua
 *@date:2017-02-22
 *@param void:none
 *@return:void
 *@retval:none
 ******************************************************************/
static void print_help(void)
{
	printf( "\nUsage:\n"
		"updater [option] [type]=[value]\n\n"
		"option:\n"
		"local	-- the file is on local\n"
		"http	-- the file is on http server\n"
		"ftp	-- the file is on ftp server\n\n"
		"type:\n"
#if 0
		"K	-- update kernel\n"
		"B	-- update nandboot\n"
		"L	-- update bootup logo\n"
		"D	-- update RAM parameter, must update with nandboot!\n"
		"MTDx	-- update mtd x\n"
		"A	-- IP address\n"
		"P	-- port number\n"
		"U	-- server username\n"
		"C	-- server password\n"
		"X	-- need check file, 1 - yes, 0 - no\n\n"
		"m	-- mac addr\n"
		"s	-- serial number\n"
		"b	-- bar code\n"
#else
		"KERNEL	-- update kernel\n"
		"A	-- update root.sqdh4\n"
		"B	-- update usr.sqsh4\n"
		"C	-- update usr.jffs2!\n"
#endif
		"example:\n"
		"updater local K=/mnt/zImage\n"
		"updater ftp K=/path/file1 A=a.b.c.d P=port U=aaa C=xxx\n");
}

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

#if 1
/*****************************************************************
 *@brief:update partition table kernel size after update kernel
 *@author:cao_donghua
 *@date:2017-02-22
 *@param *spartname:part name
 *@param *filename: update filename for get file size
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
static int update_partition_table_kernel_size(char *spartname, const char *filename) 
{
	int ret = 0;
	char partname[100];
	FILE *p = NULL;
	int number = 0;
	char *buffer = NULL;
	struct stat stat_buf;
	int in_fd;
	unsigned int file_size;
	
	buffer = (char *)calloc(1, sizeof(char)*4);
	memset(buffer, 0, 4);
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
		printf("err:no this partition directory\n");
		return -1;	
	}

	/*
	* third step: get the correspond mtd block index from this partition directory mtd_index file 
	*/
	sprintf(partname, "/sys/kernel/partition_table/%s/%s", spartname, "file_length");
	//printf("partition:%s\n", partname);
	p = fopen(partname, "wb+");
	if (p == NULL){
		printf("open file failed!\n");
		fclose(p);
		return -1;
	}

	/*
	* cdh:get kernel image file size for file size
	*/
	in_fd = open(filename, O_RDONLY);
	if (in_fd < 0) {
		fprintf(stderr, "open %s failed\n", filename);
		close(in_fd);
		fclose(p);
		return -1;
	}
	
	if (fstat(in_fd, &stat_buf) == -1) {
		fprintf(stderr, "get fstat failed\n");
		close(in_fd);
		fclose(p);
		return -1;
	}
	
	file_size = stat_buf.st_size;
	
	
	/*
	* fourth step:transmit int to string
	*/
	sprintf(buffer, "%8x", file_size);
	//printf("%s, new kernel size:0x%x\n", buffer, file_size);

	/*
	* fifth step:update write new kernel size to partition table kernel part file length
	*/
	number = fwrite(buffer, 8, 1, p);
	if (number < 0) {
		printf("write file failed!\n");
		close(in_fd);
		fclose(p);
		return -1;
	}

	/*
	* sixth step:free buffer
	*/
	free(buffer);
	buffer = NULL;
	
	close(in_fd);
	fclose(p);
	
	return ret;
}
#endif

/*****************************************************************
 *@brief:update main entry
 *@author:cao_donghua
 *@date:2017-02-22
 *@param argc:parameter count
 *@param *argv[]: parameter array
 *@return:int
 *@retval:0:success/ other value:fail
 ******************************************************************/
int main(int argc, char *argv[])
{
	char KLinkPath[PATH_LEN];// kernel path
	int  KLpartition = 0;
	char ALinkPath[PATH_LEN];// root.sqsh4 path
	int  ALpartition = 0;
	char BLinkPath[PATH_LEN];// usr.sqsh4 path
	int  BLpartition = 0;
	char CLinkPath[PATH_LEN];// usr.jffs2 path
	int  CLpartition = 0;
	char partname[PARTITION_NAME_LEN]; // cdh:add for save part name
	char kernelimg[KERNEL_IMG_NAME_LEN]; // cdh:add for save kernel image name
	int i = 0, idex = 0;
	T_PARTITION_INFO partition_info = {0};
	char update_flag = 1;

	memset(kernelimg, 0, KERNEL_IMG_NAME_LEN); // cdh:add for avoid uimage and zimage cross update
	memset(partname, 0, PARTITION_NAME_LEN); // cdh:add
	memset(KLinkPath, 0, PATH_LEN);
	memset(ALinkPath, 0, PATH_LEN);
	memset(BLinkPath, 0, PATH_LEN);
	memset(CLinkPath, 0, PATH_LEN);


	memset(&partition_info, 0, sizeof(T_PARTITION_INFO));
	partition_info.partition_cnt = PARTITION_CNT;

	//init 
	strncpy((char *)partition_info.partition_name_info[0].partition_name, UPDATE_NULL, strlen(UPDATE_NULL));
	strncpy((char *)partition_info.partition_name_info[1].partition_name, UPDATE_NULL, strlen(UPDATE_NULL));
	strncpy((char *)partition_info.partition_name_info[2].partition_name, UPDATE_NULL, strlen(UPDATE_NULL));

	/*
	* at least three parameters, if less more than three parameterm, for example:updater [option] [type]=[value], return err after print prompt
	*/ 
	if(argc < 3)
	{
		print_help();
		return -1;
	}

	/*
	* if the first parameter is -h, this instruction need prompting, and return after print prompt
	*/ 
	if (!strcmp(argv[1], "-h")) {
		print_help();
		return -1;
	}


	/*     
	*  updater: update app format
	*  for example:
	*  updater local KERNEL=/tmp/uImage
	*  first step analyse parameter mean
	*  KERNEL=/tmp/uImage
	*  A=/tmp/root.sqsh4
	*  B=/tmp/usr.sqsh4
	*  C=/tmp/usr.jffs2
	*  MAC: add after now
	*  ENV: add after now
	*/ 
	for(i = 2; i < argc; i ++)
	{
		if(argv[i][0] == 'K' && argv[i][6] == '=')  
		{
			/* update KERNEL=/tmp/zImage */ 
			memset(partname, 0, PARTITION_NAME_LEN); 
			strncpy(partname, argv[i] + 0, 6);
			strcpy(KLinkPath, argv[i] + 7);
			//strncpy((char *)partition_info.partition_name_info[0].partition_name, partname, 6);

			/* add for avoid uimage and zimage cross update */ 
			idex  = strlen(argv[i]) - 6;
			strncpy(kernelimg, argv[i] + idex, 6);
			printf("kernel image:%s, idex:%d\n", kernelimg, idex);
			printf("partname:%s\n", partname);
			printf("KLinkPath:%s\n", KLinkPath);
			if (get_partition_mtdblock_index(partname, &KLpartition)) {
				KLpartition = 0xFF; // cdh:get mtd block index failed
			}
			strncpy((char *)partition_info.partition_name_info[0].partition_name, UPDATE_ING, strlen(UPDATE_ING));
#if 0			
			printf("kernel mtd part:%d\n", KLpartition);
			printf("kernel mtd path:%s\n", KLinkPath);
#endif
		}
		else if(argv[i][0] == 'A' && (argv[i][1] == '=' || argv[i][2] == '='))
		{
			memset(partname, 0, PARTITION_NAME_LEN); 
		    if(argv[i][1] == '='){
    			/* update A=/mnt/root.jffs2 */
    			strncpy(partname, argv[i] + 0, 1);
    			strcpy(ALinkPath, argv[i] + 2);
				//strncpy((char *)partition_info.partition_name_info[1].partition_name, partname, 1);
            }
            else {
    			/* update AK=/mnt/root.jffs2 */
    			strncpy(partname, argv[i] + 0, 2);
    			strcpy(ALinkPath, argv[i] + 3);
				//strncpy((char *)partition_info.partition_name_info[1].partition_name, partname, 2);
				strncpy((char *)partition_info.partition_name_info[1].partition_name, UPDATE_ING, strlen(UPDATE_ING));
            }
            printf("partname:%s\n", partname);
			printf("ALpartition:%s\n", ALinkPath);
			if (get_partition_mtdblock_index(partname, &ALpartition)) {
				ALpartition = 0xFF; // cdh:get mtd block index failed
			}
#if 0
			printf("root sqsh4 mtd part:%d\n", ALpartition);
			printf("root sqsh4 mtd path:%s\n", ALinkPath);
#endif

		}
		else if(argv[i][0] == 'B' && (argv[i][1] == '=' || argv[i][2] == '='))
		{
			memset(partname, 0, PARTITION_NAME_LEN); 
		    if(argv[i][1] == '='){
                /* update B=/mnt/usr.jffs2 */
                strncpy(partname, argv[i] + 0, 1);
                strcpy(BLinkPath, argv[i] + 2);
				//strncpy((char *)partition_info.partition_name_info[2].partition_name, partname, 1);
            }
            else {
    			/* update BK=/mnt/usr.jffs2 */
    			strncpy(partname, argv[i] + 0, 2);
    			strcpy(BLinkPath, argv[i] + 3);
				
				//strncpy((char *)partition_info.partition_name_info[2].partition_name, partname, 2);
				strncpy((char *)partition_info.partition_name_info[2].partition_name, UPDATE_ING, strlen(UPDATE_ING));
            }

			printf("partname:%s\n", partname);
			printf("BLinkPath:%s\n", BLinkPath);
			if (get_partition_mtdblock_index(partname, &BLpartition)) {
				BLpartition = 0xFF; // cdh:get mtd block index failed
			}
#if 0
			printf("usr sqsh4 mtd part:%d\n", BLpartition);
			printf("usr sqsh4 mtd path:%s\n", BLinkPath);
#endif
		}
		else if(argv[i][0] == 'C' && (argv[i][1] == '=' || argv[i][2] == '='))
		{
			memset(partname, 0, PARTITION_NAME_LEN); 
		    if(argv[i][1] == '='){
                /* update C=/mnt/jffs2.yaffs2 */
                strncpy(partname, argv[i] + 0, 1);
                strcpy(CLinkPath, argv[i] + 2);
            }
            else {
    			/* update CK=/mnt/jffs2.yaffs2 */
    			strncpy(partname, argv[i] + 0, 2);
    			strcpy(CLinkPath, argv[i] + 3);
            }

			printf("partname:%s\n", partname);
			printf("CLinkPath:%s\n", CLinkPath);
			if (get_partition_mtdblock_index(partname, &CLpartition)) {
				KLpartition = 0xFF; // cdh:get mtd block index failed
			}
#if 0
			printf("usr jffs2 mtd part:%d\n", CLpartition);
			printf("usr jffs2 mtd path:%s\n", CLinkPath);
#endif
		}
		else
		{
			continue;
		}
	}

#if 0
	printf("kernel path:%s\n", KLinkPath);
	printf("root.sqsh4  path:%s\n", ALinkPath);
	printf("usr.sqsh4   path:%s\n", BLinkPath);
	printf("usr.jffs2   path:%s\n", CLinkPath);
#endif

    /*
        *step 2:judge update file from local or net
        *
        */
	if(strcmp(argv[1], "local") == 0) // cdh:local update
	{
		printf("Update from local file\n");
	}
	else
	{
		printf("param error\n");
		return -1;
	}

    //fha_interface_set_protect(0);

    /*
    	 * step 3: start to update file, accord to update file
    	 *
    	 *
    	 */
	if (!init_systemv_sem()) g_bIsSem = 1;  // cdh: I don't know why do this???
	if(-1 == fha_interface_Update_ASA_data((const char* )&partition_info))
	{
		//Sif (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
		printf("update asa_data %d failed\n", CLpartition);
	}
#if 1
	if(KLinkPath[0] != 0)
	{
		if ((strcmp(kernelimg, "zImage") == 0) || (strcmp(kernelimg, "uImage") == 0)) {
			/* update kernel uImage */
			if(UpdateKernelMTD(KLinkPath, KLpartition) == 0)
			{
				if (update_partition_table_kernel_size(partname, KLinkPath) != 0) {
					if (g_bIsSem) semctl(sem_id, KERNEL_UPDATE_FILE, SETVAL, 0);
					printf("update KERNEL bin failure\n");
				    update_flag = 0;
					//memset(partition_info.partition_name_info[0].partition_name, 0, 6);
				}else {
					if (g_bIsSem) semctl(sem_id, KERNEL_UPDATE_FILE, SETVAL, 1);
					printf("update KERNEL bin success\n");
					strncpy((char *)partition_info.partition_name_info[0].partition_name, UPDATE_END, strlen(UPDATE_END));
				}
			}
			else
			{
				if (g_bIsSem) semctl(sem_id, KERNEL_UPDATE_FILE, SETVAL, 0);
				printf("update KERNEL bin failure\n");
				update_flag = 0;
				//memset(partition_info.partition_name_info[0].partition_name, 0, 6);
			}
		}else {
			if (g_bIsSem) semctl(sem_id, KERNEL_UPDATE_FILE, SETVAL, 0);
				printf("update Err KERNEL image bin failure\n");
			//memset(partition_info.partition_name_info[0].partition_name, 0, 6);
		}

	}

	if(ALinkPath[0] != 0)
	{
		/* update root.sqdh4 file */
		if(UpdateFsMTD(ALinkPath, ALpartition) == 0)
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 1);
			printf("update mtd%d success\n", ALpartition);
			strncpy((char *)partition_info.partition_name_info[1].partition_name, UPDATE_END, strlen(UPDATE_END));
		}
		else
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
			printf("update mtd%d failed\n", ALpartition);
			update_flag = 0;
			//memset(partition_info.partition_name_info[1].partition_name, 0, 6);
		}
	}

	if(BLinkPath[0] != 0)
	{
		/* update usr.sqsh4 file */
		if(UpdateFsMTD(BLinkPath, BLpartition) == 0)
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 1);
			printf("update mtd%d success\n", BLpartition);
			strncpy((char *)partition_info.partition_name_info[2].partition_name, UPDATE_END, strlen(UPDATE_END));
		}
		else
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
			printf("update mtd%d failed\n", BLpartition);
			update_flag = 0;
			//memset(partition_info.partition_name_info[2].partition_name, 0, 6);
		}
	}

	if(CLinkPath[0] != 0)
	{
		/* update usr.jffs2 file */
		if(UpdateFsMTD(CLinkPath, CLpartition) == 0)
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 1);
			printf("update mtd%d success\n", CLpartition);
		}
		else
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
			printf("update mtd%d failed\n", CLpartition);
		}
	}
#endif

	if(g_nand_flash_flag == 1 && update_flag == 1)
	{
		if(-1 == fha_interface_Update_ASA_data((const char* )&partition_info))
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
				printf("update asa_data %d failed\n", CLpartition);
		}

		memset(&partition_info, 0, sizeof(T_PARTITION_INFO));
		if(-1 == fha_interface_get_ASA_data((const char*)&partition_info))
		{
			if (g_bIsSem) semctl(sem_id, APP_UPDATE_FILE, SETVAL, 0);
				printf("update asa_data %d failed\n", CLpartition);
		}


		printf("partition_info.partition_cnt:%ld\n", partition_info.partition_cnt);
		printf("partition_info.partition_name_info[0].partition_name:%s\n", partition_info.partition_name_info[0].partition_name);
		printf("partition_info.partition_name_info[1].partition_name:%s\n", partition_info.partition_name_info[1].partition_name);
		printf("partition_info.partition_name_info[2].partition_name:%s\n", partition_info.partition_name_info[2].partition_name);

	    //fha_interface_set_protect(1);
	}

	printf("######### Update End! You Should Reboot The System ######!\n");
	return 0;
}
