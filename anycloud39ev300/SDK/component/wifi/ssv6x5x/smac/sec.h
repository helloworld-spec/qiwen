/*
 * Copyright 2002-2004, Instant802 Networks, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SEC_H
#define SEC_H

#include <linux/types.h>
#include <linux/ieee80211.h>
#ifdef SSV_MAC80211
#include "ssv_mac80211.h"
#else
#include <net/mac80211.h>
#endif

#define CCMP_TK_LEN 16
#define TKIP_KEY_LEN 32
#define WEP_KEY_LEN 13

struct ssv_crypto_ops {
	const char *name;
	struct list_head list;

	/* init new crypto context (e.g., allocate private data space,
	 * select IV, etc.); returns NULL on failure or pointer to allocated
	 * private data on success */
	void *(*init) (int keyidx);

	/* deinitialize crypto context and free allocated private data */
	void (*deinit) (void *priv);

	/* encrypt/decrypt return < 0 on error or >= 0 on success. The return
	 * value from decrypt_mpdu is passed as the keyidx value for
	 * decrypt_msdu. skb must have enough head and tail room for the
	 * encryption; if not, error will be returned; these functions are
	 * called for all MPDUs (i.e., fragments).
	 */
	int (*encrypt_mpdu) (struct sk_buff * skb, int hdr_len, void *priv);
	int (*decrypt_mpdu) (struct sk_buff * skb, int hdr_len, void *priv);

	/* These functions are called for full MSDUs, i.e. full frames.
	 * These can be NULL if full MSDU operations are not needed. */
	int (*encrypt_msdu) (struct sk_buff * skb, int hdr_len, void *priv);
	int (*decrypt_msdu) (struct sk_buff * skb, int keyidx, int hdr_len,
			     void *priv);

	int (*set_tx_pn) (u8 * seq, void *priv);
	int (*set_key) (void *key, int len, u8 * seq, void *priv);
	int (*get_key) (void *key, int len, u8 * seq, void *priv);

	/* procfs handler for printing out key information and possible
	 * statistics */
	char *(*print_stats) (char *p, void *priv);

	/* Crypto specific flag get/set for configuration settings */
	unsigned long (*get_flags) (void *priv);
	unsigned long (*set_flags) (unsigned long flags, void *priv);
#ifdef MULTI_THREAD_ENCRYPT        
	int (*encrypt_prepare) (struct sk_buff * skb, int hdr_len, void *priv);
	int (*decrypt_prepare) (struct sk_buff * skb, int hdr_len, void *priv);
#endif

	/* maximum number of bytes added by encryption; encrypt buf is
	 * allocated with extra_prefix_len bytes, copy of in_buf, and
	 * extra_postfix_len; encrypt need not use all this space, but
	 * the result must start at the beginning of the buffer and correct
	 * length must be returned */
	int extra_mpdu_prefix_len, extra_mpdu_postfix_len;
	int extra_msdu_prefix_len, extra_msdu_postfix_len;
};


struct ssv_crypto_data {
    struct ssv_crypto_ops      *ops;
    void                       *priv;
    #ifdef HAS_CRYPTO_LOCK
    rwlock_t                    lock;
    #endif // HAS_CRYPTO_LOCK
};

//int ieee80211_crypto_ccmp_encrypt(struct sk_buff *skb, u8 *key, u8 keyidx, u8 *pn);
//int ieee80211_crypto_ccmp_decrypt(struct PKT_Info_st *ppkt, u8 *key, u8 keyidx, u8 *pn);
struct ssv_crypto_ops *get_crypto_ccmp_ops(void);
struct ssv_crypto_ops *get_crypto_tkip_ops(void);
struct ssv_crypto_ops *get_crypto_wep_ops(void);
#ifdef CONFIG_SSV_WAPI
struct ssv_crypto_ops *get_crypto_wpi_ops(void);
#endif //CONFIG_SSV_WAPI
#endif /* SEC_H */
