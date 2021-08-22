#ifndef _DEMUX_FS_H_
#define _DEMUX_FS_H_

#include <stdio.h>

/**
 * demux_fs_read: read file data
 * @mux_fp[IN]: file pointer for demux
 * @data[OUT]: read data buffer
 * @size[IN]: appointed read size
 * return: real read bytes
 */
static inline long demux_fs_read(long demux_fp, void *data, long size)
{
	return fread(data, 1, size, (FILE *)demux_fp);
}

long demux_fs_write(long demux_fp, const void *data, long size);

/**
 * demux_fs_seek: seek to appointed pos
 * @mux_fp[IN]: file pointer for mux
 * @offset[IN]: seek offset
 * @whence[IN]: SEEK_SET=0, SEEK_CUR=1 or SEEK_END=2
 * return: current file pos
 */
long demux_fs_seek(long demux_fp, long offset, long whence);

static inline long demux_fs_tell(long demux_fp)
{
    return ftell((FILE *)demux_fp);
}

static inline long demux_fs_exist(long demux_fp)
{
	int ret = fseek((FILE *)demux_fp, 0, SEEK_CUR);

	return ((0 == ret)?1:0);
}

#endif
