#ifndef _TW_COMMON_H_
#define _TW_COMMON_H_

#include <stdlib.h>
#include <string.h>

#include "ak_common.h"

/* struct size: 44 bytes */
struct wave_header {
	char riff[4];           /* "RIFF"  0 */
	long file_size;         /* in bytes 4 */
	char wavefmt[8];        /* "WAVE" 8 */
	long chunk_size;        /* in bytes (16 for PCM) 16 */
	short format_tag;       /* 1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law 20 */
	short num_chans;        /* 1=mono, 2=stereo 22 */
	long sample_rate;       /* 24 */
	long bytes_per_sec;     /* 28 */
	short bytes_per_samp;   /* 2=16-bit mono, 4=16-bit stereo 32 */
	short bits_per_samp;    /* 34 */
	char data[4];           /* "data" 36 */
	long data_length;       /* in bytes 40 */
};

static inline FILE* open_wave_file(const char *wave_file)
{
    return fopen(wave_file, "wb+");
}

/**
 * update_head_len - update head length
 * @wav_fp[IN]: wave file handle 
 * @whead[IN]: wave heead
 * @data_len[IN]: data length
 * return: NULL
 * notes:
 */
static inline void update_head_len(FILE *wav_fp,
                        struct wave_header *whead, int data_len)
{
    if (wav_fp) {
        /* fill data len, and write back wave head */
        whead->data_length = data_len;
        whead->file_size = whead->data_length + 44 - 8;

        fseek(wav_fp, 0, SEEK_SET);
        fwrite(whead, 1, sizeof(struct wave_header), wav_fp);
    }
}

/**
 * write_wave_header - write wave header
 * @wav_fp[IN]: wave file handle 
 * @whead[IN]: wave heead
 * @sample_rate[IN]: sample rate
 * return: 0 success, -1 failed
 * notes:
 */
static inline int write_wave_header(FILE *wav_fp,
                        struct wave_header *whead, int sample_rate)
{
    int ret = AK_FAILED;

	if (wav_fp) {
    	/* fill wav head */
    	memcpy(whead->riff, "RIFF", 4);
    	whead->file_size = 0;
    	memcpy(whead->wavefmt, "WAVEfmt ", 8);
    	whead->chunk_size = 16;
    	whead->format_tag = 1;
    	whead->num_chans = 1;
    	whead->sample_rate = sample_rate;
    	whead->bytes_per_sec = (sample_rate << 1);
    	whead->bytes_per_samp = 2;
    	whead->bits_per_samp = 16;
    	memcpy(whead->data, "data", 4);
    	whead->data_length = 0;

    	fwrite(whead, 1, sizeof(struct wave_header), wav_fp);
        ret = AK_SUCCESS;
	}

    return ret;
}

#endif
