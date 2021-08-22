#include <errno.h>
#include <string.h>

#include "ak_common.h"
#include "demux_fs.h"

long demux_fs_write(long demux_fp, const void *data, long size)
{
    return -1;
}

/**
 * demux_fs_seek: seek to appointed pos
 * @mux_fp[IN]: file pointer for mux
 * @offset[IN]: seek offset
 * @whence[IN]: SEEK_SET=0, SEEK_CUR=1 or SEEK_END=2
 * return: current file pos
 */
long demux_fs_seek(long demux_fp, long offset, long whence)
{
    long pos = 0;
	FILE *fp = (FILE *)demux_fp;

    if(0 == fseek(fp, offset, whence)) {
    	pos = ftell(fp);
    	if(-1 == pos) {
	    	ak_print_error_ex("ftell failed: %s\n", strerror(errno));
	    }
    } else {
    	ak_print_error_ex("fseek failed: %s\n", strerror(errno));
    }
    
    return pos;
}
