#ifndef _AENC_IPCSRV_H_
#define _AENC_IPCSRV_H_

enum aenc_level_index {
	AFTER_ENC_LEVEL = 0x00,	// save after encode audio
	BEFORE_ENC_LEVEL,	// save before encode audio(pcm)
	AENC_MAX_LEVEL
};

int aenc_save_stream_to_file(int origin_type, unsigned char *buf, int len,
							enum aenc_level_index save_level);

void aenc_sys_ipc_register(void);

void aenc_sys_ipc_unregister(void);

void aenc_sysipc_bind_handle(void *handle);

void aenc_sysipc_unbind_handle(int origin_type);

#endif
