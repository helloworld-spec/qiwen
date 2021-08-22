#ifndef __NET_CFG80211_H
#define __NET_CFG80211_H
/*
 * 802.11 device and configuration interface
 *
 * Copyright 2006-2010	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <nl80211.h>
#include <ieee80211.h>
#include <regulatory.h>


#define IEEE80211_CHAN_NO_HT40 \
	(IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)

#define EVENT_BG_SCAN_REPORT            0x00000018

/**
 * struct vif_params - describes virtual interface parameters
 * @use_4addr: use 4-address frames
 */
struct vif_params {
       int use_4addr;
};

/**
 * struct key_params - key information
 *
 * Information about a key
 *
 * @key: key material
 * @key_len: length of key material
 * @cipher: cipher suite selector
 * @seq: sequence counter (IV/PN) for TKIP and CCMP keys, only used
 *	with the get_key() callback, must be in little endian,
 *	length given by @seq_len.
 * @seq_len: length of @seq.
 */
struct key_params {
	u8 *key;
	u8 *seq;
	int key_len;
	int seq_len;
	u32 cipher;
};

/**
 * enum survey_info_flags - survey information flags
 *
 * @SURVEY_INFO_NOISE_DBM: noise (in dBm) was filled in
 * @SURVEY_INFO_IN_USE: channel is currently being used
 * @SURVEY_INFO_CHANNEL_TIME: channel active time (in ms) was filled in
 * @SURVEY_INFO_CHANNEL_TIME_BUSY: channel busy time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_EXT_BUSY: extension channel busy time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_RX: channel receive time was filled in
 * @SURVEY_INFO_CHANNEL_TIME_TX: channel transmit time was filled in
 *
 * Used by the driver to indicate which info in &struct survey_info
 * it has filled in during the get_survey().
 */
enum survey_info_flags {
	SURVEY_INFO_NOISE_DBM = 1<<0,
	SURVEY_INFO_IN_USE = 1<<1,
	SURVEY_INFO_CHANNEL_TIME = 1<<2,
	SURVEY_INFO_CHANNEL_TIME_BUSY = 1<<3,
	SURVEY_INFO_CHANNEL_TIME_EXT_BUSY = 1<<4,
	SURVEY_INFO_CHANNEL_TIME_RX = 1<<5,
	SURVEY_INFO_CHANNEL_TIME_TX = 1<<6,
};

/**
 * struct survey_info - channel survey response
 *
 * @channel: the channel this survey record reports, mandatory
 * @filled: bitflag of flags from &enum survey_info_flags
 * @noise: channel noise in dBm. This and all following fields are
 *     optional
 * @channel_time: amount of time in ms the radio spent on the channel
 * @channel_time_busy: amount of time the primary channel was sensed busy
 * @channel_time_ext_busy: amount of time the extension channel was sensed busy
 * @channel_time_rx: amount of time the radio spent receiving data
 * @channel_time_tx: amount of time the radio spent transmitting data
 *
 * Used by dump_survey() to report back per-channel survey information.
 *
 * This structure can later be expanded with things like
 * channel duty cycle etc.
 */
struct survey_info {
	struct ieee80211_channel *channel;
	u64 channel_time;
	u64 channel_time_busy;
	u64 channel_time_ext_busy;
	u64 channel_time_rx;
	u64 channel_time_tx;
	u32 filled;
	s8 noise;
};

/**
 * struct beacon_parameters - beacon parameters
 *
 * Used to configure the beacon for an interface.
 *
 * @head: head portion of beacon (before TIM IE)
 *     or %NULL if not changed
 * @tail: tail portion of beacon (after TIM IE)
 *     or %NULL if not changed
 * @interval: beacon interval or zero if not changed
 * @dtim_period: DTIM period or zero if not changed
 * @head_len: length of @head
 * @tail_len: length of @tail
 */
struct beacon_parameters {
	u8 *head, *tail;
	int interval, dtim_period;
	int head_len, tail_len;
};

/**
 * enum plink_action - actions to perform in mesh peers
 *
 * @PLINK_ACTION_INVALID: action 0 is reserved
 * @PLINK_ACTION_OPEN: start mesh peer link establishment
 * @PLINK_ACTION_BLOCK: block traffic from this mesh peer
 */
enum plink_actions {
	PLINK_ACTION_INVALID,
	PLINK_ACTION_OPEN,
	PLINK_ACTION_BLOCK,
};

/**
 * struct station_parameters - station parameters
 *
 * Used to change and create a new station.
 *
 * @vlan: vlan interface station should belong to
 * @supported_rates: supported rates in IEEE 802.11 format
 *	(or NULL for no change)
 * @supported_rates_len: number of supported rates
 * @sta_flags_mask: station flags that changed
 *	(bitmask of BIT(NL80211_STA_FLAG_...))
 * @sta_flags_set: station flags values
 *	(bitmask of BIT(NL80211_STA_FLAG_...))
 * @listen_interval: listen interval or -1 for no change
 * @aid: AID or zero for no change
 * @plink_action: plink action to take
 * @plink_state: set the peer link state for a station
 * @ht_capa: HT capabilities of station
 */
struct station_parameters {
	u8 *supported_rates;
	struct net_device *vlan;
	u32 sta_flags_mask, sta_flags_set;
	int listen_interval;
	u16 aid;
	u8 supported_rates_len;
	u8 plink_action;
	u8 plink_state;
	struct ieee80211_ht_cap *ht_capa;
};

/**
 * enum station_info_flags - station information flags
 *
 * Used by the driver to indicate which info in &struct station_info
 * it has filled in during get_station() or dump_station().
 *
 * @STATION_INFO_INACTIVE_TIME: @inactive_time filled
 * @STATION_INFO_RX_BYTES: @rx_bytes filled
 * @STATION_INFO_TX_BYTES: @tx_bytes filled
 * @STATION_INFO_LLID: @llid filled
 * @STATION_INFO_PLID: @plid filled
 * @STATION_INFO_PLINK_STATE: @plink_state filled
 * @STATION_INFO_SIGNAL: @signal filled
 * @STATION_INFO_TX_BITRATE: @txrate fields are filled
 *  (tx_bitrate, tx_bitrate_flags and tx_bitrate_mcs)
 * @STATION_INFO_RX_PACKETS: @rx_packets filled
 * @STATION_INFO_TX_PACKETS: @tx_packets filled
 * @STATION_INFO_TX_RETRIES: @tx_retries filled
 * @STATION_INFO_TX_FAILED: @tx_failed filled
 * @STATION_INFO_RX_DROP_MISC: @rx_dropped_misc filled
 * @STATION_INFO_SIGNAL_AVG: @signal_avg filled
 * @STATION_INFO_RX_BITRATE: @rxrate fields are filled
 * @STATION_INFO_BSS_PARAM: @bss_param filled
 * @STATION_INFO_CONNECTED_TIME: @connected_time filled
 */
enum station_info_flags {
	STATION_INFO_INACTIVE_TIME	= 1<<0,
	STATION_INFO_RX_BYTES		= 1<<1,
	STATION_INFO_TX_BYTES		= 1<<2,
	STATION_INFO_LLID		= 1<<3,
	STATION_INFO_PLID		= 1<<4,
	STATION_INFO_PLINK_STATE	= 1<<5,
	STATION_INFO_SIGNAL		= 1<<6,
	STATION_INFO_TX_BITRATE		= 1<<7,
	STATION_INFO_RX_PACKETS		= 1<<8,
	STATION_INFO_TX_PACKETS		= 1<<9,
	STATION_INFO_TX_RETRIES		= 1<<10,
	STATION_INFO_TX_FAILED		= 1<<11,
	STATION_INFO_RX_DROP_MISC	= 1<<12,
	STATION_INFO_SIGNAL_AVG		= 1<<13,
	STATION_INFO_RX_BITRATE		= 1<<14,
	STATION_INFO_BSS_PARAM          = 1<<15,
	STATION_INFO_CONNECTED_TIME	= 1<<16
};

/**
 * enum station_info_rate_flags - bitrate info flags
 *
 * Used by the driver to indicate the specific rate transmission
 * type for 802.11n transmissions.
 *
 * @RATE_INFO_FLAGS_MCS: @tx_bitrate_mcs filled
 * @RATE_INFO_FLAGS_40_MHZ_WIDTH: 40 Mhz width transmission
 * @RATE_INFO_FLAGS_SHORT_GI: 400ns guard interval
 */
enum rate_info_flags {
	RATE_INFO_FLAGS_MCS		= 1<<0,
	RATE_INFO_FLAGS_40_MHZ_WIDTH	= 1<<1,
	RATE_INFO_FLAGS_SHORT_GI	= 1<<2,
};

/**
 * struct rate_info - bitrate information
 *
 * Information about a receiving or transmitting bitrate
 *
 * @flags: bitflag of flags from &enum rate_info_flags
 * @mcs: mcs index if struct describes a 802.11n bitrate
 * @legacy: bitrate in 100kbit/s for 802.11abg
 */
struct rate_info {
	u8 flags;
	u8 mcs;
	u16 legacy;
};

/**
 * enum station_info_rate_flags - bitrate info flags
 *
 * Used by the driver to indicate the specific rate transmission
 * type for 802.11n transmissions.
 *
 * @BSS_PARAM_FLAGS_CTS_PROT: whether CTS protection is enabled
 * @BSS_PARAM_FLAGS_SHORT_PREAMBLE: whether short preamble is enabled
 * @BSS_PARAM_FLAGS_SHORT_SLOT_TIME: whether short slot time is enabled
 */
enum bss_param_flags {
	BSS_PARAM_FLAGS_CTS_PROT	= 1<<0,
	BSS_PARAM_FLAGS_SHORT_PREAMBLE	= 1<<1,
	BSS_PARAM_FLAGS_SHORT_SLOT_TIME	= 1<<2,
};

/**
 * struct sta_bss_parameters - BSS parameters for the attached station
 *
 * Information about the currently associated BSS
 *
 * @flags: bitflag of flags from &enum bss_param_flags
 * @dtim_period: DTIM period for the BSS
 * @beacon_interval: beacon interval
 */
struct sta_bss_parameters {
	u8 flags;
	u8 dtim_period;
	u16 beacon_interval;
};

/**
 * struct station_info - station information
 *
 * Station information filled by driver for get_station() and dump_station.
 *
 * @filled: bitflag of flags from &enum station_info_flags
 * @connected_time: time(in secs) since a station is last connected
 * @inactive_time: time since last station activity (tx/rx) in milliseconds
 * @rx_bytes: bytes received from this station
 * @tx_bytes: bytes transmitted to this station
 * @llid: mesh local link id
 * @plid: mesh peer link id
 * @plink_state: mesh peer link state
 * @signal: signal strength of last received packet in dBm
 * @signal_avg: signal strength average in dBm
 * @txrate: current unicast bitrate from this station
 * @rxrate: current unicast bitrate to this station
 * @rx_packets: packets received from this station
 * @tx_packets: packets transmitted to this station
 * @tx_retries: cumulative retry counts
 * @tx_failed: number of failed transmissions (retries exceeded, no ACK)
 * @rx_dropped_misc:  Dropped for un-specified reason.
 * @bss_param: current BSS parameters
 * @generation: generation number for nl80211 dumps.
 *	This number should increase every time the list of stations
 *	changes, i.e. when a station is added or removed, so that
 *	userspace can tell whether it got a consistent snapshot.
 */
struct station_info {
	u32 filled;
	u32 connected_time;
	u32 inactive_time;
	u32 rx_bytes;
	u32 tx_bytes;
	u16 llid;
	u16 plid;
	u8 plink_state;
	s8 signal;
	s8 signal_avg;
	struct rate_info txrate;
	struct rate_info rxrate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_retries;
	u32 tx_failed;
	u32 rx_dropped_misc;
	struct sta_bss_parameters bss_param;

	int 	generation;
	int	dblevel;
};

/**
 * enum monitor_flags - monitor flags
 *
 * Monitor interface configuration flags. Note that these must be the bits
 * according to the nl80211 flags.
 *
 * @MONITOR_FLAG_FCSFAIL: pass frames with bad FCS
 * @MONITOR_FLAG_PLCPFAIL: pass frames with bad PLCP
 * @MONITOR_FLAG_CONTROL: pass control frames
 * @MONITOR_FLAG_OTHER_BSS: disable BSSID filtering
 * @MONITOR_FLAG_COOK_FRAMES: report frames after processing
 */
enum monitor_flags {
	MONITOR_FLAG_FCSFAIL		= 1<<NL80211_MNTR_FLAG_FCSFAIL,
	MONITOR_FLAG_PLCPFAIL		= 1<<NL80211_MNTR_FLAG_PLCPFAIL,
	MONITOR_FLAG_CONTROL		= 1<<NL80211_MNTR_FLAG_CONTROL,
	MONITOR_FLAG_OTHER_BSS		= 1<<NL80211_MNTR_FLAG_OTHER_BSS,
	MONITOR_FLAG_COOK_FRAMES	= 1<<NL80211_MNTR_FLAG_COOK_FRAMES,
};

/**
 * enum mpath_info_flags -  mesh path information flags
 *
 * Used by the driver to indicate which info in &struct mpath_info it has filled
 * in during get_station() or dump_station().
 *
 * @MPATH_INFO_FRAME_QLEN: @frame_qlen filled
 * @MPATH_INFO_SN: @sn filled
 * @MPATH_INFO_METRIC: @metric filled
 * @MPATH_INFO_EXPTIME: @exptime filled
 * @MPATH_INFO_DISCOVERY_TIMEOUT: @discovery_timeout filled
 * @MPATH_INFO_DISCOVERY_RETRIES: @discovery_retries filled
 * @MPATH_INFO_FLAGS: @flags filled
 */
enum mpath_info_flags {
	MPATH_INFO_FRAME_QLEN		= BIT(0),
	MPATH_INFO_SN			= BIT(1),
	MPATH_INFO_METRIC		= BIT(2),
	MPATH_INFO_EXPTIME		= BIT(3),
	MPATH_INFO_DISCOVERY_TIMEOUT	= BIT(4),
	MPATH_INFO_DISCOVERY_RETRIES	= BIT(5),
	MPATH_INFO_FLAGS		= BIT(6),
};

/**
 * struct mpath_info - mesh path information
 *
 * Mesh path information filled by driver for get_mpath() and dump_mpath().
 *
 * @filled: bitfield of flags from &enum mpath_info_flags
 * @frame_qlen: number of queued frames for this destination
 * @sn: target sequence number
 * @metric: metric (cost) of this mesh path
 * @exptime: expiration time for the mesh path from now, in msecs
 * @flags: mesh path flags
 * @discovery_timeout: total mesh path discovery timeout, in msecs
 * @discovery_retries: mesh path discovery retries
 * @generation: generation number for nl80211 dumps.
 *	This number should increase every time the list of mesh paths
 *	changes, i.e. when a station is added or removed, so that
 *	userspace can tell whether it got a consistent snapshot.
 */
struct mpath_info {
	u32 filled;
	u32 frame_qlen;
	u32 sn;
	u32 metric;
	u32 exptime;
	u32 discovery_timeout;
	u8 discovery_retries;
	u8 flags;

	int generation;
};

/**
 * struct bss_parameters - BSS parameters
 *
 * Used to change BSS parameters (mainly for AP mode).
 *
 * @use_cts_prot: Whether to use CTS protection
 *	(0 = no, 1 = yes, -1 = do not change)
 * @use_short_preamble: Whether the use of short preambles is allowed
 *	(0 = no, 1 = yes, -1 = do not change)
 * @use_short_slot_time: Whether the use of short slot time is allowed
 *	(0 = no, 1 = yes, -1 = do not change)
 * @basic_rates: basic rates in IEEE 802.11 format
 *	(or NULL for no change)
 * @basic_rates_len: number of basic rates
 * @ap_isolate: do not forward packets between connected stations
 * @ht_opmode: HT Operation mode
 * 	(u16 = opmode, -1 = do not change)
 */
struct bss_parameters {
	int use_cts_prot;
	int use_short_preamble;
	int use_short_slot_time;
	u8 *basic_rates;
	u8 basic_rates_len;
	int ap_isolate;
	int ht_opmode;
};

/*
 * struct mesh_config - 802.11s mesh configuration
 *
 * These parameters can be changed while the mesh is active.
 */
struct mesh_config {
	/* Timeouts in ms */
	/* Mesh plink management parameters */
	u16 dot11MeshRetryTimeout;
	u16 dot11MeshConfirmTimeout;
	u16 dot11MeshHoldingTimeout;
	u16 dot11MeshMaxPeerLinks;
	u8  dot11MeshMaxRetries;
	u8  dot11MeshTTL;
	/* ttl used in path selection information elements */
	u8  element_ttl;
	bool auto_open_plinks;
	/* HWMP parameters */
	u8  dot11MeshHWMPmaxPREQretries;
	u32 path_refresh_time;
	u16 min_discovery_timeout;
	u32 dot11MeshHWMPactivePathTimeout;
	u16 dot11MeshHWMPpreqMinInterval;
	u16 dot11MeshHWMPnetDiameterTraversalTime;
	u8  dot11MeshHWMPRootMode;
};

/**
 * struct mesh_setup - 802.11s mesh setup configuration
 * @mesh_id: the mesh ID
 * @mesh_id_len: length of the mesh ID, at least 1 and at most 32 bytes
 * @path_sel_proto: which path selection protocol to use
 * @path_metric: which metric to use
 * @ie: vendor information elements (optional)
 * @ie_len: length of vendor information elements
 * @is_authenticated: this mesh requires authentication
 * @is_secure: this mesh uses security
 *
 * These parameters are fixed when the mesh is created.
 */
struct mesh_setup {
	const u8 *mesh_id;
	u8 mesh_id_len;
	u8  path_sel_proto;
	u8  path_metric;
	const u8 *ie;
	u8 ie_len;
	bool is_authenticated;
	bool is_secure;
};

/**
 * struct ieee80211_txq_params - TX queue parameters
 * @queue: TX queue identifier (NL80211_TXQ_Q_*)
 * @txop: Maximum burst time in units of 32 usecs, 0 meaning disabled
 * @cwmin: Minimum contention window [a value of the form 2^n-1 in the range
 *	1..32767]
 * @cwmax: Maximum contention window [a value of the form 2^n-1 in the range
 *	1..32767]
 * @aifs: Arbitration interframe space [0..255]
 */
struct ieee80211_txq_params {
	enum nl80211_txq_q queue;
	u16 txop;
	u16 cwmin;
	u16 cwmax;
	u8 aifs;
};

/* from net/wireless.h */
struct wiphy;

/**
 * DOC: Scanning and BSS list handling
 *
 * The scanning process itself is fairly simple, but cfg80211 offers quite
 * a bit of helper functionality. To start a scan, the scan operation will
 * be invoked with a scan definition. This scan definition contains the
 * channels to scan, and the SSIDs to send probe requests for (including the
 * wildcard, if desired). A passive scan is indicated by having no SSIDs to
 * probe. Additionally, a scan request may contain extra information elements
 * that should be added to the probe request. The IEs are guaranteed to be
 * well-formed, and will not exceed the maximum length the driver advertised
 * in the wiphy structure.
 *
 * When scanning finds a BSS, cfg80211 needs to be notified of that, because
 * it is responsible for maintaining the BSS list; the driver should not
 * maintain a list itself. For this notification, various functions exist.
 *
 * Since drivers do not maintain a BSS list, there are also a number of
 * functions to search for a BSS and obtain information about it from the
 * BSS structure cfg80211 maintains. The BSS list is also made available
 * to userspace.
 */

/**
 * struct cfg80211_ssid - SSID description
 * @ssid: the SSID
 * @ssid_len: length of the ssid
 */
struct cfg80211_ssid {
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
};

/**
 * struct cfg80211_scan_request - scan request description
 *
 * @ssids: SSIDs to scan for (active scan only)
 * @n_ssids: number of SSIDs
 * @channels: channels to scan on.
 * @n_channels: total number of channels to scan
 * @ie: optional information element(s) to add into Probe Request or %NULL
 * @ie_len: length of ie in octets
 * @wiphy: the wiphy this was for
 * @dev: the interface
 * @aborted: (internal) scan request was notified as aborted
 */
struct cfg80211_scan_request {
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	const u8 *ie;
	size_t ie_len;

	/* internal */
	struct wiphy *wiphy;
	struct net_device *dev;
	bool aborted;

	/* keep last */
	struct ieee80211_channel *channels[ZEROSIZE];
};

/**
 * struct cfg80211_sched_scan_request - scheduled scan request description
 *
 * @ssids: SSIDs to scan for (passed in the probe_reqs in active scans)
 * @n_ssids: number of SSIDs
 * @n_channels: total number of channels to scan
 * @interval: interval between each scheduled scan cycle
 * @ie: optional information element(s) to add into Probe Request or %NULL
 * @ie_len: length of ie in octets
 * @wiphy: the wiphy this was for
 * @dev: the interface
 * @channels: channels to scan
 */
struct cfg80211_sched_scan_request {
	struct cfg80211_ssid *ssids;
	int n_ssids;
	u32 n_channels;
	u32 interval;
	const u8 *ie;
	size_t ie_len;

	/* internal */
	struct wiphy *wiphy;
	struct net_device *dev;

	/* keep last */
	struct ieee80211_channel *channels[ZEROSIZE];
};

/**
 * enum cfg80211_signal_type - signal type
 *
 * @CFG80211_SIGNAL_TYPE_NONE: no signal strength information available
 * @CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm)
 * @CFG80211_SIGNAL_TYPE_UNSPEC: signal strength, increasing from 0 through 100
 */
enum cfg80211_signal_type {
	CFG80211_SIGNAL_TYPE_NONE,
	CFG80211_SIGNAL_TYPE_MBM,
	CFG80211_SIGNAL_TYPE_UNSPEC,
};

/**
 * struct cfg80211_bss - BSS description
 *
 * This structure describes a BSS (which may also be a mesh network)
 * for use in scan results and similar.
 *
 * @channel: channel this BSS is on
 * @bssid: BSSID of the BSS
 * @tsf: timestamp of last received update
 * @beacon_interval: the beacon interval as from the frame
 * @capability: the capability field in host byte order
 * @information_elements: the information elements (Note that there
 *	is no guarantee that these are well-formed!); this is a pointer to
 *	either the beacon_ies or proberesp_ies depending on whether Probe
 *	Response frame has been received
 * @len_information_elements: total length of the information elements
 * @beacon_ies: the information elements from the last Beacon frame
 * @len_beacon_ies: total length of the beacon_ies
 * @proberesp_ies: the information elements from the last Probe Response frame
 * @len_proberesp_ies: total length of the proberesp_ies
 * @signal: signal strength value (type depends on the wiphy's signal_type)
 * @free_priv: function pointer to free private data
 * @priv: private area for driver use, has at least wiphy->bss_priv_size bytes
 */
struct cfg80211_bss {
	struct ieee80211_channel *channel;
	u8 ssid[32];
	u8 ssid_len;
	u8 bssid[ETH_ALEN];
	u64 tsf;
	u16 beacon_interval;
	u16 capability;
	u8 *information_elements;
	size_t len_information_elements;
	u8 *beacon_ies;
	size_t len_beacon_ies;
	u8 *proberesp_ies;
	size_t len_proberesp_ies;

	s32 signal;

	void (*free_priv)(struct cfg80211_bss *bss);
	u8 priv[ZEROSIZE];
};

/**
 * ieee80211_bss_get_ie - find IE with given ID
 * @bss: the bss to search
 * @ie: the IE ID
 * Returns %NULL if not found.
 */
const u8 *ieee80211_bss_get_ie(struct cfg80211_bss *bss, u8 ie);


/**
 * struct cfg80211_crypto_settings - Crypto settings
 * @wpa_versions: indicates which, if any, WPA versions are enabled
 *	(from enum nl80211_wpa_versions)
 * @cipher_group: group key cipher suite (or 0 if unset)
 * @n_ciphers_pairwise: number of AP supported unicast ciphers
 * @ciphers_pairwise: unicast key cipher suites
 * @n_akm_suites: number of AKM suites
 * @akm_suites: AKM suites
 * @control_port: Whether user space controls IEEE 802.1X port, i.e.,
 *	sets/clears %NL80211_STA_FLAG_AUTHORIZED. If true, the driver is
 *	required to assume that the port is unauthorized until authorized by
 *	user space. Otherwise, port is marked authorized by default.
 * @control_port_ethertype: the control port protocol that should be
 *	allowed through even on unauthorized ports
 * @control_port_no_encrypt: TRUE to prevent encryption of control port
 *	protocol frames.
 */
struct cfg80211_crypto_settings {
	u32 wpa_versions;
	u32 cipher_group;
	int n_ciphers_pairwise;
	u32 ciphers_pairwise[NL80211_MAX_NR_CIPHER_SUITES];
	int n_akm_suites;
	u32 akm_suites[NL80211_MAX_NR_AKM_SUITES];
	bool control_port;
	__be16 control_port_ethertype;
	bool control_port_no_encrypt;
};

/**
 * struct cfg80211_auth_request - Authentication request data
 *
 * This structure provides information needed to complete IEEE 802.11
 * authentication.
 *
 * @bss: The BSS to authenticate with.
 * @auth_type: Authentication type (algorithm)
 * @ie: Extra IEs to add to Authentication frame or %NULL
 * @ie_len: Length of ie buffer in octets
 * @key_len: length of WEP key for shared key authentication
 * @key_idx: index of WEP key for shared key authentication
 * @key: WEP key for shared key authentication
 * @local_state_change: This is a request for a local state only, i.e., no
 *	Authentication frame is to be transmitted and authentication state is
 *	to be changed without having to wait for a response from the peer STA
 *	(AP).
 */
struct cfg80211_auth_request {
	struct cfg80211_bss *bss;
	const u8 *ie;
	size_t ie_len;
	enum nl80211_auth_type auth_type;
	const u8 *key;
	u8 key_len, key_idx;
	bool local_state_change;
};

/**
 * struct cfg80211_assoc_request - (Re)Association request data
 *
 * This structure provides information needed to complete IEEE 802.11
 * (re)association.
 * @bss: The BSS to associate with.
 * @ie: Extra IEs to add to (Re)Association Request frame or %NULL
 * @ie_len: Length of ie buffer in octets
 * @use_mfp: Use management frame protection (IEEE 802.11w) in this association
 * @crypto: crypto settings
 * @prev_bssid: previous BSSID, if not %NULL use reassociate frame
 */
struct cfg80211_assoc_request {
	struct cfg80211_bss *bss;
	const u8 *ie, *prev_bssid;
	size_t ie_len;
	struct cfg80211_crypto_settings crypto;
	bool use_mfp;
};

/**
 * struct cfg80211_deauth_request - Deauthentication request data
 *
 * This structure provides information needed to complete IEEE 802.11
 * deauthentication.
 *
 * @bss: the BSS to deauthenticate from
 * @ie: Extra IEs to add to Deauthentication frame or %NULL
 * @ie_len: Length of ie buffer in octets
 * @reason_code: The reason code for the deauthentication
 * @local_state_change: This is a request for a local state only, i.e., no
 *	Deauthentication frame is to be transmitted.
 */
struct cfg80211_deauth_request {
	struct cfg80211_bss *bss;
	const u8 *ie;
	size_t ie_len;
	u16 reason_code;
	bool local_state_change;
};

/**
 * struct cfg80211_disassoc_request - Disassociation request data
 *
 * This structure provides information needed to complete IEEE 802.11
 * disassocation.
 *
 * @bss: the BSS to disassociate from
 * @ie: Extra IEs to add to Disassociation frame or %NULL
 * @ie_len: Length of ie buffer in octets
 * @reason_code: The reason code for the disassociation
 * @local_state_change: This is a request for a local state only, i.e., no
 *	Disassociation frame is to be transmitted.
 */
struct cfg80211_disassoc_request {
	struct cfg80211_bss *bss;
	const u8 *ie;
	size_t ie_len;
	u16 reason_code;
	bool local_state_change;
};

/**
 * struct cfg80211_ibss_params - IBSS parameters
 *
 * This structure defines the IBSS parameters for the join_ibss()
 * method.
 *
 * @ssid: The SSID, will always be non-null.
 * @ssid_len: The length of the SSID, will always be non-zero.
 * @bssid: Fixed BSSID requested, maybe be %NULL, if set do not
 *	search for IBSSs with a different BSSID.
 * @channel: The channel to use if no IBSS can be found to join.
 * @channel_fixed: The channel should be fixed -- do not search for
 *	IBSSs to join on other channels.
 * @ie: information element(s) to include in the beacon
 * @ie_len: length of that
 * @beacon_interval: beacon interval to use
 * @privacy: this is a protected network, keys will be configured
 *	after joining
 * @basic_rates: bitmap of basic rates to use when creating the IBSS
 * @mcast_rate: per-band multicast rate index + 1 (0: disabled)
 */
struct cfg80211_ibss_params {
	u8 *ssid;
	u8 *bssid;
	struct ieee80211_channel *channel;
	u8 *ie;
	u8 ssid_len, ie_len;
	u16 beacon_interval;
	u32 basic_rates;
	bool channel_fixed;
	bool privacy;
	int mcast_rate[IEEE80211_NUM_BANDS];
	char *key;
	u8 key_len;
};

/**
 * struct cfg80211_connect_params - Connection parameters
 *
 * This structure provides information needed to complete IEEE 802.11
 * authentication and association.
 *
 * @channel: The channel to use or %NULL if not specified (auto-select based
 *	on scan results)
 * @bssid: The AP BSSID or %NULL if not specified (auto-select based on scan
 *	results)
 * @ssid: SSID
 * @ssid_len: Length of ssid in octets
 * @auth_type: Authentication type (algorithm)
 * @ie: IEs for association request
 * @ie_len: Length of assoc_ie in octets
 * @privacy: indicates whether privacy-enabled APs should be used
 * @crypto: crypto settings
 * @key_len: length of WEP key for shared key authentication
 * @key_idx: index of WEP key for shared key authentication
 * @key: WEP key for shared key authentication
 */
struct cfg80211_connect_params {
	struct ieee80211_channel *channel;
	u8 *bssid;
	u8 ssid[36];
	size_t ssid_len;
	enum nl80211_auth_type auth_type;
	u8 *ie;
	size_t ie_len;
	bool privacy;
	struct cfg80211_crypto_settings crypto;
	u8 key[32];
	u8 key_len, key_idx;
};

/**
 * enum wiphy_params_flags - set_wiphy_params bitfield values
 * @WIPHY_PARAM_RETRY_SHORT: wiphy->retry_short has changed
 * @WIPHY_PARAM_RETRY_LONG: wiphy->retry_long has changed
 * @WIPHY_PARAM_FRAG_THRESHOLD: wiphy->frag_threshold has changed
 * @WIPHY_PARAM_RTS_THRESHOLD: wiphy->rts_threshold has changed
 * @WIPHY_PARAM_COVERAGE_CLASS: coverage class changed
 */
enum wiphy_params_flags {
	WIPHY_PARAM_RETRY_SHORT		= 1 << 0,
	WIPHY_PARAM_RETRY_LONG		= 1 << 1,
	WIPHY_PARAM_FRAG_THRESHOLD	= 1 << 2,
	WIPHY_PARAM_RTS_THRESHOLD	= 1 << 3,
	WIPHY_PARAM_COVERAGE_CLASS	= 1 << 4,
};

/*
 * cfg80211_bitrate_mask - masks for bitrate control
 */
struct cfg80211_bitrate_mask {
	struct {
		u32 legacy;
		/* TODO: add support for masking MCS rates; e.g.: */
		/* u8 mcs[IEEE80211_HT_MCS_MASK_LEN]; */
	} control[IEEE80211_NUM_BANDS];
};
/**
 * struct cfg80211_pmksa - PMK Security Association
 *
 * This structure is passed to the set/del_pmksa() method for PMKSA
 * caching.
 *
 * @bssid: The AP's BSSID.
 * @pmkid: The PMK material itself.
 */
struct cfg80211_pmksa {
	u8 *bssid;
	u8 *pmkid;
};

/**
 * struct cfg80211_wowlan_trig_pkt_pattern - packet pattern
 * @mask: bitmask where to match pattern and where to ignore bytes,
 *	one bit per byte, in same format as nl80211
 * @pattern: bytes to match where bitmask is 1
 * @pattern_len: length of pattern (in bytes)
 *
 * Internal note: @mask and @pattern are allocated in one chunk of
 * memory, free @mask only!
 */
struct cfg80211_wowlan_trig_pkt_pattern {
	u8 *mask, *pattern;
	int pattern_len;
};

/**
 * struct cfg80211_wowlan - Wake on Wireless-LAN support info
 *
 * This structure defines the enabled WoWLAN triggers for the device.
 * @any: wake up on any activity -- special trigger if device continues
 *	operating as normal during suspend
 * @disconnect: wake up if getting disconnected
 * @magic_pkt: wake up on receiving magic packet
 * @patterns: wake up on receiving packet matching a pattern
 * @n_patterns: number of patterns
 */
struct cfg80211_wowlan {
	bool any, disconnect, magic_pkt;
	struct cfg80211_wowlan_trig_pkt_pattern *patterns;
	int n_patterns;
};



/*
 * wireless hardware and networking interfaces structures
 * and registration/helper functions
 */

/**
 * enum wiphy_flags - wiphy capability flags
 *
 * @WIPHY_FLAG_CUSTOM_REGULATORY:  tells us the driver for this device
 * 	has its own custom regulatory domain and cannot identify the
 * 	ISO / IEC 3166 alpha2 it belongs to. When this is enabled
 * 	we will disregard the first regulatory hint (when the
 * 	initiator is %REGDOM_SET_BY_CORE).
 * @WIPHY_FLAG_STRICT_REGULATORY: tells us the driver for this device will
 *	ignore regulatory domain settings until it gets its own regulatory
 *	domain via its regulatory_hint() unless the regulatory hint is
 *	from a country IE. After its gets its own regulatory domain it will
 *	only allow further regulatory domain settings to further enhance
 *	compliance. For example if channel 13 and 14 are disabled by this
 *	regulatory domain no user regulatory domain can enable these channels
 *	at a later time. This can be used for devices which do not have
 *	calibration information guaranteed for frequencies or settings
 *	outside of its regulatory domain.
 * @WIPHY_FLAG_DISABLE_BEACON_HINTS: enable this if your driver needs to ensure
 *	that passive scan flags and beaconing flags may not be lifted by
 *	cfg80211 due to regulatory beacon hints. For more information on beacon
 *	hints read the documenation for regulatory_hint_found_beacon()
 * @WIPHY_FLAG_NETNS_OK: if not set, do not allow changing the netns of this
 *	wiphy at all
 * @WIPHY_FLAG_ENFORCE_COMBINATIONS: Set this flag to enforce interface
 *	combinations for this device. This flag is used for backward
 *	compatibility only until all drivers advertise combinations and
 *	they will always be enforced.
 * @WIPHY_FLAG_PS_ON_BY_DEFAULT: if set to true, powersave will be enabled
 *	by default -- this flag will be set depending on the kernel's default
 *	on wiphy_new(), but can be changed by the driver if it has a good
 *	reason to override the default
 * @WIPHY_FLAG_4ADDR_AP: supports 4addr mode even on AP (with a single station
 *	on a VLAN interface)
 * @WIPHY_FLAG_4ADDR_STATION: supports 4addr mode even as a station
 * @WIPHY_FLAG_CONTROL_PORT_PROTOCOL: This device supports setting the
 *	control port protocol ethertype. The device also honours the
 *	control_port_no_encrypt flag.
 * @WIPHY_FLAG_IBSS_RSN: The device supports IBSS RSN.
 * @WIPHY_FLAG_MESH_AUTH: The device supports mesh authentication by routing
 *	auth frames to userspace. See @NL80211_MESH_SETUP_USERSPACE_AUTH.
 * @WIPHY_FLAG_SUPPORTS_SCHED_SCAN: The device supports scheduled scans.
 */
enum wiphy_flags {
	WIPHY_FLAG_CUSTOM_REGULATORY		= BIT(0),
	WIPHY_FLAG_STRICT_REGULATORY		= BIT(1),
	WIPHY_FLAG_DISABLE_BEACON_HINTS		= BIT(2),
	WIPHY_FLAG_NETNS_OK			= BIT(3),
	WIPHY_FLAG_PS_ON_BY_DEFAULT		= BIT(4),
	WIPHY_FLAG_4ADDR_AP			= BIT(5),
	WIPHY_FLAG_4ADDR_STATION		= BIT(6),
	WIPHY_FLAG_CONTROL_PORT_PROTOCOL	= BIT(7),
	WIPHY_FLAG_IBSS_RSN			= BIT(8),
	WIPHY_FLAG_MESH_AUTH			= BIT(10),
	WIPHY_FLAG_SUPPORTS_SCHED_SCAN		= BIT(11),
	WIPHY_FLAG_ENFORCE_COMBINATIONS		= BIT(12),
};

/**
 * struct ieee80211_iface_limit - limit on certain interface types
 * @max: maximum number of interfaces of these types
 * @types: interface types (bits)
 */
struct ieee80211_iface_limit {
	u16 max;
	u16 types;
};

/**
 * struct ieee80211_iface_combination - possible interface combination
 * @limits: limits for the given interface types
 * @n_limits: number of limitations
 * @num_different_channels: can use up to this many different channels
 * @max_interfaces: maximum number of interfaces in total allowed in this
 *	group
 * @beacon_int_infra_match: In this combination, the beacon intervals
 *	between infrastructure and AP types must match. This is required
 *	only in special cases.
 *
 * These examples can be expressed as follows:
 *
 * Allow #STA <= 1, #AP <= 1, matching BI, channels = 1, 2 total:
 *
 *  struct ieee80211_iface_limit limits1[] = {
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_STATION), },
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_AP}, },
 *  };
 *  struct ieee80211_iface_combination combination1 = {
 *	.limits = limits1,
 *	.n_limits = ARRAY_SIZE(limits1),
 *	.max_interfaces = 2,
 *	.beacon_int_infra_match = true,
 *  };
 *
 *
 * Allow #{AP, P2P-GO} <= 8, channels = 1, 8 total:
 *
 *  struct ieee80211_iface_limit limits2[] = {
 *	{ .max = 8, .types = BIT(NL80211_IFTYPE_AP) |
 *			     BIT(NL80211_IFTYPE_P2P_GO), },
 *  };
 *  struct ieee80211_iface_combination combination2 = {
 *	.limits = limits2,
 *	.n_limits = ARRAY_SIZE(limits2),
 *	.max_interfaces = 8,
 *	.num_different_channels = 1,
 *  };
 *
 *
 * Allow #STA <= 1, #{P2P-client,P2P-GO} <= 3 on two channels, 4 total.
 * This allows for an infrastructure connection and three P2P connections.
 *
 *  struct ieee80211_iface_limit limits3[] = {
 *	{ .max = 1, .types = BIT(NL80211_IFTYPE_STATION), },
 *	{ .max = 3, .types = BIT(NL80211_IFTYPE_P2P_GO) |
 *			     BIT(NL80211_IFTYPE_P2P_CLIENT), },
 *  };
 *  struct ieee80211_iface_combination combination3 = {
 *	.limits = limits3,
 *	.n_limits = ARRAY_SIZE(limits3),
 *	.max_interfaces = 4,
 *	.num_different_channels = 2,
 *  };
 */
struct ieee80211_iface_combination {
	const struct ieee80211_iface_limit *limits;
	u32 num_different_channels;
	u16 max_interfaces;
	u8 n_limits;
	bool beacon_int_infra_match;
};

struct mac_address {
	u8 addr[ETH_ALEN];
};

struct ieee80211_txrx_stypes {
	u16 tx, rx;
};

/**
 * enum wiphy_wowlan_support_flags - WoWLAN support flags
 * @WIPHY_WOWLAN_ANY: supports wakeup for the special "any"
 *	trigger that keeps the device operating as-is and
 *	wakes up the host on any activity, for example a
 *	received packet that passed filtering; note that the
 *	packet should be preserved in that case
 * @WIPHY_WOWLAN_MAGIC_PKT: supports wakeup on magic packet
 *	(see nl80211.h)
 * @WIPHY_WOWLAN_DISCONNECT: supports wakeup on disconnect
 */
enum wiphy_wowlan_support_flags {
	WIPHY_WOWLAN_ANY	= BIT(0),
	WIPHY_WOWLAN_MAGIC_PKT	= BIT(1),
	WIPHY_WOWLAN_DISCONNECT	= BIT(2),
};

/**
 * struct wiphy_wowlan_support - WoWLAN support data
 * @flags: see &enum wiphy_wowlan_support_flags
 * @n_patterns: number of supported wakeup patterns
 *	(see nl80211.h for the pattern definition)
 * @pattern_max_len: maximum length of each pattern
 * @pattern_min_len: minimum length of each pattern
 */
struct wiphy_wowlan_support {
	u32 flags;
	int n_patterns;
	int pattern_max_len;
	int pattern_min_len;
};

/**
 * struct wiphy - wireless hardware description
 * @reg_notifier: the driver's regulatory notification callback,
 *	note that if your driver uses wiphy_apply_custom_regulatory()
 *	the reg_notifier's request can be passed as NULL
 * @regd: the driver's regulatory domain, if one was requested via
 * 	the regulatory_hint() API. This can be used by the driver
 *	on the reg_notifier() if it chooses to ignore future
 *	regulatory domain changes caused by other drivers.
 * @signal_type: signal type reported in &struct cfg80211_bss.
 * @cipher_suites: supported cipher suites
 * @n_cipher_suites: number of supported cipher suites
 * @retry_short: Retry limit for short frames (dot11ShortRetryLimit)
 * @retry_long: Retry limit for long frames (dot11LongRetryLimit)
 * @frag_threshold: Fragmentation threshold (dot11FragmentationThreshold);
 *	-1 = fragmentation disabled, only odd values >= 256 used
 * @rts_threshold: RTS threshold (dot11RTSThreshold); -1 = RTS/CTS disabled
 * @_net: the network namespace this wiphy currently lives in
 * @perm_addr: permanent MAC address of this device
 * @addr_mask: If the device supports multiple MAC addresses by masking,
 *	set this to a mask with variable bits set to 1, e.g. if the last
 *	four bits are variable then set it to 00:...:00:0f. The actual
 *	variable bits shall be determined by the interfaces added, with
 *	interfaces not matching the mask being rejected to be brought up.
 * @n_addresses: number of addresses in @addresses.
 * @addresses: If the device has more than one address, set this pointer
 *	to a list of addresses (6 bytes each). The first one will be used
 *	by default for perm_addr. In this case, the mask should be set to
 *	all-zeroes. In this case it is assumed that the device can handle
 *	the same number of arbitrary MAC addresses.
 * @debugfsdir: debugfs directory used for this wiphy, will be renamed
 *	automatically on wiphy renames
 * @dev: (virtual) struct device for this wiphy
 * @wext: wireless extension handlers
 * @priv: driver private data (sized according to wiphy_new() parameter)
 * @interface_modes: bitmask of interfaces types valid for this wiphy,
 *	must be set by driver
 * @iface_combinations: Valid interface combinations array, should not
 *	list single interface types.
 * @n_iface_combinations: number of entries in @iface_combinations array.
 * @software_iftypes: bitmask of software interface types, these are not
 *	subject to any restrictions since they are purely managed in SW.
 * @flags: wiphy flags, see &enum wiphy_flags
 * @bss_priv_size: each BSS struct has private data allocated with it,
 *	this variable determines its size
 * @max_scan_ssids: maximum number of SSIDs the device can scan for in
 *	any given scan
 * @max_scan_ie_len: maximum length of user-controlled IEs device can
 *	add to probe request frames transmitted during a scan, must not
 *	include fixed IEs like supported rates
 * @coverage_class: current coverage class
 * @fw_version: firmware version for ethtool reporting
 * @hw_version: hardware version for ethtool reporting
 * @max_num_pmkids: maximum number of PMKIDs supported by device
 * @privid: a pointer that drivers can use to identify if an arbitrary
 *	wiphy is theirs, e.g. in global notifiers
 * @bands: information about bands/channels supported by this device
 *
 * @mgmt_stypes: bitmasks of frame subtypes that can be subscribed to or
 *	transmitted through nl80211, points to an array indexed by interface
 *	type
 *
 * @available_antennas_tx: bitmap of antennas which are available to be
 *	configured as TX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_rx is set.
 *
 * @available_antennas_rx: bitmap of antennas which are available to be
 *	configured as RX antennas. Antenna configuration commands will be
 *	rejected unless this or @available_antennas_tx is set.
 *
 * @max_remain_on_channel_duration: Maximum time a remain-on-channel operation
 *	may request, if implemented.
 *
 * @wowlan: WoWLAN support information
 */
struct wiphy {

	/* Supported interface modes, OR together BIT(NL80211_IFTYPE_...) */
	u16 interface_modes;


	enum cfg80211_signal_type signal_type;

	u8 max_scan_ssids;

	int n_cipher_suites;
	const u32 *cipher_suites;

	struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS];

	struct ieee80211_txrx_stypes *mgmt_stypes;

	u16 max_remain_on_channel_duration;
	void *priv;
};

extern struct ieee80211_channel *ieee80211_get_channel(struct wiphy *wiphy,
						  int freq);


/**
 * wiphy_new - create a new wiphy for use with cfg80211
 *
 * @ops: The configuration operations for this device
 * @sizeof_priv: The size of the private area to allocate
 *
 * Create a new wiphy and associate the given operations with it.
 * @sizeof_priv bytes are allocated for private use.
 *
 * The returned pointer must be assigned to each netdev's
 * ieee80211_ptr for proper operation.
 */

/**
 * wiphy_register - register a wiphy with cfg80211
 *
 * @wiphy: The wiphy to register.
 *
 * Returns a non-negative wiphy index or a negative error code.
 */
extern int wiphy_register(struct wiphy *wiphy);

/**
 * wiphy_unregister - deregister a wiphy from cfg80211
 *
 * @wiphy: The wiphy to unregister.
 *
 * After this call, no more requests can be made with this priv
 * pointer, but the call may sleep to wait for an outstanding
 * request that is being handled.
 */
extern void wiphy_unregister(struct wiphy *wiphy);

/**
 * wiphy_free - free wiphy
 *
 * @wiphy: The wiphy to free
 */
extern void wiphy_free(struct wiphy *wiphy);

/* internal structs */
struct cfg80211_conn;
struct cfg80211_internal_bss;
struct cfg80211_cached_keys;

#define MAX_AUTH_BSSES		4

//static __inline void *wiphy_priv(struct wiphy *wiphy)
//{
//	return &wiphy->priv;
//}


/**
 * struct wireless_dev - wireless per-netdev state
 *
 * This structure must be allocated by the driver/stack
 * that uses the ieee80211_ptr field in struct net_device
 * (this is intentional so it can be allocated along with
 * the netdev.)
 *
 * @wiphy: pointer to hardware description
 * @iftype: interface type
 * @list: (private) Used to collect the interfaces
 * @netdev: (private) Used to reference back to the netdev
 * @current_bss: (private) Used by the internal configuration code
 * @channel: (private) Used by the internal configuration code to track
 *	user-set AP, monitor and WDS channels for wireless extensions
 * @bssid: (private) Used by the internal configuration code
 * @ssid: (private) Used by the internal configuration code
 * @ssid_len: (private) Used by the internal configuration code
 * @mesh_id_len: (private) Used by the internal configuration code
 * @mesh_id_up_len: (private) Used by the internal configuration code
 * @wext: (private) Used by the internal wireless extensions compat code
 * @use_4addr: indicates 4addr mode is used on this interface, must be
 *	set by driver (if supported) on add_interface BEFORE registering the
 *	netdev and may otherwise be used by driver read-only, will be update
 *	by cfg80211 on change_interface
 * @mgmt_registrations: list of registrations for management frames
 * @mgmt_registrations_lock: lock for the list
 * @mtx: mutex used to lock data in this struct
 * @cleanup_work: work struct used for cleanup that can't be done directly
 * @beacon_interval: beacon interval used on this device for transmitting
 *	beacons, 0 when not valid
 */

struct cfg80211_conn {
	struct cfg80211_connect_params params;
	/* these are sub-states of the _CONNECTING sme_state */
	enum {
		CFG80211_CONN_IDLE,
		CFG80211_CONN_SCANNING,
		CFG80211_CONN_SCAN_AGAIN,
		CFG80211_CONN_AUTHENTICATE_NEXT,
		CFG80211_CONN_AUTHENTICATING,
		CFG80211_CONN_ASSOCIATE_NEXT,
		CFG80211_CONN_ASSOCIATING,
		CFG80211_CONN_DEAUTH_ASSOC_FAIL,
	} state;
	u8 bssid[ETH_ALEN], prev_bssid[ETH_ALEN];
	u8 *ie;
	size_t ie_len;
	bool auto_auth, prev_bssid_valid;
};

struct cfg80211_cached_keys {
	struct key_params params[6];
	u8 data[6][WLAN_MAX_KEY_LEN];
	int def, defmgmt;
};

struct cfg80211_internal_bss {
//	struct rb_node rbn;
	unsigned long ts;
	int  beacon_ies_allocated;
	int  proberesp_ies_allocated;
	/* must be last because of priv member */
	struct cfg80211_bss pub;
};

struct wireless_dev {
	struct wiphy *wiphy;
	enum nl80211_iftype iftype;

	struct net_device *netdev;

	/* currently used for IBSS and SME - might be rearranged later */
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len/*, mesh_id_len, mesh_id_up_len*/;
	bool ps;
};

enum nl80211_chan_width {
	NL80211_CHAN_WIDTH_20_NOHT,
	NL80211_CHAN_WIDTH_20,
	NL80211_CHAN_WIDTH_40,
	NL80211_CHAN_WIDTH_80,
	NL80211_CHAN_WIDTH_80P80,
	NL80211_CHAN_WIDTH_160,
	NL80211_CHAN_WIDTH_5,
	NL80211_CHAN_WIDTH_10,
};

struct cfg80211_chan_def {
	struct ieee80211_channel *chan;
	enum nl80211_chan_width width;
	u32 center_freq1;
	u32 center_freq2;
};

struct cfg80211_beacon_data {
	const u8 *head, *tail;
	const u8 *beacon_ies;
	const u8 *proberesp_ies;
	const u8 *assocresp_ies;
	const u8 *probe_resp;

	size_t head_len, tail_len;
	size_t beacon_ies_len;
	size_t proberesp_ies_len;
	size_t assocresp_ies_len;
	size_t probe_resp_len;
};

enum nl80211_hidden_ssid {
	NL80211_HIDDEN_SSID_NOT_IN_USE,
	NL80211_HIDDEN_SSID_ZERO_LEN,
	NL80211_HIDDEN_SSID_ZERO_CONTENTS
};

enum nl80211_acl_policy {
	NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED,
	NL80211_ACL_POLICY_DENY_UNLESS_LISTED,
};

struct cfg80211_acl_data {
	enum nl80211_acl_policy acl_policy;
	int n_acl_entries;

	/* Keep it last */
	struct mac_address *mac_addrs;
};


struct cfg80211_ap_settings {
	struct cfg80211_chan_def chandef;

	struct cfg80211_beacon_data beacon;

	int beacon_interval, dtim_period;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	int inactivity_timeout;
	u8 p2p_ctwindow;
	bool p2p_opp_ps;
	const struct cfg80211_acl_data *acl;
};



struct cfg80211_ops {

	int	(*change_virtual_intf)(struct wiphy *wiphy,
				       struct net_device *dev,
				       enum nl80211_iftype type, u32 *flags,
				       struct vif_params *params);

	int	(*add_key)(struct wiphy *wiphy, struct net_device *netdev,
			   u8 key_index, bool pairwise, const u8 *mac_addr,
			   struct key_params *params);

	int	(*del_key)(struct wiphy *wiphy, struct net_device *netdev,
			   u8 key_index, bool pairwise, const u8 *mac_addr);
	int	(*set_default_key)(struct wiphy *wiphy,
				   struct net_device *netdev,
				   u8 key_index, bool unicast, bool multicast);

	int	(*start_ap)(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_ap_settings *settings);
	int	(*change_beacon)(struct wiphy *wiphy, struct net_device *dev,
				 struct cfg80211_beacon_data *info);
	int	(*stop_ap)(struct wiphy *wiphy, struct net_device *dev);

	int	(*get_station)(struct wiphy *wiphy, struct net_device *dev,
			       const u8 *mac, struct station_info *sinfo);
	int	(*dump_station)(struct wiphy *wiphy, struct net_device *dev,
				int idx, u8 *mac, struct station_info *sinfo);

	int (*scan)(struct wiphy *wiphy, struct net_device *dev,
                   struct cfg80211_scan_request *request);
				   
	int	(*wifi_connect)(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_connect_params *sme);
	int	(*disconnect)(struct wiphy *wiphy, struct net_device *dev,
			      u16 reason_code);

	int	(*join_ibss)(struct wiphy *wiphy, struct net_device *dev,
			     struct cfg80211_ibss_params *params);
	int	(*leave_ibss)(struct wiphy *wiphy, struct net_device *dev);

	int	(*set_wiphy_params)(struct wiphy *wiphy, u32 changed);

	int	(*set_tx_power)(struct wiphy *wiphy, struct net_device *dev,
				enum nl80211_tx_power_setting type, int mbm);

	int (*mgmt_tx)(struct wiphy *wiphy,
                      struct net_device *dev,
                      struct ieee80211_channel *chan, bool offchan,
                      enum nl80211_channel_type channel_type,
                      bool channel_type_valid, unsigned int wait,
                      const u8 * buf, size_t len,
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0) || defined(COMPAT_WIRELESS)
                      bool no_cck,
//#endif
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
                      bool dont_wait_for_ack,
//#endif
                      u64 * cookie);
	int	(*mgmt_tx_cancel_wait)(struct wiphy *wiphy,
				       struct net_device *dev,
				       u64 cookie);
	int	(*remain_on_channel)(struct wiphy *wiphy,
				     struct net_device *dev,
				     struct ieee80211_channel *chan,
				     unsigned int duration,
				     u64 *cookie);
	int	(*cancel_remain_on_channel)(struct wiphy *wiphy,
					    struct net_device *dev,
					    u64 cookie);
	int	(*set_power_mgmt)(struct wiphy *wiphy, struct net_device *dev,
				  bool enabled, int timeout);

	int	(*set_cqm_rssi_config)(struct wiphy *wiphy,
				       struct net_device *dev,
				       s32 rssi_thold, u32 rssi_hyst);

	void	(*mgmt_frame_register)(struct wiphy *wiphy,
				       struct net_device *dev,
				       u16 frame_type, bool reg);
				
	int	(*set_channel)(struct wiphy *wiphy,
                          struct net_device *dev,
                          struct ieee80211_channel *chan,
                          enum nl80211_channel_type channel_type);
};


extern int ieee80211_channel_to_frequency(int chan, enum ieee80211_band band);

struct cfg80211_bss*
cfg80211_inform_bss(struct wiphy *wiphy,
		    struct ieee80211_channel *channel,
		    const u8 *bssid,
		    const u8 *essid,
		    u8 essid_len,
		    u64 timestamp, u16 capability, u16 beacon_interval,
		    const u8 *ie, size_t ielen,
		    s32 signal, gfp_t gfp);

#endif /* __NET_CFG80211_H */

