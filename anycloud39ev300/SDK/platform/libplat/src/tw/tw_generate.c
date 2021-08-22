#include <stdio.h>
#include <math.h>

#include "tw_common.h"
#include "ak_tw.h"

#define DIM(arr) (sizeof(arr)/sizeof(arr[0]))

#define TW_DEFAULT_WAVE_FILE    "/tmp/tone_generate.wav"

/* 44.1K or 48K is OK, sample rate don't be too low. */
#define TONE_SR     48000 /* sample rate */
#define TONE_BASE   2000
#define PI          (3.141592654)

enum tone_window_id {
    WIN_NONE = 0,
    WIN_FULL_COS,
    WIN_TRIANLE,
    WIN_HALF_SIN
};

struct tw_pcm {
    short *buf;
    long n_point;
};

struct tone_wave_generate {
    FILE *wav_fp;		/* file handle */
    double *window;
    short *silence;
    struct tw_pcm *period_pcm;

    int sample_rate;    /* only support 44.1K and 48K, default is 48K */
    long win_half_size;
    long win_cycle;     /* ms */
};

static const char tw_gen_version[] = "libplat_tw V2.0.00 filter_V1.9.01_svn5171";

static struct tone_wave_generate tw_gen = {
    .wav_fp = NULL,
    .window = NULL,
    .silence = NULL,
    .period_pcm = NULL,
    .win_half_size = 0,
    .win_cycle = 4
};

static const long TONE_CODE_HIGH_CANDIDATE[] = {
    6000+TONE_BASE,
    5500+TONE_BASE,
    5000+TONE_BASE,
    4500+TONE_BASE,
};

static const long TONE_CODE_LOW_CANDIDATE[] = {
    3500+TONE_BASE,
    3000+TONE_BASE,
    2500+TONE_BASE,
    2000+TONE_BASE,
};

static const long TONE_SYNC_CANDIDATE[] = {
    1000+TONE_BASE,
     500+TONE_BASE
};

typedef struct _T_TONE_GROUP {
    /* each period has at most 3 tones */
    long tone[3];
}T_TONE_GROUP;

/* start section is formed by 2 groups */
static const T_TONE_GROUP START_GROUPS[] = {
    {{7000+TONE_BASE}},
    {{0+TONE_BASE}}
};

/* end section is form by 1 group */
static const T_TONE_GROUP END_GROUPS[] = {
    {{0+TONE_BASE}}
};

/**
 * init_tone_window - generate window
 *      Window is a symmetric envelope.
 *      apply first half of window to start of the sine waves.
 *      apply reverse order of half window to ending of the sine waves.
 * @win_id[IN]: window ID in "enum tone_window_id"
 * return: none
 * notes:
 */
static void init_tone_window(enum tone_window_id win_id)
{
    int i = 0;
    double t;

    /* exclude 0 and 1 */
    tw_gen.win_half_size = (tw_gen.win_cycle * tw_gen.sample_rate / 1000 / 2);
    tw_gen.window = (double *)malloc(tw_gen.win_half_size
        * sizeof(tw_gen.window[0]));

    switch (win_id) {
    case WIN_FULL_COS:
        t = (2 * PI * 1000 / tw_gen.win_cycle / tw_gen.sample_rate);
        for (i=0; i<tw_gen.win_half_size; i++) {
            tw_gen.window[i] = (1 - cos(t*i)) / 2;
        }
        break;

    case WIN_TRIANLE:
        for (i=0; i<tw_gen.win_half_size; i++) {
            tw_gen.window[i] = (double)i/tw_gen.win_half_size;
        }
        break;

    case WIN_HALF_SIN:
        t = (PI * 1000 / tw_gen.win_cycle / tw_gen.sample_rate);
        for (i=0; i<tw_gen.win_half_size; i++) {
            tw_gen.window[i] = sin(t*i);
        }
        break;

    default:
        for (i=0; i<tw_gen.win_half_size; i++) {
            tw_gen.window[i] = 1;
        }
        break;
    }
}

/**
 * gen_tone_wave - Generate a tone wave vector
 *      the output is a buffer that consist of pre-silence,
 *      sine waves and post-silence.
 * @freq[IN]: frequency of the tone (Hz)
 * @amplitude[IN]: amplitude of the wave, (-1, 1)
 * @duration[out]: duration of the wave in ms, not including pre- and post- silence
 * @pre_gap[IN]: silence in ms that lead the wave
 * @post_gap[IN]: silence in ms that follow the wave
 * @tone_pcm[OUT]: pcm data info
 * return: 0 success, -1 failed.
 * notes:
 */
static int gen_tone_wave(long freq, double amplitude, long duration,
            long pre_gap, long post_gap, struct tw_pcm *tone_pcm)
{
    /* temp buf to store generated PCM, 100ms max */
    int pcm_len = (tw_gen.sample_rate / 10);
    short *period_pcm16 = (short *)calloc(1, pcm_len * sizeof(short));
    if (!period_pcm16) {
        return AK_FAILED;
    }

    double t = (2 * PI * freq / tw_gen.sample_rate);
    long period_len = ((duration+pre_gap+post_gap) * tw_gen.sample_rate / 1000);
    long tone_len = (duration * tw_gen.sample_rate / 1000);
    /* tone is after pre-gap */
    short *tone_pcm16 = &period_pcm16[pre_gap*tw_gen.sample_rate/1000];
    int i;

    memset(period_pcm16, 0, period_len * sizeof(period_pcm16[0]));

    /* fill tone (sin waves) */
    for (i=0; i<tone_len; i++) {
        double sample_f = sin(t * i) * amplitude;
        tone_pcm16[i] = (short)(sample_f * 32767);
    }

    /* apply window */
    for (i=0; i<tw_gen.win_half_size; i++) {
        /* entry */
        tone_pcm16[i] = (short)(tw_gen.window[i] * tone_pcm16[i]);
        /* leave */
        tone_pcm16[tone_len-tw_gen.win_half_size+i] =
            (short)(tw_gen.window[tw_gen.win_half_size-1-i]
            * tone_pcm16[tone_len-tw_gen.win_half_size+i]);
    }

    tone_pcm->buf = period_pcm16;
    tone_pcm->n_point = period_len;

    return AK_SUCCESS;
}

static int mix_tones(T_TONE_GROUP *tone_group)
{
    int ret = AK_FAILED;
    /* temp buf to store generated PCM, 100ms max */
    int period_len = (tw_gen.sample_rate / 10);
    long *period_pcm32 = (long *)calloc(1, (period_len * sizeof(long)));
    if (!period_pcm32) {
        goto mix_end;
    }

    tw_gen.period_pcm = (struct tw_pcm *)calloc(1, sizeof(struct tw_pcm));
    if (!tw_gen.period_pcm) {
        goto mix_end;
    }

    int i = 0;
    int j = 0;
    struct tw_pcm tone_pcm = {0};

    /* mix tones */
    for (i=0; i<DIM(tone_group->tone); i++) {
        if (tone_group->tone[i] != 0) {
            if (!gen_tone_wave(tone_group->tone[i], 0.95/3, 50, 0, 0, &tone_pcm)) {
                for (j=0; j<tone_pcm.n_point; j++) {
                    period_pcm32[j] += tone_pcm.buf[j];
                }

                tw_gen.period_pcm->n_point = tone_pcm.n_point;
                free(tone_pcm.buf);
                tone_pcm.buf = NULL;
                tone_pcm.n_point = 0;
            }
        }
    }

    /* fade from 100% to 1/4 */
    double fadeout_step = ((double)1.0 - (double)1.0/4)
        / (tw_gen.period_pcm->n_point);
    double fadeout = 1.0;

    /* average, and convert to 16bit format */
    short *period16 = (short *)period_pcm32;
    for (i=0; i<tw_gen.period_pcm->n_point; i++) {
        period16[i] = (short)(period_pcm32[i] * fadeout);
        fadeout -= fadeout_step;
    }

    tw_gen.period_pcm->buf = period16;
    ret = AK_SUCCESS;

mix_end:
    if (ret) {
        if (period_pcm32) {
            free(period_pcm32);
            period_pcm32 = NULL;
        }
        if (tw_gen.period_pcm) {
            if (tw_gen.period_pcm->buf) {
                free(tw_gen.period_pcm->buf);
                tw_gen.period_pcm->buf = NULL;
            }

            free(tw_gen.period_pcm);
            tw_gen.period_pcm = NULL;
        }
    }

    return ret;
}

/**
 * generate_tone - generate tone
 * @str[IN]: string data 
 * @str_size[IN]: string length
 * return: output length
 * notes:
 */
static long generate_tone(char *str, long str_size)
{
    int i = 0;
    char crc = 0;
    long len = 0;
    char *code = NULL;
    T_TONE_GROUP *group_seq = NULL;

    /* copy input string to internal buffer, crc is the last byte */
    char *text_seq = (char *)malloc(str_size + 1);
    if (!text_seq) {
        goto generate_end;
    }

    memcpy(text_seq, str, str_size);
    for (i=0; i<str_size; i++) {
        crc ^= text_seq[i];
    }
    text_seq[i] = crc;
    str_size++;

    /* transform text to code */
    long code_size = (str_size << 1);
    /* code is a vector, each code number is represented by a single period */
    code = (char *)malloc(code_size);
    if (!code) {
        goto generate_end;
    }
    for (i=0; i<str_size; i++) {
        code[2*i] = (text_seq[i]>>4) & 0xf;
        code[2*i+1] = text_seq[i] & 0xf;
    }

    /*
     * tone_group_sequence is a matric to store tone frequencies of the whole
     * text sequence, including start + code + crc + stop
     */
    long n_group = DIM(START_GROUPS) + code_size + DIM(END_GROUPS);
    group_seq = (T_TONE_GROUP *)calloc(1, (n_group * sizeof(T_TONE_GROUP)));
    if (!group_seq) {
        goto generate_end;
    }

    /* START section */
    long group_index = 0;
    memcpy(&group_seq[group_index], START_GROUPS, sizeof(START_GROUPS));
    /* data starts from 3rd group */
    group_index = DIM(START_GROUPS);

    /* code & crc section */
    for (i=0; i<code_size; i++) {
        T_TONE_GROUP *group = &group_seq[group_index];
        int tone_high = code[i] & 0x3;
        int tone_low  = (code[i]>>2) & 0x3;

        group->tone[0] = TONE_CODE_HIGH_CANDIDATE[tone_high];
        group->tone[1] = TONE_CODE_LOW_CANDIDATE[tone_low];
        group->tone[2] = TONE_SYNC_CANDIDATE[i%2]; // add a sync tone
        ++group_index;
    }

    /* STOP section */
    memcpy(&group_seq[group_index], END_GROUPS, sizeof(END_GROUPS));

    /* convert to wave */
    init_tone_window(WIN_FULL_COS);
    for (i=0;i<n_group; i++) {
        mix_tones(&group_seq[i]);

        /* output */
        if (tw_gen.wav_fp && tw_gen.period_pcm) {
            int period_len = (tw_gen.period_pcm->n_point << 1);
            fwrite(tw_gen.period_pcm->buf, 1, period_len, tw_gen.wav_fp);
            len += period_len;
        }
    }

generate_end:
    if (group_seq) {
        free(group_seq);
        group_seq = NULL;
    }
    if (code) {
        free(code);
        code = NULL;
    }
    if (text_seq) {
        free(text_seq);
        text_seq = NULL;
    }

    return len;
}

static inline void write_silence_data(size_t silence_points)
{
    memset(tw_gen.silence, 0, silence_points*2);
    fwrite(tw_gen.silence, 2, silence_points, tw_gen.wav_fp);
}

/**
 * fill_silence_data - fill with 0 data
 * return: silence points
 * notes: prefix/postfix with 0.5s of silence
 */
static inline size_t fill_silence_data(void)
{
    size_t silence_points = (size_t)(tw_gen.sample_rate >> 1);

    if (tw_gen.silence) {
        write_silence_data(silence_points);
        free(tw_gen.silence);
        tw_gen.silence = NULL;
    } else {
        tw_gen.silence = (short *)malloc(silence_points << 1);
        write_silence_data(silence_points);
    }

    return silence_points;
}

/* call "ak_tw_release_gen" to free data outside. */
static void copy_to_out_buffer(struct tw_generate *gen)
{
    gen->data = (unsigned char *)malloc(gen->data_len);
    if (gen->data) {
        fseek(tw_gen.wav_fp, 0, SEEK_SET);
        fread(gen->data, 1, gen->data_len, tw_gen.wav_fp);
    }
}

/**
 * release_tw_resource - release tone wave resource
 * return: NULL
 */
static void release_tw_resource(void)
{
    if (tw_gen.wav_fp) {
        fclose(tw_gen.wav_fp);
        tw_gen.wav_fp = NULL;
    }

     if (tw_gen.period_pcm) {
        if (tw_gen.period_pcm->buf) {
            free(tw_gen.period_pcm->buf);
            tw_gen.period_pcm->buf = NULL;
        }

        free(tw_gen.period_pcm);
        tw_gen.period_pcm = NULL;
    }

    if (tw_gen.window) {
        free(tw_gen.window);
        tw_gen.window = NULL;
    }
}

/**
 * ak_tw_get_generate - get generate tone wave info according to string.
 * @gen[IN/OUT]: tone wave generate control and data
 * return: 0 success, -1 failed
 * notes: 1. sample rate 44.1K/48K, 16 bits per sample, mono channel.
 *      2. 88.2K/96K bytes per seconds.
 *      3. you can generate string according to ssid and passwd.
 *      4. you MUST call "ak_tw_release_gen" to free the out buffer.
 *      5. the out buffer data is wave format,
 *          skip the first 44 bytes if you do not use.
 */
int ak_tw_get_generate(struct tw_generate *gen)
{
    if(!gen) {
        ak_print_error_ex("arguments error\n");
        return AK_FAILED;
    }
    if(!gen->gen_str) {
        ak_print_error_ex("arguments error\n");
        return AK_FAILED;
    }

    /* check sample rate */
    if ((44100 != gen->sample_rate) || (48000 != gen->sample_rate)) {
        gen->sample_rate = TONE_SR;
    }
    tw_gen.sample_rate = gen->sample_rate;

    char *wav_file = NULL;
    if (gen->write_file_flag && gen->wav_file) {
        wav_file = gen->wav_file;
    }else {
        wav_file = TW_DEFAULT_WAVE_FILE;
    }

    ak_print_normal_ex("generate string: %s\n", gen->gen_str);
    ak_print_normal_ex("sample rate: %d\n", tw_gen.sample_rate);
    ak_print_normal_ex("wave file: %s\n", wav_file);
    tw_gen.wav_fp = open_wave_file(wav_file);
    if (!tw_gen.wav_fp) {
        return AK_FAILED;
    }

    struct wave_header whead;
    if (write_wave_header(tw_gen.wav_fp, &whead, gen->sample_rate)) {
        return AK_FAILED;
    }

    size_t silence_points = fill_silence_data();

	int ret = AK_FAILED;
	int len = generate_tone(gen->gen_str, (long)strlen(gen->gen_str));
    if (len > 0) {
        /* postfix with 0.5s of silence */
        silence_points += fill_silence_data();
        gen->data_len = (len + silence_points*2);
        update_head_len(tw_gen.wav_fp, &whead, gen->data_len);
        gen->data_len += sizeof(whead);
        ak_print_normal_ex("generate success\n");
        ret = AK_SUCCESS;
    } else {
        if (gen->write_file_flag) {
            remove(wav_file);
        }
        ak_print_normal_ex("generate failed\n");
    }

    /* the out buffer datas are the same as in wave file. */
    copy_to_out_buffer(gen);

    /* remove temp wave file if user do not appoint. */
    if (!gen->write_file_flag) {
        remove(wav_file);
    }

    release_tw_resource();

	return ret;
}

/**
 * ak_tw_release_generate - release tone wave generation info,
 *          especially the out buffer.
 * @gen[IN/OUT]: tone wave generate control and data
 * return: 0 success, -1 failed
 */
int ak_tw_release_generate(struct tw_generate *gen)
{
    if(!gen) {
        ak_print_error_ex("arguments error\n");
        return AK_FAILED;
    }
    if(gen->data) {
        free(gen->data);
        gen->data = NULL;
    }

    return AK_SUCCESS;
}
