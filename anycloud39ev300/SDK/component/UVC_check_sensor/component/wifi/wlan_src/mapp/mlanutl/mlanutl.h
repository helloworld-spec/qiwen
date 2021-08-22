/** @file  mlanutl.h
  *
  * @brief This file contains definitions for application
  *
  * (C) Copyright 2011-2012 Marvell International Ltd. All Rights Reserved
  *
  * MARVELL CONFIDENTIAL
  * The source code contained or described herein and all documents related to
  * the source code ("Material") are owned by Marvell International Ltd or its
  * suppliers or licensors. Title to the Material remains with Marvell International Ltd
  * or its suppliers and licensors. The Material contains trade secrets and
  * proprietary and confidential information of Marvell or its suppliers and
  * licensors. The Material is protected by worldwide copyright and trade secret
  * laws and treaty provisions. No part of the Material may be used, copied,
  * reproduced, modified, published, uploaded, posted, transmitted, distributed,
  * or disclosed in any way without Marvell's prior express written permission.
  *
  * No license under any patent, copyright, trade secret or other intellectual
  * property right is granted to or conferred upon you by disclosure or delivery
  * of the Materials, either expressly, by implication, inducement, estoppel or
  * otherwise. Any license under such intellectual property rights must be
  * express and approved by Marvell in writing.
  *
  */
/************************************************************************
Change log:
     11/26/2008: initial version
************************************************************************/
#ifndef _MLANUTL_H_
#define _MLANUTL_H_

/** Include header files */
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <sys/socket.h>
#include    <sys/ioctl.h>
#include    <linux/if.h>
#include    <linux/wireless.h>
#include    <sys/types.h>


/** 16 bits byte swap */
#define swap_byte_16(x) \
((t_u16)((((t_u16)(x) & 0x00ffU) << 8) | \
         (((t_u16)(x) & 0xff00U) >> 8)))

/** 32 bits byte swap */
#define swap_byte_32(x) \
((t_u32)((((t_u32)(x) & 0x000000ffUL) << 24) | \
         (((t_u32)(x) & 0x0000ff00UL) <<  8) | \
         (((t_u32)(x) & 0x00ff0000UL) >>  8) | \
         (((t_u32)(x) & 0xff000000UL) >> 24)))

/** Convert to correct endian format */
#ifdef  BIG_ENDIAN_SUPPORT
/** CPU to little-endian convert for 16-bit */
#define     cpu_to_le16(x)  swap_byte_16(x)
/** CPU to little-endian convert for 32-bit */
#define     cpu_to_le32(x)  swap_byte_32(x)
/** Little-endian to CPU convert for 16-bit */
#define     le16_to_cpu(x)  swap_byte_16(x)
/** Little-endian to CPU convert for 32-bit */
#define     le32_to_cpu(x)  swap_byte_32(x)
#else
/** Do nothing */
#define     cpu_to_le16(x)  (x)
/** Do nothing */
#define     cpu_to_le32(x)  (x)
/** Do nothing */
#define     le16_to_cpu(x)  (x)
/** Do nothing */
#define     le32_to_cpu(x)  (x)
#endif

/** Character, 1 byte */
typedef char t_s8;
/** Unsigned character, 1 byte */
typedef unsigned char t_u8;

/** Short integer */
typedef signed short t_s16;
/** Unsigned short integer */
typedef unsigned short t_u16;

/** Integer */
typedef signed int t_s32;
/** Unsigned integer */
typedef unsigned int t_u32;

/** Long long integer */
typedef signed long long t_s64;
/** Unsigned long long integer */
typedef unsigned long long t_u64;

/** Void pointer (4-bytes) */
typedef void t_void;

/** The attribute pack used for structure packing */
#ifndef __ATTRIB_PACK__
#define __ATTRIB_PACK__  __attribute__((packed))
#endif

/** Success */
#define MLAN_STATUS_SUCCESS         (0)
/** Failure */
#define MLAN_STATUS_FAILURE         (-1)

/** IOCTL number */
#define MLAN_ETH_PRIV               (SIOCDEVPRIVATE + 14)

/** Command buffer max length */
#define BUFFER_LENGTH       (2 * 1024)

/** Find number of elements */
#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))

/** BIT value */
#define MBIT(x)    (((t_u32)1) << (x))

#ifndef MIN
/** Find minimum value */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

/** Length of ethernet address */
#ifndef ETH_ALEN
#define ETH_ALEN            6
#endif

struct command_node
{
    char *name;
    int (*handler) (int, char **);
};

/** Private command structure */
struct eth_priv_cmd
{
    /** Command buffer */
    t_u8 *buf;
    /** Used length */
    t_u32 used_len;
    /** Total length */
    t_u32 total_len;
};

struct eth_priv_htcapinfo
{
    t_u32 ht_cap_info_bg;
    t_u32 ht_cap_info_a;
};

struct eth_priv_addba
{
    t_u32 time_out;
    t_u32 tx_win_size;
    t_u32 rx_win_size;
    t_u32 tx_amsdu;
    t_u32 rx_amsdu;
};

/** data structure for cmd getdatarate */
struct eth_priv_data_rate
{
    /** Tx data rate */
    t_u32 tx_data_rate;
    /** Rx data rate */
    t_u32 rx_data_rate;

    /** Tx channel bandwidth */
    t_u32 tx_bw;
    /** Tx guard interval */
    t_u32 tx_gi;
    /** Rx channel bandwidth */
    t_u32 rx_bw;
    /** Rx guard interval */
    t_u32 rx_gi;
};

/** data structure for cmd bandcfg */
struct eth_priv_bandcfg
{
    /** Infra band */
    t_u32 config_bands;
    /** Ad-hoc start band */
    t_u32 adhoc_start_band;
    /** Ad-hoc start channel */
    t_u32 adhoc_channel;
    t_u32 adhoc_chan_bandwidth;
    /** fw supported band */
    t_u32 fw_bands;
};

/** data structure for cmd txratecfg */
struct eth_priv_tx_rate_cfg
{
    /* LG rate: 0, HT rate: 1, VHT rate: 2 */
    t_u32 rate_format;
    /** Rate/MCS index (0xFF: auto) */
    t_u32 rate_index;
};

struct eth_priv_esuppmode_cfg
{
    /* RSN mode */
    t_u16 rsn_mode;
    /* Pairwise cipher */
    t_u8 pairwise_cipher;
    /* Group cipher */
    t_u8 group_cipher;
};

/** MLAN MAC Address Length */
#define MLAN_MAC_ADDR_LENGTH            6
#ifdef UAP_SUPPORT
/** Max clients supported by AP */
#define MAX_AP_CLIENTS   10

/** associated station info */
struct ap_client_info
{
    /** STA MAC address */
    t_u8 mac_address[MLAN_MAC_ADDR_LENGTH];
    /** Power mfg status */
    t_u8 power_mfg_status;
    /** RSSI */
    char rssi;
};

/** Type definition of eth_priv_getstalist */
struct eth_priv_getstalist
{
    /** station count */
    t_u16 sta_count;
    /** station list */
    struct ap_client_info client_info[MAX_AP_CLIENTS];
};
#endif

#define COUNTRY_CODE_LEN                3
/** Type definition of eth_priv_countrycode for CMD_COUNTRYCODE */
struct eth_priv_countrycode
{
    /** Country Code */
    t_u8 country_code[COUNTRY_CODE_LEN];
};

/** Type definition of mlan_ds_hs_cfg for MLAN_OID_PM_CFG_HS_CFG */
struct eth_priv_hs_cfg
{
    /** MTRUE to invoke the HostCmd, MFALSE otherwise */
    t_u32 is_invoke_hostcmd;
    /** Host sleep config condition */
    /** Bit0: broadcast data
     *  Bit1: unicast data
     *  Bit2: mac event
     *  Bit3: multicast data 
     */
    t_u32 conditions;
    /** GPIO pin or 0xff for interface */
    t_u32 gpio;
    /** Gap in milliseconds or or 0xff for special setting when GPIO is used to wakeup host */
    t_u32 gap;
};

/** Type definition of eth_priv_scan_time_params */
struct eth_priv_scan_time_params
{
    /** Scan channel time for specific scan in milliseconds */
    t_u32 specific_scan_time;
    /** Scan channel time for active scan in milliseconds */
    t_u32 active_scan_time;
    /** Scan channel time for passive scan in milliseconds */
    t_u32 passive_scan_time;
};

/** Type definition of eth_priv_scan_cfg */
struct eth_priv_scan_cfg
{
    /** Scan type */
    t_u32 scan_type;
    /** BSS mode for scanning */
    t_u32 scan_mode;
    /** Scan probe */
    t_u32 scan_probe;
    /** Scan time parameters */
    struct eth_priv_scan_time_params scan_time;
    /** Extended Scan */
    t_u32 ext_scan;
};

/** Type definition of mlan_power group info */
struct eth_priv_power_group
{
    t_u32 first_rate_ind;
    t_u32 last_rate_ind;
    t_s32 power_min;
    t_s32 power_max;
    t_s32 power_step;
};

    /** max power table size */
#define MAX_POWER_TABLE_SIZE    128

/** Type definition of mlan_power_cfg_ext */
struct eth_priv_power_cfg_ext
{
    /** Length of power_data */
    t_u32 len;
    /** Buffer of power configuration data */
    t_u32 power_data[MAX_POWER_TABLE_SIZE];
};

/** Type definition of eth_priv_ds_ps_cfg */
struct eth_priv_ds_ps_cfg
{
    /** PS null interval in seconds */
    t_u32 ps_null_interval;
    /** Multiple DTIM interval */
    t_u32 multiple_dtim_interval;
    /** Listen interval */
    t_u32 listen_interval;
    /** Adhoc awake period */
    t_u32 adhoc_awake_period;
    /** Beacon miss timeout in milliseconds */
    t_u32 bcn_miss_timeout;
    /** Delay to PS in milliseconds */
    t_s32 delay_to_ps;
    /** PS mode */
    t_u32 ps_mode;
};

#ifdef STA_SUPPORT

/** Maximum length of SSID */
#define MRVDRV_MAX_SSID_LENGTH          32

/** Maximum length of SSID list */
#define MRVDRV_MAX_SSID_LIST_LENGTH         10

/** Maximum number of channels that can be sent in a setuserscan ioctl */
#define WLAN_IOCTL_USER_SCAN_CHAN_MAX  50

/** Maximum channel scratch */
#define MAX_CHAN_SCRATCH  100

/** Maximum channel number for b/g band */
#define MAX_CHAN_BG_BAND  14

/** Maximum number of probes to send on each channel */
#define MAX_PROBES        4

/** Scan all the channels in specified band */
#define BAND_SPECIFIED    0x80

/** Maximum size of IEEE Information Elements */
#define IEEE_MAX_IE_SIZE  256

/** Maximum number of TID */
#define MAX_NUM_TID       8

/** Type enumeration of WMM AC_QUEUES */
typedef enum _mlan_wmm_ac_e
{
    WMM_AC_BK,
    WMM_AC_BE,
    WMM_AC_VI,
    WMM_AC_VO
} __ATTRIB_PACK__ mlan_wmm_ac_e;

/** Enumeration for scan mode */
enum
{
    MLAN_SCAN_MODE_UNCHANGED = 0,
    MLAN_SCAN_MODE_BSS,
    MLAN_SCAN_MODE_IBSS,
    MLAN_SCAN_MODE_ANY
};

/** Enumeration for scan type */
enum
{
    MLAN_SCAN_TYPE_UNCHANGED = 0,
    MLAN_SCAN_TYPE_ACTIVE,
    MLAN_SCAN_TYPE_PASSIVE
};

/** IEEE Type definitions  */
typedef enum _IEEEtypes_ElementId_e
{
    SSID = 0,
    SUPPORTED_RATES = 1,
    FH_PARAM_SET = 2,
    DS_PARAM_SET = 3,
    CF_PARAM_SET = 4,

    IBSS_PARAM_SET = 6,

    COUNTRY_INFO = 7,

    POWER_CONSTRAINT = 32,
    POWER_CAPABILITY = 33,
    TPC_REQUEST = 34,
    TPC_REPORT = 35,
    SUPPORTED_CHANNELS = 36,
    CHANNEL_SWITCH_ANN = 37,
    QUIET = 40,
    IBSS_DFS = 41,
    HT_CAPABILITY = 45,
    HT_OPERATION = 61,
    BSSCO_2040 = 72,
    OVERLAPBSSSCANPARAM = 74,
    EXT_CAPABILITY = 127,

    ERP_INFO = 42,
    EXTENDED_SUPPORTED_RATES = 50,

    VENDOR_SPECIFIC_221 = 221,
    WMM_IE = VENDOR_SPECIFIC_221,

    WPS_IE = VENDOR_SPECIFIC_221,

    WPA_IE = VENDOR_SPECIFIC_221,
    RSN_IE = 48,
} __ATTRIB_PACK__ IEEEtypes_ElementId_e;

/** Capability Bit Map*/
#ifdef BIG_ENDIAN_SUPPORT
typedef struct _IEEEtypes_CapInfo_t
{
    t_u8 rsrvd1:2;
    t_u8 dsss_ofdm:1;
    t_u8 rsvrd2:2;
    t_u8 short_slot_time:1;
    t_u8 rsrvd3:1;
    t_u8 spectrum_mgmt:1;
    t_u8 chan_agility:1;
    t_u8 pbcc:1;
    t_u8 short_preamble:1;
    t_u8 privacy:1;
    t_u8 cf_poll_rqst:1;
    t_u8 cf_pollable:1;
    t_u8 ibss:1;
    t_u8 ess:1;
} __ATTRIB_PACK__ IEEEtypes_CapInfo_t, *pIEEEtypes_CapInfo_t;
#else
typedef struct _IEEEtypes_CapInfo_t
{
    /** Capability Bit Map : ESS */
    t_u8 ess:1;
    /** Capability Bit Map : IBSS */
    t_u8 ibss:1;
    /** Capability Bit Map : CF pollable */
    t_u8 cf_pollable:1;
    /** Capability Bit Map : CF poll request */
    t_u8 cf_poll_rqst:1;
    /** Capability Bit Map : privacy */
    t_u8 privacy:1;
    /** Capability Bit Map : Short preamble */
    t_u8 short_preamble:1;
    /** Capability Bit Map : PBCC */
    t_u8 pbcc:1;
    /** Capability Bit Map : Channel agility */
    t_u8 chan_agility:1;
    /** Capability Bit Map : Spectrum management */
    t_u8 spectrum_mgmt:1;
    /** Capability Bit Map : Reserved */
    t_u8 rsrvd3:1;
    /** Capability Bit Map : Short slot time */
    t_u8 short_slot_time:1;
    /** Capability Bit Map : APSD */
    t_u8 apsd:1;
    /** Capability Bit Map : Reserved */
    t_u8 rsvrd2:1;
    /** Capability Bit Map : DSS OFDM */
    t_u8 dsss_ofdm:1;
    /** Capability Bit Map : Reserved */
    t_u8 rsrvd1:2;
} __ATTRIB_PACK__ IEEEtypes_CapInfo_t, *pIEEEtypes_CapInfo_t;
#endif /* BIG_ENDIAN_SUPPORT */

/** IEEE IE header */
typedef struct _IEEEtypes_Header_t
{
    /** Element ID */
    t_u8 element_id;
    /** Length */
    t_u8 len;
} __ATTRIB_PACK__ IEEEtypes_Header_t, *pIEEEtypes_Header_t;

/** IEEE IE header */
#define IEEE_HEADER_LEN   sizeof(IEEEtypes_Header_t)

/** Vendor specific IE header */
typedef struct _IEEEtypes_VendorHeader_t
{
    /** Element ID */
    t_u8 element_id;
    /** Length */
    t_u8 len;
    /** OUI */
    t_u8 oui[3];
    /** OUI type */
    t_u8 oui_type;
    /** OUI subtype */
    t_u8 oui_subtype;
    /** Version */
    t_u8 version;
} __ATTRIB_PACK__ IEEEtypes_VendorHeader_t, *pIEEEtypes_VendorHeader_t;

/** Vendor specific IE */
typedef struct _IEEEtypes_VendorSpecific_t
{
    /** Vendor specific IE header */
    IEEEtypes_VendorHeader_t vend_hdr;
    /** IE Max - size of previous fields */
    t_u8 data[IEEE_MAX_IE_SIZE - sizeof(IEEEtypes_VendorHeader_t)];
}
__ATTRIB_PACK__ IEEEtypes_VendorSpecific_t, *pIEEEtypes_VendorSpecific_t;

/** IEEE IE */
typedef struct _IEEEtypes_Generic_t
{
    /** Generic IE header */
    IEEEtypes_Header_t ieee_hdr;
    /** IE Max - size of previous fields */
    t_u8 data[IEEE_MAX_IE_SIZE - sizeof(IEEEtypes_Header_t)];
}
__ATTRIB_PACK__ IEEEtypes_Generic_t, *pIEEEtypes_Generic_t;

typedef struct _wlan_get_scan_table_fixed
{
    /** BSSID of this network */
    t_u8 bssid[MLAN_MAC_ADDR_LENGTH];
    /** Channel this beacon/probe response was detected */
    t_u8 channel;
    /** RSSI for the received packet */
    t_u8 rssi;
    /** TSF value from the firmware at packet reception */
    t_u64 network_tsf;
} wlan_get_scan_table_fixed;

/**
 *  Structure passed in the wlan_ioctl_get_scan_table_info for each
 *    BSS returned in the WLAN_GET_SCAN_RESP IOCTL
 */
typedef struct _wlan_ioctl_get_scan_table_entry
{
    /**
     *  Fixed field length included in the response.
     *
     *  Length value is included so future fixed fields can be added to the
     *   response without breaking backwards compatibility.  Use the length
     *   to find the offset for the bssInfoLength field, not a sizeof() calc.
     */
    t_u32 fixed_field_length;

    /**
     *  Length of the BSS Information (probe resp or beacon) that
     *    follows after the fixed_field_length
     */
    t_u32 bss_info_length;

    /**
     *  Always present, fixed length data fields for the BSS
     */
    wlan_get_scan_table_fixed fixed_fields;

    /* 
     *  Probe response or beacon scanned for the BSS.
     *
     *  Field layout:
     *   - TSF              8 octets
     *   - Beacon Interval  2 octets
     *   - Capability Info  2 octets
     *
     *   - IEEE Infomation Elements; variable number & length per 802.11 spec
     */
    /* t_u8 bss_info_buffer[1]; */
} wlan_ioctl_get_scan_table_entry;

/**
 *  Sructure to retrieve the scan table
 */
typedef struct
{
    /**
     *  - Zero based scan entry to start retrieval in command request
     *  - Number of scans entries returned in command response
     */
    t_u32 scan_number;
    /**
     * Buffer marker for multiple wlan_ioctl_get_scan_table_entry structures.
     *   Each struct is padded to the nearest 32 bit boundary.
     */
    t_u8 scan_table_entry_buf[1];

} wlan_ioctl_get_scan_table_info;

typedef struct
{
    t_u8 chan_number;  /**< Channel Number to scan */
    t_u8 radio_type;   /**< Radio type: 'B/G' Band = 0, 'A' Band = 1 */
    t_u8 scan_type;    /**< Scan type: Active = 1, Passive = 2 */
    t_u8 reserved;    /**< Reserved */
    t_u32 scan_time;   /**< Scan duration in milliseconds; if 0 default used */
} __ATTRIB_PACK__ wlan_ioctl_user_scan_chan;

typedef struct
{
    char ssid[MRVDRV_MAX_SSID_LENGTH + 1];  /**< SSID */
    t_u8 max_len;                              /**< Maximum length of SSID */
} __ATTRIB_PACK__ wlan_ioctl_user_scan_ssid;

typedef struct
{

    /** Flag set to keep the previous scan table intact */
    t_u8 keep_previous_scan;    /* Do not erase the existing scan results */

    /** BSS mode to be sent in the firmware command */
    t_u8 bss_mode;

    /** Configure the number of probe requests for active chan scans */
    t_u8 num_probes;

    /** Reserved */
    t_u8 reserved;

    /** BSSID filter sent in the firmware command to limit the results */
    t_u8 specific_bssid[ETH_ALEN];

    /** SSID filter list used in the to limit the scan results */
    wlan_ioctl_user_scan_ssid ssid_list[MRVDRV_MAX_SSID_LIST_LENGTH];

    /** Variable number (fixed maximum) of channels to scan up */
    wlan_ioctl_user_scan_chan chan_list[WLAN_IOCTL_USER_SCAN_CHAN_MAX];

} __ATTRIB_PACK__ wlan_ioctl_user_scan_cfg;

/** Max Ie length */
#define MAX_IE_SIZE             256

/** custom IE */
typedef struct _custom_ie
{
    /** IE Index */
    t_u16 ie_index;
    /** Mgmt Subtype Mask */
    t_u16 mgmt_subtype_mask;
    /** IE Length */
    t_u16 ie_length;
    /** IE buffer */
    t_u8 ie_buffer[MAX_IE_SIZE];
} __ATTRIB_PACK__ custom_ie;

/**
 * Hex or Decimal to Integer
 * @param   num string to convert into decimal or hex
 */
#define A2HEXDECIMAL(num)  \
    (strncasecmp("0x", (num), 2)?(unsigned int) strtoll((num),NULL,0):a2hex((num)))

/** Convert character to integer */
#define CHAR2INT(x) (((x) >= 'A') ? ((x) - 'A' + 10) : ((x) - '0'))

/** Convert TLV header from little endian format to CPU format */
#define endian_convert_tlv_header_in(x)            \
    {                                               \
        (x)->tag = le16_to_cpu((x)->tag);       \
        (x)->length = le16_to_cpu((x)->length); \
    }

/** Convert TLV header to little endian format from CPU format */
#define endian_convert_tlv_header_out(x)            \
    {                                               \
        (x)->tag = cpu_to_le16((x)->tag);       \
        (x)->length = cpu_to_le16((x)->length); \
    }
/** Max IE index to FW */
#define MAX_MGMT_IE_INDEX_TO_FW         4
/** Max IE index per BSS */
#define MAX_MGMT_IE_INDEX               16

/** Private command ID to pass custom IE list */
#define CUSTOM_IE_CFG          (SIOCDEVPRIVATE + 13)
/* TLV Definitions */

/** Maximum IE buffer length */
#define MAX_IE_BUFFER_LEN 256

/** TLV: Management IE list */
#define MRVL_MGMT_IE_LIST_TLV_ID          (PROPRIETARY_TLV_BASE_ID + 0x69)      // 0x0169

/** TLV: Max Management IE */
#define MRVL_MAX_MGMT_IE_TLV_ID           (PROPRIETARY_TLV_BASE_ID + 0xaa)      // 0x01aa

/** custom IE info */
typedef struct _custom_ie_info
{
    /** size of buffer */
    t_u16 buf_size;
    /** no of buffers of buf_size */
    t_u16 buf_count;
} __ATTRIB_PACK__ custom_ie_info;

/** TLV buffer : Max Mgmt IE */
typedef struct _tlvbuf_max_mgmt_ie
{
    /** Type */
    t_u16 type;
    /** Length */
    t_u16 len;
    /** No of tuples */
    t_u16 count;
    /** custom IE info tuples */
    custom_ie_info info[0];
} __ATTRIB_PACK__ tlvbuf_max_mgmt_ie;

/** TLV buffer : custom IE */
typedef struct _eth_priv_ds_misc_custom_ie
{
    /** Type */
    t_u16 type;
    /** Length */
    t_u16 len;
    /** IE data */
    custom_ie ie_data[0];
} __ATTRIB_PACK__ eth_priv_ds_misc_custom_ie;

/*-----------------------------------------------------------------*/
/** Register Memory Access Group */
/*-----------------------------------------------------------------*/
/** Enumeration for register type */
enum _mlan_reg_type
{
    MLAN_REG_MAC = 1,
    MLAN_REG_BBP,
    MLAN_REG_RF,
    MLAN_REG_CAU = 5,
};

/** Type definition of mlan_ds_reg_rw for MLAN_OID_REG_RW */
struct eth_priv_ds_reg_rw
{
    /** Register type */
    t_u32 type;
    /** Offset */
    t_u32 offset;
    /** Value */
    t_u32 value;
};

/** Maximum EEPROM data */
#define MAX_EEPROM_DATA 256

/** Type definition of mlan_ds_read_eeprom for MLAN_OID_EEPROM_RD */
struct eth_priv_ds_read_eeprom
{
    /** Multiples of 4 */
    t_u16 offset;
    /** Number of bytes */
    t_u16 byte_count;
    /** Value */
    t_u8 value[MAX_EEPROM_DATA];
};

/** Type definition of mlan_ds_mem_rw for MLAN_OID_MEM_RW */
struct eth_priv_ds_mem_rw
{
    /** Address */
    t_u32 addr;
    /** Value */
    t_u32 value;
};

/** Type definition of mlan_ds_reg_mem for MLAN_IOCTL_REG_MEM */
struct eth_priv_ds_reg_mem
{
    /** Sub-command */
    t_u32 sub_command;
    /** Register memory access parameter */
    union
    {
        /** Register access for MLAN_OID_REG_RW */
        struct eth_priv_ds_reg_rw reg_rw;
        /** EEPROM access for MLAN_OID_EEPROM_RD */
        struct eth_priv_ds_read_eeprom rd_eeprom;
        /** Memory access for MLAN_OID_MEM_RW */
        struct eth_priv_ds_mem_rw mem_rw;
    } param;
};

/** Data structure of WMM QoS information */
typedef struct _IEEEtypes_WmmQosInfo_t
{
#ifdef BIG_ENDIAN_SUPPORT
    /** QoS UAPSD */
    t_u8 qos_uapsd:1;
    /** Reserved */
    t_u8 reserved:3;
    /** Parameter set count */
    t_u8 para_set_count:4;
#else
    /** Parameter set count */
    t_u8 para_set_count:4;
    /** Reserved */
    t_u8 reserved:3;
    /** QoS UAPSD */
    t_u8 qos_uapsd:1;
#endif                          /* BIG_ENDIAN_SUPPORT */
} __ATTRIB_PACK__ IEEEtypes_WmmQosInfo_t;

/** Size of a TSPEC.  Used to allocate necessary buffer space in commands */
#define WMM_TSPEC_SIZE              63

/** Maximum number of AC QOS queues available in the driver/firmware */
#define MAX_AC_QUEUES               4

/** Maximum number of User Priorities */
#define MAX_USER_PRIORITIES         8

/** Extra IE bytes allocated in messages for appended IEs after a TSPEC */
#define WMM_ADDTS_EXTRA_IE_BYTES    256

/**
 *  @brief Enumeration for the command result from an ADDTS or DELTS command
 */
typedef enum
{
    TSPEC_RESULT_SUCCESS = 0,
    TSPEC_RESULT_EXEC_FAILURE = 1,
    TSPEC_RESULT_TIMEOUT = 2,
    TSPEC_RESULT_DATA_INVALID = 3,
} __ATTRIB_PACK__ mlan_wmm_tspec_result_e;

/**
 *  @brief Enumeration for the action field in the Queue configure command
 */
typedef enum
{
    WMM_QUEUE_CONFIG_ACTION_GET = 0,
    WMM_QUEUE_CONFIG_ACTION_SET = 1,
    WMM_QUEUE_CONFIG_ACTION_DEFAULT = 2,

    WMM_QUEUE_CONFIG_ACTION_MAX
} __ATTRIB_PACK__ mlan_wmm_queue_config_action_e;

/**
 *   @brief Enumeration for the action field in the queue stats command
 */
typedef enum
{
    WMM_STATS_ACTION_START = 0,
    WMM_STATS_ACTION_STOP = 1,
    WMM_STATS_ACTION_GET_CLR = 2,
    WMM_STATS_ACTION_SET_CFG = 3,       /* Not currently used */
    WMM_STATS_ACTION_GET_CFG = 4,       /* Not currently used */

    WMM_STATS_ACTION_MAX
} __ATTRIB_PACK__ mlan_wmm_stats_action_e;

/** Data structure of WMM Aci/Aifsn */
typedef struct _IEEEtypes_WmmAciAifsn_t
{
#ifdef BIG_ENDIAN_SUPPORT
    /** Reserved */
    t_u8 reserved:1;
    /** Aci */
    t_u8 aci:2;
    /** Acm */
    t_u8 acm:1;
    /** Aifsn */
    t_u8 aifsn:4;
#else
    /** Aifsn */
    t_u8 aifsn:4;
    /** Acm */
    t_u8 acm:1;
    /** Aci */
    t_u8 aci:2;
    /** Reserved */
    t_u8 reserved:1;
#endif
} __ATTRIB_PACK__ IEEEtypes_WmmAciAifsn_t, *pIEEEtypes_WmmAciAifsn_t;

/** Data structure of WMM ECW */
typedef struct _IEEEtypes_WmmEcw_t
{
#ifdef BIG_ENDIAN_SUPPORT
    /** Maximum Ecw */
    t_u8 ecw_max:4;
    /** Minimum Ecw */
    t_u8 ecw_min:4;
#else
    /** Minimum Ecw */
    t_u8 ecw_min:4;
    /** Maximum Ecw */
    t_u8 ecw_max:4;
#endif
} __ATTRIB_PACK__ IEEEtypes_WmmEcw_t, *pIEEEtypes_WmmEcw_t;

/** Data structure of WMM AC parameters  */
typedef struct _IEEEtypes_WmmAcParameters_t
{
    IEEEtypes_WmmAciAifsn_t aci_aifsn;      /**< AciAifSn */
    IEEEtypes_WmmEcw_t ecw;                 /**< Ecw */
    t_u16 tx_op_limit;                      /**< Tx op limit */
} __ATTRIB_PACK__ IEEEtypes_WmmAcParameters_t, *pIEEEtypes_WmmAcParameters_t;

/** Data structure of WMM Info IE  */
typedef struct _IEEEtypes_WmmInfo_t
{

    /**
     * WMM Info IE - Vendor Specific Header:
     *   element_id  [221/0xdd]
     *   Len         [7]
     *   Oui         [00:50:f2]
     *   OuiType     [2]
     *   OuiSubType  [0]
     *   Version     [1]
     */
    IEEEtypes_VendorHeader_t vend_hdr;

    /** QoS information */
    IEEEtypes_WmmQosInfo_t qos_info;

} __ATTRIB_PACK__ IEEEtypes_WmmInfo_t, *pIEEEtypes_WmmInfo_t;

/** Data structure of WMM parameter IE  */
typedef struct _IEEEtypes_WmmParameter_t
{
    /**
     * WMM Parameter IE - Vendor Specific Header:
     *   element_id  [221/0xdd]
     *   Len         [24]
     *   Oui         [00:50:f2]
     *   OuiType     [2]
     *   OuiSubType  [1]
     *   Version     [1]
     */
    IEEEtypes_VendorHeader_t vend_hdr;

    /** QoS information */
    IEEEtypes_WmmQosInfo_t qos_info;
    /** Reserved */
    t_u8 reserved;

    /** AC Parameters Record WMM_AC_BE, WMM_AC_BK, WMM_AC_VI, WMM_AC_VO */
    IEEEtypes_WmmAcParameters_t ac_params[MAX_AC_QUEUES];

} __ATTRIB_PACK__ IEEEtypes_WmmParameter_t, *pIEEEtypes_WmmParameter_t;

/**
 *  @brief IOCTL structure to send an ADDTS request and retrieve the response.
 *
 *  IOCTL structure from the application layer relayed to firmware to
 *    instigate an ADDTS management frame with an appropriate TSPEC IE as well
 *    as any additional IEs appended in the ADDTS Action frame.
 *
 *  @sa wlan_wmm_addts_req_ioctl
 */
typedef struct
{
    mlan_wmm_tspec_result_e commandResult; /**< Firmware execution result */

    t_u32 timeout_ms;                   /**< Timeout value in milliseconds */
    t_u8 ieeeStatusCode;                /**< IEEE status code */

    t_u32 ieDataLen;
    t_u8 ieData[WMM_TSPEC_SIZE   /**< TSPEC to send in the ADDTS */
                + WMM_ADDTS_EXTRA_IE_BYTES]; /**< ADDTS extra IE buf */
} wlan_ioctl_wmm_addts_req_t;

/**
 *  @brief IOCTL structure to send a DELTS request.
 *
 *  IOCTL structure from the application layer relayed to firmware to
 *    instigate an DELTS management frame with an appropriate TSPEC IE.
 *
 *  @sa wlan_wmm_delts_req_ioctl
 */
typedef struct
{
    mlan_wmm_tspec_result_e commandResult;  /**< Firmware execution result */
    t_u8 ieeeReasonCode;          /**< IEEE reason code sent, unused for WMM */

    t_u32 ieDataLen;
    t_u8 ieData[WMM_TSPEC_SIZE];  /**< TSPEC to send in the DELTS */
} wlan_ioctl_wmm_delts_req_t;

/**
 *  @brief IOCTL structure to configure a specific AC Queue's parameters
 *
 *  IOCTL structure from the application layer relayed to firmware to
 *    get, set, or default the WMM AC queue parameters.
 *
 *  - msduLifetimeExpiry is ignored if set to 0 on a set command
 *
 *  @sa wlan_wmm_queue_config_ioctl
 */
typedef struct
{
    mlan_wmm_queue_config_action_e action; /**< Set, Get, or Default */
    mlan_wmm_ac_e accessCategory;          /**< WMM_AC_BK(0) to WMM_AC_VO(3) */
    t_u16 msduLifetimeExpiry;              /**< lifetime expiry in TUs */
    t_u8 supportedRates[10];               /**< Not supported yet */
} wlan_ioctl_wmm_queue_config_t;

/** Number of bins in the histogram for the HostCmd_DS_WMM_QUEUE_STATS */
#define WMM_STATS_PKTS_HIST_BINS  7

/**
 *  @brief IOCTL structure to start, stop, and get statistics for a WMM AC
 *
 *  IOCTL structure from the application layer relayed to firmware to
 *    start or stop statistical collection for a given AC.  Also used to
 *    retrieve and clear the collected stats on a given AC.
 *
 *  @sa wlan_wmm_queue_stats_ioctl
 */
typedef struct
{
    mlan_wmm_stats_action_e action;  /**< Start, Stop, or Get  */
    t_u8 userPriority;   /**< User Priority (0 to 7) */
    t_u16 pktCount;      /**< Number of successful packets transmitted */
    t_u16 pktLoss;       /**< Packets lost; not included in pktCount   */
    t_u32 avgQueueDelay; /**< Average Queue delay in microseconds */
    t_u32 avgTxDelay;    /**< Average Transmission delay in microseconds */
    t_u16 usedTime;      /**< Calculated used time - units of 32 microsec */
    t_u16 policedTime;   /**< Calculated policed time - units of 32 microsec */

    /** @brief Queue Delay Histogram; number of packets per queue delay range
     *
     *  [0] -  0ms <= delay < 5ms
     *  [1] -  5ms <= delay < 10ms
     *  [2] - 10ms <= delay < 20ms
     *  [3] - 20ms <= delay < 30ms
     *  [4] - 30ms <= delay < 40ms
     *  [5] - 40ms <= delay < 50ms
     *  [6] - 50ms <= delay < msduLifetime (TUs)
     */
    t_u16 delayHistogram[WMM_STATS_PKTS_HIST_BINS];
} wlan_ioctl_wmm_queue_stats_t;

/**
 *  @brief IOCTL and command sub structure for a Traffic stream status.
 */
typedef struct
{
    t_u8 tid;               /**< TSID: Range: 0->7 */
    t_u8 valid;             /**< TSID specified is valid  */
    t_u8 accessCategory;    /**< AC TSID is active on */
    t_u8 userPriority;      /**< UP specified for the TSID */

    t_u8 psb;               /**< Power save mode for TSID: 0 (legacy), 1 (UAPSD) */
    t_u8 flowDir;           /**< Upstream (0), Downlink(1), Bidirectional(3) */
    t_u16 mediumTime;       /**< Medium time granted for the TSID */
} __ATTRIB_PACK__ HostCmd_DS_WMM_TS_STATUS,
    wlan_ioctl_wmm_ts_status_t, wlan_cmd_wmm_ts_status_t;

/**
 *  @brief IOCTL sub structure for a specific WMM AC Status
 */
typedef struct
{
    /** WMM Acm */
    t_u8 wmmAcm;
    /** Flow required flag */
    t_u8 flowRequired;
    /** Flow created flag */
    t_u8 flowCreated;
    /** Disabled flag */
    t_u8 disabled;
    /** delivery enabled */
    t_u8 deliveryEnabled;
    /** trigger enabled */
    t_u8 triggerEnabled;
} wlan_ioctl_wmm_queue_status_ac_t;

/**
 *  @brief IOCTL structure to retrieve the WMM AC Queue status
 *
 *  IOCTL structure from the application layer to retrieve:
 *     - ACM bit setting for the AC
 *     - Firmware status (flow required, flow created, flow disabled)
 *
 *  @sa wlan_wmm_queue_status_ioctl
 */
typedef struct
{
    /** WMM AC queue status */
    wlan_ioctl_wmm_queue_status_ac_t acStatus[MAX_AC_QUEUES];
} wlan_ioctl_wmm_queue_status_t;
#endif /* STA_SUPPORT */

/** Command RET code, MSB is set to 1 */
#define HostCmd_RET_BIT                 0x8000
/** General purpose action : Get */
#define HostCmd_ACT_GEN_GET             0x0000
/** General purpose action : Set */
#define HostCmd_ACT_GEN_SET             0x0001
/** General purpose action : Clear */
#define HostCmd_ACT_GEN_CLEAR           0x0004
/** General purpose action : Remove */
#define HostCmd_ACT_GEN_REMOVE          0x0004

/** TLV  type ID definition */
#define PROPRIETARY_TLV_BASE_ID     0x0100

/** MrvlIEtypesHeader_t */
typedef struct MrvlIEtypesHeader
{
    /** Header type */
    t_u16 type;
    /** Header length */
    t_u16 len;
} __ATTRIB_PACK__ MrvlIEtypesHeader_t;

/** MrvlIEtypes_Data_t */
typedef struct MrvlIEtypes_Data_t
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** Data */
    t_u8 data[1];
} __ATTRIB_PACK__ MrvlIEtypes_Data_t;

/** HostCmd_DS_802_11_SUBSCRIBE_EVENT */
typedef struct MAPP_HostCmd_DS_802_11_SUBSCRIBE_EVENT
{
    /** Action */
    t_u16 action;
    /** Events */
    t_u16 events;
} __ATTRIB_PACK__ HostCmd_DS_802_11_SUBSCRIBE_EVENT;

/** MrvlIEtypes_RssiParamSet_t */
typedef struct MrvlIEtypes_RssiThreshold
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** RSSI value */
    t_u8 RSSI_value;
    /** RSSI frequency */
    t_u8 RSSI_freq;
} __ATTRIB_PACK__ MrvlIEtypes_RssiThreshold_t;

/** MrvlIEtypes_SnrThreshold_t */
typedef struct MrvlIEtypes_SnrThreshold
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** SNR value */
    t_u8 SNR_value;
    /** SNR frequency */
    t_u8 SNR_freq;
} __ATTRIB_PACK__ MrvlIEtypes_SnrThreshold_t;

/** MrvlIEtypes_FailureCount_t */
typedef struct MrvlIEtypes_FailureCount
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** Failure value */
    t_u8 fail_value;
    /** Failure frequency */
    t_u8 fail_freq;
} __ATTRIB_PACK__ MrvlIEtypes_FailureCount_t;

/** MrvlIEtypes_BeaconsMissed_t */
typedef struct MrvlIEtypes_BeaconsMissed
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** Number of beacons missed */
    t_u8 beacon_missed;
    /** Reserved */
    t_u8 reserved;
} __ATTRIB_PACK__ MrvlIEtypes_BeaconsMissed_t;

/** MrvlIEtypes_LinkQuality_t */
typedef struct MrvlIEtypes_LinkQuality
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** Link SNR threshold */
    t_u16 link_SNR_thrs;
    /** Link SNR frequency */
    t_u16 link_SNR_freq;
    /** Minimum rate value */
    t_u16 min_rate_val;
    /** Minimum rate frequency */
    t_u16 min_rate_freq;
    /** Tx latency value */
    t_u32 tx_latency_val;
    /** Tx latency threshold */
    t_u32 tx_latency_thrs;
} __ATTRIB_PACK__ MrvlIEtypes_LinkQuality_t;

/** Host Command ID : 802.11 subscribe event */
#define HostCmd_CMD_802_11_SUBSCRIBE_EVENT       0x0075

/** TLV type : Beacon RSSI low */
#define TLV_TYPE_RSSI_LOW           (PROPRIETARY_TLV_BASE_ID + 0x04)    // 0x0104
/** TLV type : Beacon SNR low */
#define TLV_TYPE_SNR_LOW            (PROPRIETARY_TLV_BASE_ID + 0x05)    // 0x0105
/** TLV type : Fail count */
#define TLV_TYPE_FAILCOUNT          (PROPRIETARY_TLV_BASE_ID + 0x06)    // 0x0106
/** TLV type : BCN miss */
#define TLV_TYPE_BCNMISS            (PROPRIETARY_TLV_BASE_ID + 0x07)    // 0x0107
/** TLV type : Beacon RSSI high */
#define TLV_TYPE_RSSI_HIGH          (PROPRIETARY_TLV_BASE_ID + 0x16)    // 0x0116
/** TLV type : Beacon SNR high */
#define TLV_TYPE_SNR_HIGH           (PROPRIETARY_TLV_BASE_ID + 0x17)    // 0x0117

/** TLV type :Link Quality */
#define TLV_TYPE_LINK_QUALITY       (PROPRIETARY_TLV_BASE_ID + 0x24)    // 0x0124

/** TLV type : Data RSSI low */
#define TLV_TYPE_RSSI_LOW_DATA      (PROPRIETARY_TLV_BASE_ID + 0x26)    // 0x0126
/** TLV type : Data SNR low */
#define TLV_TYPE_SNR_LOW_DATA       (PROPRIETARY_TLV_BASE_ID + 0x27)    // 0x0127
/** TLV type : Data RSSI high */
#define TLV_TYPE_RSSI_HIGH_DATA     (PROPRIETARY_TLV_BASE_ID + 0x28)    // 0x0128
/** TLV type : Data SNR high */
#define TLV_TYPE_SNR_HIGH_DATA      (PROPRIETARY_TLV_BASE_ID + 0x29)    // 0x0129

/** MrvlIEtypes_PreBeaconLost_t */
typedef struct MrvlIEtypes_PreBeaconLost
{
    /** Header */
    MrvlIEtypesHeader_t header;
    /** Pre-Beacon Lost */
    t_u8 pre_beacon_lost;
    /** Reserved */
    t_u8 reserved;
} __ATTRIB_PACK__ MrvlIEtypes_PreBeaconLost_t;

/** TLV type: Pre-Beacon Lost */
#define TLV_TYPE_PRE_BEACON_LOST    (PROPRIETARY_TLV_BASE_ID + 0x49)    // 0x0149

/** AutoTx_MacFrame_t */
typedef struct AutoTx_MacFrame
{
    t_u16 interval;           /**< in seconds */
    t_u8 priority;            /**< User Priority: 0~7, ignored if non-WMM */
    t_u8 reserved;            /**< set to 0 */
    t_u16 frame_len;           /**< Length of MAC frame payload */
    t_u8 dest_mac_addr[MLAN_MAC_ADDR_LENGTH];           /**< Destination MAC address */
    t_u8 src_mac_addr[MLAN_MAC_ADDR_LENGTH];            /**< Source MAC address */
    t_u8 payload[];                       /**< Payload */
} __ATTRIB_PACK__ AutoTx_MacFrame_t;

/** MrvlIEtypes_AutoTx_t */
typedef struct MrvlIEtypes_AutoTx
{
    MrvlIEtypesHeader_t header;             /**< Header */
    AutoTx_MacFrame_t auto_tx_mac_frame;      /**< Auto Tx MAC frame */
} __ATTRIB_PACK__ MrvlIEtypes_AutoTx_t;

/** HostCmd_DS_802_11_AUTO_TX */
typedef struct MAPP_HostCmd_DS_802_11_AUTO_TX
{
    /** Action */
    t_u16 action;               /* 0 = ACT_GET; 1 = ACT_SET; */
    MrvlIEtypes_AutoTx_t auto_tx;        /**< Auto Tx */
} __ATTRIB_PACK__ HostCmd_DS_802_11_AUTO_TX;

/** Host Command ID : 802.11 auto Tx */
#define HostCmd_CMD_802_11_AUTO_TX          0x0082

/** TLV type : Auto Tx */
#define TLV_TYPE_AUTO_TX            (PROPRIETARY_TLV_BASE_ID + 0x18)    // 0x0118

/** HostCmd_DS_802_11_CFG_DATA */
typedef struct MAPP_HostCmd_DS_802_11_CFG_DATA
{
    /** Action */
    t_u16 action;
    /** Type */
    t_u16 type;
    /** Data length */
    t_u16 data_len;
    /** Data */
    t_u8 data[1];
} __ATTRIB_PACK__ HostCmd_DS_802_11_CFG_DATA;

/** Host Command ID : Configuration data */
#define HostCmd_CMD_CFG_DATA                  0x008f

/** mlan_ioctl_11h_tpc_resp */
typedef struct
{
    int status_code; /**< Firmware command result status code */
    int tx_power;    /**< Reported TX Power from the TPC Report */
    int link_margin; /**< Reported Link margin from the TPC Report */
    int rssi;        /**< RSSI of the received TPC Report frame */
} __ATTRIB_PACK__ mlan_ioctl_11h_tpc_resp;

/** Host Command ID : 802.11 TPC adapt req */
#define HostCmd_CMD_802_11_TPC_ADAPT_REQ      0x0060

/** HostCmd_DS_802_11_CRYPTO */
typedef struct MAPP_HostCmd_DS_802_11_CRYPTO
{
    t_u16 encdec;         /**< Decrypt=0, Encrypt=1 */
    t_u16 algorithm;      /**< RC4=1 AES=2 , AES_KEY_WRAP=3 */
    t_u16 key_IV_length;    /**< Length of Key IV (bytes)   */
    t_u8 keyIV[32];       /**< Key IV */
    t_u16 key_length;      /**< Length of Key (bytes) */
    t_u8 key[32];         /**< Key */
    MrvlIEtypes_Data_t data;   /**< Plain text if encdec=Encrypt, Ciphertext data if encdec=Decrypt*/
} __ATTRIB_PACK__ HostCmd_DS_802_11_CRYPTO;

/** HostCmd_DS_802_11_CRYPTO_AES_CCM */
typedef struct MAPP_HostCmd_DS_802_11_CRYPTO_AES_CCM
{
    t_u16 encdec;         /**< Decrypt=0, Encrypt=1 */
    t_u16 algorithm;      /**< AES_CCM=4 */
    t_u16 key_length;      /**< Length of Key (bytes)  */
    t_u8 key[32];         /**< Key  */
    t_u16 nonce_length;    /**< Length of Nonce (bytes) */
    t_u8 nonce[14];       /**< Nonce */
    t_u16 AAD_length;      /**< Length of AAD (bytes) */
    t_u8 AAD[32];         /**< AAD */
    MrvlIEtypes_Data_t data;   /**< Plain text if encdec=Encrypt, Ciphertext data if encdec=Decrypt*/
} __ATTRIB_PACK__ HostCmd_DS_802_11_CRYPTO_AES_CCM;

/** AES CCM cipher test */
#define CIPHER_TEST_AES_CCM (4)

/** Host Command ID : 802.11 crypto */
#define HostCmd_CMD_802_11_CRYPTO             0x0078

/** Read/Write Mac register */
#define HostCmd_CMD_MAC_REG_ACCESS            0x0019
/** Read/Write BBP register */
#define HostCmd_CMD_BBP_REG_ACCESS            0x001a
/** Read/Write RF register */
#define HostCmd_CMD_RF_REG_ACCESS             0x001b

/** Host Command ID : CAU register access */
#define HostCmd_CMD_CAU_REG_ACCESS            0x00ed

/** Host Command ID : Memory access */
#define HostCmd_CMD_MEM_ACCESS                0x0086

/** Maximum length of lines in configuration file */
#define MAX_CONFIG_LINE                 1024
/** MAC BROADCAST */
#define MAC_BROADCAST   0x1FF
/** MAC MULTICAST */
#define MAC_MULTICAST   0x1FE

/** HostCmd_DS_REG */
typedef struct MAPP_HostCmd_DS_REG
{
    /** Read or write */
    t_u16 action;
    /** Register offset */
    t_u16 offset;
    /** Value */
    t_u32 value;
} __ATTRIB_PACK__ HostCmd_DS_REG;

/** HostCmd_DS_MEM */
typedef struct MAPP_HostCmd_DS_MEM
{
    /** Read or write */
    t_u16 action;
    /** Reserved */
    t_u16 reserved;
    /** Address */
    t_u32 addr;
    /** Value */
    t_u32 value;
} __ATTRIB_PACK__ HostCmd_DS_MEM;

typedef struct _HostCmd_DS_MEF_CFG
{
    /** Criteria */
    t_u32 Criteria;
    /** Number of entries */
    t_u16 NumEntries;
} __ATTRIB_PACK__ HostCmd_DS_MEF_CFG;

typedef struct _MEF_CFG_DATA
{
    /** Size */
    t_u16 size;
    /** Data */
    HostCmd_DS_MEF_CFG data;
} __ATTRIB_PACK__ MEF_CFG_DATA;

/** HostCmd_DS_GEN */
typedef struct MAPP_HostCmd_DS_GEN
{
    /** Command */
    t_u16 command;
    /** Size */
    t_u16 size;
    /** Sequence number */
    t_u16 seq_num;
    /** Result */
    t_u16 result;
} __ATTRIB_PACK__ HostCmd_DS_GEN;

/** Size of HostCmd_DS_GEN */
#define S_DS_GEN    sizeof(HostCmd_DS_GEN)

/** pkt_header */
typedef struct _pkt_header
{
    /** pkt_len */
    t_u32 pkt_len;
    /** pkt_type */
    t_u32 TxPktType;
    /** tx control */
    t_u32 TxControl;
} pkt_header;

/** eth_priv_802_11_header packet from FW with length */
typedef struct _eth_priv_mgmt_frame_tx
{
    /** Packet Length */
    t_u16 frm_len;
    /** Frame Control */
    t_u16 frm_ctl;
    /** Duration ID */
    t_u16 duration_id;
    /** Address1 */
    t_u8 addr1[ETH_ALEN];
    /** Address2 */
    t_u8 addr2[ETH_ALEN];
    /** Address3 */
    t_u8 addr3[ETH_ALEN];
    /** Sequence Control */
    t_u16 seq_ctl;
    /** Address4 */
    t_u8 addr4[ETH_ALEN];
    /** Frame payload */
    t_u8 payload[0];
} __ATTRIB_PACK__ eth_priv_mgmt_frame_tx;

/** frame tx ioctl number */
#define FRAME_TX_IOCTL               (SIOCDEVPRIVATE + 12)

#endif /* _MLANUTL_H_ */
