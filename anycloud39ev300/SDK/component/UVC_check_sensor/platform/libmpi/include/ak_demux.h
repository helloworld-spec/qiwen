#ifndef _AK_DEMUX_H_
#define _AK_DEMUX_H_

/**
 * ak_demux_open: open dumxer lib
 * @file_path[IN]: media file path, including full name
 * @start[IN]: start flag
 * return: demux handle, NULL failed
 */
void* ak_demux_open(const char *file_path, int start);

/**
 * ak_demux_get_data: get data after demux
 * @demux_handle[IN]: opened demux handle
 * @type[IN]: demux type
 * return: the stream packet after demux
 */
struct video_stream* ak_demux_get_data(void *demux_handle, int *type);

/**
 * ak_demux_free_data: free the demux resource
 * @video[IN]: the stream packet after demux
 * return: void
 */
void ak_demux_free_data(struct video_stream *video);

/**
 * ak_demux_get_total_time: get media file's total time
 * @file_path[IN]: media file path, including full name
 * return: total time in ms
 */
int ak_demux_get_total_time(const char *file_path);


/**
 * ak_demux_close: close dumxer lib
 * @demux_handle[IN]: opened demux handle
 * return: void
 */
void ak_demux_close(void *demux_handle);

#endif
