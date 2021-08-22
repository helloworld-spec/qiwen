#ifndef  _WPA_H
#define  _WPA_H

#include "xrf_type.h"
#include "defs.h"
#include "netif.h"
#include "pbuf.h"

//#define __bitwise __attribute__((bitwise))
//#define __force __attribute__((force))
#define __bitwise
#define __force

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321

#define __BYTE_ORDER __LITTLE_ENDIAN

enum { MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR };

#define EAPOL_VERSION 1

enum { IEEE802_1X_TYPE_EAP_PACKET = 0,
       IEEE802_1X_TYPE_EAPOL_START = 1,
       IEEE802_1X_TYPE_EAPOL_LOGOFF = 2,
       IEEE802_1X_TYPE_EAPOL_KEY = 3,
       IEEE802_1X_TYPE_EAPOL_ENCAPSULATED_ASF_ALERT = 4
};

enum { EAPOL_KEY_TYPE_RC4 = 1, EAPOL_KEY_TYPE_RSN = 2,
       EAPOL_KEY_TYPE_WPA = 254 };

struct wpa_ie_data {
	int proto;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int capabilities;
	size_t num_pmkid;
	const u8 *pmkid;
	int mgmt_group_cipher;
};



enum wpa_sm_conf_params {
	RSNA_PMK_LIFETIME /* dot11RSNAConfigPMKLifetime */,
	RSNA_PMK_REAUTH_THRESHOLD /* dot11RSNAConfigPMKReauthThreshold */,
	RSNA_SA_TIMEOUT /* dot11RSNAConfigSATimeout */,
	WPA_PARAM_PROTO,
	WPA_PARAM_PAIRWISE,
	WPA_PARAM_GROUP,
	WPA_PARAM_KEY_MGMT,
	WPA_PARAM_MGMT_GROUP,
	WPA_PARAM_RSN_ENABLED,
	WPA_PARAM_MFP
};


#ifdef DEBUG
#define wpa_printf(a, ...)  printf(__VA_ARGS__)
#define wpa_hexdump(a, b, c, d)  dump_hex(b,(unsigned char*)c,d)
#else
#define wpa_printf(a,...)
#define wpa_hexdump(a, b, c, d)
#endif

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#ifndef bswap_16
#define bswap_16(a) ((((u16) (a) << 8) & 0xff00) | (((u16) (a) >> 8) & 0xff))
#endif

#ifndef bswap_32
#define bswap_32(a) ((((u32) (a) << 24) & 0xff000000) | \
		     (((u32) (a) << 8) & 0xff0000) | \
     		     (((u32) (a) >> 8) & 0xff00) | \
     		     (((u32) (a) >> 24) & 0xff))
#endif


#define WPA_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))
#define WPA_PUT_BE16(a, val)			\
	do {					\
		(a)[0] = ((u16) (val)) >> 8;	\
		(a)[1] = ((u16) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define WPA_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | \
			 ((u32) (a)[2]))
#define WPA_PUT_BE24(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[2] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
			 (((u32) (a)[2]) << 8) | ((u32) (a)[3]))
#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_LE32(a) ((((u32) (a)[3]) << 24) | (((u32) (a)[2]) << 16) | \
			 (((u32) (a)[1]) << 8) | ((u32) (a)[0]))
#define WPA_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_GET_BE64(a) ((((u64) (a)[0]) << 56) | (((u64) (a)[1]) << 48) | \
			 (((u64) (a)[2]) << 40) | (((u64) (a)[3]) << 32) | \
			 (((u64) (a)[4]) << 24) | (((u64) (a)[5]) << 16) | \
			 (((u64) (a)[6]) << 8) | ((u64) (a)[7]))
#define WPA_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8) (((u64) (val)) >> 56);	\
		(a)[1] = (u8) (((u64) (val)) >> 48);	\
		(a)[2] = (u8) (((u64) (val)) >> 40);	\
		(a)[3] = (u8) (((u64) (val)) >> 32);	\
		(a)[4] = (u8) (((u64) (val)) >> 24);	\
		(a)[5] = (u8) (((u64) (val)) >> 16);	\
		(a)[6] = (u8) (((u64) (val)) >> 8);	\
		(a)[7] = (u8) (((u64) (val)) & 0xff);	\
	} while (0)

#define WPA_GET_LE64(a) ((((u64) (a)[7]) << 56) | (((u64) (a)[6]) << 48) | \
			 (((u64) (a)[5]) << 40) | (((u64) (a)[4]) << 32) | \
			 (((u64) (a)[3]) << 24) | (((u64) (a)[2]) << 16) | \
			 (((u64) (a)[1]) << 8) | ((u64) (a)[0]))

typedef u16 __bitwise be16;
typedef u16 __bitwise le16;
typedef u32 __bitwise be32;
typedef u32 __bitwise le32;
typedef u64 __bitwise be64;
typedef u64 __bitwise le64;

#define le_to_host16(n) ((__force u16) (le16) (n))
#define host_to_le16(n) ((__force le16) (u16) (n))
#define be_to_host16(n) bswap_16((__force u16) (be16) (n))
#define host_to_be16(n) ((__force be16) bswap_16((n)))
#define le_to_host32(n) ((__force u32) (le32) (n))
#define host_to_le32(n) ((__force le32) (u32) (n))
#define be_to_host32(n) bswap_32((__force u32) (be32) (n))
#define host_to_be32(n) ((__force be32) bswap_32((n)))
#define le_to_host64(n) ((__force u64) (le64) (n))
#define host_to_le64(n) ((__force le64) (u64) (n))
#define be_to_host64(n) bswap_64((__force u64) (be64) (n))
#define host_to_be64(n) ((__force be64) bswap_64((n)))

//wmj+
#define STRUCT_PACKED		__attribute__((packed))



#define MAX_SSID_LEN 32

#define ETH_P_EAPOL ETH_P_PAE
/**
 * struct wpa_driver_capa - Driver capability information
 */
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA		0x00000001
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA2		0x00000002
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK	0x00000004
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK	0x00000008
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA_NONE	0x00000010
#define WPA_DRIVER_CAPA_KEY_MGMT_FT		0x00000020
#define WPA_DRIVER_CAPA_KEY_MGMT_FT_PSK		0x00000040

#define WPA_DRIVER_CAPA_ENC_WEP40	0x00000001
#define WPA_DRIVER_CAPA_ENC_WEP104	0x00000002
#define WPA_DRIVER_CAPA_ENC_TKIP	0x00000004
#define WPA_DRIVER_CAPA_ENC_CCMP	0x00000008

#define WPA_DRIVER_AUTH_OPEN		0x00000001
#define WPA_DRIVER_AUTH_SHARED		0x00000002
#define WPA_DRIVER_AUTH_LEAP		0x00000004

/* Driver generated WPA/RSN IE */
#define WPA_DRIVER_FLAGS_DRIVER_IE	0x00000001
/* Driver needs static WEP key setup after association command */
#define WPA_DRIVER_FLAGS_SET_KEYS_AFTER_ASSOC 0x00000002
#define WPA_DRIVER_FLAGS_USER_SPACE_MLME 0x00000004
/* Driver takes care of RSN 4-way handshake internally; PMK is configured with
 * struct wpa_driver_ops::set_key using alg = WPA_ALG_PMK */
#define WPA_DRIVER_FLAGS_4WAY_HANDSHAKE 0x00000008
#define WPA_DRIVER_FLAGS_WIRED		0x00000010
/* Driver provides separate commands for authentication and association (SME in
 * wpa_supplicant). */
#define WPA_DRIVER_FLAGS_SME		0x00000020
/* Driver supports AP mode */
#define WPA_DRIVER_FLAGS_AP		0x00000040
/* Driver needs static WEP key setup after association has been completed */
#define WPA_DRIVER_FLAGS_SET_KEYS_AFTER_ASSOC_DONE	0x00000080


#define IEEE80211_MODE_INFRA	0
#define IEEE80211_MODE_IBSS	1
#define IEEE80211_MODE_AP	2

#define IEEE80211_CAP_ESS	0x0001
#define IEEE80211_CAP_IBSS	0x0002
#define IEEE80211_CAP_PRIVACY	0x0010

#define WPA_SCAN_QUAL_INVALID		BIT(0)
#define WPA_SCAN_NOISE_INVALID		BIT(1)
#define WPA_SCAN_LEVEL_INVALID		BIT(2)
#define WPA_SCAN_LEVEL_DBM		BIT(3)
#define WPA_SCAN_AUTHENTICATED		BIT(4)
#define WPA_SCAN_ASSOCIATED		BIT(5)

struct wpa_driver_capa {

	unsigned int key_mgmt;


	unsigned int enc;


	unsigned int auth;


	unsigned int flags;

	int max_scan_ssids;

	/**
	 * max_remain_on_chan - Maximum remain-on-channel duration in msec
	 */
	unsigned int max_remain_on_chan;
};

#define DEFAULT_EAP_WORKAROUND ((unsigned int) -1)
#define DEFAULT_EAPOL_FLAGS (EAPOL_FLAG_REQUIRE_KEY_UNICAST | \
			     EAPOL_FLAG_REQUIRE_KEY_BROADCAST)
#define DEFAULT_PROTO (WPA_PROTO_WPA | WPA_PROTO_RSN)
#define DEFAULT_KEY_MGMT (WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_IEEE8021X)
#define DEFAULT_PAIRWISE (WPA_CIPHER_CCMP | WPA_CIPHER_TKIP)
#define DEFAULT_GROUP (WPA_CIPHER_CCMP | WPA_CIPHER_TKIP | \
		       WPA_CIPHER_WEP104 | WPA_CIPHER_WEP40)
#define DEFAULT_FRAGMENT_SIZE 1398


#define WPA_MAX_SSID_LEN 32

/* IEEE 802.11i */
#define PMKID_LEN 16
#define PMK_LEN 32
#define WPA_REPLAY_COUNTER_LEN 8
#define WPA_NONCE_LEN 32
#define WPA_KEY_RSC_LEN 8
#define WPA_GMK_LEN 32
#define WPA_GTK_MAX_LEN 32

#define WPA_SELECTOR_LEN 4
#define WPA_VERSION 1
#define RSN_SELECTOR_LEN 4
#define RSN_VERSION 1


#define RSN_SELECTOR(a, b, c, d) \
	((((u32) (a)) << 24) | (((u32) (b)) << 16) | (((u32) (c)) << 8) | \
	 (u32) (d))

#define WPA_AUTH_KEY_MGMT_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x50, 0xf2, 0)
#define WPA_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x50, 0xf2, 1)
#define WPA_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x50, 0xf2, 2)
#if 0
#define WPA_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x50, 0xf2, 3)
#endif
#define WPA_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x50, 0xf2, 4)
#define WPA_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x50, 0xf2, 5)


#define RSN_AUTH_KEY_MGMT_UNSPEC_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#ifdef CONFIG_IEEE80211R
#define RSN_AUTH_KEY_MGMT_FT_802_1X RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#define RSN_AUTH_KEY_MGMT_FT_PSK RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#endif /* CONFIG_IEEE80211R */
#define RSN_AUTH_KEY_MGMT_802_1X_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_AUTH_KEY_MGMT_PSK_SHA256 RSN_SELECTOR(0x00, 0x0f, 0xac, 6)

#define RSN_CIPHER_SUITE_NONE RSN_SELECTOR(0x00, 0x0f, 0xac, 0)
#define RSN_CIPHER_SUITE_WEP40 RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#define RSN_CIPHER_SUITE_TKIP RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#if 0
#define RSN_CIPHER_SUITE_WRAP RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#endif
#define RSN_CIPHER_SUITE_CCMP RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#define RSN_CIPHER_SUITE_WEP104 RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#ifdef CONFIG_IEEE80211W
#define RSN_CIPHER_SUITE_AES_128_CMAC RSN_SELECTOR(0x00, 0x0f, 0xac, 6)
#endif /* CONFIG_IEEE80211W */

/* EAPOL-Key Key Data Encapsulation
 * GroupKey and PeerKey require encryption, otherwise, encryption is optional.
 */
#define RSN_KEY_DATA_GROUPKEY RSN_SELECTOR(0x00, 0x0f, 0xac, 1)
#if 0
#define RSN_KEY_DATA_STAKEY RSN_SELECTOR(0x00, 0x0f, 0xac, 2)
#endif
#define RSN_KEY_DATA_MAC_ADDR RSN_SELECTOR(0x00, 0x0f, 0xac, 3)
#define RSN_KEY_DATA_PMKID RSN_SELECTOR(0x00, 0x0f, 0xac, 4)
#ifdef CONFIG_PEERKEY
#define RSN_KEY_DATA_SMK RSN_SELECTOR(0x00, 0x0f, 0xac, 5)
#define RSN_KEY_DATA_NONCE RSN_SELECTOR(0x00, 0x0f, 0xac, 6)
#define RSN_KEY_DATA_LIFETIME RSN_SELECTOR(0x00, 0x0f, 0xac, 7)
#define RSN_KEY_DATA_ERROR RSN_SELECTOR(0x00, 0x0f, 0xac, 8)
#endif /* CONFIG_PEERKEY */
#ifdef CONFIG_IEEE80211W
#define RSN_KEY_DATA_IGTK RSN_SELECTOR(0x00, 0x0f, 0xac, 9)
#endif /* CONFIG_IEEE80211W */

#define WPA_OUI_TYPE RSN_SELECTOR(0x00, 0x50, 0xf2, 1)

#define RSN_SELECTOR_PUT(a, val) WPA_PUT_BE32((u8 *) (a), (val))
#define RSN_SELECTOR_GET(a) WPA_GET_BE32((const u8 *) (a))

#define RSN_NUM_REPLAY_COUNTERS_1 0
#define RSN_NUM_REPLAY_COUNTERS_2 1
#define RSN_NUM_REPLAY_COUNTERS_4 2
#define RSN_NUM_REPLAY_COUNTERS_16 3


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

#ifdef CONFIG_IEEE80211W
#define WPA_IGTK_LEN 16
#endif /* CONFIG_IEEE80211W */


/* IEEE 802.11, 7.3.2.25.3 RSN Capabilities */
#define WPA_CAPABILITY_PREAUTH BIT(0)
#define WPA_CAPABILITY_NO_PAIRWISE BIT(1)
/* B2-B3: PTKSA Replay Counter */
/* B4-B5: GTKSA Replay Counter */
#define WPA_CAPABILITY_MFPR BIT(6)
#define WPA_CAPABILITY_MFPC BIT(7)
#define WPA_CAPABILITY_PEERKEY_ENABLED BIT(9)


/* IEEE 802.11r */
#define MOBILITY_DOMAIN_ID_LEN 2
#define FT_R0KH_ID_MAX_LEN 48
#define FT_R1KH_ID_LEN 6
#define WPA_PMK_NAME_LEN 16


/* IEEE 802.11, 8.5.2 EAPOL-Key frames */
#define WPA_KEY_INFO_TYPE_MASK ((u16) (BIT(0) | BIT(1) | BIT(2)))
#define WPA_KEY_INFO_TYPE_HMAC_MD5_RC4 BIT(0)
#define WPA_KEY_INFO_TYPE_HMAC_SHA1_AES BIT(1)
#define WPA_KEY_INFO_TYPE_AES_128_CMAC 3
#define WPA_KEY_INFO_KEY_TYPE BIT(3) /* 1 = Pairwise, 0 = Group key */
/* bit4..5 is used in WPA, but is reserved in IEEE 802.11i/RSN */
#define WPA_KEY_INFO_KEY_INDEX_MASK (BIT(4) | BIT(5))
#define WPA_KEY_INFO_KEY_INDEX_SHIFT 4
#define WPA_KEY_INFO_INSTALL BIT(6) /* pairwise */
#define WPA_KEY_INFO_TXRX BIT(6) /* group */
#define WPA_KEY_INFO_ACK BIT(7)
#define WPA_KEY_INFO_MIC BIT(8)
#define WPA_KEY_INFO_SECURE BIT(9)
#define WPA_KEY_INFO_ERROR BIT(10)
#define WPA_KEY_INFO_REQUEST BIT(11)
#define WPA_KEY_INFO_ENCR_KEY_DATA BIT(12) /* IEEE 802.11i/RSN only */
#define WPA_KEY_INFO_SMK_MESSAGE BIT(13)

#define AES_ENCRYPT	1
#define AES_DECRYPT	0

#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct aes_key_st AES_KEY;

# define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ ((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
# define PUTU32(ct, st) { (ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); (ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }


struct wpa_gtk_data {
	enum wpa_alg alg;
	int tx, key_rsc_len, keyidx;
	u8 gtk[32];
	int gtk_len;
};

struct ieee802_1x_hdr {
	u8 version;
	u8 type;
	be16 length;
	/* followed by length octets of data */
} STRUCT_PACKED;


struct wpa_ssid {
//	struct wpa_ssid *next;
//	struct wpa_ssid *pnext;
	int id;
	int priority;
	u8 *ssid;
	size_t ssid_len;
	u8 bssid[ETH_ALEN];
	int bssid_set;
	u8 psk[32];
	int psk_set;
	char *passphrase;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int proto;
	int auth_alg;
	int scan_ssid;

#define EAPOL_FLAG_REQUIRE_KEY_UNICAST BIT(0)
#define EAPOL_FLAG_REQUIRE_KEY_BROADCAST BIT(1)

	int eapol_flags;

//	struct eap_peer_config eap;
#define NUM_WEP_KEYS 4
#define MAX_WEP_KEY_LEN 16
	/**
	 * wep_key - WEP keys
	 */
	u8 wep_key[NUM_WEP_KEYS][MAX_WEP_KEY_LEN];

	/**
	 * wep_key_len - WEP key lengths
	 */
	size_t wep_key_len[NUM_WEP_KEYS];

	/**
	 * wep_tx_keyidx - Default key index for TX frames using WEP
	 */
	int wep_tx_keyidx;
	int proactive_key_caching;
	int mixed_cell;
	int leap;
	int non_leap;
	unsigned int eap_workaround;
	enum wpas_mode {
		WPAS_MODE_INFRA = 0,
		WPAS_MODE_IBSS = 1,
		WPAS_MODE_AP = 2,
	} mode;
	int disabled;
	int peerkey;
	char *id_str;
	int frequency;
	int wpa_ptk_rekey;
	int *scan_freq;
	char *bgscan;
	int *freq_list;
};

union wpa_event_data {
	/**
	 * struct assoc_info - Data for EVENT_ASSOC and EVENT_ASSOCINFO events
	 *
	 * This structure is optional for EVENT_ASSOC calls and required for
	 * EVENT_ASSOCINFO calls. By using EVENT_ASSOC with this data, the
	 * driver interface does not need to generate separate EVENT_ASSOCINFO
	 * calls.
	 */
	struct assoc_info {
		/**
		 * req_ies - (Re)Association Request IEs
		 *
		 * If the driver generates WPA/RSN IE, this event data must be
		 * returned for WPA handshake to have needed information. If
		 * wpa_supplicant-generated WPA/RSN IE is used, this
		 * information event is optional.
		 *
		 * This should start with the first IE (fixed fields before IEs
		 * are not included).
		 */
		const u8 *req_ies;

		/**
		 * req_ies_len - Length of req_ies in bytes
		 */
		size_t req_ies_len;

		/**
		 * resp_ies - (Re)Association Response IEs
		 *
		 * Optional association data from the driver. This data is not
		 * required WPA, but may be useful for some protocols and as
		 * such, should be reported if this is available to the driver
		 * interface.
		 *
		 * This should start with the first IE (fixed fields before IEs
		 * are not included).
		 */
		const u8 *resp_ies;

		/**
		 * resp_ies_len - Length of resp_ies in bytes
		 */
		size_t resp_ies_len;

		/**
		 * beacon_ies - Beacon or Probe Response IEs
		 *
		 * Optional Beacon/ProbeResp data: IEs included in Beacon or
		 * Probe Response frames from the current AP (i.e., the one
		 * that the client just associated with). This information is
		 * used to update WPA/RSN IE for the AP. If this field is not
		 * set, the results from previous scan will be used. If no
		 * data for the new AP is found, scan results will be requested
		 * again (without scan request). At this point, the driver is
		 * expected to provide WPA/RSN IE for the AP (if WPA/WPA2 is
		 * used).
		 *
		 * This should start with the first IE (fixed fields before IEs
		 * are not included).
		 */
		const u8 *beacon_ies;

		/**
		 * beacon_ies_len - Length of beacon_ies */
		size_t beacon_ies_len;

		/**
		 * freq - Frequency of the operational channel in MHz
		 */
		unsigned int freq;

		/**
		 * addr - Station address (for AP mode)
		 */
		const u8 *addr;
	} assoc_info;

	/**
	 * struct disassoc_info - Data for EVENT_DISASSOC events
	 */
	struct disassoc_info {
		/**
		 * addr - Station address (for AP mode)
		 */
		const u8 *addr;

		/**
		 * reason_code - Reason Code (host byte order) used in
		 *	Deauthentication frame
		 */
		u16 reason_code;
	} disassoc_info;

	/**
	 * struct deauth_info - Data for EVENT_DEAUTH events
	 */
	struct deauth_info {
		/**
		 * addr - Station address (for AP mode)
		 */
		const u8 *addr;

		/**
		 * reason_code - Reason Code (host byte order) used in
		 *	Deauthentication frame
		 */
		u16 reason_code;
	} deauth_info;

	/**
	 * struct michael_mic_failure - Data for EVENT_MICHAEL_MIC_FAILURE
	 */
	struct michael_mic_failure {
		int unicast;
		const u8 *src;
	} michael_mic_failure;

	/**
	 * struct interface_status - Data for EVENT_INTERFACE_STATUS
	 */
	struct interface_status {
		char ifname[100];
		enum {
			EVENT_INTERFACE_ADDED, EVENT_INTERFACE_REMOVED
		} ievent;
	} interface_status;

	/**
	 * struct pmkid_candidate - Data for EVENT_PMKID_CANDIDATE
	 */
	struct pmkid_candidate {
		/** BSSID of the PMKID candidate */
		u8 bssid[ETH_ALEN];
		/** Smaller the index, higher the priority */
		int index;
		/** Whether RSN IE includes pre-authenticate flag */
		int preauth;
	} pmkid_candidate;

	/**
	 * struct stkstart - Data for EVENT_STKSTART
	 */
	struct stkstart {
		u8 peer[ETH_ALEN];
	} stkstart;

	/**
	 * struct ft_ies - FT information elements (EVENT_FT_RESPONSE)
	 *
	 * During FT (IEEE 802.11r) authentication sequence, the driver is
	 * expected to use this event to report received FT IEs (MDIE, FTIE,
	 * RSN IE, TIE, possible resource request) to the supplicant. The FT
	 * IEs for the next message will be delivered through the
	 * struct wpa_driver_ops::update_ft_ies() callback.
	 */
	struct ft_ies {
		const u8 *ies;
		size_t ies_len;
		int ft_action;
		u8 target_ap[ETH_ALEN];
		/** Optional IE(s), e.g., WMM TSPEC(s), for RIC-Request */
		const u8 *ric_ies;
		/** Length of ric_ies buffer in octets */
		size_t ric_ies_len;
	} ft_ies;

	/**
	 * struct ibss_rsn_start - Data for EVENT_IBSS_RSN_START
	 */
	struct ibss_rsn_start {
		u8 peer[ETH_ALEN];
	} ibss_rsn_start;

	/**
	 * struct auth_info - Data for EVENT_AUTH events
	 */
	struct auth_info {
		u8 peer[ETH_ALEN];
		u16 auth_type;
		u16 status_code;
		const u8 *ies;
		size_t ies_len;
	} auth;

	/**
	 * struct assoc_reject - Data for EVENT_ASSOC_REJECT events
	 */
	struct assoc_reject {
		/**
		 * resp_ies - (Re)Association Response IEs
		 *
		 * Optional association data from the driver. This data is not
		 * required WPA, but may be useful for some protocols and as
		 * such, should be reported if this is available to the driver
		 * interface.
		 *
		 * This should start with the first IE (fixed fields before IEs
		 * are not included).
		 */
		u8 *resp_ies;

		/**
		 * resp_ies_len - Length of resp_ies in bytes
		 */
		size_t resp_ies_len;

		/**
		 * status_code - Status Code from (Re)association Response
		 */
		u16 status_code;
	} assoc_reject;

	struct timeout_event {
		u8 addr[ETH_ALEN];
	} timeout_event;

	/**
	 * struct ft_rrb_rx - Data for EVENT_FT_RRB_RX events
	 */
	struct ft_rrb_rx {
		const u8 *src;
		const u8 *data;
		size_t data_len;
	} ft_rrb_rx;

	/**
	 * struct tx_status - Data for EVENT_TX_STATUS events
	 */
	struct tx_status {
		u16 type;
		u16 stype;
		const u8 *dst;
		const u8 *data;
		size_t data_len;
		int ack;
	} tx_status;

	/**
	 * struct rx_from_unknown - Data for EVENT_RX_FROM_UNKNOWN events
	 */
	struct rx_from_unknown {
		const u8 *frame;
		size_t len;
	} rx_from_unknown;

	/**
	 * struct rx_mgmt - Data for EVENT_RX_MGMT events
	 */
	struct rx_mgmt {
		const u8 *frame;
		size_t frame_len;
		u32 datarate;
		u32 ssi_signal;
	} rx_mgmt;

	/**
	 * struct rx_action - Data for EVENT_RX_ACTION events
	 */
	struct rx_action {
		/**
		 * da - Destination address of the received Action frame
		 */
		const u8 *da;

		/**
		 * sa - Source address of the received Action frame
		 */
		const u8 *sa;

		/**
		 * bssid - Address 3 of the received Action frame
		 */
		const u8 *bssid;

		/**
		 * category - Action frame category
		 */
		u8 category;

		/**
		 * data - Action frame body after category field
		 */
		const u8 *data;

		/**
		 * len - Length of data in octets
		 */
		size_t len;

		/**
		 * freq - Frequency (in MHz) on which the frame was received
		 */
		int freq;
	} rx_action;

	/**
	 * struct remain_on_channel - Data for EVENT_REMAIN_ON_CHANNEL events
	 *
	 * This is also used with EVENT_CANCEL_REMAIN_ON_CHANNEL events.
	 */
	struct remain_on_channel {
		/**
		 * freq - Channel frequency in MHz
		 */
		unsigned int freq;

		/**
		 * duration - Duration to remain on the channel in milliseconds
		 */
		unsigned int duration;
	} remain_on_channel;

	/**
	 * struct scan_info - Optional data for EVENT_SCAN_RESULTS events
	 * @aborted: Whether the scan was aborted
	 * @freqs: Scanned frequencies in MHz (%NULL = all channels scanned)
	 * @num_freqs: Number of entries in freqs array
	 * @ssids: Scanned SSIDs (%NULL or zero-length SSID indicates wildcard
	 *	SSID)
	 * @num_ssids: Number of entries in ssids array
	 */
	struct scan_info {
		int aborted;
		const int *freqs;
		size_t num_freqs;
//		struct wpa_driver_scan_ssid ssids[WPAS_MAX_SCAN_SSIDS];
		size_t num_ssids;
	} scan_info;

	/**
	 * struct mlme_rx - Data for EVENT_MLME_RX events
	 */
	struct mlme_rx {
		const u8 *buf;
		size_t len;
		int freq;
		int channel;
		int ssi;
	} mlme_rx;

	/**
	 * struct rx_probe_req - Data for EVENT_RX_PROBE_REQ events
	 */
	struct rx_probe_req {
		/**
		 * sa - Source address of the received Probe Request frame
		 */
		const u8 *sa;

		/**
		 * ie - IEs from the Probe Request body
		 */
		const u8 *ie;

		/**
		 * ie_len - Length of ie buffer in octets
		 */
		size_t ie_len;
	} rx_probe_req;

	/**
	 * struct new_sta - Data for EVENT_NEW_STA events
	 */
	struct new_sta {
		const u8 *addr;
	} new_sta;

	/**
	 * struct eapol_rx - Data for EVENT_EAPOL_RX events
	 */
	struct eapol_rx {
		const u8 *src;
		const u8 *data;
		size_t data_len;
	} eapol_rx;

	/**
	 * struct signal_change - Data for EVENT_SIGNAL_CHANGE events
	 */
	struct signal_change {
		int above_threshold;
	} signal_change;
};

struct wpa_eapol_key {
	u8 type;
	/* Note: key_info, key_length, and key_data_length are unaligned */
	u8 key_info[2]; /* big endian */
	u8 key_length[2]; /* big endian */
	u8 replay_counter[WPA_REPLAY_COUNTER_LEN];
	u8 key_nonce[WPA_NONCE_LEN];
	u8 key_iv[16];
	u8 key_rsc[WPA_KEY_RSC_LEN];
	u8 key_id[8]; /* Reserved in IEEE 802.11i/RSN */
	u8 key_mic[16];
	u8 key_data_length[2]; /* big endian */
	/* followed by key_data_length bytes of key_data */
} STRUCT_PACKED;

struct wpa_ptk {
	u8 kck[16]; /* EAPOL-Key Key Confirmation Key (KCK) */
	u8 kek[16]; /* EAPOL-Key Key Encryption Key (KEK) */
	u8 tk1[16]; /* Temporal Key 1 (TK1) */
	union {
		u8 tk2[16]; /* Temporal Key 2 (TK2) */
		struct {
			u8 tx_mic_key[8];
			u8 rx_mic_key[8];
		}STRUCT_PACKED auth;
	}STRUCT_PACKED u;
} STRUCT_PACKED;

struct wpa_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 oui[4]; /* 24-bit OUI followed by 8-bit OUI type */
	u8 version[2]; /* little endian */
} STRUCT_PACKED;

struct rsn_ie_hdr {
	u8 elem_id; /* WLAN_EID_RSN */
	u8 len;
	u8 version[2]; /* little endian */
} STRUCT_PACKED;

struct rsn_error_kde {
	be16 mui;
	be16 error_type;
} STRUCT_PACKED;

#define PEERKEY_MAX_IE_LEN 80
struct wpa_peerkey {
//	struct wpa_peerkey *next;
	int initiator; /* whether this end was initator for SMK handshake */
	u8 addr[ETH_ALEN]; /* other end MAC address */
	u8 inonce[WPA_NONCE_LEN]; /* Initiator Nonce */
	u8 pnonce[WPA_NONCE_LEN]; /* Peer Nonce */
	u8 rsnie_i[PEERKEY_MAX_IE_LEN]; /* Initiator RSN IE */
	size_t rsnie_i_len;
	u8 rsnie_p[PEERKEY_MAX_IE_LEN]; /* Peer RSN IE */
	size_t rsnie_p_len;
	u8 smk[PMK_LEN];
	int smk_complete;
	u8 smkid[PMKID_LEN];
	u32 lifetime;
//	os_time_t expiration;
	int cipher; /* Selected cipher (WPA_CIPHER_*) */
	u8 replay_counter[WPA_REPLAY_COUNTER_LEN];
	int replay_counter_set;
	int use_sha256; /* whether AKMP indicate SHA256-based derivations */

	struct wpa_ptk stk, tstk;
	int stk_set, tstk_set;
};


#define RSN_FT_CAPAB_FT_OVER_DS BIT(0)
#define RSN_FT_CAPAB_FT_RESOURCE_REQ_SUPP BIT(1)

struct rsn_ftie {
	u8 mic_control[2];
	u8 mic[16];
	u8 anonce[WPA_NONCE_LEN];
	u8 snonce[WPA_NONCE_LEN];
	/* followed by optional parameters */
} STRUCT_PACKED;

#define FTIE_SUBELEM_R1KH_ID 1
#define FTIE_SUBELEM_GTK 2
#define FTIE_SUBELEM_R0KH_ID 3
#define FTIE_SUBELEM_IGTK 4

struct rsn_rdie {
	u8 id;
	u8 descr_count;
	le16 status_code;
} STRUCT_PACKED;


struct wpa_eapol_ie_parse {
	const u8 *wpa_ie;
	size_t wpa_ie_len;
	const u8 *rsn_ie;
	size_t rsn_ie_len;
	const u8 *pmkid;
	const u8 *gtk;
	size_t gtk_len;
	const u8 *mac_addr;
	size_t mac_addr_len;
#ifdef CONFIG_PEERKEY
	const u8 *smk;
	size_t smk_len;
	const u8 *nonce;
	size_t nonce_len;
	const u8 *lifetime;
	size_t lifetime_len;
	const u8 *error;
	size_t error_len;
#endif /* CONFIG_PEERKEY */
#ifdef CONFIG_IEEE80211W
	const u8 *igtk;
	size_t igtk_len;
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_IEEE80211R
	const u8 *mdie;
	size_t mdie_len;
	const u8 *ftie;
	size_t ftie_len;
	const u8 *reassoc_deadline;
	const u8 *key_lifetime;
#endif /* CONFIG_IEEE80211R */
};


struct wpa_bss {
//	struct dl_list list;
//	struct dl_list list_id;
	unsigned int id;
	unsigned int scan_miss_count;
	unsigned int last_update_idx;
	unsigned int flags;
	u8 bssid[ETH_ALEN];
	u8 ssid[32];
	size_t ssid_len;
	int freq;
	u16 beacon_int;
	u16 caps;
	int qual;
	int noise;
	int level;
	u64 tsf;
//	struct os_time last_update;
	size_t ie_len;
	size_t beacon_ie_len;
	/* followed by ie_len octets of IEs */
	/* followed by beacon_ie_len octets of IEs */
};

struct rsn_pmksa_cache {
	u8 pmkid[PMKID_LEN];
	u8 id[PMKID_LEN];
	u8 pmk[PMK_LEN];
	size_t pmk_len;
	u8 aa[ETH_ALEN];
};

#define PMK_CACHE_NUM 2


struct wpa_sm {
	u8 pmk[PMK_LEN];
	size_t pmk_len;
	struct wpa_ptk ptk, tptk;
	int ptk_set, tptk_set;
	u8 snonce[WPA_NONCE_LEN];
	u8 anonce[WPA_NONCE_LEN]; /* ANonce from the last 1/4 msg */
	int renew_snonce;
	u8 rx_replay_counter[WPA_REPLAY_COUNTER_LEN];
	int rx_replay_counter_set;
//	u8 request_counter[WPA_REPLAY_COUNTER_LEN];

//	struct eapol_sm *eapol; /* EAPOL state machine from upper level code */

	struct rsn_pmksa_cache  *pmksa; /* PMKSA cache */
//	struct rsn_pmksa_cache_entry *cur_pmksa; /* current PMKSA entry */
//	struct dl_list pmksa_candidates;

//	struct l2_packet_data *l2_preauth;
//	struct l2_packet_data *l2_preauth_br;
//	u8 preauth_bssid[ETH_ALEN]; /* current RSN pre-auth peer or
//				     * 00:00:00:00:00:00 if no pre-auth is
//				     * in progress */
//	struct eapol_sm *preauth_eapol;

//	/struct wpa_sm_ctx *ctx;

//	void *scard_ctx; /* context for smartcard callbacks */
//	int fast_reauth; /* whether EAP fast re-authentication is enabled */

//	void *network_ctx;
//	int peerkey_enabled;
//	int allowed_pairwise_cipher; /* bitfield of WPA_CIPHER_* */
//	int proactive_key_caching;
//	int eap_workaround;
//	void *eap_conf_ctx;
//	u8 ssid[32];
//	size_t ssid_len;
//	int wpa_ptk_rekey;

	u8 own_addr[ETH_ALEN];
//	const char *ifname;
//	const char *bridge_ifname;
	u8 bssid[ETH_ALEN];

	unsigned int dot11RSNAConfigPMKLifetime;
	unsigned int dot11RSNAConfigPMKReauthThreshold;
	unsigned int dot11RSNAConfigSATimeout;

//	unsigned int dot11RSNA4WayHandshakeFailures;

	/* Selected configuration (based on Beacon/ProbeResp WPA IE) */
	unsigned int proto;
	unsigned int pairwise_cipher;
	unsigned int group_cipher;
	unsigned int key_mgmt;
	unsigned int mgmt_group_cipher;

	int rsn_enabled; /* Whether RSN is enabled in configuration */
	int mfp; /* 0 = disabled, 1 = optional, 2 = mandatory */

	u8 *assoc_wpa_ie; /* Own WPA/RSN IE from (Re)AssocReq */
	size_t assoc_wpa_ie_len;
	u8 *ap_wpa_ie, *ap_rsn_ie;
	size_t ap_wpa_ie_len, ap_rsn_ie_len;

#ifdef CONFIG_PEERKEY
	struct wpa_peerkey *peerkey;
#endif /* CONFIG_PEERKEY */

#ifdef CONFIG_IEEE80211R
	u8 xxkey[PMK_LEN]; /* PSK or the second 256 bits of MSK */
	size_t xxkey_len;
	u8 pmk_r0[PMK_LEN];
	u8 pmk_r0_name[WPA_PMK_NAME_LEN];
	u8 pmk_r1[PMK_LEN];
	u8 pmk_r1_name[WPA_PMK_NAME_LEN];
	u8 mobility_domain[MOBILITY_DOMAIN_ID_LEN];
	u8 r0kh_id[FT_R0KH_ID_MAX_LEN];
	size_t r0kh_id_len;
	u8 r1kh_id[FT_R1KH_ID_LEN];
	int ft_completed;
	int over_the_ds_in_progress;
	u8 target_ap[ETH_ALEN]; /* over-the-DS target AP */
	int set_ptk_after_assoc;
	u8 mdie_ft_capab; /* FT Capability and Policy from target AP MDIE */
	u8 assoc_resp_ies[64]; /* MDIE and FTIE from (Re)Association Response */
	size_t assoc_resp_ies_len;
#endif /* CONFIG_IEEE80211R */
};

enum { EAP_CODE_REQUEST = 1, EAP_CODE_RESPONSE = 2, EAP_CODE_SUCCESS = 3,
       EAP_CODE_FAILURE = 4 };

/* EAP Request and Response data begins with one octet Type. Success and
 * Failure do not have additional data. */

/*
 * EAP Method Types as allocated by IANA:
 * http://www.iana.org/assignments/eap-numbers
 */
typedef enum {
	EAP_TYPE_NONE = 0,
	EAP_TYPE_IDENTITY = 1 /* RFC 3748 */,
	EAP_TYPE_NOTIFICATION = 2 /* RFC 3748 */,
	EAP_TYPE_NAK = 3 /* Response only, RFC 3748 */,
	EAP_TYPE_MD5 = 4, /* RFC 3748 */
	EAP_TYPE_OTP = 5 /* RFC 3748 */,
	EAP_TYPE_GTC = 6, /* RFC 3748 */
	EAP_TYPE_TLS = 13 /* RFC 2716 */,
	EAP_TYPE_LEAP = 17 /* Cisco proprietary */,
	EAP_TYPE_SIM = 18 /* RFC 4186 */,
	EAP_TYPE_TTLS = 21 /* RFC 5281 */,
	EAP_TYPE_AKA = 23 /* RFC 4187 */,
	EAP_TYPE_PEAP = 25 /* draft-josefsson-pppext-eap-tls-eap-06.txt */,
	EAP_TYPE_MSCHAPV2 = 26 /* draft-kamath-pppext-eap-mschapv2-00.txt */,
	EAP_TYPE_TLV = 33 /* draft-josefsson-pppext-eap-tls-eap-07.txt */,
	EAP_TYPE_TNC = 38 /* TNC IF-T v1.0-r3; note: tentative assignment;
			   * type 38 has previously been allocated for
			   * EAP-HTTP Digest, (funk.com) */,
	EAP_TYPE_FAST = 43 /* RFC 4851 */,
	EAP_TYPE_PAX = 46 /* RFC 4746 */,
	EAP_TYPE_PSK = 47 /* RFC 4764 */,
	EAP_TYPE_SAKE = 48 /* RFC 4763 */,
	EAP_TYPE_IKEV2 = 49 /* RFC 5106 */,
	EAP_TYPE_AKA_PRIME = 50 /* draft-arkko-eap-aka-kdf-10.txt */,
	EAP_TYPE_GPSK = 51 /* RFC 5433 */,
	EAP_TYPE_EXPANDED = 254 /* RFC 3748 */
} EapType;


/* SMI Network Management Private Enterprise Code for vendor specific types */
enum {
	EAP_VENDOR_IETF = 0,
	EAP_VENDOR_MICROSOFT = 0x000137 /* Microsoft */,
	EAP_VENDOR_WFA = 0x00372A /* Wi-Fi Alliance */
};

#define EAP_MSK_LEN 64
#define EAP_EMSK_LEN 64

typedef enum {
	DECISION_FAIL, DECISION_COND_SUCC, DECISION_UNCOND_SUCC
} EapDecision;

typedef enum {
	METHOD_NONE, METHOD_INIT, METHOD_CONT, METHOD_MAY_CONT, METHOD_DONE
} EapMethodState;

struct wpabuf {
	size_t size; /* total size of the allocated buffer */
	size_t used; /* length of data in the buffer */
	u8 *ext_data; /* pointer to external data; NULL if data follows
		       * struct wpabuf */
	/* optionally followed by the allocated buffer */
};


#define IEEE8021X_REPLAY_COUNTER_LEN 8
#define IEEE8021X_KEY_SIGN_LEN 16
#define IEEE8021X_KEY_IV_LEN 16

#define IEEE8021X_KEY_INDEX_FLAG 0x80
#define IEEE8021X_KEY_INDEX_MASK 0x03

struct ieee802_1x_eapol_key {
	u8 type;
	/* Note: key_length is unaligned */
	u8 key_length[2];
	/* does not repeat within the life of the keying material used to
	 * encrypt the Key field; 64-bit NTP timestamp MAY be used here */
	u8 replay_counter[IEEE8021X_REPLAY_COUNTER_LEN];
	u8 key_iv[IEEE8021X_KEY_IV_LEN]; /* cryptographically random number */
	u8 key_index; /* key flag in the most significant bit:
		       * 0 = broadcast (default key),
		       * 1 = unicast (key mapping key); key index is in the
		       * 7 least significant bits */
	/* HMAC-MD5 message integrity check computed with MS-MPPE-Send-Key as
	 * the key */
	u8 key_signature[IEEE8021X_KEY_SIGN_LEN];

	/* followed by key: if packet body length = 44 + key length, then the
	 * key field (of key_length bytes) contains the key in encrypted form;
	 * if packet body length = 44, key field is absent and key_length
	 * represents the number of least significant octets from
	 * MS-MPPE-Send-Key attribute to be used as the keying material;
	 * RC4 key used in encryption = Key-IV + MS-MPPE-Recv-Key */
} STRUCT_PACKED;

struct eap_hdr {
	u8 code;
	u8 identifier;
	be16 length; /* including code and identifier; network byte order */
	/* followed by length-4 octets of data */
} STRUCT_PACKED;

struct eap_sm {
	enum {
		EAP_INITIALIZE, EAP_DISABLED, EAP_IDLE, EAP_RECEIVED,
		EAP_GET_METHOD, EAP_METHOD, EAP_SEND_RESPONSE, EAP_DISCARD,
		EAP_IDENTITY, EAP_NOTIFICATION, EAP_RETRANSMIT, EAP_SUCCESS,
		EAP_FAILURE
	} EAP_state;
	/* Long-term local variables */
	EapType selectedMethod;
	EapMethodState methodState;
	int lastId;
	struct wpabuf *lastRespData;
	EapDecision decision;
	/* Short-term local variables */
	Boolean rxReq;
	Boolean rxSuccess;
	Boolean rxFailure;
	int reqId;
	EapType reqMethod;
	int reqVendor;
	u32 reqVendorMethod;
	Boolean ignore;
	/* Constants */
	int ClientTimeout;

	/* Miscellaneous variables */
	Boolean allowNotifications; /* peer state machine <-> methods */
	struct wpabuf *eapRespData; /* peer to lower layer */
	Boolean eapKeyAvailable; /* peer to lower layer */
	u8 *eapKeyData; /* peer to lower layer */
	size_t eapKeyDataLen; /* peer to lower layer */
//	const struct eap_method *m; /* selected EAP method */
	/* not defined in RFC 4137 */
	Boolean changed;
	void *eapol_ctx;
//	struct eapol_callbacks *eapol_cb;
	void *eap_method_priv;
	int init_phase2;
	int fast_reauth;

	Boolean rxResp /* LEAP only */;
	Boolean leap_done;
	Boolean peap_done;
	u8 req_md5[16]; /* MD5() of the current EAP packet */
	u8 last_md5[16]; /* MD5() of the previously received EAP packet; used
			  * in duplicate request detection. */

	void *msg_ctx;
	void *scard_ctx;
	void *ssl_ctx;

	unsigned int workaround;

	/* Optional challenges generated in Phase 1 (EAP-FAST) */
	u8 *peer_challenge, *auth_challenge;

	int num_rounds;
	int force_disabled;

//	struct wps_context *wps;

	int prev_failure;
};


struct eap_peer_config {
	/**
	 * identity - EAP Identity
	 *
	 * This field is used to set the real user identity or NAI (for
	 * EAP-PSK/PAX/SAKE/GPSK).
	 */
	u8 *identity;

	/**
	 * identity_len - EAP Identity length
	 */
	size_t identity_len;

	/**
	 * anonymous_identity -  Anonymous EAP Identity
	 *
	 * This field is used for unencrypted use with EAP types that support
	 * different tunnelled identity, e.g., EAP-TTLS, in order to reveal the
	 * real identity (identity field) only to the authentication server.
	 *
	 * If not set, the identity field will be used for both unencrypted and
	 * protected fields.
	 */
	u8 *anonymous_identity;

	/**
	 * anonymous_identity_len - Length of anonymous_identity
	 */
	size_t anonymous_identity_len;

	/**
	 * password - Password string for EAP
	 *
	 * This field can include either the plaintext password (default
	 * option) or a NtPasswordHash (16-byte MD4 hash of the unicode
	 * presentation of the password) if flags field has
	 * EAP_CONFIG_FLAGS_PASSWORD_NTHASH bit set to 1. NtPasswordHash can
	 * only be used with authentication mechanism that use this hash as the
	 * starting point for operation: MSCHAP and MSCHAPv2 (EAP-MSCHAPv2,
	 * EAP-TTLS/MSCHAPv2, EAP-TTLS/MSCHAP, LEAP).
	 *
	 * In addition, this field is used to configure a pre-shared key for
	 * EAP-PSK/PAX/SAKE/GPSK. The length of the PSK must be 16 for EAP-PSK
	 * and EAP-PAX and 32 for EAP-SAKE. EAP-GPSK can use a variable length
	 * PSK.
	 */
	u8 *password;

	/**
	 * password_len - Length of password field
	 */
	size_t password_len;

	/**
	 * ca_cert - File path to CA certificate file (PEM/DER)
	 *
	 * This file can have one or more trusted CA certificates. If ca_cert
	 * and ca_path are not included, server certificate will not be
	 * verified. This is insecure and a trusted CA certificate should
	 * always be configured when using EAP-TLS/TTLS/PEAP. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 *
	 * Alternatively, this can be used to only perform matching of the
	 * server certificate (SHA-256 hash of the DER encoded X.509
	 * certificate). In this case, the possible CA certificates in the
	 * server certificate chain are ignored and only the server certificate
	 * is verified. This is configured with the following format:
	 * hash:://server/sha256/cert_hash_in_hex
	 * For example: "hash://server/sha256/
	 * 5a1bc1296205e6fdbe3979728efe3920798885c1c4590b5f90f43222d239ca6a"
	 *
	 * On Windows, trusted CA certificates can be loaded from the system
	 * certificate store by setting this to cert_store://name, e.g.,
	 * ca_cert="cert_store://CA" or ca_cert="cert_store://ROOT".
	 * Note that when running wpa_supplicant as an application, the user
	 * certificate store (My user account) is used, whereas computer store
	 * (Computer account) is used when running wpasvc as a service.
	 */
	u8 *ca_cert;

	/**
	 * ca_path - Directory path for CA certificate files (PEM)
	 *
	 * This path may contain multiple CA certificates in OpenSSL format.
	 * Common use for this is to point to system trusted CA list which is
	 * often installed into directory like /etc/ssl/certs. If configured,
	 * these certificates are added to the list of trusted CAs. ca_cert
	 * may also be included in that case, but it is not required.
	 */
	u8 *ca_path;

	/**
	 * client_cert - File path to client certificate file (PEM/DER)
	 *
	 * This field is used with EAP method that use TLS authentication.
	 * Usually, this is only configured for EAP-TLS, even though this could
	 * in theory be used with EAP-TTLS and EAP-PEAP, too. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *client_cert;

	/**
	 * private_key - File path to client private key file (PEM/DER/PFX)
	 *
	 * When PKCS#12/PFX file (.p12/.pfx) is used, client_cert should be
	 * commented out. Both the private key and certificate will be read
	 * from the PKCS#12 file in this case. Full path to the file should be
	 * used since working directory may change when wpa_supplicant is run
	 * in the background.
	 *
	 * Windows certificate store can be used by leaving client_cert out and
	 * configuring private_key in one of the following formats:
	 *
	 * cert://substring_to_match
	 *
	 * hash://certificate_thumbprint_in_hex
	 *
	 * For example: private_key="hash://63093aa9c47f56ae88334c7b65a4"
	 *
	 * Note that when running wpa_supplicant as an application, the user
	 * certificate store (My user account) is used, whereas computer store
	 * (Computer account) is used when running wpasvc as a service.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *private_key;

	/**
	 * private_key_passwd - Password for private key file
	 *
	 * If left out, this will be asked through control interface.
	 */
	u8 *private_key_passwd;

	/**
	 * dh_file - File path to DH/DSA parameters file (in PEM format)
	 *
	 * This is an optional configuration file for setting parameters for an
	 * ephemeral DH key exchange. In most cases, the default RSA
	 * authentication does not use this configuration. However, it is
	 * possible setup RSA to use ephemeral DH key exchange. In addition,
	 * ciphers with DSA keys always use ephemeral DH keys. This can be used
	 * to achieve forward secrecy. If the file is in DSA parameters format,
	 * it will be automatically converted into DH params. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *dh_file;

	/**
	 * subject_match - Constraint for server certificate subject
	 *
	 * This substring is matched against the subject of the authentication
	 * server certificate. If this string is set, the server sertificate is
	 * only accepted if it contains this string in the subject. The subject
	 * string is in following format:
	 *
	 * /C=US/ST=CA/L=San Francisco/CN=Test AS/emailAddress=as@n.example.com
	 */
	u8 *subject_match;

	/**
	 * altsubject_match - Constraint for server certificate alt. subject
	 *
	 * Semicolon separated string of entries to be matched against the
	 * alternative subject name of the authentication server certificate.
	 * If this string is set, the server sertificate is only accepted if it
	 * contains one of the entries in an alternative subject name
	 * extension.
	 *
	 * altSubjectName string is in following format: TYPE:VALUE
	 *
	 * Example: EMAIL:server@example.com
	 * Example: DNS:server.example.com;DNS:server2.example.com
	 *
	 * Following types are supported: EMAIL, DNS, URI
	 */
	u8 *altsubject_match;

	/**
	 * ca_cert2 - File path to CA certificate file (PEM/DER) (Phase 2)
	 *
	 * This file can have one or more trusted CA certificates. If ca_cert2
	 * and ca_path2 are not included, server certificate will not be
	 * verified. This is insecure and a trusted CA certificate should
	 * always be configured. Full path to the file should be used since
	 * working directory may change when wpa_supplicant is run in the
	 * background.
	 *
	 * This field is like ca_cert, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *ca_cert2;

	/**
	 * ca_path2 - Directory path for CA certificate files (PEM) (Phase 2)
	 *
	 * This path may contain multiple CA certificates in OpenSSL format.
	 * Common use for this is to point to system trusted CA list which is
	 * often installed into directory like /etc/ssl/certs. If configured,
	 * these certificates are added to the list of trusted CAs. ca_cert
	 * may also be included in that case, but it is not required.
	 *
	 * This field is like ca_path, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	u8 *ca_path2;

	/**
	 * client_cert2 - File path to client certificate file
	 *
	 * This field is like client_cert, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *client_cert2;

	/**
	 * private_key2 - File path to client private key file
	 *
	 * This field is like private_key, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *private_key2;

	/**
	 * private_key2_passwd -  Password for private key file
	 *
	 * This field is like private_key_passwd, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	u8 *private_key2_passwd;

	/**
	 * dh_file2 - File path to DH/DSA parameters file (in PEM format)
	 *
	 * This field is like dh_file, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication. Full path to the
	 * file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 *
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	u8 *dh_file2;

	/**
	 * subject_match2 - Constraint for server certificate subject
	 *
	 * This field is like subject_match, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	u8 *subject_match2;

	/**
	 * altsubject_match2 - Constraint for server certificate alt. subject
	 *
	 * This field is like altsubject_match, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	u8 *altsubject_match2;

	/**
	 * eap_methods - Allowed EAP methods
	 *
	 * (vendor=EAP_VENDOR_IETF,method=EAP_TYPE_NONE) terminated list of
	 * allowed EAP methods or %NULL if all methods are accepted.
	 */
//	struct eap_method_type *eap_methods;

	/**
	 * phase1 - Phase 1 (outer authentication) parameters
	 *
	 * String with field-value pairs, e.g., "peapver=0" or
	 * "peapver=1 peaplabel=1".
	 *
	 * 'peapver' can be used to force which PEAP version (0 or 1) is used.
	 *
	 * 'peaplabel=1' can be used to force new label, "client PEAP
	 * encryption",	to be used during key derivation when PEAPv1 or newer.
	 *
	 * Most existing PEAPv1 implementation seem to be using the old label,
	 * "client EAP encryption", and wpa_supplicant is now using that as the
	 * default value.
	 *
	 * Some servers, e.g., Radiator, may require peaplabel=1 configuration
	 * to interoperate with PEAPv1; see eap_testing.txt for more details.
	 *
	 * 'peap_outer_success=0' can be used to terminate PEAP authentication
	 * on tunneled EAP-Success. This is required with some RADIUS servers
	 * that implement draft-josefsson-pppext-eap-tls-eap-05.txt (e.g.,
	 * Lucent NavisRadius v4.4.0 with PEAP in "IETF Draft 5" mode).
	 *
	 * include_tls_length=1 can be used to force wpa_supplicant to include
	 * TLS Message Length field in all TLS messages even if they are not
	 * fragmented.
	 *
	 * sim_min_num_chal=3 can be used to configure EAP-SIM to require three
	 * challenges (by default, it accepts 2 or 3).
	 *
	 * result_ind=1 can be used to enable EAP-SIM and EAP-AKA to use
	 * protected result indication.
	 *
	 * fast_provisioning option can be used to enable in-line provisioning
	 * of EAP-FAST credentials (PAC):
	 * 0 = disabled,
	 * 1 = allow unauthenticated provisioning,
	 * 2 = allow authenticated provisioning,
	 * 3 = allow both unauthenticated and authenticated provisioning
	 *
	 * fast_max_pac_list_len=num option can be used to set the maximum
	 * number of PAC entries to store in a PAC list (default: 10).
	 *
	 * fast_pac_format=binary option can be used to select binary format
	 * for storing PAC entries in order to save some space (the default
	 * text format uses about 2.5 times the size of minimal binary format).
	 *
	 * crypto_binding option can be used to control PEAPv0 cryptobinding
	 * behavior:
	 * 0 = do not use cryptobinding (default)
	 * 1 = use cryptobinding if server supports it
	 * 2 = require cryptobinding
	 *
	 * EAP-WSC (WPS) uses following options: pin=Device_Password and
	 * uuid=Device_UUID
	 */
	char *phase1;

	/**
	 * phase2 - Phase2 (inner authentication with TLS tunnel) parameters
	 *
	 * String with field-value pairs, e.g., "auth=MSCHAPV2" for EAP-PEAP or
	 * "autheap=MSCHAPV2 autheap=MD5" for EAP-TTLS.
	 */
	char *phase2;

	/**
	 * pcsc - Parameters for PC/SC smartcard interface for USIM and GSM SIM
	 *
	 * This field is used to configure PC/SC smartcard interface.
	 * Currently, the only configuration is whether this field is %NULL (do
	 * not use PC/SC) or non-NULL (e.g., "") to enable PC/SC.
	 *
	 * This field is used for EAP-SIM and EAP-AKA.
	 */
	char *pcsc;

	/**
	 * pin - PIN for USIM, GSM SIM, and smartcards
	 *
	 * This field is used to configure PIN for SIM and smartcards for
	 * EAP-SIM and EAP-AKA. In addition, this is used with EAP-TLS if a
	 * smartcard is used for private key operations.
	 *
	 * If left out, this will be asked through control interface.
	 */
	char *pin;

	/**
	 * engine - Enable OpenSSL engine (e.g., for smartcard access)
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	int engine;

	/**
	 * engine_id - Engine ID for OpenSSL engine
	 *
	 * "opensc" to select OpenSC engine or "pkcs11" to select PKCS#11
	 * engine.
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	char *engine_id;

	/**
	 * engine2 - Enable OpenSSL engine (e.g., for smartcard) (Phase 2)
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 *
	 * This field is like engine, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	int engine2;


	/**
	 * pin2 - PIN for USIM, GSM SIM, and smartcards (Phase 2)
	 *
	 * This field is used to configure PIN for SIM and smartcards for
	 * EAP-SIM and EAP-AKA. In addition, this is used with EAP-TLS if a
	 * smartcard is used for private key operations.
	 *
	 * This field is like pin2, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 *
	 * If left out, this will be asked through control interface.
	 */
	char *pin2;

	/**
	 * engine2_id - Engine ID for OpenSSL engine (Phase 2)
	 *
	 * "opensc" to select OpenSC engine or "pkcs11" to select PKCS#11
	 * engine.
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 *
	 * This field is like engine_id, but used for phase 2 (inside
	 * EAP-TTLS/PEAP/FAST tunnel) authentication.
	 */
	char *engine2_id;


	/**
	 * key_id - Key ID for OpenSSL engine
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	char *key_id;

	/**
	 * cert_id - Cert ID for OpenSSL engine
	 *
	 * This is used if the certificate operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	char *cert_id;

	/**
	 * ca_cert_id - CA Cert ID for OpenSSL engine
	 *
	 * This is used if the CA certificate for EAP-TLS is on a smartcard.
	 */
	char *ca_cert_id;

	/**
	 * key2_id - Key ID for OpenSSL engine (phase2)
	 *
	 * This is used if private key operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	char *key2_id;

	/**
	 * cert2_id - Cert ID for OpenSSL engine (phase2)
	 *
	 * This is used if the certificate operations for EAP-TLS are performed
	 * using a smartcard.
	 */
	char *cert2_id;

	/**
	 * ca_cert2_id - CA Cert ID for OpenSSL engine (phase2)
	 *
	 * This is used if the CA certificate for EAP-TLS is on a smartcard.
	 */
	char *ca_cert2_id;

	/**
	 * otp - One-time-password
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when OTP is entered through the control interface.
	 */
	u8 *otp;

	/**
	 * otp_len - Length of the otp field
	 */
	size_t otp_len;

	/**
	 * pending_req_identity - Whether there is a pending identity request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	int pending_req_identity;

	/**
	 * pending_req_password - Whether there is a pending password request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	int pending_req_password;

	/**
	 * pending_req_pin - Whether there is a pending PIN request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	int pending_req_pin;

	/**
	 * pending_req_new_password - Pending password update request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	int pending_req_new_password;

	/**
	 * pending_req_passphrase - Pending passphrase request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	int pending_req_passphrase;

	/**
	 * pending_req_otp - Whether there is a pending OTP request
	 *
	 * This field should not be set in configuration step. It is only used
	 * internally when control interface is used to request needed
	 * information.
	 */
	char *pending_req_otp;

	/**
	 * pending_req_otp_len - Length of the pending OTP request
	 */
	size_t pending_req_otp_len;

	/**
	 * pac_file - File path or blob name for the PAC entries (EAP-FAST)
	 *
	 * wpa_supplicant will need to be able to create this file and write
	 * updates to it when PAC is being provisioned or refreshed. Full path
	 * to the file should be used since working directory may change when
	 * wpa_supplicant is run in the background.
	 * Alternatively, a named configuration blob can be used by setting
	 * this to blob://blob_name.
	 */
	char *pac_file;

	/**
	 * mschapv2_retry - MSCHAPv2 retry in progress
	 *
	 * This field is used internally by EAP-MSCHAPv2 and should not be set
	 * as part of configuration.
	 */
	int mschapv2_retry;

	/**
	 * new_password - New password for password update
	 *
	 * This field is used during MSCHAPv2 password update. This is normally
	 * requested from the user through the control interface and not set
	 * from configuration.
	 */
	u8 *new_password;

	/**
	 * new_password_len - Length of new_password field
	 */
	size_t new_password_len;

	/**
	 * fragment_size - Maximum EAP fragment size in bytes (default 1398)
	 *
	 * This value limits the fragment size for EAP methods that support
	 * fragmentation (e.g., EAP-TLS and EAP-PEAP). This value should be set
	 * small enough to make the EAP messages fit in MTU of the network
	 * interface used for EAPOL. The default value is suitable for most
	 * cases.
	 */
	int fragment_size;

#define EAP_CONFIG_FLAGS_PASSWORD_NTHASH BIT(0)
	/**
	 * flags - Network configuration flags (bitfield)
	 *
	 * This variable is used for internal flags to describe further details
	 * for the network parameters.
	 * bit 0 = password is represented as a 16-byte NtPasswordHash value
	 *         instead of plaintext password
	 */
	u32 flags;
};


/**
 * struct wpa_config_blob - Named configuration blob
 *
 * This data structure is used to provide storage for binary objects to store
 * abstract information like certificates and private keys inlined with the
 * configuration data.
 */
struct wpa_config_blob {
	/**
	 * name - Blob name
	 */
	char *name;

	/**
	 * data - Pointer to binary data
	 */
	u8 *data;

	/**
	 * len - Length of binary data
	 */
	size_t len;

	/**
	 * next - Pointer to next blob in the configuration
	 */
	struct wpa_config_blob *next;
};


typedef enum { Unauthorized, Authorized } PortStatus;
typedef enum { Auto, ForceUnauthorized, ForceAuthorized } PortControl;

/**
 * struct eapol_config - Per network configuration for EAPOL state machines
 */
struct eapol_config {
	/**
	 * accept_802_1x_keys - Accept IEEE 802.1X (non-WPA) EAPOL-Key frames
	 *
	 * This variable should be set to 1 when using EAPOL state machines
	 * with non-WPA security policy to generate dynamic WEP keys. When
	 * using WPA, this should be set to 0 so that WPA state machine can
	 * process the EAPOL-Key frames.
	 */
	int accept_802_1x_keys;

#define EAPOL_REQUIRE_KEY_UNICAST BIT(0)
#define EAPOL_REQUIRE_KEY_BROADCAST BIT(1)
	/**
	 * required_keys - Which EAPOL-Key packets are required
	 *
	 * This variable determines which EAPOL-Key packets are required before
	 * marking connection authenticated. This is a bit field of
	 * EAPOL_REQUIRE_KEY_UNICAST and EAPOL_REQUIRE_KEY_BROADCAST flags.
	 */
	int required_keys;

	/**
	 * fast_reauth - Whether fast EAP reauthentication is enabled
	 */
	int fast_reauth;

	/**
	 * workaround - Whether EAP workarounds are enabled
	 */
	unsigned int workaround;

	/**
	 * eap_disabled - Whether EAP is disabled
	 */
	int eap_disabled;
};

struct eapol_sm {
	/* Timers */
	unsigned int authWhile;
	unsigned int heldWhile;
	unsigned int startWhen;
	unsigned int idleWhile; /* for EAP state machine */
	int timer_tick_enabled;

	/* Global variables */
	Boolean eapFail;
	Boolean eapolEap;
	Boolean eapSuccess;
	Boolean initialize;
	Boolean keyDone;
	Boolean keyRun;
//	PortControl portControl;
	Boolean portEnabled;
//	PortStatus suppPortStatus;  /* dot1xSuppControlledPortStatus */
	Boolean portValid;
	Boolean suppAbort;
	Boolean suppFail;
	Boolean suppStart;
	Boolean suppSuccess;
	Boolean suppTimeout;

	/* Supplicant PAE state machine */
	enum {
		SUPP_PAE_UNKNOWN = 0,
		SUPP_PAE_DISCONNECTED = 1,
		SUPP_PAE_LOGOFF = 2,
		SUPP_PAE_CONNECTING = 3,
		SUPP_PAE_AUTHENTICATING = 4,
		SUPP_PAE_AUTHENTICATED = 5,
		/* unused(6) */
		SUPP_PAE_HELD = 7,
		SUPP_PAE_RESTART = 8,
		SUPP_PAE_S_FORCE_AUTH = 9,
		SUPP_PAE_S_FORCE_UNAUTH = 10
	} SUPP_PAE_state; /* dot1xSuppPaeState */
	/* Variables */
	Boolean userLogoff;
	Boolean logoffSent;
	unsigned int startCount;
	Boolean eapRestart;
//	PortControl sPortMode;
	/* Constants */
	unsigned int heldPeriod; /* dot1xSuppHeldPeriod */
	unsigned int startPeriod; /* dot1xSuppStartPeriod */
	unsigned int maxStart; /* dot1xSuppMaxStart */

	/* Key Receive state machine */
	enum {
		KEY_RX_UNKNOWN = 0,
		KEY_RX_NO_KEY_RECEIVE, KEY_RX_KEY_RECEIVE
	} KEY_RX_state;
	/* Variables */
	Boolean rxKey;

	/* Supplicant Backend state machine */
	enum {
		SUPP_BE_UNKNOWN = 0,
		SUPP_BE_INITIALIZE = 1,
		SUPP_BE_IDLE = 2,
		SUPP_BE_REQUEST = 3,
		SUPP_BE_RECEIVE = 4,
		SUPP_BE_RESPONSE = 5,
		SUPP_BE_FAIL = 6,
		SUPP_BE_TIMEOUT = 7, 
		SUPP_BE_SUCCESS = 8
	} SUPP_BE_state; /* dot1xSuppBackendPaeState */
	/* Variables */
	Boolean eapNoResp;
	Boolean eapReq;
	Boolean eapResp;
	/* Constants */
	unsigned int authPeriod; /* dot1xSuppAuthPeriod */

	/* Statistics */
	unsigned int dot1xSuppEapolFramesRx;
	unsigned int dot1xSuppEapolFramesTx;
	unsigned int dot1xSuppEapolStartFramesTx;
	unsigned int dot1xSuppEapolLogoffFramesTx;
	unsigned int dot1xSuppEapolRespFramesTx;
	unsigned int dot1xSuppEapolReqIdFramesRx;
	unsigned int dot1xSuppEapolReqFramesRx;
	unsigned int dot1xSuppInvalidEapolFramesRx;
	unsigned int dot1xSuppEapLengthErrorFramesRx;
	unsigned int dot1xSuppLastEapolFrameVersion;
	unsigned char dot1xSuppLastEapolFrameSource[6];

	/* Miscellaneous variables (not defined in IEEE 802.1X-2004) */
	Boolean changed;
	struct eap_sm *eap;
	struct eap_peer_config *config;
	Boolean initial_req;
	u8 *last_rx_key;
	size_t last_rx_key_len;
	struct wpabuf *eapReqData; /* for EAP */
	Boolean altAccept; /* for EAP */
	Boolean altReject; /* for EAP */
	Boolean replay_counter_valid;
	u8 last_replay_counter[16];
	struct eapol_config conf;
//	struct eapol_ctx *ctx;
	enum { EAPOL_CB_IN_PROGRESS = 0, EAPOL_CB_SUCCESS, EAPOL_CB_FAILURE }
		cb_status;
	Boolean cached_pmk;

	Boolean unicast_key_received, broadcast_key_received;
};

struct wpa_supplicant {
#if 0
	struct wpa_global *global;
	struct wpa_supplicant *next;
	struct l2_packet_data *l2;
	struct l2_packet_data *l2_br;
	unsigned char own_addr[ETH_ALEN];
	char ifname[100];
#ifdef CONFIG_CTRL_IFACE_DBUS
	char *dbus_path;
#endif /* CONFIG_CTRL_IFACE_DBUS */
#ifdef CONFIG_CTRL_IFACE_DBUS_NEW
	char *dbus_new_path;
#endif /* CONFIG_CTRL_IFACE_DBUS_NEW */
	char bridge_ifname[16];
#endif
//	char *confname;
//	struct wpa_config *conf;
	int countermeasures;
//	os_time_t last_michael_mic_error;

	u8 bssid[ETH_ALEN];
	u8 pending_bssid[ETH_ALEN]; /* If wpa_state == WPA_ASSOCIATING, this
				     * field contains the targer BSSID. */
	int reassociate; /* reassociation requested */
	int disconnected; /* all connections disabled; i.e., do no reassociate
			   * before this has been cleared */
	struct wpa_ssid *current_ssid;
	struct wpa_bss *current_bss;
	int ap_ies_from_associnfo;
	unsigned int assoc_freq;

	/* Selected configuration (based on Beacon/ProbeResp WPA IE) */
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int mgmt_group_cipher;

//	void *drv_priv; /* private data used by driver_ops */
//	void *global_drv_priv;

//	struct wpa_ssid *prev_scan_ssid; 
/* previously scanned SSID;
					  * NULL = not yet initialized (start
					  * with wildcard SSID)
					  * WILDCARD_SSID_SCAN = wildcard
					  * SSID was used in the previous scan
					  */
#define WILDCARD_SSID_SCAN ((struct wpa_ssid *) 1)

#if 0
	void (*scan_res_handler)(struct wpa_supplicant *wpa_s,
				 struct wpa_scan_results *scan_res);
	struct dl_list bss; /* struct wpa_bss::list */
	struct dl_list bss_id; /* struct wpa_bss::list_id */
#endif
//	size_t num_bss;
//	unsigned int bss_update_idx;
//	unsigned int bss_next_id;

//	struct wpa_driver_ops *driver;
//	int interface_removed; 
/* whether the network interface has been
				* removed */
	struct wpa_sm *wpa;
	struct eapol_sm *eapol;

//	struct ctrl_iface_priv *ctrl_iface;

	enum wpa_states wpa_state;
//	int scanning;
//	int new_connection;
//	int reassociated_connection;

	int eapol_received; /* number of EAPOL packets received after the
			     * previous association event */

//	struct scard_data *scard;

	unsigned char last_eapol_src[ETH_ALEN];

//	int keys_cleared;

//	struct wpa_blacklist *blacklist;

//	int scan_req; 
/* manual scan request; this forces a scan even if there
		       * are no enabled networks in the configuration */
//	int scan_runs; /* number of scan runs since WPS was started */

//	struct wpa_client_mlme mlme;
	unsigned int drv_flags;
//	int max_scan_ssids;
//	unsigned int max_remain_on_chan;

//	int pending_mic_error_report;
//	int pending_mic_error_pairwise;
//	int mic_errors_seen; /* Michael MIC errors with the current PTK */

//	struct wps_context *wps;
//	int wps_success; /* WPS success event received */
//	struct wps_er *wps_er;
//	int blacklist_cleared;

//	struct wpabuf *pending_eapol_rx;
//	struct os_time pending_eapol_rx_time;
//	u8 pending_eapol_rx_src[ETH_ALEN];

	struct ibss_rsn *ibss_rsn;

#ifdef CONFIG_SME
	struct {
		u8 ssid[32];
		size_t ssid_len;
		int freq;
		u8 assoc_req_ie[80];
		size_t assoc_req_ie_len;
		int mfp;
		int ft_used;
		u8 mobility_domain[2];
		u8 *ft_ies;
		size_t ft_ies_len;
		u8 prev_bssid[ETH_ALEN];
		int prev_bssid_set;
		int auth_alg;
	} sme;
#endif /* CONFIG_SME */

#ifdef CONFIG_AP
	struct hostapd_iface *ap_iface;
	void (*ap_configured_cb)(void *ctx, void *data);
	void *ap_configured_cb_ctx;
	void *ap_configured_cb_data;
#endif /* CONFIG_AP */

//	struct wpa_ssid *bgscan_ssid;
//	const struct bgscan_ops *bgscan;
//	void *bgscan_priv;

//	int connect_without_scan;

//	int after_wps;
//	unsigned int wps_freq;
};

void init_wpa_supplicant(void);
void wpa_supplicant_event_disassoc(struct wpa_supplicant *wpa_s, u16 reason_code);
void wpa_supplicant_event_assoc(struct wpa_supplicant *wpa_s,union wpa_event_data *data);
void wpa_supplicant_set_state(struct wpa_supplicant *wpa_s,
			      enum wpa_states state);
void eapol_input(struct netif *netif, struct pbuf *pb);
extern struct wpa_supplicant wpa_s;
extern struct rsn_pmksa_cache  pmksa[PMK_CACHE_NUM];
extern struct wpa_sm sm;
#endif
