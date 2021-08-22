#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#include "mux_fs.h"

#include "ak_common.h"
#include "ak_sd_card.h"
#include "ak_thread.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define MUX_FS_DEBUG		0

enum mux_node {
	MUX_NODE_RECORD = 0x00,
	MUX_NODE_INDEX,
	MUX_NODE_NUM
};

struct mux_stat_t {
	unsigned char seek_flag;
    unsigned long total_write;
    FILE *fp;
};

struct mux_fs_t {
    unsigned char run_flag;
    unsigned char write_err;
    ak_mutex_t mutex;
};

static struct mux_fs_t mux_fs = {0};
static struct mux_stat_t mux_stat[MUX_NODE_NUM] = {{0}, {0}};

#if 0
#define MUX_MMAP_LEN	(1024*1024)

void mmap_write5_2()
{
    int i = 0;
    int iPagesize;
    int fd;

    char strData[STR_LEN] = {0};
    char strLeftData[2048] = {0};  
    unsigned short strSize = sizeof(strLeftData);

    char *start = (char *)calloc(1, (STR_LEN-1)/4);

    fd = open("data.txt", O_RDWR|O_CREAT, 00600);
    ftruncate(fd,  COUNT*(STR_LEN-1));
  
    for (i = 0; i < 4*COUNT; i++)
    {
         start = (char *)mmap(NULL, MUX_MMAP_LEN, PROT_WRITE, MAP_SHARED, 
         	fd, 0);
         if (MAP_FAILED == start) {
         	
         }

         setRecordData(strData, (STR_LEN-1)/4 +1 , strLeftData, strSize);
         memcpy(start , strData, (STR_LEN-1)/4);

         munmap(start, (STR_LEN-1)/4);
    }
    
    close(fd);
    free(start);
}
#endif

static int find_node_index(FILE *fp)
{
	int i = 0;
	int index = -1;
	
	for (i=0; i<MUX_NODE_NUM; ++i) {
		if (mux_stat[i].fp == fp) {
			index = i;
	    	break;
	    }
	}

	return index;
}

static void stat_mux_write(int write_len, FILE *fp)
{
	int index = find_node_index(fp);

	if (-1 != index) {
		if (!mux_stat[index].seek_flag && (write_len > sizeof(long))) {
			mux_stat[index].total_write += write_len;
			
			long cur_pos = ftell(fp);
			if (cur_pos < mux_stat[index].total_write) {
				ak_print_warning_ex("***** the SD card can't write totally, "
					"may be full *****\n");
				ak_print_normal_ex("fp=%p, write_offset=%d, "
					"write_len=%ld, tell=%ld\n\n", 
		    		fp, write_len, mux_stat[index].total_write, cur_pos);
			}
		}
		mux_stat[index].seek_flag = AK_FALSE;
		
#if MUX_FS_DEBUG		
	    ak_print_normal("fp=%p, write_len=%d, total_write=%ld\n", 
	    	fp, write_len, mux_stat[index].total_write);
#endif	    	
	}
}

/** write_data_to_file: write the appointed data to mux_fp file
 * @mux_fp[IN]: file pointer for mux
 * @data[IN]: write data buffer
 * @size[IN]: write data size
 * return: real write bytes
 * notes: update write time method, handle exception
 */
static long write_data_to_file(long mux_fp, const unsigned char *data, long size)
{
	int write_size = 0;
	int write_offset = 0;
	int cur_size = size;
	struct ak_timeval tv_start = {0};
	struct ak_timeval tv_end = {0};
    unsigned long tick = 0;
    FILE *fp = (FILE *)mux_fp;

	while((cur_size > 0) && (SD_STATUS_MOUNTED & ak_sd_get_status())) {
    	ak_get_ostime(&tv_start);
        write_size = fwrite(&(data[write_offset]), 1, cur_size, fp);
		ak_get_ostime(&tv_end);
		tick = ak_diff_ms_time(&tv_end, &tv_start);

		if(write_size < cur_size) {
			ak_print_normal("cur_size=%d, write_size=%d, tick=%ld\n", 
				cur_size, write_size, tick);
		}
        if(tick > 300) {
        	ak_print_normal("write_data_to_file too long: %ld(ms), cur_size=%d, "
        		"write_size=%d\n", tick, cur_size, write_size);
        }

        if(write_size <= 0) {
            ++mux_fs.write_err;
            ak_print_error("fs write error:write_size=%d, cur_size=%d, write_offset=%d\n", 
				write_size, cur_size, write_offset);
            ak_print_error("ferror=%d, write_err=%d, tick=%ld\n", 
            	ferror(fp), mux_fs.write_err, tick);
			break;
        } else {
        	mux_fs.write_err = 0;
        	cur_size -= write_size;
        	write_offset += write_size;
        }
    }

	stat_mux_write(write_offset, fp);

    return write_offset;
}

/**
 * mux_fs_init: init video record file write env.
 * @record_fp[IN]: save file pointer
 * @index_fp[IN]: tmp file pointer
 * return: void
 * notes: we'll write the record file content directly using fwrite
 */                   
void mux_fs_init(FILE *record_fp, FILE *index_fp)
{
	if (!record_fp || !index_fp) {
		ak_print_info_ex("record_fp=%p, index_fp=%p\n", record_fp, index_fp);
		return;
	}
	
	if(!mux_fs.run_flag) {
		mux_fs.run_flag = AK_TRUE;
		mux_fs.write_err = 0;
    	ak_thread_mutex_init(&(mux_fs.mutex)); 
	}

	mux_stat[MUX_NODE_RECORD].fp = record_fp;
	mux_stat[MUX_NODE_RECORD].total_write = 0;

	mux_stat[MUX_NODE_INDEX].fp = index_fp;
	mux_stat[MUX_NODE_INDEX].total_write = 0;
}

void mux_fs_flush(FILE *fp)
{
	if (!fp) {
		ak_print_info_ex("fp=%p\n", fp);
		return;
	}
	
	if(0 != fflush(fp)) {
    	ak_print_error_ex("fflush failed: %s\n", strerror(errno));
    }

#if MUX_FS_DEBUG
	ak_print_info_ex("mux_fp=%p flush called\n", fp);

	int index = find_node_index(fp);
	if (index > 0) {
		ak_print_notice_ex("fflush fp=%p, total_write=%ld\n",
			mux_stat[index].fp, mux_stat[index].total_write);
	}
#endif	
}

/**
 * mux_fs_seek: seek to appointed pos
 * @mux_fp[IN]: file pointer for mux
 * @offset[IN]: seek offset
 * @whence[IN]: SEEK_SET=0, SEEK_CUR=1 or SEEK_END=2
 * return: current file pos
 */
long mux_fs_seek(long mux_fp, long offset, long whence)
{
	if (!mux_fp) {
		ak_print_info_ex("mux_fp=0x%lX\n", mux_fp);
		return AK_FAILED;
	}
	
	long pos = 0;
	FILE *fp = (FILE *)mux_fp;

	ak_thread_mutex_lock(&mux_fs.mutex);
    if(0 == fseek(fp, offset, whence)) {
    	pos = ftell(fp);
    	if(-1 == pos) {
	    	ak_print_error_ex("ftell failed: %s\n", strerror(errno));
	    }
    } else {
    	ak_print_error_ex("fseek failed: %s\n", strerror(errno));
    }
    ak_thread_mutex_unlock(&mux_fs.mutex);

#if MUX_FS_DEBUG
	ak_print_info_ex("mux_fp=%p, offset=%ld, whence=%ld, real pos=%ld\n", 
		fp, offset, whence, pos);
#endif

	int index = find_node_index(fp);
	if (-1 != index) {
		mux_stat[index].seek_flag = AK_TRUE;
	}

    return pos;
}

/**
 * mux_fs_tell: obtain current value of the file position
 * @mux_fp[IN]: file pointer for mux
 * return: current value of the file position
 */
long mux_fs_tell(long mux_fp)
{
	if (!mux_fp) {
		ak_print_info_ex("mux_fp=0x%lX\n", mux_fp);
		return AK_FAILED;
	}
	
	ak_thread_mutex_lock(&mux_fs.mutex);
	long tell = ftell((FILE *)mux_fp);
	ak_thread_mutex_unlock(&mux_fs.mutex);
	if(-1 == tell) {
    	ak_print_error_ex("ftell failed: %s\n", strerror(errno));
    }
	    
#if MUX_FS_DEBUG
	ak_print_info_ex("mux_fp=%p, tell=%ld\n\n", (FILE *)mux_fp, tell);
#endif
	
    return tell;
}

/**
 * mux_fs_handle_exist: check mux fs handle exist
 * @mux_fp[IN]: file pointer for mux
 * return: 1 handle is available; otherwize failed
 */
long mux_fs_handle_exist(long mux_fp)
{
	if (!mux_fp) {
		ak_print_info_ex("mux_fp=0x%lX\n", mux_fp);
		return AK_FAILED;
	}
	
	ak_thread_mutex_lock(&mux_fs.mutex);
	int ret = fseek((FILE *)mux_fp, 0, SEEK_CUR);
	ak_thread_mutex_unlock(&mux_fs.mutex);

#if MUX_FS_DEBUG
	ak_print_info_ex("mux_fp=%p, ret=%d\n", (FILE *)mux_fp, ret);
#endif

	return ((0 == ret)?1:0);
}

/**
 * mux_fs_read: read appointed size data from file
 * @mux_fp[IN]: file pointer for mux
 * @data[OUT]: buffer for reading data
 * @size[IN]: appointed read size
 * return: read real bytes
 */
long mux_fs_read(long mux_fp, void *data, long size)
{
	if (!mux_fp) {
		ak_print_info_ex("mux_fp=0x%lX\n", mux_fp);
		return AK_FAILED;
	}
	if (!data) {
		ak_print_info_ex("data=%p\n", data);
		return AK_FAILED;
	}
	if (size <= 0) {
		ak_print_error_ex("size=%ld\n", size);
		return AK_FAILED;
	}
	
	ak_thread_mutex_lock(&mux_fs.mutex);
	size_t read_size = fread(data, 1, size, (FILE *)mux_fp);
	ak_thread_mutex_unlock(&mux_fs.mutex);

#if MUX_FS_DEBUG
	ak_print_info_ex("mux_fp=%p, size=%ld, read_size=%d\n",
		(FILE *)mux_fp, size, read_size);
#endif
	
    return read_size;
}

/**
 * mux_fs_write: write appointed size data into file
 * @mux_fp[IN]: file pointer for mux
 * @data[IN]: write data buffer
 * @size[IN]: appointed write size
 * return: real write bytes
 */
long mux_fs_write(long mux_fp, const void *data, long size)
{
	if (!mux_fp) {
		ak_print_info_ex("mux_fp=0x%lX\n", mux_fp);
		return AK_FAILED;
	}
	if (!data) {
		ak_print_info_ex("data=%p\n", data);
		return AK_FAILED;
	}
	if (size <= 0) {
		ak_print_error_ex("size=%ld\n", size);
		return AK_FAILED;
	}
	
    ak_thread_mutex_lock(&mux_fs.mutex);
    long write_size = write_data_to_file(mux_fp, (unsigned char *)data, size);
    ak_thread_mutex_unlock(&mux_fs.mutex);
    
    return write_size;
}
