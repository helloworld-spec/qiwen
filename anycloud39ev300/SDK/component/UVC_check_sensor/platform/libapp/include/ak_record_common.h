#ifndef _AK_RECORD_COMMON_H_
#define _AK_RECORD_COMMON_H_

#define RECORD_FILE_PREFIX_MAX_LEN  20	//file prefix max len in bytes
#define RECORD_FILE_PATH_MAX_LEN    255	//file path max len in bytes

/* record file type */
enum record_file_type {
	RECORD_FILE_TYPE_AVI = 0x00,
	RECORD_FILE_TYPE_MP4,
	RECORD_FILE_TYPE_NUM
};

/* record exception */
enum record_exception {
    RECORD_EXCEPT_NONE = 0x00,	    	//no exception
    RECORD_EXCEPT_SD_REMOVED = 0x01,	//SD card removed
    RECORD_EXCEPT_SD_UMOUNT = 0x02,	    //SD card unmount
	RECORD_EXCEPT_SD_NO_SPACE = 0x04,	//SD card space not enough
	RECORD_EXCEPT_SD_RO = 0x08,		    //SD card read only
	RECORD_EXCEPT_NO_VIDEO = 0x10,	    //can't capture video data
	RECORD_EXCEPT_NO_AUDIO = 0x20,	    //can't capture audio data
	RECORD_EXCEPT_MUX_SYSERR = 0x40, 	//mux system error
};

#endif
