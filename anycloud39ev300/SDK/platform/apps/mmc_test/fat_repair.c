#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include <time.h>
#include <fcntl.h>
#include <sys/statfs.h>

#include "anyka_types.h"
#include "mtdlib.h"
#include "file.h"
#include "driver.h"
#include "globallib.h"
#include "global.h"

#include "global_val.h"
#include "printcolor.h"
#include "timecount.h"
#include "main.h"
#include "fat_repair.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

int g_handle;

/**
 * medium_read - read buff from medium for callback use
 * @medium: medium handle
 * @buf: buff for read data.
 * @start: offset of medium
 * @size:  read block num . per block is 512 bytes.
 * return: block num . per block is 512 bytes.
 * notes:
 */
static T_U32 medium_read(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
	off64_t offset_size = (off64_t)start * 512;
	T_U32 read_size = 0;

	if (g_handle == 0) {
		return 0;
	}

	lseek64(g_handle, offset_size, SEEK_SET);
	read_size = read(g_handle, buf, size * 512);
	if (read_size <= 0) {
		return 0;
	}

	return read_size/512;
}

/**
 * medium_write - write buff to medium for callback use
 * @medium: medium handle
 * @buf: buff for write data.
 * @start: offset of medium
 * @size:  wrute block num . per block is 512 bytes.
 * return: block num . per block is 512 bytes.
 * notes:
 */
static T_U32 medium_write(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
	off64_t offset_size = (off64_t)start * 512;
	T_U32 write_size;

	if (g_handle == 0) {
		return 0;
	}

	lseek64(g_handle, offset_size, SEEK_SET);
	write_size = write(g_handle, buf, size * 512);
	if (write_size <= 0) {
		return 0;
	}
	return size;
}

static T_BOOL medium_flush(T_PMEDIUM medium)
{
	return AK_TRUE;
}

/**
 * Creat_Medium - create a medium handle
 * @capacity: tfcard size
 * return: T_PMEDIUM point addr
 * notes:
 */
static T_PMEDIUM Creat_Medium(T_U32 capacity)
{
	T_U32 i;
	T_PMEDIUM pmedium = AK_NULL;
	T_U32 BytsPerSec = 512;


	pmedium = (T_PMEDIUM)malloc(sizeof(T_MEDIUM));

	if(pmedium == AK_NULL) {
		return AK_NULL;
	}

	i = 0;
	while (BytsPerSec > 1) {
		BytsPerSec >>= 1;
		i++;
	}
	pmedium->SecBit = (T_U8) i;

	// if the device is sd, we will set pagesize is 16k
	pmedium->PageBit = (T_U8)i;
	pmedium->SecPerPg = 1;

	pmedium->type = TYPE_MEDIUM;
	pmedium->read = (F_ReadSector)medium_read;
	pmedium->write = (F_WriteSector)medium_write;
	pmedium->capacity = capacity;
	pmedium->flush = medium_flush;
	pmedium->type = MEDIUM_SD;
	pmedium->msg = AK_NULL;
	pmedium->DeleteSec = AK_NULL ;
	return pmedium;
}

/**
 * anyka_fsGetSecond - get unix time seconds for callback use
 * return: unix time seconds
 * notes:
 */
static T_U32 anyka_fsGetSecond(T_VOID)
{
	return time(0);
}

static T_VOID anyka_fsSetSecond(T_U32 seconds)
{

}

static T_S32 anyka_fsUniToAsc(const T_U16 *pUniStr, T_U32 UniStrLen,
                    T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
	int i;
	if ((pUniStr == NULL) || (pAnsibuf == NULL)) {
		return 0 ;
	}
	for(i=0; i < UniStrLen; i++){
		pAnsibuf[i] = pUniStr[i];
	}
	return i;
}

static T_S32 anyka_fsAscToUni(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,
                    T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
	int i;
	if ((pAnsiStr == NULL) || (pUniBuf == NULL)) {
		return 0 ;
	}
	for(i=0; i < AnsiStrLen; i++){
		 pUniBuf[i] = pAnsiStr[i];
	}
	return i;
}

static T_pVOID anyka_fsRamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID var_new;

	var_new = malloc(size+512);
	//printf("anyka_fsRamAlloc():var=%p, sz=%ld\n", var_new, size);
	return var_new;
}

static T_pVOID anyka_fsRamRealloc(T_pVOID var, T_U32 size, T_S8 *filename,
				T_U32 fileline)
{
	T_pVOID var_new;

	var_new = (T_pVOID)malloc( size);
	memcpy(var_new, var, size);
	free(var);
	//printf("anyka_fsRamRealloc():var=%p, sz=%ld\n", var_new, size);
	return var_new;
}

static T_pVOID anyka_fsRamFree(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
#if 1
	//printf("anyka_fsRamFree():var=%p\n", var);
	free(var);
#else
	if (var) {
		free(var);
	}
#endif

	return NULL;
}

static T_S32 anyka_fsOsCrtSem(T_U32 initial_count, T_U8 suspend_type,
				T_S8 *filename, T_U32 fileline)
{
	return 1;
}

static T_S32 anyka_fsOsDelSem(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
	return 1;
}

static T_S32 anyka_fsOsObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *filename,
				T_U32 fileline)
{
	return 1;
}

static T_S32 anyka_fsOsRelSem(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
	return 1;
}

static T_U32 anyka_fsGetChipID(T_VOID)
{
	return 0;                                                                                       //v300 FS_AK39XX_H3 v200 FS_AK39XX_H240
}

static T_pVOID anyka_fsMemCpy(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memcpy(dst, src, count);
}

static T_pVOID anyka_fsMemSet(T_pVOID buf, T_S32 value, T_U32 count)
{
	return memset(buf, value, count);
}

static T_pVOID anyka_fsMemMov(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}

static T_S32 anyka_fsMemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count)
{
	return memcmp(buf1, buf2, count);
}


/**
 * ak_del_file_main - del the last record. fix the tf card read only status.
 * @file_name[IN]: filename of the last record file
 * @dev_name[IN]:  mmcblk device
 * @total_size[IN]: size of tf card
 * return:  null
 */
/*
void ak_repair_tf()
*/
//void ak_repair_tf(const char *file_name, const char *dev_name, unsigned long total_size)
int ak_repair_tf( char *pc_dev, char *pc_path)
{
	T_FSINITINFO fsInfo;
	T_PMEDIUM medium = AK_NULL;
	T_U32 driver = 0;
	T_U16 del_file[LEN_PATH_FILE];
	char ac_file_path[LEN_PATH_FILE];
	int i_color= COLOR_FRONT_RED;
	int i_status = RESULT_STATUS_FAIL;
	int i_res = FIX_STATUS_ERROR;
	struct timeval timeval_begin, timeval_end ;
	int i_hint = FIX_RESULT_UNKNOWN , i;
	unsigned long long i_total_sector;

	timeval_mark( &timeval_begin );
	snprintf(ac_file_path, LEN_PATH_FILE, FAT_FIX_TEMPLATE, pc_path);
	/*
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"pc_dev: %s", pc_dev);
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"ac_file_path: %s", ac_file_path);
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"i_total_sector: %lu", i_total_sector);
	*/
	if ( ( g_handle = open((const char *)pc_dev, O_RDWR) ) <= 0) {
		DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"pc_dev= '%s' strerror(%d)= '%s'\n", pc_dev, errno, strerror(errno));
		return i_status;
		//goto print_fatfs_res;
	}

	/*some callback function init*/
	fsInfo.fGetSecond = anyka_fsGetSecond;
	fsInfo.fSetSecond = anyka_fsSetSecond;
	fsInfo.fUniToAsc = anyka_fsUniToAsc;
	fsInfo.fAscToUni = anyka_fsAscToUni;
	fsInfo.fRamAlloc = anyka_fsRamAlloc;
	fsInfo.fRamRealloc = anyka_fsRamRealloc;
	fsInfo.fRamFree = anyka_fsRamFree;
	fsInfo.fCrtSem = anyka_fsOsCrtSem;
	fsInfo.fDelSem = anyka_fsOsDelSem;
	fsInfo.fObtSem = anyka_fsOsObtSem;
	fsInfo.fRelSem = anyka_fsOsRelSem;

	fsInfo.fMemCpy = anyka_fsMemCpy;
	fsInfo.fMemSet = anyka_fsMemSet;
	fsInfo.fMemMov = anyka_fsMemMov;
	fsInfo.fMemCmp = anyka_fsMemCmp;

	if( gc_fatfs_printf == AK_TRUE ) {
		fsInfo.fPrintf = (F_Printf)printf;
	}
	else {
		fsInfo.fPrintf = (F_Printf)printf_null;
	}

	fsInfo.fGetChipId = anyka_fsGetChipID;
	Global_Initial(&fsInfo);
	i_total_sector = get_disk_size( g_handle ) / SEC_SIZE ;
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"i_total_sector=%llu", i_total_sector);
	if ( ( medium = Creat_Medium( i_total_sector) ) == NULL ) {
		close(g_handle);
		//return i_status;
		goto print_fatfs_res;
	}

	driver = Driver_Initial(medium, SIZE_SECTOR);
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"Driver_Initial() driver=%ld\n", driver);
	if(!driver) {
		close(g_handle);
		Global_UninstallDriver(driver);
		Global_Destroy( ) ;
		FREE_POINT( medium )
		//return i_status;
		goto print_fatfs_res;
	}
	Global_MountDriver(driver, 0);
	for(i = 0; i < strlen((char *)ac_file_path); i ++) {
		del_file[i] = ac_file_path[i];
	}
	del_file[i] = 0;

	i_res = File_check_and_repair_file(del_file);

	Driver_Destroy((T_PDRIVER)driver);
	Global_UninstallDriver(driver);
	Global_Destroy( ) ;
	FREE_POINT( medium )
	close(g_handle);
	if ( ( g_card_result.ai_status_us[ TEST_STATUS_FATFS ] = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) > 0 ) {
	}

	switch( i_res ) {
		case FIX_STATUS_ERROR :
			i_color = COLOR_FRONT_RED;
			i_hint = FIX_RESULT_ERROR;
			i_status = RESULT_STATUS_FAIL;
  			break;
		case FIX_STATUS_NONEED :
			i_color = COLOR_FRONT_GREEN;
			i_hint = FIX_RESULT_NONEED;
			i_status = RESULT_STATUS_PASS;
			break;
		case FIX_STATUS_SUCCESS :
			i_color = COLOR_FRONT_YELLOW;
			i_hint = FIX_RESULT_SUCCESS;
			i_status = RESULT_STATUS_PASS;
			break;
		default :
			i_color = COLOR_FRONT_RED;
			i_hint = FIX_RESULT_UNKNOWN;
			i_status = RESULT_STATUS_FAIL;
	}
print_fatfs_res:
	g_card_result.i_fatfs_res = i_res;
	if( gc_key_value_res == AK_FALSE ) {
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
		           "###########################################\n"
		           "FATFS %s\n"
		           "TEST SEC= %.2f\n%s\n"
		           "###########################################\n",
		           gac_hint_status[ i_status ],
		           ( double )g_card_result.ai_status_us[ TEST_STATUS_FATFS ] / SEC2USEC, gac_hint_fatfs[ i_hint ] );
	}
	return i_status;

}

/*
 * get_fs_sector: get rom size of TF-card by path
 * path[IN]: TF-card path
 * return: TF-card sector number 512Byte a sector, -1 failed;
 */
unsigned int get_fs_sector(const char *path)                                                          //获取文件系统的扇区数(不包括分区表?)
{
	struct statfs disk_statfs;
	unsigned int total_size;

	memset(&disk_statfs, 0x00, sizeof(struct statfs));
	while(statfs(path, &disk_statfs) == -1)	{
		if(errno != EINTR) {
			if ( gc_view_detail == AK_TRUE ) {
				DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED, "strerror(%d)= '%s'\n", errno, strerror(errno))
			}
			return -1;
		}
	}
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"disk_statfs.f_bsize= %d disk_statfs.f_blocks= %lu", disk_statfs.f_bsize , disk_statfs.f_blocks );
	total_size = disk_statfs.f_bsize / SEC_SIZE * disk_statfs.f_blocks;

	return total_size;
}

int printf_null( const char *format, ... )
{
	return 0;
}
/*
int ak_format_tf( char *pc_dev )
{
	T_FSINITINFO fsInfo;
	T_PMEDIUM medium = AK_NULL;
	T_U32 driver = 0;
	int i_color= COLOR_FRONT_RED;
	int i_status = RESULT_STATUS_FAIL;
	int i_res = FIX_STATUS_ERROR;
	struct timeval timeval_begin, timeval_end ;
	unsigned long long i_total_sector;
	ULL i_us = 0;
	timeval_mark( &timeval_begin );

	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "pc_dev= '%s'", pc_dev );
	if ( ( g_handle = open((const char *)pc_dev, O_RDWR) ) <= 0) {
		DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"pc_dev= '%s' strerror(%d)= '%s'\n", pc_dev, errno, strerror(errno));
		return i_status;
	}

	fsInfo.fGetSecond = anyka_fsGetSecond;
	fsInfo.fSetSecond = anyka_fsSetSecond;
	fsInfo.fUniToAsc = anyka_fsUniToAsc;
	fsInfo.fAscToUni = anyka_fsAscToUni;
	fsInfo.fRamAlloc = anyka_fsRamAlloc;
	fsInfo.fRamRealloc = anyka_fsRamRealloc;
	fsInfo.fRamFree = anyka_fsRamFree;
	fsInfo.fCrtSem = anyka_fsOsCrtSem;
	fsInfo.fDelSem = anyka_fsOsDelSem;
	fsInfo.fObtSem = anyka_fsOsObtSem;
	fsInfo.fRelSem = anyka_fsOsRelSem;

	fsInfo.fMemCpy = anyka_fsMemCpy;
	fsInfo.fMemSet = anyka_fsMemSet;
	fsInfo.fMemMov = anyka_fsMemMov;
	fsInfo.fMemCmp = anyka_fsMemCmp;

	if( gc_fatfs_printf == AK_TRUE ) {
		fsInfo.fPrintf = (F_Printf)printf;
	}
	else {
		fsInfo.fPrintf = (F_Printf)printf_null;
	}

	fsInfo.fGetChipId = anyka_fsGetChipID;
	Global_Initial(&fsInfo);
	i_total_sector = get_disk_size( g_handle ) / SEC_SIZE ;
	if ( ( medium = Creat_Medium( i_total_sector) ) == NULL ) {
		close(g_handle);
		goto print_format_res;
	}

	driver = Driver_Initial(medium, SIZE_SECTOR);
	if(driver == 0) {
		close(g_handle);
		Global_UninstallDriver(driver);
		Global_Destroy( ) ;
		FREE_POINT( medium )
		goto print_format_res;
	}
	Driver_Destroy((T_PDRIVER)driver);

	//T_U32 start = ((T_PDRIVER)driver)->StartSector;
	T_U32 start = 0;
	//T_PFAT fat = ((T_PDRIVER)driver)->msg;
	//T_U32 total = ((T_PDRIVER)driver)->capacity + fat->RootAddr + fat->RootDirSectors;
	T_U32 total = medium->capacity;

	driver = ( T_U32 )Driver_Format1(medium, start, total, 4096, 0, FAT_FS_32);
	//i_res = Driver_Format(0,4096, FAT_FS_32);
	i_res = Global_MountDriver(driver, 0);
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "i_res= %d" , i_res)

	Global_UninstallDriver(driver);
	Global_Destroy( ) ;
	FREE_POINT( medium )
	close(g_handle);
	if ( ( i_us = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) > 0 ) {
	}
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "driver= %d" , driver )
	switch( i_res ) {
		case FORMAT_STATUS_FAIL :
			i_color = COLOR_FRONT_RED;
  			break;
		case FORMAT_STATUS_SUCCESS :
			i_color = COLOR_FRONT_GREEN;
			break;
		default :
			i_res = FORMAT_STATUS_FAIL;
			i_color = COLOR_FRONT_RED;
	}
print_format_res:
	g_card_result.i_fatfs_res = i_res;
	if( gc_key_value_res == AK_FALSE ) {
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
		           "###########################################\n"
		           "FORMAT %s\n"
		           "SEC= %.2f\n"
		           "###########################################\n",
		           gac_hint_status[ i_res ],
		           ( double )i_us / SEC2USEC );
	}
	return i_status;

}
*/