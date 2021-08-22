#ifndef _MUX_FS_H_
#define _MUX_FS_H_

#include <stdio.h>

/**
 * mux_fs_init: init video record file write env.
 * @record_fp[IN]: save file pointer
 * @index_fp[IN]: tmp file pointer
 * return: void
 * notes: we'll write the record file content directly using fwrite
 */                   
void mux_fs_init(FILE *record_fp, FILE *index_fp);

/**
 * mux_fs_flush: flush appointed file
 * @mux_fp[IN]: file pointer for mux
 * return: none
 */
void mux_fs_flush(FILE *fp);

/**
 * anyka_fs_seek: seek to appointed pos
 * @mux_fp[IN]: file pointer for mux
 * @offset[IN]: seek offset
 * @whence[IN]: SEEK_SET, SEEK_CUR or SEEK_END
 * return: current file pos
 */
long mux_fs_seek(long mux_fp, long offset, long whence);

/**
 * mux_fs_tell: obtain current value of the file position
 * @mux_fp[IN]: file pointer for mux
 * return: current value of the file position
 */
long mux_fs_tell(long mux_fp);

/**
 * mux_fs_handle_exist: 
 * @hFile[IN]: file pointer
 * return: 
 */
long mux_fs_handle_exist(long mux_fp);

/**
 * mux_fs_read: read appointed size data from file
 * @mux_fp[IN]: file pointer for mux
 * @data[OUT]: buffer for reading data
 * @size[IN]: appointed read size
 * return: read real bytes
 */
long mux_fs_read(long mux_fp, void *data, long size);

/**
 * mux_fs_write: write appointed size data into file
 * @mux_fp[IN]: file pointer for mux
 * @data[IN]: write data buffer
 * @size[IN]: appointed write size
 * return: real write bytes
 */
long mux_fs_write(long mux_fp, const void *data, long size);

#endif
