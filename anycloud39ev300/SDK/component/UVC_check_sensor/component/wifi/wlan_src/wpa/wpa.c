#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"


#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "if_sdio.h"
#include "host.h"
#include "dev.h"


#include "cfg.h"
#include "lwip/pbuf.h"
#include "lwip/sockets.h"
#include "lwip/timers.h"
#include "wpa.h"

#include "cmd.h"
#include "cfg.h"
#include "netdevice.h"
#include "netif/etharp.h"
#include "decl.h"

#include "wpa.h"
//#define TEST_EAPOL

static void wpa_supplicant_timeout(void *eloop_ctx, void *timeout_ctx);
static int wpa_supplicant_pairwise_gtk(struct wpa_sm *sm,
				       const struct wpa_eapol_key *key,
				       const u8 *gtk, size_t gtk_len,
				       int key_info);
static int ieee80211w_set_keys(struct wpa_sm *sm,
			       struct wpa_eapol_ie_parse *ie);
						 
struct wpa_supplicant wpa_s;
struct wpa_sm sm;
struct rsn_pmksa_cache  pmksa[PMK_CACHE_NUM];

int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
	AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out,
	const AES_KEY *key);

void init_wpa_supplicant()
{
	p_info("init_wpa_supplicant");
	memset(&wpa_s, 0 ,sizeof(struct wpa_supplicant));
	memset(&sm, 0 ,sizeof(struct wpa_sm));
	wpa_s.wpa = &sm;
	sm.pmksa = pmksa;
}

static void wpa_sm_set_state(struct wpa_sm *sm, enum wpa_states state)
{
	wpa_s.wpa_state = state;
}

static enum wpa_states wpa_sm_get_state(struct wpa_sm *sm)
{
	return wpa_s.wpa_state;
}

void wpa_supplicant_set_state(struct wpa_supplicant *wpa_s,
			      enum wpa_states state)
{
	wpa_s->wpa_state = state;
}

void wpa_sm_notify_assoc(struct wpa_sm *sm, const u8 *bssid)
{
	int clear_ptk = 1;

	if (sm == NULL)
		return;

	wpa_printf(MSG_DEBUG, "WPA: Association event - clear replay counter");
	os_memcpy(sm->bssid, bssid, ETH_ALEN);
	os_memset(sm->rx_replay_counter, 0, WPA_REPLAY_COUNTER_LEN);
	sm->rx_replay_counter_set = 0;
	sm->renew_snonce = 1;
//	if (os_memcmp(sm->preauth_bssid, bssid, ETH_ALEN) == 0)
//		rsn_preauth_deinit(sm);

#ifdef CONFIG_IEEE80211R
	if (wpa_ft_is_completed(sm)) {
		/*
		 * Clear portValid to kick EAPOL state machine to re-enter
		 * AUTHENTICATED state to get the EAPOL port Authorized.
		 */
		eapol_sm_notify_portValid(sm->eapol, FALSE);
		wpa_supplicant_key_neg_complete(sm, sm->bssid, 1);

		/* Prepare for the next transition */
		wpa_ft_prepare_auth_request(sm, NULL);

		clear_ptk = 0;
	}
#endif /* CONFIG_IEEE80211R */

	if (clear_ptk) {
		/*
		 * IEEE 802.11, 8.4.10: Delete PTK SA on (re)association if
		 * this is not part of a Fast BSS Transition.
		 */
		wpa_printf(MSG_DEBUG, "WPA: Clear old PTK");
		sm->ptk_set = 0;
		sm->tptk_set = 0;
	}
}



void wpa_supplicant_event_assoc(struct wpa_supplicant *wpa_s,
				       union wpa_event_data *data)
{
	u8 bssid[ETH_ALEN];
//	int ft_completed;
//	int bssid_changed;
//	struct wpa_driver_capa capa;

	wpa_supplicant_set_state(wpa_s, WPA_ASSOCIATED);
	os_memcpy(bssid, wpa_s->bssid, ETH_ALEN);
	wpa_sm_notify_assoc(wpa_s->wpa, bssid);

}

void wpa_sm_notify_disassoc(struct wpa_sm *sm)
{
//	rsn_preauth_deinit(sm);
//	if (wpa_sm_get_state(sm) == WPA_4WAY_HANDSHAKE)
//		sm->dot11RSNA4WayHandshakeFailures++;
}


void wpa_supplicant_event_disassoc(struct wpa_supplicant *wpa_s,
					  u16 reason_code)
{
//	const u8 *bssid;
#ifdef CONFIG_SME
	int authenticating;
	u8 prev_pending_bssid[ETH_ALEN];

	authenticating = wpa_s->wpa_state == WPA_AUTHENTICATING;
	os_memcpy(prev_pending_bssid, wpa_s->pending_bssid, ETH_ALEN);
#endif /* CONFIG_SME */

	wpa_supplicant_set_state(wpa_s, WPA_DISCONNECTED);
}


static void wpa_eapol_key_dump(const struct wpa_eapol_key *key)
{
#ifdef DEBUG
	u16 key_info = WPA_GET_BE16(key->key_info);
#endif
	wpa_printf(MSG_DEBUG, "  EAPOL-Key type=%d", key->type);
	wpa_printf(MSG_DEBUG, "  key_info 0x%x (ver=%d keyidx=%d rsvd=%d %s"
		   "%s%s%s%s%s%s%s)",
		   key_info, key_info & WPA_KEY_INFO_TYPE_MASK,
		   (key_info & WPA_KEY_INFO_KEY_INDEX_MASK) >>
		   WPA_KEY_INFO_KEY_INDEX_SHIFT,
		   (key_info & (BIT(13) | BIT(14) | BIT(15))) >> 13,
		   key_info & WPA_KEY_INFO_KEY_TYPE ? "Pairwise" : "Group",
		   key_info & WPA_KEY_INFO_INSTALL ? " Install" : "",
		   key_info & WPA_KEY_INFO_ACK ? " Ack" : "",
		   key_info & WPA_KEY_INFO_MIC ? " MIC" : "",
		   key_info & WPA_KEY_INFO_SECURE ? " Secure" : "",
		   key_info & WPA_KEY_INFO_ERROR ? " Error" : "",
		   key_info & WPA_KEY_INFO_REQUEST ? " Request" : "",
		   key_info & WPA_KEY_INFO_ENCR_KEY_DATA ? " Encr" : "");
	wpa_printf(MSG_DEBUG, "  key_length=%u key_data_length=%u",
		   WPA_GET_BE16(key->key_length),
		   WPA_GET_BE16(key->key_data_length));
	wpa_hexdump(MSG_DEBUG, "  replay_counter",
		    (const u8*)key->replay_counter, WPA_REPLAY_COUNTER_LEN);
	wpa_hexdump(MSG_DEBUG, "  key_nonce", (const u8*)key->key_nonce, WPA_NONCE_LEN);
	wpa_hexdump(MSG_DEBUG, "  key_iv", (const u8*)key->key_iv, 16);
	wpa_hexdump(MSG_DEBUG, "  key_rsc", (const u8*)key->key_rsc, 8);
	wpa_hexdump(MSG_DEBUG, "  key_id (reserved)", (const u8*)key->key_id, 8);
	wpa_hexdump(MSG_DEBUG, "  key_mic", (const u8*)key->key_mic, 16);
}

void inc_byte_array(u8 *counter, size_t len)
{
	int pos = len - 1;
	while (pos >= 0) {
		counter[pos]++;
		if (counter[pos] != 0)
			break;
		pos--;
	}
}

int hmac_md5_vector(const u8 *key, size_t key_len, size_t num_elem,
		    const u8 *addr[], const size_t *len, u8 *mac)
{
	u8 k_pad[64]; /* padding - key XORd with ipad/opad */
	u8 tk[16];
	const u8 *_addr[6];
	size_t i, _len[6];

	if (num_elem > 5) {
		/*
		 * Fixed limit on the number of fragments to avoid having to
		 * allocate memory (which could fail).
		 */
		return -1;
	}

        /* if key is longer than 64 bytes reset it to key = MD5(key) */
        if (key_len > 64) {
		if (md5_vector(1, &key, &key_len, tk))
			return -1;
		key = tk;
		key_len = 16;
        }

	/* the HMAC_MD5 transform looks like:
	 *
	 * MD5(K XOR opad, MD5(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected */

	/* start out by storing key in ipad */
	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);

	/* XOR key with ipad values */
	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x36;

	/* perform inner MD5 */
	_addr[0] = k_pad;
	_len[0] = 64;
	for (i = 0; i < num_elem; i++) {
		_addr[i + 1] = addr[i];
		_len[i + 1] = len[i];
	}
	if (md5_vector(1 + num_elem, _addr, _len, mac))
		return -1;

	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);
	/* XOR key with opad values */
	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x5c;

	/* perform outer MD5 */
	_addr[0] = k_pad;
	_len[0] = 64;
	_addr[1] = mac;
	_len[1] = MD5_MAC_LEN;
	return md5_vector(2, _addr, _len, mac);
}

int hmac_md5(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
	      u8 *mac)
{
	return hmac_md5_vector(key, key_len, 1, &data, &data_len, mac);
}

int wpa_eapol_key_mic(const u8 *key, int ver, const u8 *buf, size_t len,
		      u8 *mic)
{
	u8 hash[SHA1_MAC_LEN];

	switch (ver) {
	case WPA_KEY_INFO_TYPE_HMAC_MD5_RC4:
		return hmac_md5(key, 16, buf, len, mic);
	case WPA_KEY_INFO_TYPE_HMAC_SHA1_AES:
		if (hmac_sha1(key, 16, buf, len, hash))
			return -1;
		os_memcpy(mic, hash, MD5_MAC_LEN);
		break;
#if defined(CONFIG_IEEE80211R) || defined(CONFIG_IEEE80211W)
	case WPA_KEY_INFO_TYPE_AES_128_CMAC:
		return omac1_aes_128(key, buf, len, mic);
#endif /* CONFIG_IEEE80211R || CONFIG_IEEE80211W */
	default:
		return -1;
	}

	return 0;
}


static int wpa_supplicant_verify_eapol_key_mic(struct wpa_sm *sm,
					       struct wpa_eapol_key *key,
					       u16 ver,
					       const u8 *buf, size_t len)
{
	u8 mic[16];
	int ok = 0;

	os_memcpy(mic, key->key_mic, 16);
	if (sm->tptk_set) {
		os_memset(key->key_mic, 0, 16);
		wpa_eapol_key_mic(sm->tptk.kck, ver, buf, len,
				  key->key_mic);
		if (os_memcmp(mic, key->key_mic, 16) != 0) {
			wpa_printf(MSG_WARNING, "WPA: Invalid EAPOL-Key MIC "
				   "when using TPTK - ignoring TPTK");
		} else {
			ok = 1;
			sm->tptk_set = 0;
			sm->ptk_set = 1;
			os_memcpy(&sm->ptk, &sm->tptk, sizeof(sm->ptk));
		}
	}

	if (!ok && sm->ptk_set) {
		os_memset(key->key_mic, 0, 16);
		wpa_eapol_key_mic(sm->ptk.kck, ver, buf, len,
				  key->key_mic);
		if (os_memcmp(mic, key->key_mic, 16) != 0) {
			wpa_printf(MSG_WARNING, "WPA: Invalid EAPOL-Key MIC "
				   "- dropping packet");
			return -1;
		}
		ok = 1;
	}

	if (!ok) {
		wpa_printf(MSG_WARNING, "WPA: Could not verify EAPOL-Key MIC "
			   "- dropping packet");
		return -1;
	}

	os_memcpy(sm->rx_replay_counter, key->replay_counter,
		  WPA_REPLAY_COUNTER_LEN);
	sm->rx_replay_counter_set = 1;
	return 0;
}


static int wpa_parse_generic(const u8 *pos, const u8 *end,
			     struct wpa_eapol_ie_parse *ie)
{
	if (pos[1] == 0)
		return 1;

	if (pos[1] >= 6 &&
	    RSN_SELECTOR_GET(pos + 2) == WPA_OUI_TYPE &&
	    pos[2 + WPA_SELECTOR_LEN] == 1 &&
	    pos[2 + WPA_SELECTOR_LEN + 1] == 0) {
		ie->wpa_ie = pos;
		ie->wpa_ie_len = pos[1] + 2;
		wpa_hexdump(MSG_DEBUG, "WPA: WPA IE in EAPOL-Key",
			    (const u8*)ie->wpa_ie, ie->wpa_ie_len);
		return 0;
	}

	if (pos + 1 + RSN_SELECTOR_LEN < end &&
	    pos[1] >= RSN_SELECTOR_LEN + PMKID_LEN &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_PMKID) {
		ie->pmkid = pos + 2 + RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: PMKID in EAPOL-Key",
			    (const u8*)pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_GROUPKEY) {
		ie->gtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->gtk_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: GTK in EAPOL-Key",
				(const u8*)pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_MAC_ADDR) {
		ie->mac_addr = pos + 2 + RSN_SELECTOR_LEN;
		ie->mac_addr_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: MAC Address in EAPOL-Key",
			    (const u8*)pos, pos[1] + 2);
		return 0;
	}

#ifdef CONFIG_PEERKEY
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_SMK) {
		ie->smk = pos + 2 + RSN_SELECTOR_LEN;
		ie->smk_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump_key(MSG_DEBUG, "WPA: SMK in EAPOL-Key",
				(const u8*)pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_NONCE) {
		ie->nonce = pos + 2 + RSN_SELECTOR_LEN;
		ie->nonce_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: Nonce in EAPOL-Key",
			    (const u8*)pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_LIFETIME) {
		ie->lifetime = pos + 2 + RSN_SELECTOR_LEN;
		ie->lifetime_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: Lifetime in EAPOL-Key",
			    (const u8*)pos, pos[1] + 2);
		return 0;
	}

	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_ERROR) {
		ie->error = pos + 2 + RSN_SELECTOR_LEN;
		ie->error_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: Error in EAPOL-Key",
			    (const u8*)pos, pos[1] + 2);
		return 0;
	}
#endif /* CONFIG_PEERKEY */

#ifdef CONFIG_IEEE80211W
	if (pos[1] > RSN_SELECTOR_LEN + 2 &&
	    RSN_SELECTOR_GET(pos + 2) == RSN_KEY_DATA_IGTK) {
		ie->igtk = pos + 2 + RSN_SELECTOR_LEN;
		ie->igtk_len = pos[1] - RSN_SELECTOR_LEN;
		wpa_hexdump(MSG_DEBUG, "WPA: IGTK in EAPOL-Key",
				(const u8*)pos, pos[1] + 2);
		return 0;
	}
#endif /* CONFIG_IEEE80211W */

	return 0;
}

static u8 * wpa_alloc_eapol(u8 type,
			    const void *data, u16 data_len,
			    size_t *msg_len, void **data_pos)
{
	struct ieee802_1x_hdr *hdr;

	*msg_len = sizeof(*hdr) + data_len;
	hdr = (struct ieee802_1x_hdr *)os_malloc(*msg_len);
	if (hdr == NULL)
		return NULL;

	hdr->version =  EAPOL_VERSION;
	hdr->type = type;
	hdr->length = host_to_be16(data_len);

	if (data)
		os_memcpy(hdr + 1, data, data_len);
	else
		os_memset(hdr + 1, 0, data_len);

	if (data_pos)
		*data_pos = hdr + 1;

	return (u8 *) hdr;
}

static u8 * wpa_sm_alloc_eapol(struct wpa_sm *sm, u8 type,
				      const void *data, u16 data_len,
				      size_t *msg_len, void **data_pos)
{
	return wpa_alloc_eapol(type, data, data_len,
				    msg_len, data_pos);
}


int wpa_supplicant_parse_ies(const u8 *buf, size_t len,
			     struct wpa_eapol_ie_parse *ie)
{
	const u8 *pos, *end;
	int ret = 0;

	os_memset(ie, 0, sizeof(*ie));
	for (pos = buf, end = pos + len; pos + 1 < end; pos += 2 + pos[1]) {
		if (pos[0] == 0xdd &&
		    ((pos == buf + len - 1) || pos[1] == 0)) {
			/* Ignore padding */
			break;
		}
		if (pos + 2 + pos[1] > end) {
			wpa_printf(MSG_DEBUG, "WPA: EAPOL-Key Key Data "
				   "underflow (ie=%d len=%d pos=%d)",
				   pos[0], pos[1], (int) (pos - buf));
			wpa_hexdump(MSG_DEBUG, "WPA: Key Data",
					(const u8*)buf, len);
			ret = -1;
			break;
		}
		if (*pos == WLAN_EID_RSN) {
			ie->rsn_ie = pos;
			ie->rsn_ie_len = pos[1] + 2;
			wpa_hexdump(MSG_DEBUG, "WPA: RSN IE in EAPOL-Key",
				    (const u8*)ie->rsn_ie, ie->rsn_ie_len);
#ifdef CONFIG_IEEE80211R
		} else if (*pos == WLAN_EID_MOBILITY_DOMAIN) {
			ie->mdie = pos;
			ie->mdie_len = pos[1] + 2;
			wpa_hexdump(MSG_DEBUG, "WPA: MDIE in EAPOL-Key",
				    (const u8*)ie->mdie, ie->mdie_len);
		} else if (*pos == WLAN_EID_FAST_BSS_TRANSITION) {
			ie->ftie = pos;
			ie->ftie_len = pos[1] + 2;
			wpa_hexdump(MSG_DEBUG, "WPA: FTIE in EAPOL-Key",
				    (const u8*)ie->ftie, ie->ftie_len);
		} else if (*pos == WLAN_EID_TIMEOUT_INTERVAL && pos[1] >= 5) {
			if (pos[2] == WLAN_TIMEOUT_REASSOC_DEADLINE) {
				ie->reassoc_deadline = pos;
				wpa_hexdump(MSG_DEBUG, "WPA: Reassoc Deadline "
					    "in EAPOL-Key",
					    (const u8*)ie->reassoc_deadline, pos[1] + 2);
			} else if (pos[2] == WLAN_TIMEOUT_KEY_LIFETIME) {
				ie->key_lifetime = pos;
				wpa_hexdump(MSG_DEBUG, "WPA: KeyLifetime "
					    "in EAPOL-Key",
					    (const u8*)ie->key_lifetime, pos[1] + 2);
			} else {
				wpa_hexdump(MSG_DEBUG, "WPA: Unrecognized "
					    "EAPOL-Key Key Data IE",
					    (const u8*)pos, 2 + pos[1]);
			}
#endif /* CONFIG_IEEE80211R */
		} else if (*pos == WLAN_EID_VENDOR_SPECIFIC) {
			ret = wpa_parse_generic(pos, end, ie);
			if (ret < 0)
				break;
			if (ret > 0) {
				ret = 0;
				break;
			}
		} else {
			wpa_hexdump(MSG_DEBUG, "WPA: Unrecognized EAPOL-Key "
				    "Key Data IE", (const u8*)pos, 2 + pos[1]);
		}
	}

	return ret;
}


static int wpa_supplicant_get_pmk(struct wpa_sm *sm,
				  const unsigned char *src_addr,
				  const u8 *pmkid)
{
	int i;
	for(i = 0;i< PMK_CACHE_NUM ;i++)
	{
		if(os_memcmp(sm->pmksa[i].aa,src_addr,ETH_ALEN) == 0)
		{
			os_memcpy(sm->pmk, sm->pmksa[i].pmk, sm->pmksa[i].pmk_len);
			sm->pmk_len = sm->pmksa[i].pmk_len;
			return 0;
		}
	}
	return -2;
//	return 0;
}

int os_get_random(unsigned char *buf, size_t len)
{
	int i,j;
	uint32_t random_num;
	char tmp[16];
	int keylen = 4;
	const u8 *addr = (const u8 *)&random_num;
	random_num = jiffies;
	
	md5_vector(1, &addr, (const size_t *)&keylen, (u8*)tmp);
	for(i = 0, j = 0;i < len ;i++)
	{
		buf[i] = tmp[j++];
		if(j >= 16)
		{
			random_num++;
			md5_vector(1, &addr, (const size_t *)&keylen, (u8*)tmp);
			j = 0;
		}
	}
	return 0;
}

struct pmk_to_ptk_arg {
   	const u8 *pmk; 
	size_t pmk_len; 
	const char *label;
	const u8 *addr1; 
	const u8 *addr2;
       u8 *nonce1; 
	u8 *nonce2;
    u8 *ptk; 
	size_t ptk_len; 
	int use_sha256;
};

struct pmk_to_ptk_arg ptk_arg;

void wpa_pmk_to_ptk(struct pmk_to_ptk_arg *arg)
{
	u8 *data;
/*	const u8 *pmk; size_t pmk_len; const char *label;
	const u8 *addr1; const u8 *addr2;
	const u8 *nonce1; const u8 *nonce2;
	u8 *ptk; size_t ptk_len; int use_sha256;
											 */
	if(arg==0)
		return;
/*	pmk = arg->pmk;
	pmk_len = arg->pmk_len;
	label = arg->label;
	addr1 = arg->addr1;
	addr2 = arg->addr2;
	nonce1 = arg->nonce1;
	nonce2 = arg->nonce2;
	ptk = arg->ptk;
	ptk_len = arg->ptk_len;
	use_sha256 = arg->use_sha256;  */
//release ok
	data = (u8*)os_malloc(2 * ETH_ALEN + 2 * WPA_NONCE_LEN);
	if(data == 0)
		return;
	if (os_memcmp(arg->addr1, arg->addr2, ETH_ALEN) < 0) {
		os_memcpy(data, arg->addr1, ETH_ALEN);
		os_memcpy(data + ETH_ALEN, arg->addr2, ETH_ALEN);
	} else {
		os_memcpy(data, arg->addr2, ETH_ALEN);
		os_memcpy(data + ETH_ALEN, arg->addr1, ETH_ALEN);
	}

	if (os_memcmp(arg->nonce1, arg->nonce2, WPA_NONCE_LEN) < 0) {
		os_memcpy(data + 2 * ETH_ALEN, arg->nonce1, WPA_NONCE_LEN);
		os_memcpy(data + 2 * ETH_ALEN + WPA_NONCE_LEN, arg->nonce2,
			  WPA_NONCE_LEN);
	} else {
		os_memcpy(data + 2 * ETH_ALEN, arg->nonce2, WPA_NONCE_LEN);
		os_memcpy(data + 2 * ETH_ALEN + WPA_NONCE_LEN, arg->nonce1,
			  WPA_NONCE_LEN);
	}

	wpa_hexdump(MSG_DEBUG, "pmk:", arg->pmk, arg->pmk_len);
	wpa_hexdump(MSG_DEBUG, "own_addr:", arg->addr1, 6);
	wpa_hexdump(MSG_DEBUG, "bssid:", arg->addr2, 6);
	wpa_hexdump(MSG_DEBUG, "snonce:", arg->nonce1, 32);
	wpa_hexdump(MSG_DEBUG, "key_nonce:", arg->nonce2, 32);
	wpa_hexdump(MSG_DEBUG, "label:", (const u8*)arg->label, strlen(arg->label));
	wpa_hexdump(MSG_DEBUG, "data:", data, 2 * ETH_ALEN + 2 * WPA_NONCE_LEN);
#ifdef CONFIG_IEEE80211W
	if (use_sha256)
		sha256_prf(arg->pmk, arg->pmk_len, arg->label, data, sizeof(data),
			   arg->ptk, arg->ptk_len);
	else
#endif /* CONFIG_IEEE80211W */
		sha1_prf(arg->pmk, arg->pmk_len, arg->label, data, 2 * ETH_ALEN + 2 * WPA_NONCE_LEN/*sizeof(data)*/, arg->ptk,
			 arg->ptk_len);

	wpa_printf(MSG_DEBUG, "WPA: PTK derivation - A1=" MACSTR " A2=" MACSTR,
		   MAC2STR(arg->addr1), MAC2STR(arg->addr2));
	wpa_hexdump(MSG_DEBUG, "WPA: PMK", (const u8*)arg->pmk, arg->pmk_len);
	wpa_hexdump(MSG_DEBUG, "WPA: PTK", (const u8*)arg->ptk, arg->ptk_len);

	os_free(data);
}

static int is_zero_ether_addr(const u8 *a)
{
	return !(a[0] | a[1] | a[2] | a[3] | a[4] | a[5]);
}

int wpa_sm_ether_send(struct wpa_sm *sm, const u8 *dest,
				    u16 proto, const u8 *buf, size_t len)
{

  struct eth_hdr *ethhdr;
  struct pbuf *p, *q;
  int rem_len;
  
  p = pbuf_alloc(PBUF_RAW, len + sizeof(struct eth_hdr), PBUF_POOL);
  
  if(p == 0)
	  return -1;

  buf -= sizeof(struct eth_hdr);
  len += sizeof(struct eth_hdr);

  if (p->tot_len >= len) {

     rem_len = p->tot_len;
    for(q = p; q != NULL; q = q->next) {

       if(rem_len > 0)
       	{
       		memcpy(q->payload, (char*)buf + (p->tot_len - rem_len), q->len);
			rem_len -= q->len;
			
       	}
	   else
	   	p_err("wpa_sm_ether_send memcpy err\n");
    }
  }	

  ethhdr = (struct eth_hdr *)p->payload;

	memcpy((void*)&ethhdr->dest,dest,ETHARP_HWADDR_LEN);
	memcpy((void*)&ethhdr->src, p_netif->hwaddr,ETHARP_HWADDR_LEN);

  ethhdr->type = PP_HTONS(proto);

  wpa_hexdump(MSG_DEBUG, "WPA: ether_send ", (const u8*)p->payload , p->len);
  /* send the packet */
  p_netif->linkoutput(p_netif, p);

  pbuf_free(p);
  return 0;
}



void wpa_eapol_key_send(struct wpa_sm *sm, const u8 *kck,
			int ver, const u8 *dest, u16 proto,
			u8 *msg, size_t msg_len, u8 *key_mic)
{
	if (is_zero_ether_addr(dest) && is_zero_ether_addr(sm->bssid)) {
		/*
		 * Association event was not yet received; try to fetch
		 * BSSID from the driver.
		 */
		if (/*wpa_sm_get_bssid(sm, sm->bssid)*/-1 < 0) {
			wpa_printf(MSG_DEBUG, "WPA: Failed to read BSSID for "
				   "EAPOL-Key destination address");
		} else {
			dest = sm->bssid;
			wpa_printf(MSG_DEBUG, "WPA: Use BSSID (" MACSTR
				   ") as the destination for EAPOL-Key",
				   MAC2STR(dest));
		}
	}
	if (key_mic &&
	    wpa_eapol_key_mic(kck, ver, msg, msg_len, key_mic)) {
		wpa_printf(MSG_ERROR, "WPA: Failed to generate EAPOL-Key "
			   "version %d MIC", ver);
		goto out;
	}
	wpa_hexdump(MSG_MSGDUMP, "WPA: TX EAPOL-Key", msg, msg_len);
	wpa_sm_ether_send(sm, dest, proto, msg, msg_len);
//	eapol_sm_notify_tx_eapol_key(sm->eapol);
out:
	os_free(msg);
}


int wpa_supplicant_send_2_of_4(struct wpa_sm *sm, const unsigned char *dst,
			       const struct wpa_eapol_key *key,
			       int ver, const u8 *nonce,
			       const u8 *wpa_ie, size_t wpa_ie_len,
			       struct wpa_ptk *ptk)
{
	size_t rlen;
	struct wpa_eapol_key *reply;
	u8 *rbuf;
	u8 *rsn_ie_buf = NULL;

	if (wpa_ie == NULL) {
		wpa_printf(MSG_WARNING, "WPA: No wpa_ie set - cannot "
			   "generate msg 2/4");
		return -1;
	}

#ifdef CONFIG_IEEE80211R
	if (wpa_key_mgmt_ft(sm->key_mgmt)) {
		int res;

		/*
		 * Add PMKR1Name into RSN IE (PMKID-List) and add MDIE and
		 * FTIE from (Re)Association Response.
		 */
		 //release ok
		rsn_ie_buf = os_malloc(wpa_ie_len + 2 + 2 + PMKID_LEN +
				       sm->assoc_resp_ies_len);
		if (rsn_ie_buf == NULL)
			return -1;
		os_memcpy(rsn_ie_buf, wpa_ie, wpa_ie_len);
		res = wpa_insert_pmkid(rsn_ie_buf, wpa_ie_len,
				       sm->pmk_r1_name);
		if (res < 0) {
			os_free(rsn_ie_buf);
			return -1;
		}
		wpa_ie_len += res;

		if (sm->assoc_resp_ies) {
			os_memcpy(rsn_ie_buf + wpa_ie_len, sm->assoc_resp_ies,
				  sm->assoc_resp_ies_len);
			wpa_ie_len += sm->assoc_resp_ies_len;
		}

		wpa_ie = rsn_ie_buf;
	}
#endif /* CONFIG_IEEE80211R */

	wpa_hexdump(MSG_DEBUG, "WPA: WPA IE for msg 2/4", (const u8*)wpa_ie, wpa_ie_len);


//安全释放
	rbuf = wpa_sm_alloc_eapol(sm, IEEE802_1X_TYPE_EAPOL_KEY,
				  NULL, sizeof(*reply) + wpa_ie_len,
				  &rlen, (void *) &reply);
	if (rbuf == NULL) {
		if(rsn_ie_buf)
			os_free(rsn_ie_buf);
		return -1;
	}

	reply->type = sm->proto == WPA_PROTO_RSN ?
		EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
	WPA_PUT_BE16(reply->key_info,
		     ver | WPA_KEY_INFO_KEY_TYPE | WPA_KEY_INFO_MIC);
	if (sm->proto == WPA_PROTO_RSN)
		WPA_PUT_BE16(reply->key_length, 0);
	else
		os_memcpy(reply->key_length, key->key_length, 2);
	os_memcpy(reply->replay_counter, key->replay_counter,
		  WPA_REPLAY_COUNTER_LEN);

	WPA_PUT_BE16(reply->key_data_length, wpa_ie_len);
	os_memcpy(reply + 1, wpa_ie, wpa_ie_len);
	if(rsn_ie_buf)
		os_free(rsn_ie_buf);

	os_memcpy(reply->key_nonce, nonce, WPA_NONCE_LEN);

	wpa_printf(MSG_DEBUG, "WPA: Sending EAPOL-Key 2/4");
	wpa_eapol_key_send(sm, ptk->kck, ver, dst, ETH_P_EAPOL,
			   rbuf, rlen, reply->key_mic);

	return 0;
}

const u8_t own_addr[6] ={0x00, 0x1a, 0x6b, 0x6e, 0x70, 0xc2};
const u8_t snonce[32] = {
0x4c  , 0x18  , 0xeb  , 0xf7  , 0x54  , 0x4a  , 0x48  , 0x98  , 0x88  , 0x3a  , 0x61  , 0x90  , 0x40  , 0xce  , 0x7f  , 0x96  , 0x46  , 0x40  , 0xdc  , 0xcd  , 0xeb  , 0x5a  , 0x47  , 0x3e  , 0x46  , 0x79  , 0x7e  , 0x00  , 0x30  , 0x3b  , 0x25  , 0x41
//	0xd6 , 0xa6 , 0x79 , 0x2c , 0x82 , 0xd6 , 0xa0 , 0xf2 , 0xec , 0x56 , 0x1a , 0x9e , 0xd5 , 0xe7 , 0x1b , 0xe7 , 0x22 , 0xd4 , 0xf3 , 0x4e , 0xae , 0xd2, 0xfe , 0x40 , 0xa1 , 0x39 , 0x91 , 0x4a , 0x17 , 0xa3 , 0x44 , 0x27
	};
static int wpa_derive_ptk(struct wpa_sm *sm, const unsigned char *src_addr,
			  const struct wpa_eapol_key *key,
			  struct wpa_ptk *ptk)
{
	size_t ptk_len = sm->pairwise_cipher == WPA_CIPHER_CCMP ? 48 : 64;
#ifdef CONFIG_IEEE80211R
	if (wpa_key_mgmt_ft(sm->key_mgmt))
		return wpa_derive_ptk_ft(sm, src_addr, key, ptk, ptk_len);
#endif /* CONFIG_IEEE80211R */

	ptk_arg.pmk = sm->pmk;
	ptk_arg.pmk_len = sm->pmk_len;
	ptk_arg.label = "Pairwise key expansion";
	ptk_arg.addr1 = sm->own_addr;
//	ptk_arg.addr1 = own_addr;
	ptk_arg.addr2 =  sm->bssid;
	ptk_arg.nonce1 = sm->snonce;
//	ptk_arg.nonce1 = (u8*)snonce;
	ptk_arg.nonce2 = (u8*)key->key_nonce;
//	ptk_arg.nonce2[31] = 0x76;
	ptk_arg.ptk = (u8 *) ptk;
	ptk_arg.ptk_len = ptk_len;
	ptk_arg.use_sha256 = wpa_key_mgmt_sha256(sm->key_mgmt);

	p_dbg("wpa_derive_ptk,sha256:%d\n",wpa_key_mgmt_sha256(sm->key_mgmt));
/*	wpa_hexdump(MSG_DEBUG, "pmk:", ptk_arg.pmk, ptk_arg.pmk_len);
	wpa_hexdump(MSG_DEBUG, "own_addr:", ptk_arg.addr1, 6);
	wpa_hexdump(MSG_DEBUG, "bssid:", ptk_arg.addr2, 6);
	wpa_hexdump(MSG_DEBUG, "snonce:", ptk_arg.nonce1, 32);
	wpa_hexdump(MSG_DEBUG, "key_nonce:", ptk_arg.nonce2, 32);*/
	
	wpa_pmk_to_ptk(&ptk_arg);

	wpa_hexdump(MSG_DEBUG,  "ptk:", (u8 *) ptk, ptk_len);
	
	return 0;
}

#ifdef TEST_EAPOL	
uint8_t test_snonce[32] = {
0x4e,0xc0,0x6b,0x30,0xca,0x66,0xb2,0x9f,
0xda,0x11,0x6c,0xe1,0x53,0x06,0x3b,0x29,
0xcf,0x4b,0xc5,0x50,0x7d,0x5f,0xc4,0x71,
0x65,0xeb,0x2e,0xe4,0xea,0x2d,0xd4,0x78
};
	
#endif

static void wpa_supplicant_process_1_of_4(struct wpa_sm *sm,
					  const unsigned char *src_addr,
					  const struct wpa_eapol_key *key,
					  u16 ver)
{

	struct wpa_eapol_ie_parse ie;
	struct wpa_ptk *ptk;
	u8 buf[8];
	int res;

/*
	if (wpa_sm_get_network_ctx(sm) == NULL) {
		wpa_printf(MSG_WARNING, "WPA: No SSID info found (msg 1 of "
			   "4).");
		return;
	}*/

	wpa_sm_set_state(sm, WPA_4WAY_HANDSHAKE);
	wpa_printf(MSG_DEBUG, "WPA: RX message 1 of 4-Way Handshake from "
		   MACSTR " (ver=%d)", MAC2STR(src_addr), ver);

	os_memset(&ie, 0, sizeof(ie));

	if (sm->proto == WPA_PROTO_RSN) {
		/* RSN: msg 1/4 should contain PMKID for the selected PMK */
		const u8 *_buf = (const u8 *) (key + 1);
		size_t len = WPA_GET_BE16(key->key_data_length);
		wpa_hexdump(MSG_DEBUG, "RSN: msg 1/4 key data", (const u8*)_buf, len);
		wpa_supplicant_parse_ies(_buf, len, &ie);
		if (ie.pmkid) {
			wpa_hexdump(MSG_DEBUG, "RSN: PMKID from "
				    "Authenticator", (const u8*)ie.pmkid, PMKID_LEN);
		}
	}

	res = wpa_supplicant_get_pmk(sm, src_addr, ie.pmkid);
	if (res == -2) {
		wpa_printf(MSG_DEBUG, "RSN: Do not reply to msg 1/4 - "
			   "requesting full EAP authentication");
		return;
	}
	if (res)
		goto failed;

	if (sm->renew_snonce) {
#ifdef TEST_EAPOL		
		memcpy(sm->snonce,test_snonce,32);
#else
		if (os_get_random(sm->snonce, WPA_NONCE_LEN)) {
			wpa_printf(MSG_WARNING,
				"WPA: Failed to get random data for SNonce");
			goto failed;
		}
#endif
		sm->renew_snonce = 0;
		wpa_hexdump(MSG_DEBUG, "WPA: Renewed SNonce",
			    sm->snonce, WPA_NONCE_LEN);
	}

	/* Calculate PTK which will be stored as a temporary PTK until it has
	 * been verified when processing message 3/4. */
	ptk = &sm->tptk;
	wpa_derive_ptk(sm, src_addr, key, ptk);
	/* Supplicant: swap tx/rx Mic keys */
	os_memcpy(buf, ptk->u.auth.tx_mic_key, 8);
	os_memcpy(ptk->u.auth.tx_mic_key, ptk->u.auth.rx_mic_key, 8);
	os_memcpy(ptk->u.auth.rx_mic_key, buf, 8);
	sm->tptk_set = 1;

	if (wpa_supplicant_send_2_of_4(sm, sm->bssid, key, ver, sm->snonce,
				       sm->assoc_wpa_ie, sm->assoc_wpa_ie_len,
				       ptk))
		goto failed;

	os_memcpy(sm->anonce, key->key_nonce, WPA_NONCE_LEN);
	return;
failed:
//	wpa_sm_deauthenticate(sm, WLAN_REASON_UNSPECIFIED);
	return;
}

int wpa_compare_rsn_ie(int ft_initial_assoc,
		       const u8 *ie1, size_t ie1len,
		       const u8 *ie2, size_t ie2len)
{
	if (ie1 == NULL || ie2 == NULL)
		return -1;

	if (ie1len == ie2len && os_memcmp(ie1, ie2, ie1len) == 0)
		return 0; /* identical IEs */

#ifdef CONFIG_IEEE80211R
	if (ft_initial_assoc) {
		struct wpa_ie_data ie1d, ie2d;
		/*
		 * The PMKID-List in RSN IE is different between Beacon/Probe
		 * Response/(Re)Association Request frames and EAPOL-Key
		 * messages in FT initial mobility domain association. Allow
		 * for this, but verify that other parts of the RSN IEs are
		 * identical.
		 */
		if (wpa_parse_wpa_ie_rsn(ie1, ie1len, &ie1d) < 0 ||
		    wpa_parse_wpa_ie_rsn(ie2, ie2len, &ie2d) < 0)
			return -1;
		if (ie1d.proto == ie2d.proto &&
		    ie1d.pairwise_cipher == ie2d.pairwise_cipher &&
		    ie1d.group_cipher == ie2d.group_cipher &&
		    ie1d.key_mgmt == ie2d.key_mgmt &&
		    ie1d.capabilities == ie2d.capabilities &&
		    ie1d.mgmt_group_cipher == ie2d.mgmt_group_cipher)
			return 0;
	}
#endif /* CONFIG_IEEE80211R */

	return -1;
}


static int wpa_supplicant_validate_ie(struct wpa_sm *sm,
				      const unsigned char *src_addr,
				      struct wpa_eapol_ie_parse *ie)
{
	if (sm->ap_wpa_ie == NULL && sm->ap_rsn_ie == NULL) {
		wpa_printf(MSG_DEBUG, "WPA: No WPA/RSN IE for this AP known. "
			   "Trying to get from scan results");
//		if (/*wpa_sm_get_beacon_ie(sm)*/-1 < 0) {
//			wpa_printf(MSG_WARNING, "WPA: Could not find AP from "
//				   "the scan results");
//		} else {
			wpa_printf(MSG_DEBUG, "WPA: Found the current AP from "
				   "updated scan results");
//		}
	}

	if (ie->wpa_ie == NULL && ie->rsn_ie == NULL &&
	    (sm->ap_wpa_ie || sm->ap_rsn_ie)) {
/*		wpa_report_ie_mismatch(sm, "IE in 3/4 msg does not match "
				       "with IE in Beacon/ProbeResp (no IE?)",
				       src_addr, ie->wpa_ie, ie->wpa_ie_len,
				       ie->rsn_ie, ie->rsn_ie_len);*/
		return -1;
	}

	if ((ie->wpa_ie && sm->ap_wpa_ie &&
	     (ie->wpa_ie_len != sm->ap_wpa_ie_len ||
	      os_memcmp(ie->wpa_ie, sm->ap_wpa_ie, ie->wpa_ie_len) != 0)) ||
	    (ie->rsn_ie && sm->ap_rsn_ie &&
	     wpa_compare_rsn_ie(wpa_key_mgmt_ft(sm->key_mgmt),
				sm->ap_rsn_ie, sm->ap_rsn_ie_len,
				ie->rsn_ie, ie->rsn_ie_len))) {
	/*	wpa_report_ie_mismatch(sm, "IE in 3/4 msg does not match "
				       "with IE in Beacon/ProbeResp",
				       src_addr, ie->wpa_ie, ie->wpa_ie_len,
				       ie->rsn_ie, ie->rsn_ie_len);*/
		return -1;
	}

	if (sm->proto == WPA_PROTO_WPA &&
	    ie->rsn_ie && sm->ap_rsn_ie == NULL && sm->rsn_enabled) {
	/*	wpa_report_ie_mismatch(sm, "Possible downgrade attack "
				       "detected - RSN was enabled and RSN IE "
				       "was in msg 3/4, but not in "
				       "Beacon/ProbeResp",
				       src_addr, ie->wpa_ie, ie->wpa_ie_len,
				       ie->rsn_ie, ie->rsn_ie_len);*/
		return -1;
	}

#ifdef CONFIG_IEEE80211R
	if (wpa_key_mgmt_ft(sm->key_mgmt) &&
	    wpa_supplicant_validate_ie_ft(sm, src_addr, ie) < 0)
		return -1;
#endif /* CONFIG_IEEE80211R */

	return 0;
}


int wpa_supplicant_send_4_of_4(struct wpa_sm *sm, const unsigned char *dst,
			       const struct wpa_eapol_key *key,
			       u16 ver, u16 key_info,
			       const u8 *kde, size_t kde_len,
			       struct wpa_ptk *ptk)
{
	size_t rlen;
	struct wpa_eapol_key *reply;
	u8 *rbuf;

	if (kde)
		wpa_hexdump(MSG_DEBUG, "WPA: KDE for msg 4/4", (const u8*)kde, kde_len);

//安全释放
	rbuf = wpa_sm_alloc_eapol(sm, IEEE802_1X_TYPE_EAPOL_KEY, NULL,
				  sizeof(*reply) + kde_len,
				  &rlen, (void *) &reply);
	if (rbuf == NULL)
		return -1;

	reply->type = sm->proto == WPA_PROTO_RSN ?
		EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
	key_info &= WPA_KEY_INFO_SECURE;
	key_info |= ver | WPA_KEY_INFO_KEY_TYPE | WPA_KEY_INFO_MIC;
	WPA_PUT_BE16(reply->key_info, key_info);
	if (sm->proto == WPA_PROTO_RSN)
		WPA_PUT_BE16(reply->key_length, 0);
	else
		os_memcpy(reply->key_length, key->key_length, 2);
	os_memcpy(reply->replay_counter, key->replay_counter,WPA_REPLAY_COUNTER_LEN);

	WPA_PUT_BE16(reply->key_data_length, kde_len);
	if (kde)
		os_memcpy(reply + 1, kde, kde_len);

	wpa_printf(MSG_DEBUG, "WPA: Sending EAPOL-Key 4/4");
	wpa_eapol_key_send(sm, ptk->kck, ver, dst, ETH_P_EAPOL,
			   rbuf, rlen, reply->key_mic);

	return 0;
}


static int wpa_supplicant_install_ptk(struct wpa_sm *sm,
				      const struct wpa_eapol_key *key)
{
	int rsclen;
//	enum wpa_alg alg;
	const u8 *key_rsc;
	u8 null_rsc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	struct key_params params;
	
	wpa_printf(MSG_DEBUG, "WPA: Installing PTK to the driver.");

	switch (sm->pairwise_cipher) {
	case WPA_CIPHER_CCMP:
//		alg = WPA_ALG_CCMP;
//		keylen = 16;
		rsclen = 6;
		params.cipher = WLAN_CIPHER_SUITE_CCMP;
		params.key = sm->ptk.tk1;
		params.key_len = 16;
		break;
	case WPA_CIPHER_TKIP:
	//	alg = WPA_ALG_TKIP;
	//	keylen = 32;
		rsclen = 6;
		params.cipher = WLAN_CIPHER_SUITE_TKIP;
		params.key = sm->ptk.tk1;
		params.key_len = 32;
		break;
	case WPA_CIPHER_NONE:
		wpa_printf(MSG_DEBUG, "WPA: Pairwise Cipher Suite: "
			   "NONE - do not use pairwise keys");
		return 0;
	default:
		wpa_printf(MSG_WARNING, "WPA: Unsupported pairwise cipher %d",
			   sm->pairwise_cipher);
		return -1;
	}

	if (sm->proto == WPA_PROTO_RSN) {
		key_rsc = null_rsc;
	} else {
		key_rsc = key->key_rsc;
		wpa_hexdump(MSG_DEBUG, "WPA: RSC", (const u8*)key_rsc, rsclen);
	}
	params.seq = (u8 *)key_rsc;
	params.seq_len = rsclen;
 
	//lbs_cfg_add_key(&priv, priv.dev,
	//		   0, (bool)0,
	//		   &params);	
	p_err_miss;
	
/*	if (wpa_sm_set_key(sm, alg, sm->bssid, 0, 1, key_rsc, rsclen,
			   (u8 *) sm->ptk.tk1, keylen) < 0) {
		wpa_printf(MSG_WARNING, "WPA: Failed to set PTK to the "
			   "driver (alg=%d keylen=%d bssid=" MACSTR ")",
			   alg, keylen, MAC2STR(sm->bssid));
		return -1;
	}

	if (sm->wpa_ptk_rekey) {
		eloop_cancel_timeout(wpa_sm_rekey_ptk, sm, NULL);
		eloop_register_timeout(sm->wpa_ptk_rekey, 0, wpa_sm_rekey_ptk,
				       sm, NULL);
	}*/

	return 0;
}

static void wpa_supplicant_process_3_of_4(struct wpa_sm *sm,
					  const struct wpa_eapol_key *key,
					  u16 ver)
{
	u16 key_info, keylen, len;
	const u8 *pos;
	struct wpa_eapol_ie_parse ie;

	wpa_sm_set_state(sm, WPA_4WAY_HANDSHAKE);
	wpa_printf(MSG_DEBUG, "WPA: RX message 3 of 4-Way Handshake from "
		   MACSTR " (ver=%d)", MAC2STR(sm->bssid), ver);

	key_info = WPA_GET_BE16(key->key_info);

	pos = (const u8 *) (key + 1);
	len = WPA_GET_BE16(key->key_data_length);
	wpa_hexdump(MSG_DEBUG, "WPA: IE KeyData", (const u8*)pos, len);
	wpa_supplicant_parse_ies(pos, len, &ie);
//	wpa_supplicant_parse_ies(sm->ap_rsn_ie, sm->ap_rsn_ie_len, &ie);
	if (ie.gtk && !(key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
		wpa_printf(MSG_WARNING, "WPA: GTK IE in unencrypted key data");
		goto failed;
	}
#ifdef CONFIG_IEEE80211W
	if (ie.igtk && !(key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
		wpa_printf(MSG_WARNING, "WPA: IGTK KDE in unencrypted key "
			   "data");
		goto failed;
	}

	if (ie.igtk && ie.igtk_len != sizeof(struct wpa_igtk_kde)) {
		wpa_printf(MSG_WARNING, "WPA: Invalid IGTK KDE length %lu",
			   (unsigned long) ie.igtk_len);
		goto failed;
	}
#endif /* CONFIG_IEEE80211W */

	if (wpa_supplicant_validate_ie(sm, sm->bssid, &ie) < 0)
		goto failed;

	if (os_memcmp(sm->anonce, key->key_nonce, WPA_NONCE_LEN) != 0) {
		wpa_printf(MSG_WARNING, "WPA: ANonce from message 1 of 4-Way "
			   "Handshake differs from 3 of 4-Way Handshake - drop"
			   " packet (src=" MACSTR ")", MAC2STR(sm->bssid));
		goto failed;
	}

	keylen = WPA_GET_BE16(key->key_length);
	switch (sm->pairwise_cipher) {
	case WPA_CIPHER_CCMP:
		if (keylen != 16) {
			wpa_printf(MSG_WARNING, "WPA: Invalid CCMP key length "
				   "%d (src=" MACSTR ")",
				   keylen, MAC2STR(sm->bssid));
			goto failed;
		}
		break;
	case WPA_CIPHER_TKIP:
		if (keylen != 32) {
			wpa_printf(MSG_WARNING, "WPA: Invalid TKIP key length "
				   "%d (src=" MACSTR ")",
				   keylen, MAC2STR(sm->bssid));
			goto failed;
		}
		break;
	}

	if (wpa_supplicant_send_4_of_4(sm, sm->bssid, key, ver, key_info,
				       NULL, 0, &sm->ptk)) {
		goto failed;
	}

	/* SNonce was successfully used in msg 3/4, so mark it to be renewed
	 * for the next 4-Way Handshake. If msg 3 is received again, the old
	 * SNonce will still be used to avoid changing PTK. */
	sm->renew_snonce = 1;

	if (key_info & WPA_KEY_INFO_INSTALL) {
		if (wpa_supplicant_install_ptk(sm, key))
			goto failed;
	}

	if (key_info & WPA_KEY_INFO_SECURE) {
		p_err("wpa_sm_mlme_setprotection\n");
/*
		wpa_sm_mlme_setprotection(
			sm, sm->bssid, MLME_SETPROTECTION_PROTECT_TYPE_RX,
			MLME_SETPROTECTION_KEY_TYPE_PAIRWISE);
		eapol_sm_notify_portValid(sm->eapol, TRUE);*/
	}
	wpa_sm_set_state(sm, WPA_GROUP_HANDSHAKE);

	if (ie.gtk &&
	    wpa_supplicant_pairwise_gtk(sm, key,
					ie.gtk, ie.gtk_len, key_info) < 0) {
		wpa_printf(MSG_INFO, "RSN: Failed to configure GTK");
		goto failed;
	}

	if (ieee80211w_set_keys(sm, &ie) < 0) {
		wpa_printf(MSG_INFO, "RSN: Failed to configure IGTK");
		goto failed;
	}

	return;

failed:
//	wpa_sm_deauthenticate(sm, WLAN_REASON_UNSPECIFIED);
	return;
}



static int wpa_supplicant_check_group_cipher(int group_cipher,
					     int keylen, int maxkeylen,
					     int *key_rsc_len,
					     enum wpa_alg *alg)
{
	int ret = 0;

	switch (group_cipher) {
	case WPA_CIPHER_CCMP:
		if (keylen != 16 || maxkeylen < 16) {
			ret = -1;
			break;
		}
		*key_rsc_len = 6;
		*alg = WPA_ALG_CCMP;
		break;
	case WPA_CIPHER_TKIP:
		if (keylen != 32 || maxkeylen < 32) {
			ret = -1;
			break;
		}
		*key_rsc_len = 6;
		*alg = WPA_ALG_TKIP;
		break;
	case WPA_CIPHER_WEP104:
		if (keylen != 13 || maxkeylen < 13) {
			ret = -1;
			break;
		}
		*key_rsc_len = 0;
		*alg = WPA_ALG_WEP;
		break;
	case WPA_CIPHER_WEP40:
		if (keylen != 5 || maxkeylen < 5) {
			ret = -1;
			break;
		}
		*key_rsc_len = 0;
		*alg = WPA_ALG_WEP;
		break;
	default:
		wpa_printf(MSG_WARNING, "WPA: Unsupported Group Cipher %d",
			   group_cipher);
		return -1;
	}

	if (ret < 0 ) {
	/*	wpa_printf(MSG_WARNING, "WPA: Unsupported %s Group Cipher key "
			   "length %d (%d).",
			   wpa_cipher_txt(group_cipher), keylen, maxkeylen);*/
	}

	return ret;
}


static int wpa_supplicant_install_gtk(struct wpa_sm *sm,
				      const struct wpa_gtk_data *gd,
				      const u8 *key_rsc)
{
	const u8 *_gtk = gd->gtk;
	u8 gtk_buf[32];
	struct key_params params;
	
	wpa_hexdump(MSG_DEBUG, "WPA: Group Key", gd->gtk, gd->gtk_len);
	wpa_printf(MSG_DEBUG, "WPA: Installing GTK to the driver "
		   "(keyidx=%d tx=%d len=%d).", gd->keyidx, gd->tx,
		   gd->gtk_len);
	wpa_hexdump(MSG_DEBUG, "WPA: RSC", key_rsc, gd->key_rsc_len);
	if (sm->group_cipher == WPA_CIPHER_TKIP) {
		/* Swap Tx/Rx keys for Michael MIC */
		os_memcpy(gtk_buf, gd->gtk, 16);
		os_memcpy(gtk_buf + 16, gd->gtk + 24, 8);
		os_memcpy(gtk_buf + 24, gd->gtk + 16, 8);
		_gtk = gtk_buf;
	}

	
	switch (gd->alg) {
	case WPA_ALG_NONE:
		params.cipher = 0;
		break;
	case WPA_ALG_WEP:
		if (gd->gtk_len == 5)
			params.cipher = WLAN_CIPHER_SUITE_WEP40;
		else if (gd->gtk_len == 13)
			params.cipher = WLAN_CIPHER_SUITE_WEP104;
		else
			return -EINVAL;
		break;
	case WPA_ALG_TKIP:
		params.cipher = WLAN_CIPHER_SUITE_TKIP;
		break;
	case WPA_ALG_CCMP:
		params.cipher = WLAN_CIPHER_SUITE_CCMP;
		break;
	case WPA_ALG_IGTK:
		params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
		break;
	default:
		return -EOPNOTSUPP;
	}
	
	if (sm->pairwise_cipher == WPA_CIPHER_NONE) {

		params.key = (u8 *)_gtk;
		params.key_len = gd->gtk_len;
		params.seq = (u8 *)key_rsc;
		params.seq_len = gd->key_rsc_len;
 
		//lbs_cfg_add_key(&priv, priv.dev,
		//	    gd->keyidx, (bool)0,
		//	   &params);	
		p_err_miss;
		
/*		if (wpa_sm_set_key(sm, gd->alg,
				   (u8 *) "\xff\xff\xff\xff\xff\xff",
				   gd->keyidx, 1, key_rsc, gd->key_rsc_len,
				   _gtk, gd->gtk_len) < 0) {
			wpa_printf(MSG_WARNING, "WPA: Failed to set "
				   "GTK to the driver (Group only).");
			return -1;
		}*/
		
	} 
	else
	{

		params.key = (u8 *)_gtk;
		params.key_len = gd->gtk_len;
		params.seq = (u8 *)key_rsc;
		params.seq_len = gd->key_rsc_len;

		//s_cfg_add_key(&priv, priv.dev,
		//   gd->keyidx, (bool)gd->tx,
		//  &params);	
		p_err_miss;
/*	if (wpa_sm_set_key(sm, gd->alg,
				  (u8 *) "\xff\xff\xff\xff\xff\xff",
				  gd->keyidx, gd->tx, key_rsc, gd->key_rsc_len,
				  _gtk, gd->gtk_len) < 0) {
		wpa_printf(MSG_WARNING, "WPA: Failed to set GTK to "
			   "the driver (alg=%d keylen=%d keyidx=%d)",
			   gd->alg, gd->gtk_len, gd->keyidx);
		return -1;*/
	}

	return 0;
}

static int wpa_supplicant_gtk_tx_bit_workaround(const struct wpa_sm *sm,
						int tx)
{
	if (tx && sm->pairwise_cipher != WPA_CIPHER_NONE) {
		/* Ignore Tx bit for GTK if a pairwise key is used. One AP
		 * seemed to set this bit (incorrectly, since Tx is only when
		 * doing Group Key only APs) and without this workaround, the
		 * data connection does not work because wpa_supplicant
		 * configured non-zero keyidx to be used for unicast. */
		wpa_printf(MSG_INFO, "WPA: Tx bit set for GTK, but pairwise "
			   "keys are used - ignore Tx bit");
		return 0;
	}
	return tx;
}

static void wpa_supplicant_key_neg_complete(struct wpa_sm *sm,
					    const u8 *addr, int secure)
{
	wpa_printf(MSG_INFO,
		"WPA: Key negotiation completed with "
		MACSTR " [PTK=%s GTK=%s]", MAC2STR(addr),
		/*wpa_cipher_txt(sm->pairwise_cipher)*/"?",
		/*wpa_cipher_txt(sm->group_cipher)*/"?");

  	sys_untimeout((sys_timeout_handler)wpa_supplicant_timeout, NULL);
	wpa_sm_set_state(sm, WPA_COMPLETED);
	//执行到这里可以确认连接上了
	//wake_up_connect_wait();
	p_err_miss;
	
	if (secure) {
		p_err("wpa_supplicant_key_neg_complete secure\n");

	}

#ifdef CONFIG_IEEE80211R
	if (wpa_key_mgmt_ft(sm->key_mgmt)) {
		/* Prepare for the next transition */
		wpa_ft_prepare_auth_request(sm, NULL);
	}
#endif /* CONFIG_IEEE80211R */
}


static int wpa_supplicant_pairwise_gtk(struct wpa_sm *sm,
				       const struct wpa_eapol_key *key,
				       const u8 *gtk, size_t gtk_len,
				       int key_info)
{
	struct wpa_gtk_data gd;

	/*
	 * IEEE Std 802.11i-2004 - 8.5.2 EAPOL-Key frames - Figure 43x
	 * GTK KDE format:
	 * KeyID[bits 0-1], Tx [bit 2], Reserved [bits 3-7]
	 * Reserved [bits 0-7]
	 * GTK
	 */

	os_memset(&gd, 0, sizeof(gd));
	wpa_hexdump(MSG_DEBUG, "RSN: received GTK in pairwise handshake",
			gtk, gtk_len);

	if (gtk_len < 2 || gtk_len - 2 > sizeof(gd.gtk))
		return -1;

	gd.keyidx = gtk[0] & 0x3;
	gd.tx = wpa_supplicant_gtk_tx_bit_workaround(sm,
						     !!(gtk[0] & BIT(2)));
	gtk += 2;
	gtk_len -= 2;

	os_memcpy(gd.gtk, gtk, gtk_len);
	gd.gtk_len = gtk_len;

	if (wpa_supplicant_check_group_cipher(sm->group_cipher,
					      gtk_len, gtk_len,
					      &gd.key_rsc_len, &gd.alg) ||
	    wpa_supplicant_install_gtk(sm, &gd, key->key_rsc)) {
		wpa_printf(MSG_DEBUG, "RSN: Failed to install GTK");
		return -1;
	}

	wpa_supplicant_key_neg_complete(sm, sm->bssid,
					key_info & WPA_KEY_INFO_SECURE);
	return 0;
}


static int ieee80211w_set_keys(struct wpa_sm *sm,
			       struct wpa_eapol_ie_parse *ie)
{
#ifdef CONFIG_IEEE80211W
	if (sm->mgmt_group_cipher != WPA_CIPHER_AES_128_CMAC)
		return 0;

	if (ie->igtk) {
		const struct wpa_igtk_kde *igtk;
		u16 keyidx;
		if (ie->igtk_len != sizeof(*igtk))
			return -1;
		igtk = (const struct wpa_igtk_kde *) ie->igtk;
		keyidx = WPA_GET_LE16(igtk->keyid);
		wpa_printf(MSG_DEBUG, "WPA: IGTK keyid %d "
			   "pn %02x%02x%02x%02x%02x%02x",
			   keyidx, MAC2STR(igtk->pn));
		wpa_hexdump_key(MSG_DEBUG, "WPA: IGTK",
				igtk->igtk, WPA_IGTK_LEN);
		if (keyidx > 4095) {
			wpa_printf(MSG_WARNING, "WPA: Invalid IGTK KeyID %d",
				   keyidx);
			return -1;
		}
		if (wpa_sm_set_key(sm, WPA_ALG_IGTK,
				   (u8 *) "\xff\xff\xff\xff\xff\xff",
				   keyidx, 0, igtk->pn, sizeof(igtk->pn),
				   igtk->igtk, WPA_IGTK_LEN) < 0) {
			wpa_printf(MSG_WARNING, "WPA: Failed to configure IGTK"
				   " to the driver");
			return -1;
		}
	}

	return 0;
#else /* CONFIG_IEEE80211W */
	return 0;
#endif /* CONFIG_IEEE80211W */
}

static int wpa_supplicant_process_1_of_2_rsn(struct wpa_sm *sm,
					     const u8 *keydata,
					     size_t keydatalen,
					     u16 key_info,
					     struct wpa_gtk_data *gd)
{
	int maxkeylen;
	struct wpa_eapol_ie_parse ie;

	wpa_hexdump(MSG_DEBUG, "RSN: msg 1/2 key data", keydata, keydatalen);
	wpa_supplicant_parse_ies(keydata, keydatalen, &ie);
	if (ie.gtk && !(key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
		wpa_printf(MSG_WARNING, "WPA: GTK IE in unencrypted key data");
		return -1;
	}
	if (ie.gtk == NULL) {
		wpa_printf(MSG_INFO, "WPA: No GTK IE in Group Key msg 1/2");
		return -1;
	}
	maxkeylen = gd->gtk_len = ie.gtk_len - 2;

	if (wpa_supplicant_check_group_cipher(sm->group_cipher,
					      gd->gtk_len, maxkeylen,
					      &gd->key_rsc_len, &gd->alg))
		return -1;

	wpa_hexdump(MSG_DEBUG, "RSN: received GTK in group key handshake",
		    ie.gtk, ie.gtk_len);
	gd->keyidx = ie.gtk[0] & 0x3;
	gd->tx = wpa_supplicant_gtk_tx_bit_workaround(sm,
						      !!(ie.gtk[0] & BIT(2)));
	if (ie.gtk_len - 2 > sizeof(gd->gtk)) {
		wpa_printf(MSG_INFO, "RSN: Too long GTK in GTK IE "
			   "(len=%lu)", (unsigned long) ie.gtk_len - 2);
		return -1;
	}
	os_memcpy(gd->gtk, ie.gtk + 2, ie.gtk_len - 2);

	if (ieee80211w_set_keys(sm, &ie) < 0)
		wpa_printf(MSG_INFO, "RSN: Failed to configure IGTK");

	return 0;
}

#define S_SWAP(a,b) do { u8 t = S[a]; S[a] = S[b]; S[b] = t; } while(0)


#if 1
int rc4_skip(const u8 *key, size_t keylen, size_t skip,
	     u8 *data, size_t data_len)
{
	u32 i, j, k;
	u8 *S, *pos;
	size_t kpos;

	p_dbg("enter rc4_skip\n");
	//release ok
	S = (u8 *)os_malloc(256);
	if(S == 0)
		return -1;

	/* Setup RC4 state */
	for (i = 0; i < 256; i++)
		S[i] = i;
	j = 0;
	kpos = 0;
	for (i = 0; i < 256; i++) {
		j = (j + S[i] + key[kpos]) & 0xff;
		kpos++;
		if (kpos >= keylen)
			kpos = 0;
		S_SWAP(i, j);
	}

	/* Skip the start of the stream */
	i = j = 0;
	for (k = 0; k < skip; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
	}

	/* Apply RC4 to data */
	pos = data;
	for (k = 0; k < data_len; k++) {
		i = (i + 1) & 0xff;
		j = (j + S[i]) & 0xff;
		S_SWAP(i, j);
		*pos++ ^= S[(S[i] + S[j]) & 0xff];
	}
	os_free(S);
	return 0;
}
#endif

void * aes_decrypt_init(const u8 *key, size_t len)
{
	AES_KEY *ak;
	//release ok
	ak = (AES_KEY *)os_malloc(sizeof(*ak));
	if (ak == NULL)
		return NULL;
	if (AES_set_decrypt_key(key, 8 * len, ak) < 0) {
		os_free(ak);
		return NULL;
	}
	return ak;
}

void aes_decrypt(void *ctx, const u8 *crypt, u8 *plain)
{
	AES_decrypt(crypt, plain, ctx);
}

void aes_decrypt_deinit(void *ctx)
{
	if(ctx)
	os_free(ctx);
}

int aes_unwrap(const u8 *kek, int n, const u8 *cipher, u8 *plain)
{
	u8 a[8], *r, b[16];
	int i, j;
	void *ctx;

	/* 1) Initialize variables. */
	os_memcpy(a, cipher, 8);
	r = plain;
	os_memcpy(r, cipher + 8, 8 * n);

	ctx = aes_decrypt_init(kek, 16);
	if (ctx == NULL)
		return -1;

	/* 2) Compute intermediate values.
	 * For j = 5 to 0
	 *     For i = n to 1
	 *         B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
	 *         A = MSB(64, B)
	 *         R[i] = LSB(64, B)
	 */
	for (j = 5; j >= 0; j--) {
		r = plain + (n - 1) * 8;
		for (i = n; i >= 1; i--) {
			os_memcpy(b, a, 8);
			b[7] ^= n * j + i;

			os_memcpy(b + 8, r, 8);
			aes_decrypt(ctx, b, b);
			os_memcpy(a, b, 8);
			os_memcpy(r, b + 8, 8);
			r -= 8;
		}
	}
	aes_decrypt_deinit(ctx);

	/* 3) Output results.
	 *
	 * These are already in @plain due to the location of temporary
	 * variables. Just verify that the IV matches with the expected value.
	 */
	for (i = 0; i < 8; i++) {
		if (a[i] != 0xa6)
			return -1;
	}

	return 0;
}

const u8_t test_ek[32]  = {
0x15 ,0x07 ,0x3f ,0x31 ,0x42 ,0x71 ,0xf3 ,0xeb ,0x35 ,0x55 ,0xed ,0x96 ,0xe5 ,0x1e ,0x28 ,0xeb ,0x7a ,0x4e ,0x87 ,0x46 ,0xbb ,0x64 ,0x01 ,0xa4 ,0xa8 ,0x26 ,0x0e ,0xa8 ,0x87 ,0x02 ,0xf0 ,0x31
//0x15 ,0x07 ,0x3f ,0x31 ,0x42 ,0x71 ,0xf3 ,0xeb ,0x35 ,0x55 ,0xed ,0x96 ,0xe5 ,0x1e ,0x29 ,0x13 ,0x76 ,0x3e ,0x63 ,0xf2 ,0x4e ,0x03 ,0x63 ,0x3b ,0xbd ,0xd1 ,0x2b ,0xc3 ,0x32 ,0xba ,0xf3 ,0x31
};
const u8_t test_gtk[32] = {
0x03 ,0x17 ,0x7f ,0xd1 ,0x74 ,0x9a ,0xb1 ,0x3f ,0xf2 ,0x72 ,0x2b ,0xdd ,0x6f ,0xc0 ,0xdf ,0xba ,0x5f ,0xee ,0xb4 ,0xf2 ,0x67 ,0xf3 ,0xf4 ,0xfe ,0x47 ,0x68 ,0x52 ,0xb8 ,0xc9 ,0x4f ,0x33 ,0x3e
//0xd2 ,0xcc ,0x9b ,0xe3 ,0x9a ,0xcd ,0x6f ,0xc4 ,0x57 ,0x4f ,0x63 ,0x0e ,0x36 ,0x5a ,0xbe ,0xad ,0x2b ,0x73 ,0x40 ,0xd7 ,0x94 ,0xd2 ,0xb4 ,0xc9 ,0xdd ,0xa8 ,0x36 ,0x8a ,0xb5 ,0x9e ,0xfb ,0x50
};

const u8_t test_kek[32] = {
0x43 ,0x42 ,0x8c ,0x89 ,0x3a ,0x78 ,0x1c ,0xfe ,0x72 ,0xef ,0x62 ,0xeb ,0x1a ,0xbf ,0xd7 ,0x1b
};

const u8_t test_cipher[32] = {
0x4c ,0xfb ,0xf0 ,0x93 ,0x1d ,0xb5 ,0xb2 ,0x6c
};

static int wpa_supplicant_process_1_of_2_wpa(struct wpa_sm *sm,
					     const struct wpa_eapol_key *key,
					     size_t keydatalen, int key_info,
					     size_t extra_len, u16 ver,
					     struct wpa_gtk_data *gd)
{
	size_t maxkeylen;
	u8 ek[32];

	gd->gtk_len = WPA_GET_BE16(key->key_length);
	maxkeylen = keydatalen;
	if (keydatalen > extra_len) {
		wpa_printf(MSG_INFO, "WPA: Truncated EAPOL-Key packet:"
			   " key_data_length=%lu > extra_len=%lu",
			   (unsigned long) keydatalen,
			   (unsigned long) extra_len);
		return -1;
	}
	if (ver == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES) {
		if (maxkeylen < 8) {
			wpa_printf(MSG_INFO, "WPA: Too short maxkeylen (%lu)",
				   (unsigned long) maxkeylen);
			return -1;
		}
		maxkeylen -= 8;
	}

	if (wpa_supplicant_check_group_cipher(sm->group_cipher,
					      gd->gtk_len, maxkeylen,
					      &gd->key_rsc_len, &gd->alg))
		return -1;

	gd->keyidx = (key_info & WPA_KEY_INFO_KEY_INDEX_MASK) >>
		WPA_KEY_INFO_KEY_INDEX_SHIFT;
	if (ver == WPA_KEY_INFO_TYPE_HMAC_MD5_RC4) {
		os_memcpy(ek, key->key_iv, 16);
		os_memcpy(ek + 16, sm->ptk.kek, 16);
		if (keydatalen > sizeof(gd->gtk)) {
			wpa_printf(MSG_WARNING, "WPA: RC4 key data "
				   "too long (%lu)",
				   (unsigned long) keydatalen);
			return -1;
		}
		os_memcpy(gd->gtk, key + 1, keydatalen);
//		os_memcpy(ek, test_ek, keydatalen);
//		os_memcpy(gd->gtk, test_gtk, keydatalen);
		wpa_hexdump(MSG_DEBUG, "ek",
		    (u8 *)ek, 32);
		wpa_hexdump(MSG_DEBUG, "gtk1",
		    (u8 *)gd->gtk, keydatalen);
		if (rc4_skip(ek, /*32*/16, 256, gd->gtk, keydatalen)) {
			wpa_printf(MSG_ERROR, "WPA: RC4 failed");
			return -1;
		}

		wpa_hexdump(MSG_DEBUG, "gtk2",
		    (u8 *)gd->gtk, keydatalen);
		
	} else if (ver == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES) {
		if (keydatalen % 8) {
			wpa_printf(MSG_WARNING, "WPA: Unsupported AES-WRAP "
				   "len %lu", (unsigned long) keydatalen);
			return -1;
		}
		if (maxkeylen > sizeof(gd->gtk)) {
			wpa_printf(MSG_WARNING, "WPA: AES-WRAP key data "
				   "too long (keydatalen=%lu maxkeylen=%lu)",
				   (unsigned long) keydatalen,
				   (unsigned long) maxkeylen);
			return -1;
		}
		wpa_hexdump(MSG_DEBUG,"kek",(u8 *)(sm->ptk.kek), 16);
		wpa_hexdump(MSG_DEBUG, "cipher",
		   (u8 *)(key + 1), 8);
		wpa_hexdump(MSG_DEBUG, "gtk2",
		    (u8 *)gd->gtk, 32);
		if (aes_unwrap(sm->ptk.kek, maxkeylen / 8,
			       (const u8 *) (key + 1), gd->gtk)) {
			wpa_printf(MSG_WARNING, "WPA: AES unwrap "
				   "failed - could not decrypt GTK");
			return -1;
		}
		wpa_hexdump(MSG_DEBUG, "gtk2",
		    (u8 *)gd->gtk, 32);
		
	} else {
		wpa_printf(MSG_WARNING, "WPA: Unsupported key_info type %d",
			   ver);
		return -1;
	}
	gd->tx = wpa_supplicant_gtk_tx_bit_workaround(
		sm, !!(key_info & WPA_KEY_INFO_TXRX));
	return 0;
}


static int wpa_supplicant_send_2_of_2(struct wpa_sm *sm,
				      const struct wpa_eapol_key *key,
				      int ver, u16 key_info)
{
	size_t rlen;
	struct wpa_eapol_key *reply;
	u8 *rbuf;
//安全释放
	rbuf = wpa_sm_alloc_eapol(sm, IEEE802_1X_TYPE_EAPOL_KEY, NULL,
				  sizeof(*reply), (size_t *)&rlen, (void *) &reply);
	if (rbuf == NULL)
		return -1;

	reply->type = sm->proto == WPA_PROTO_RSN ?
		EAPOL_KEY_TYPE_RSN : EAPOL_KEY_TYPE_WPA;
	key_info &= WPA_KEY_INFO_KEY_INDEX_MASK;
	key_info |= ver | WPA_KEY_INFO_MIC | WPA_KEY_INFO_SECURE;
	WPA_PUT_BE16(reply->key_info, key_info);
	if (sm->proto == WPA_PROTO_RSN)
		WPA_PUT_BE16(reply->key_length, 0);
	else
		os_memcpy(reply->key_length, key->key_length, 2);
	os_memcpy(reply->replay_counter, key->replay_counter,
		  WPA_REPLAY_COUNTER_LEN);

	WPA_PUT_BE16(reply->key_data_length, 0);

	wpa_printf(MSG_DEBUG, "WPA: Sending EAPOL-Key 2/2");
	wpa_eapol_key_send(sm, sm->ptk.kck, ver, sm->bssid, ETH_P_EAPOL,
			   rbuf, rlen, reply->key_mic);

	return 0;
}


static void wpa_supplicant_process_1_of_2(struct wpa_sm *sm,
					  const unsigned char *src_addr,
					  const struct wpa_eapol_key *key,
					  int extra_len, u16 ver)
{
	u16 key_info, keydatalen;
	int rekey, ret;
	struct wpa_gtk_data gd;

	os_memset(&gd, 0, sizeof(gd));

	rekey = wpa_sm_get_state(sm) == WPA_COMPLETED;
	
	wpa_printf(MSG_DEBUG, "WPA: RX message 1 of Group Key Handshake from "
		   MACSTR " (ver=%d)", MAC2STR(src_addr), ver);

	key_info = WPA_GET_BE16(key->key_info);
	keydatalen = WPA_GET_BE16(key->key_data_length);

	if (sm->proto == WPA_PROTO_RSN) {
		ret = wpa_supplicant_process_1_of_2_rsn(sm,
							(const u8 *) (key + 1),
							keydatalen, key_info,
							&gd);
	} else {
		ret = wpa_supplicant_process_1_of_2_wpa(sm, key, keydatalen,
							key_info, extra_len,
							ver, &gd);
	}

	wpa_sm_set_state(sm, WPA_GROUP_HANDSHAKE);

	if (ret)
		goto failed;

	if (wpa_supplicant_install_gtk(sm, &gd, key->key_rsc) ||
	    wpa_supplicant_send_2_of_2(sm, key, ver, key_info))
		goto failed;

	if (rekey) {
		wpa_printf(MSG_INFO, "WPA: Group rekeying "
			"completed with " MACSTR " [GTK=%s]",
			MAC2STR(sm->bssid), /*wpa_cipher_txt(sm->group_cipher)*/"^_^");
//		wpa_sm_cancel_auth_timeout(sm);
		sys_untimeout((sys_timeout_handler)wpa_supplicant_timeout, NULL);
		wpa_sm_set_state(sm, WPA_COMPLETED);
	} else {
		wpa_supplicant_key_neg_complete(sm, sm->bssid,
						key_info &
						WPA_KEY_INFO_SECURE);
	}
	return;

failed:
//	wpa_sm_deauthenticate(sm, WLAN_REASON_UNSPECIFIED);
	return;
}

/* Decrypt RSN EAPOL-Key key data (RC4 or AES-WRAP) */
static int wpa_supplicant_decrypt_key_data(struct wpa_sm *sm,
					   struct wpa_eapol_key *key, u16 ver)
{
	u16 keydatalen = WPA_GET_BE16(key->key_data_length);

	wpa_hexdump(MSG_DEBUG, "RSN: encrypted key data",
		    (u8 *) (key + 1), keydatalen);
	if (!sm->ptk_set) {
		wpa_printf(MSG_WARNING, "WPA: PTK not available, "
			   "cannot decrypt EAPOL-Key key data.");
		return -1;
	}

	/* Decrypt key data here so that this operation does not need
	 * to be implemented separately for each message type. */
	if (ver == WPA_KEY_INFO_TYPE_HMAC_MD5_RC4) {
		u8 ek[32];
		os_memcpy(ek, key->key_iv, 16);
		os_memcpy(ek + 16, sm->ptk.kek, 16);
		if (rc4_skip(ek, /*32*/16, 256, (u8 *) (key + 1), keydatalen)) {
			wpa_printf(MSG_ERROR, "WPA: RC4 failed");
			return -1;
		}
	} else if (ver == WPA_KEY_INFO_TYPE_HMAC_SHA1_AES ||
		   ver == WPA_KEY_INFO_TYPE_AES_128_CMAC) {
		u8 *buf;
		if (keydatalen % 8) {
			wpa_printf(MSG_WARNING, "WPA: Unsupported "
				   "AES-WRAP len %d", keydatalen);
			return -1;
		}
		keydatalen -= 8; /* AES-WRAP adds 8 bytes */
		//release ok
		buf = (u8*)os_malloc(keydatalen);
		if (buf == NULL) {
			wpa_printf(MSG_WARNING, "WPA: No memory for "
				   "AES-UNWRAP buffer");
			return -1;
		}
		if (aes_unwrap(sm->ptk.kek, keydatalen / 8,
			       (u8 *) (key + 1), buf)) {
			os_free(buf);
			wpa_printf(MSG_WARNING, "WPA: AES unwrap failed - "
				   "could not decrypt EAPOL-Key key data");
			return -1;
		}
		os_memcpy(key + 1, buf, keydatalen);
		os_free(buf);
		WPA_PUT_BE16(key->key_data_length, keydatalen);
	} else {
		wpa_printf(MSG_WARNING, "WPA: Unsupported key_info type %d",
			   ver);
		return -1;
	}
	wpa_hexdump(MSG_DEBUG, "WPA: decrypted EAPOL-Key key data",
			(u8 *) (key + 1), keydatalen);
	return 0;
}


#ifdef TEST_EAPOL	
uint8_t rcv_key[113 - 14] = {
/*0x00,0x22,0xfa,0xa6,0x72,0x2e,0x6c,0xe8,0x73,0x2d,0xb2,0x9e,0x88,0x8e,*/0x02,0x03,
0x00,0x5f,0xfe,0x00,0x89,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xbe,
0xb2,0x16,0x49,0x62,0x58,0x13,0xdb,0x74,0xfe,0x52,0x14,0x81,0xa8,0xd2,0x87,0x15,
0x07,0x3f,0x31,0x42,0x71,0xf3,0xeb,0x35,0x55,0xed,0x96,0xe5,0x1e,0x27,0xc0,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00
};
#endif

int wpa_sm_rx_eapol(struct wpa_sm *sm, const u8 *src_addr,
		    const u8 *buf, size_t len)
{
	size_t plen, data_len, extra_len;
	struct ieee802_1x_hdr *hdr;
	struct wpa_eapol_key *key;
	u16 key_info, ver;
	u8 *tmp;
	int ret = -1;
	struct wpa_peerkey *peerkey = NULL;

#ifdef TEST_EAPOL
	buf = rcv_key;
	len = 113 - 14;
#endif

#ifdef CONFIG_IEEE80211R
	sm->ft_completed = 0;
#endif /* CONFIG_IEEE80211R */

	if (len < sizeof(*hdr) + sizeof(*key)) {
		wpa_printf(MSG_DEBUG, "WPA: EAPOL frame too short to be a WPA "
			   "EAPOL-Key (len %lu, expecting at least %lu)",
			   (unsigned long) len,
			   (unsigned long) sizeof(*hdr) + sizeof(*key));
		return 0;
	}
//release ok
	tmp = (u8*)os_malloc(len);
	if (tmp == NULL)
		return -1;
	os_memcpy(tmp, buf, len);

	hdr = (struct ieee802_1x_hdr *) tmp;
	key = (struct wpa_eapol_key *) (hdr + 1);
	plen = be_to_host16(hdr->length);
	data_len = plen + sizeof(*hdr);
	wpa_printf(MSG_DEBUG, "IEEE 802.1X RX: version=%d type=%d length=%lu",
		   hdr->version, hdr->type, (unsigned long) plen);

	if (hdr->version < EAPOL_VERSION) {
		/* TODO: backwards compatibility */
	}
	if (hdr->type != IEEE802_1X_TYPE_EAPOL_KEY) {
		wpa_printf(MSG_DEBUG, "WPA: EAPOL frame (type %u) discarded, "
			"not a Key frame", hdr->type);
		ret = 0;
		goto out;
	}
	if (plen > len - sizeof(*hdr) || plen < sizeof(*key)) {
		wpa_printf(MSG_DEBUG, "WPA: EAPOL frame payload size %lu "
			   "invalid (frame size %lu)",
			   (unsigned long) plen, (unsigned long) len);
		ret = 0;
		goto out;
	}

	if (key->type != EAPOL_KEY_TYPE_WPA && key->type != EAPOL_KEY_TYPE_RSN)
	{
		wpa_printf(MSG_DEBUG, "WPA: EAPOL-Key type (%d) unknown, "
			   "discarded", key->type);
		ret = 0;
		goto out;
	}
	wpa_eapol_key_dump(key);

//	eapol_sm_notify_lower_layer_success(sm->eapol, 0);
	wpa_hexdump(MSG_MSGDUMP, "WPA: RX EAPOL-Key", tmp, len);
	if (data_len < len) {
		wpa_printf(MSG_DEBUG, "WPA: ignoring %lu bytes after the IEEE "
			   "802.1X data", (unsigned long) len - data_len);
	}
	key_info = WPA_GET_BE16(key->key_info);
	ver = key_info & WPA_KEY_INFO_TYPE_MASK;
	if (ver != WPA_KEY_INFO_TYPE_HMAC_MD5_RC4 &&
#if defined(CONFIG_IEEE80211R) || defined(CONFIG_IEEE80211W)
	    ver != WPA_KEY_INFO_TYPE_AES_128_CMAC &&
#endif /* CONFIG_IEEE80211R || CONFIG_IEEE80211W */
	    ver != WPA_KEY_INFO_TYPE_HMAC_SHA1_AES) {
		wpa_printf(MSG_INFO, "WPA: Unsupported EAPOL-Key descriptor "
			   "version %d.", ver);
		goto out;
	}

#ifdef CONFIG_IEEE80211R
	if (wpa_key_mgmt_ft(sm->key_mgmt)) {
		/* IEEE 802.11r uses a new key_info type (AES-128-CMAC). */
		if (ver != WPA_KEY_INFO_TYPE_AES_128_CMAC) {
			wpa_printf(MSG_INFO, "FT: AP did not use "
				   "AES-128-CMAC.");
			goto out;
		}
	} else
#endif /* CONFIG_IEEE80211R */
#ifdef CONFIG_IEEE80211W
	if (wpa_key_mgmt_sha256(sm->key_mgmt)) {
		if (ver != WPA_KEY_INFO_TYPE_AES_128_CMAC) {
			wpa_printf(MSG_INFO, "WPA: AP did not use the "
				   "negotiated AES-128-CMAC.");
			goto out;
		}
	} else
#endif /* CONFIG_IEEE80211W */
	if (sm->pairwise_cipher == WPA_CIPHER_CCMP &&
	    ver != WPA_KEY_INFO_TYPE_HMAC_SHA1_AES) {
		wpa_printf(MSG_INFO, "WPA: CCMP is used, but EAPOL-Key "
			   "descriptor version (%d) is not 2.", ver);
		if (sm->group_cipher != WPA_CIPHER_CCMP &&
		    !(key_info & WPA_KEY_INFO_KEY_TYPE)) {
			/* Earlier versions of IEEE 802.11i did not explicitly
			 * require version 2 descriptor for all EAPOL-Key
			 * packets, so allow group keys to use version 1 if
			 * CCMP is not used for them. */
			wpa_printf(MSG_INFO, "WPA: Backwards compatibility: "
				   "allow invalid version for non-CCMP group "
				   "keys");
		} else
			goto out;
	}

#ifdef CONFIG_PEERKEY
	for (peerkey = sm->peerkey; peerkey; peerkey = peerkey->next) {
		if (os_memcmp(peerkey->addr, src_addr, ETH_ALEN) == 0)
			break;
	}

	if (!(key_info & WPA_KEY_INFO_SMK_MESSAGE) && peerkey) {
		if (!peerkey->initiator && peerkey->replay_counter_set &&
		    os_memcmp(key->replay_counter, peerkey->replay_counter,
			      WPA_REPLAY_COUNTER_LEN) <= 0) {
			wpa_printf(MSG_WARNING, "RSN: EAPOL-Key Replay "
				   "Counter did not increase (STK) - dropping "
				   "packet");
			goto out;
		} else if (peerkey->initiator) {
			u8 _tmp[WPA_REPLAY_COUNTER_LEN];
			os_memcpy(_tmp, key->replay_counter,
				  WPA_REPLAY_COUNTER_LEN);
			inc_byte_array(_tmp, WPA_REPLAY_COUNTER_LEN);
			if (os_memcmp(_tmp, peerkey->replay_counter,
				      WPA_REPLAY_COUNTER_LEN) != 0) {
				wpa_printf(MSG_DEBUG, "RSN: EAPOL-Key Replay "
					   "Counter did not match (STK) - "
					   "dropping packet");
				goto out;
			}
		}
	}

	if (peerkey && peerkey->initiator && (key_info & WPA_KEY_INFO_ACK)) {
		wpa_printf(MSG_INFO, "RSN: Ack bit in key_info from STK peer");
		goto out;
	}
#endif /* CONFIG_PEERKEY */

	if (!peerkey && sm->rx_replay_counter_set &&
	    os_memcmp(key->replay_counter, sm->rx_replay_counter,
		      WPA_REPLAY_COUNTER_LEN) <= 0) {
		wpa_printf(MSG_WARNING, "WPA: EAPOL-Key Replay Counter did not"
			   " increase - dropping packet");
		goto out;
	}

	if (!(key_info & (WPA_KEY_INFO_ACK | WPA_KEY_INFO_SMK_MESSAGE))
#ifdef CONFIG_PEERKEY
	    && (peerkey == NULL || !peerkey->initiator)
#endif /* CONFIG_PEERKEY */
		) {
		wpa_printf(MSG_INFO, "WPA: No Ack bit in key_info");
		goto out;
	}

	if (key_info & WPA_KEY_INFO_REQUEST) {
		wpa_printf(MSG_INFO, "WPA: EAPOL-Key with Request bit - "
			   "dropped");
		goto out;
	}

	if ((key_info & WPA_KEY_INFO_MIC) && !peerkey &&
	    wpa_supplicant_verify_eapol_key_mic(sm, key, ver, tmp, data_len))
		goto out;

#ifdef CONFIG_PEERKEY
	if ((key_info & WPA_KEY_INFO_MIC) && peerkey &&
	    peerkey_verify_eapol_key_mic(sm, peerkey, key, ver, tmp, data_len))
		goto out;
#endif /* CONFIG_PEERKEY */

	extra_len = data_len - sizeof(*hdr) - sizeof(*key);

	if (WPA_GET_BE16(key->key_data_length) > extra_len) {
		wpa_printf(MSG_INFO, "WPA: Invalid EAPOL-Key "
			"frame - key_data overflow (%d > %lu)",
			WPA_GET_BE16(key->key_data_length),
			(unsigned long) extra_len);
		goto out;
	}
	extra_len = WPA_GET_BE16(key->key_data_length);


	if (sm->proto == WPA_PROTO_RSN &&
	    (key_info & WPA_KEY_INFO_ENCR_KEY_DATA)) {
		if (wpa_supplicant_decrypt_key_data(sm, key, ver))
			goto out;
		extra_len = WPA_GET_BE16(key->key_data_length);
	}

	if (key_info & WPA_KEY_INFO_KEY_TYPE) {
		if (key_info & WPA_KEY_INFO_KEY_INDEX_MASK) {
			wpa_printf(MSG_WARNING, "WPA: Ignored EAPOL-Key "
				   "(Pairwise) with non-zero key index");
			goto out;
		}
		if (peerkey) {
			/* PeerKey 4-Way Handshake */
			p_err("no peerkey_rx_eapol_4way\n");
//			peerkey_rx_eapol_4way(sm, peerkey, key, key_info, ver);
		} else if (key_info & WPA_KEY_INFO_MIC) {
			/* 3/4 4-Way Handshake */
			wpa_supplicant_process_3_of_4(sm, key, ver);
		} else {
			/* 1/4 4-Way Handshake */
			wpa_supplicant_process_1_of_4(sm, src_addr, key,
						      ver);
		}
	} else if (key_info & WPA_KEY_INFO_SMK_MESSAGE) {
		/* PeerKey SMK Handshake */
//		peerkey_rx_eapol_smk(sm, src_addr, key, extra_len, key_info,
//				     ver);
		p_err("no WPA_KEY_INFO_SMK_MESSAGE\n");
	} else {
		if (key_info & WPA_KEY_INFO_MIC) {
			/* 1/2 Group Key Handshake */
			wpa_supplicant_process_1_of_2(sm, src_addr, key,
						      extra_len, ver);
		} else {
			wpa_printf(MSG_WARNING, "WPA: EAPOL-Key (Group) "
				   "without Mic bit - dropped");
		}
	}

	ret = 1;

out:
	os_free(tmp);
	return ret;
}


int eapol_sm_rx_eapol(struct eapol_sm *sm, const u8 *src, const u8 *buf,
		      size_t len)
{
	const struct ieee802_1x_hdr *hdr;
	const struct ieee802_1x_eapol_key *key;
	int data_len;
	int res = 1;
	size_t plen;

	if (sm == NULL)
		return 0;
	sm->dot1xSuppEapolFramesRx++;
	if (len < sizeof(*hdr)) {
		sm->dot1xSuppInvalidEapolFramesRx++;
		return 0;
	}
	hdr = (const struct ieee802_1x_hdr *) buf;
	sm->dot1xSuppLastEapolFrameVersion = hdr->version;
	os_memcpy(sm->dot1xSuppLastEapolFrameSource, src, ETH_ALEN);
	if (hdr->version < EAPOL_VERSION) {
		/* TODO: backwards compatibility */
	}
	plen = be_to_host16(hdr->length);
	if (plen > len - sizeof(*hdr)) {
		sm->dot1xSuppEapLengthErrorFramesRx++;
		return 0;
	}

	sm->conf.workaround = ENABLE;

	if (sm->conf.workaround &&
	    plen < len - sizeof(*hdr) &&
	    hdr->type == IEEE802_1X_TYPE_EAP_PACKET &&
	    len - sizeof(*hdr) > sizeof(struct eap_hdr)) {
		const struct eap_hdr *ehdr =
			(const struct eap_hdr *) (hdr + 1);
		u16 elen;

		elen = be_to_host16(ehdr->length);
		if (elen > plen && elen <= len - sizeof(*hdr)) {
			/*
			 * Buffalo WHR-G125 Ver.1.47 seems to send EAP-WPS
			 * packets with too short EAPOL header length field
			 * (14 octets). This is fixed in firmware Ver.1.49.
			 * As a workaround, fix the EAPOL header based on the
			 * correct length in the EAP packet.
			 */
			wpa_printf(MSG_DEBUG, "EAPOL: Workaround - fix EAPOL "
				   "payload length based on EAP header: "
				   "%d -> %d", (int) plen, elen);
			plen = elen;
		}
	}

	data_len = plen + sizeof(*hdr);

	switch (hdr->type) {
	case IEEE802_1X_TYPE_EAP_PACKET:
		if (sm->cached_pmk) {
			/* Trying to use PMKSA caching, but Authenticator did
			 * not seem to have a matching entry. Need to restart
			 * EAPOL state machines.
			 */
//			eapol_sm_abort_cached(sm);
		}
//		wpabuf_free(sm->eapReqData);
//		sm->eapReqData = wpabuf_alloc_copy(hdr + 1, plen);
		if (sm->eapReqData) {
			wpa_printf(MSG_DEBUG, "EAPOL: Received EAP-Packet "
				   "frame");
			sm->eapolEap = TRUE;
//			eapol_sm_step(sm);
		}
		break;
	case IEEE802_1X_TYPE_EAPOL_KEY:
		if (plen < sizeof(*key)) {
			wpa_printf(MSG_DEBUG, "EAPOL: Too short EAPOL-Key "
				   "frame received");
			break;
		}
		key = (const struct ieee802_1x_eapol_key *) (hdr + 1);
		if (key->type == EAPOL_KEY_TYPE_WPA ||
		    key->type == EAPOL_KEY_TYPE_RSN) {
			/* WPA Supplicant takes care of this frame. */
			wpa_printf(MSG_DEBUG, "EAPOL: Ignoring WPA EAPOL-Key "
				   "frame in EAPOL state machines");
			res = 0;
			break;
		}
		if (key->type != EAPOL_KEY_TYPE_RC4) {
			wpa_printf(MSG_DEBUG, "EAPOL: Ignored unknown "
				   "EAPOL-Key type %d", key->type);
			break;
		}
		if(sm->last_rx_key)
		{
			os_free(sm->last_rx_key);
			sm->last_rx_key = 0;
		}//unkown
		sm->last_rx_key = (u8*)os_malloc(data_len);
		if (sm->last_rx_key) {
			wpa_printf(MSG_DEBUG, "EAPOL: Received EAPOL-Key "
				   "frame");
			os_memcpy(sm->last_rx_key, buf, data_len);
			sm->last_rx_key_len = data_len;
			sm->rxKey = TRUE;
//			eapol_sm_step(sm);
		}
		break;
	default:
		wpa_printf(MSG_DEBUG, "EAPOL: Received unknown EAPOL type %d",
			   hdr->type);
		sm->dot1xSuppInvalidEapolFramesRx++;
		break;
	}

	return res;
}

void wpa_supplicant_disassociate(struct wpa_supplicant *wpa_s,
				 int reason_code)
{
	if (!is_zero_ether_addr(wpa_s->bssid)) {

		//s_cfg_disconnect(0, 0, reason_code);
		p_err_miss;
	}

	wpa_s->current_ssid = NULL;
	wpa_s->current_bss = NULL;
}


static void wpa_supplicant_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct wpa_supplicant *wpas = &wpa_s;
	const u8 *bssid = wpas->bssid;
	if (is_zero_ether_addr(bssid))
		bssid = wpas->pending_bssid;
	wpa_printf(MSG_INFO, "Authentication with " MACSTR " timed out.",
		MAC2STR(bssid));

	wpa_sm_notify_disassoc(wpas->wpa);
	wpa_supplicant_disassociate(wpas, WLAN_REASON_DEAUTH_LEAVING);
	wpas->reassociate = 1;

}


void wpa_supplicant_req_auth_timeout(struct wpa_supplicant *wpa_s,
				     int sec, int usec)
{
	wpa_printf(MSG_DEBUG, "Setting authentication timeout: %d sec "
		"%d usec", sec, usec);
	sys_untimeout((sys_timeout_handler)wpa_supplicant_timeout, NULL);
	sys_timeout(sec*1000+usec, (sys_timeout_handler)wpa_supplicant_timeout, NULL);

}


 
void wpa_supplicant_rx_eapol(void *ctx, const u8 *src_addr,
			     const u8 *buf, size_t len)
{
	struct wpa_supplicant *wpa_s = ctx;

	wpa_printf(MSG_DEBUG, "RX EAPOL from " MACSTR, MAC2STR(src_addr));
	wpa_hexdump(MSG_MSGDUMP, "RX EAPOL", (const u8*)buf, len);

	if (wpa_s->wpa_state < WPA_ASSOCIATED) {
		/*
		 * There is possible race condition between receiving the
		 * association event and the EAPOL frame since they are coming
		 * through different paths from the driver. In order to avoid
		 * issues in trying to process the EAPOL frame before receiving
		 * association information, lets queue it for processing until
		 * the association event is received.
		 */
		wpa_printf(MSG_DEBUG, "Not associated - Delay processing of "
			   "received EAPOL frame");
/*		wpabuf_free(wpa_s->pending_eapol_rx);
		wpa_s->pending_eapol_rx = wpabuf_alloc_copy(buf, len);
		if (wpa_s->pending_eapol_rx) {
			os_get_time(&wpa_s->pending_eapol_rx_time);
			os_memcpy(wpa_s->pending_eapol_rx_src, src_addr,
				  ETH_ALEN);
		}*/
		return;
	}

#ifdef CONFIG_AP
	if (wpa_s->ap_iface) {
		wpa_supplicant_ap_rx_eapol(wpa_s, src_addr, buf, len);
		return;
	}
#endif /* CONFIG_AP */

	if (wpa_s->key_mgmt == WPA_KEY_MGMT_NONE) {
		wpa_printf(MSG_DEBUG, "Ignored received EAPOL frame since no key management is configured");
		return;
	}

	if (wpa_s->eapol_received == 0 &&
	    (!(wpa_s->drv_flags & WPA_DRIVER_FLAGS_4WAY_HANDSHAKE) ||
	     !wpa_key_mgmt_wpa_psk(wpa_s->key_mgmt) ||
	     wpa_s->wpa_state != WPA_COMPLETED) &&
	    (wpa_s->current_ssid == NULL ||
	     wpa_s->current_ssid->mode != IEEE80211_MODE_IBSS)) {
		/* Timeout for completing IEEE 802.1X and WPA authentication */
		wpa_supplicant_req_auth_timeout(
			wpa_s,
			(wpa_key_mgmt_wpa_ieee8021x(wpa_s->key_mgmt) ||
			 wpa_s->key_mgmt == WPA_KEY_MGMT_IEEE8021X_NO_WPA ||
			 wpa_s->key_mgmt == WPA_KEY_MGMT_WPS) ?
			70 : 10, 0);
	}
	wpa_s->eapol_received++;

	if (wpa_s->countermeasures) {
		wpa_printf(MSG_INFO, "WPA: Countermeasures - dropped EAPOL packet");
		return;
	}

#ifdef CONFIG_IBSS_RSN
	if (wpa_s->current_ssid &&
	    wpa_s->current_ssid->mode == WPAS_MODE_IBSS) {
		ibss_rsn_rx_eapol(wpa_s->ibss_rsn, src_addr, buf, len);
		return;
	}
#endif /* CONFIG_IBSS_RSN */

	/* Source address of the incoming EAPOL frame could be compared to the
	 * current BSSID. However, it is possible that a centralized
	 * Authenticator could be using another MAC address than the BSSID of
	 * an AP, so just allow any address to be used for now. The replies are
	 * still sent to the current BSSID (if available), though. */

	os_memcpy(wpa_s->last_eapol_src, src_addr, ETH_ALEN);
	if (!wpa_key_mgmt_wpa_psk(wpa_s->key_mgmt) &&
	    eapol_sm_rx_eapol(wpa_s->eapol, src_addr, buf, len) > 0)
		return;
	
//	wpa_drv_poll(wpa_s);
	if (!(wpa_s->drv_flags & WPA_DRIVER_FLAGS_4WAY_HANDSHAKE))
		wpa_sm_rx_eapol(wpa_s->wpa, src_addr, buf, len);
	else if (wpa_key_mgmt_wpa_ieee8021x(wpa_s->key_mgmt)) {
		/*
		 * Set portValid = TRUE here since we are going to skip 4-way
		 * handshake processing which would normally set portValid. We
		 * need this to allow the EAPOL state machines to be completed
		 * without going through EAPOL-Key handshake.
		 */
		 p_err("no eapol_sm_notify_portValid\n");
//		eapol_sm_notify_portValid(wpa_s->eapol, TRUE);
	}
}


void eapol_input(struct netif *netif, struct pbuf *pb)
{
	struct eth_hdr *hdr;
	if(pb == 0)
		return;
	hdr = pb->payload;
	
	if(hdr->type != PP_HTONS(ETH_P_EAPOL))
		return;
	wpa_supplicant_rx_eapol(&wpa_s, 
		(const uint8_t*)&(hdr->src) ,
		((const uint8_t*)pb->payload) + 14, 
		pb->len - 14);
	pbuf_free(pb);
}

