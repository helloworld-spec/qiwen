/** @file  mlanutl.c
  *
  * @brief Program to control parameters in the mlandriver
  *
  * Usage: mlanutl mlanX cmd [...]
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
     11/04/2011: initial version
************************************************************************/

#include    "mlanutl.h"

/** mlanutl version number */
#define MLANUTL_VER "M1.3"

/** Initial number of total private ioctl calls */
#define IW_INIT_PRIV_NUM    128
/** Maximum number of total private ioctl calls supported */
#define IW_MAX_PRIV_NUM     1024

/** Marvell private command identifier */
#define CMD_MARVELL         "MRVL_CMD"

/********************************************************
        Local Variables
********************************************************/
#define BAND_B       (1U << 0)
#define BAND_G       (1U << 1)
#define BAND_A       (1U << 2)
#define BAND_GN      (1U << 3)
#define BAND_AN      (1U << 4)

static t_s8 *band[] = {
    "B",
    "G",
    "A",
    "GN",
    "AN",
    "GAC",
    "AAC",
};

static t_s8 *adhoc_bw[] = {
    "20 MHz",
    "HT 40 MHz above",
    " ",
    "HT 40 MHz below",
    "VHT 80 MHz",
};

#ifdef DEBUG_LEVEL1
#define MMSG        MBIT(0)
#define MFATAL      MBIT(1)
#define MERROR      MBIT(2)
#define MDATA       MBIT(3)
#define MCMND       MBIT(4)
#define MEVENT      MBIT(5)
#define MINTR       MBIT(6)
#define MIOCTL      MBIT(7)

#define MDAT_D      MBIT(16)
#define MCMD_D      MBIT(17)
#define MEVT_D      MBIT(18)
#define MFW_D       MBIT(19)
#define MIF_D       MBIT(20)

#define MENTRY      MBIT(28)
#define MWARN       MBIT(29)
#define MINFO       MBIT(30)
#define MHEX_DUMP   MBIT(31)
#endif

static int process_version(int argc, char *argv[]);
static int process_bandcfg(int argc, char *argv[]);
static int process_hostcmd(int argc, char *argv[]);
static int process_httxcfg(int argc, char *argv[]);
static int process_htcapinfo(int argc, char *argv[]);
static int process_addbapara(int argc, char *argv[]);
static int process_aggrpriotbl(int argc, char *argv[]);
static int process_addbareject(int argc, char *argv[]);
static int process_delba(int argc, char *argv[]);
static int process_rejectaddbareq(int argc, char *argv[]);
static int process_datarate(int argc, char *argv[]);
static int process_txratecfg(int argc, char *argv[]);
static int process_esuppmode(int argc, char *argv[]);
static int process_passphrase(int argc, char *argv[]);
static int process_deauth(int argc, char *argv[]);
#ifdef UAP_SUPPORT
static int process_getstalist(int argc, char *argv[]);
#endif
#if defined(WIFI_DIRECT_SUPPORT)
#if defined(STA_SUPPORT) && defined(UAP_SUPPORT)
static int process_bssrole(int argc, char *argv[]);
#endif
#endif
#ifdef STA_SUPPORT
static int process_setuserscan(int argc, char *argv[]);
static int process_getscantable(int argc, char *argv[]);
#endif
static int process_deepsleep(int argc, char *argv[]);
static int process_ipaddr(int argc, char *argv[]);
static int process_otpuserdata(int argc, char *argv[]);
static int process_countrycode(int argc, char *argv[]);
static int process_tcpackenh(int argc, char *argv[]);
#ifdef REASSOCIATION
static int process_assocessid(int argc, char *argv[]);
#endif
#ifdef STA_SUPPORT
static int process_listeninterval(int argc, char *argv[]);
#endif
#ifdef DEBUG_LEVEL1
static int process_drvdbg(int argc, char *argv[]);
#endif
static int process_hscfg(int argc, char *argv[]);
static int process_hssetpara(int argc, char *argv[]);
static int process_wakeupresaon(int argc, char *argv[]);
static int process_scancfg(int argc, char *argv[]);
static int process_warmreset(int argc, char *argv[]);
static int process_txpowercfg(int argc, char *argv[]);
static int process_pscfg(int argc, char *argv[]);
static int process_sleeppd(int argc, char *argv[]);
static int process_txcontrol(int argc, char *argv[]);
static int process_customie(int argc, char *argv[]);
static int process_regrdwr(int argc, char *argv[]);
static int process_rdeeprom(int argc, char *argv[]);
static int process_memrdwr(int argc, char *argv[]);
static int process_sdcmd52rw(int argc, char *argv[]);
static int process_mefcfg(int argc, char *argv[]);
#ifdef STA_SUPPORT
static int process_arpfilter(int argc, char *argv[]);
#endif
static int process_cfgdata(int argc, char *argv[]);
static int process_mgmtframetx(int argc, char *argv[]);
static int process_mgmt_frame_passthrough(int argc, char *argv[]);
static int process_qconfig(int argc, char *argv[]);

struct command_node command_list[] = {
    {"version", process_version},
    {"bandcfg", process_bandcfg},
    {"hostcmd", process_hostcmd},
    {"httxcfg", process_httxcfg},
    {"htcapinfo", process_htcapinfo},
    {"addbapara", process_addbapara},
    {"aggrpriotbl", process_aggrpriotbl},
    {"addbareject", process_addbareject},
    {"delba", process_delba},
    {"rejectaddbareq", process_rejectaddbareq},
    {"getdatarate", process_datarate},
    {"txratecfg", process_txratecfg},
    {"esuppmode", process_esuppmode},
    {"passphrase", process_passphrase},
    {"deauth", process_deauth},
#ifdef UAP_SUPPORT
    {"getstalist", process_getstalist},
#endif
#if defined(WIFI_DIRECT_SUPPORT)
#if defined(STA_SUPPORT) && defined(UAP_SUPPORT)
    {"bssrole", process_bssrole},
#endif
#endif
#ifdef STA_SUPPORT
    {"setuserscan", process_setuserscan},
    {"getscantable", process_getscantable},
#endif
    {"deepsleep", process_deepsleep},
    {"ipaddr", process_ipaddr},
    {"otpuserdata", process_otpuserdata},
    {"countrycode", process_countrycode},
    {"tcpackenh", process_tcpackenh},
#ifdef REASSOCIATION
    {"assocessid", process_assocessid},
#endif
#ifdef STA_SUPPORT
    {"listeninterval", process_listeninterval},
#endif
#ifdef DEBUG_LEVEL1
    {"drvdbg", process_drvdbg},
#endif
    {"hscfg", process_hscfg},
    {"hssetpara", process_hssetpara},
    {"wakeupreason", process_wakeupresaon},
    {"scancfg", process_scancfg},
    {"warmreset", process_warmreset},
    {"txpowercfg", process_txpowercfg},
    {"pscfg", process_pscfg},
    {"sleeppd", process_sleeppd},
    {"txcontrol", process_txcontrol},
    {"customie", process_customie},
    {"regrdwr", process_regrdwr},
    {"rdeeprom", process_rdeeprom},
    {"memrdwr", process_memrdwr},
    {"sdcmd52rw", process_sdcmd52rw},
    {"mefcfg", process_mefcfg},
#ifdef STA_SUPPORT
    {"arpfilter", process_arpfilter},
#endif
    {"cfgdata", process_cfgdata},
    {"mgmtframetx", process_mgmtframetx},
    {"mgmtframectrl", process_mgmt_frame_passthrough},
    {"qconfig", process_qconfig},
};

static t_s8 *usage[] = {
    "Usage: ",
    "   mlanutl -v  (version)",
    "   mlanutl <ifname> <cmd> [...]",
    "   where",
    "   ifname : wireless network interface name, such as mlanX or uapX",
    "   cmd : version",
    "         bandcfg",
    "         hostcmd",
    "         httxcfg",
    "         htcapinfo",
    "         addbapara",
    "         aggrpriotbl",
    "         addbareject",
    "         delba",
    "         rejectaddbareq",
    "         getdatarate",
    "         txratecfg",
    "         esuppmode",
    "         passphrase",
    "         deauth",
#ifdef UAP_SUPPORT
    "         getstalist",
#endif
#if defined(WIFI_DIRECT_SUPPORT)
#if defined(STA_SUPPORT) && defined(UAP_SUPPORT)
    "         bssrole",
#endif
#endif
#ifdef STA_SUPPORT
    "         setuserscan",
    "         getscantable",
#endif
    "         deepsleep",
    "         ipaddr",
    "         otpuserdata",
    "         countrycode",
    "         tcpackenh",
#ifdef REASSOCIATION
    "         assocessid",
#endif
#ifdef STA_SUPPORT
    "         listeninterval",
#endif
#ifdef DEBUG_LEVEL1
    "         drvdbg",
#endif
    "         hscfg",
    "         hssetpara",
    "         scancfg",
    "         warmreset",
    "         txpowercfg",
    "         pscfg",
    "         sleeppd",
    "         txcontrol",
    "         customie",
    "         regrdwr",
    "         rdeeprom",
    "         memrdwr",
    "         sdcmd52rw",
    "         mefcfg",
#ifdef STA_SUPPORT
    "         arpfilter",
#endif
    "         cfgdata",
    "         mgmtframetx",
    "         mgmtframectrl",
};

/** Socket */
t_s32 sockfd;
/** Device name */
t_s8 dev_name[IFNAMSIZ + 1];
#define HOSTCMD "hostcmd"

/********************************************************
        Global Variables
********************************************************/

/********************************************************
        Local Functions
********************************************************/

/**
 *    @brief isdigit for String.
 *
 *    @param x            Char string
 *    @return             MLAN_STATUS_FAILURE for non-digit.
 *                        MLAN_STATUS_SUCCESS for digit
 */
static int
ISDIGIT(t_s8 * x)
{
    unsigned int i;
    for (i = 0; i < strlen(x); i++)
        if (isdigit(x[i]) == 0)
            return MLAN_STATUS_FAILURE;
    return MLAN_STATUS_SUCCESS;
}

/**
 * Check of decimal or hex string
 * @param   num string
 */
#define IS_HEX_OR_DIGIT(num) \
    (strncasecmp("0x", (num), 2)?ISDIGIT((num)):ishexstring((num)))

/**
 *  @brief Convert char to hex integer
 *
 *  @param chr      Char to convert
 *  @return         Hex integer or 0
 */
int
hexval(t_s32 chr)
{
    if (chr >= '0' && chr <= '9')
        return chr - '0';
    if (chr >= 'A' && chr <= 'F')
        return chr - 'A' + 10;
    if (chr >= 'a' && chr <= 'f')
        return chr - 'a' + 10;

    return 0;
}

/**
 *  @brief Hump hex data
 *
 *  @param prompt   A pointer prompt buffer
 *  @param p        A pointer to data buffer
 *  @param len      The len of data buffer
 *  @param delim    Delim char
 *  @return         Hex integer
 */
t_void
hexdump(t_s8 * prompt, t_void * p, t_s32 len, t_s8 delim)
{
    t_s32 i;
    t_u8 *s = p;

    if (prompt) {
        printf("%s: len=%d\n", prompt, (int) len);
    }
    for (i = 0; i < len; i++) {
        if (i != len - 1)
            printf("%02x%c", *s++, delim);
        else
            printf("%02x\n", *s);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

/**
 *  @brief Convert char to hex integer
 *
 *  @param chr      Char
 *  @return         Hex integer
 */
t_u8
hexc2bin(t_s8 chr)
{
    if (chr >= '0' && chr <= '9')
        chr -= '0';
    else if (chr >= 'A' && chr <= 'F')
        chr -= ('A' - 10);
    else if (chr >= 'a' && chr <= 'f')
        chr -= ('a' - 10);

    return chr;
}

/**
 *  @brief Convert string to hex integer
 *
 *  @param s        A pointer string buffer
 *  @return         Hex integer
 */
t_u32
a2hex(t_s8 * s)
{
    t_u32 val = 0;

    if (!strncasecmp("0x", s, 2)) {
        s += 2;
    }

    while (*s && isxdigit(*s)) {
        val = (val << 4) + hexc2bin(*s++);
    }

    return val;
}

/*
 *  @brief Convert String to integer
 *
 *  @param value    A pointer to string
 *  @return         Integer
 */
t_u32
a2hex_or_atoi(t_s8 * value)
{
    if (value[0] == '0' && (value[1] == 'X' || value[1] == 'x')) {
        return a2hex(value + 2);
    } else if (isdigit(*value)) {
        return atoi(value);
    } else {
        return *value;
    }
}

/**
 *  @brief Convert string to hex
 *
 *  @param ptr      A pointer to data buffer
 *  @param chr      A pointer to return integer
 *  @return         A pointer to next data field
 */
t_s8 *
convert2hex(t_s8 * ptr, t_u8 * chr)
{
    t_u8 val;

    for (val = 0; *ptr && isxdigit(*ptr); ptr++) {
        val = (val * 16) + hexval(*ptr);
    }

    *chr = val;

    return ptr;
}

/**
 *  @brief Check the Hex String
 *  @param s  A pointer to the string
 *  @return   MLAN_STATUS_SUCCESS --HexString, MLAN_STATUS_FAILURE --not HexString
 */
int
ishexstring(t_s8 * s)
{
    int ret = MLAN_STATUS_FAILURE;
    t_s32 tmp;

    if (!strncasecmp("0x", s, 2)) {
        s += 2;
    }
    while (*s) {
        tmp = toupper(*s);
        if (((tmp >= 'A') && (tmp <= 'F')) || ((tmp >= '0') && (tmp <= '9'))) {
            ret = MLAN_STATUS_SUCCESS;
        } else {
            ret = MLAN_STATUS_FAILURE;
            break;
        }
        s++;
    }

    return ret;
}

/**
 *  @brief Convert String to Integer
 *  @param buf      A pointer to the string
 *  @return         Integer
 */
int
atoval(t_s8 * buf)
{
    if (!strncasecmp(buf, "0x", 2))
        return a2hex(buf + 2);
    else if (!ishexstring(buf))
        return a2hex(buf);
    else
        return atoi(buf);
}

/**
 *  @brief Display usage
 *
 *  @return       NA
 */
static t_void
display_usage(t_void)
{
    t_u32 i;
    for (i = 0; i < NELEMENTS(usage); i++)
        fprintf(stderr, "%s\n", usage[i]);
}

/**
 *  @brief Find and execute command
 *
 *  @param argc     Number of arguments
 *  @param argv     A pointer to arguments array
 *  @return         MLAN_STATUS_SUCCESS for success, otherwise failure
 */
static int
process_command(int argc, char *argv[])
{
    int i = 0, ret = MLAN_STATUS_FAILURE;
    struct command_node *node = NULL;

    for (i = 0; i < NELEMENTS(command_list); i++) {
        node = &command_list[i];
        if (!strcasecmp(node->name, argv[2])) {
            ret = node->handler(argc, argv);
            break;
        }
    }

    return ret;
}

/**
 *  @brief Prepare command buffer
 *  @param buffer   Command buffer to be filled
 *  @param cmd      Command id
 *  @param num      Number of arguments
 *  @param args     Arguments list
 *  @return         MLAN_STATUS_SUCCESS
 */
static int
prepare_buffer(t_u8 * buffer, t_s8 * cmd, t_u32 num, char *args[])
{
    t_u8 *pos = NULL;
    unsigned int i = 0;

    memset(buffer, 0, BUFFER_LENGTH);

    /* Flag it for our use */
    pos = buffer;
    strncpy((char *) pos, CMD_MARVELL, strlen(CMD_MARVELL));
    pos += (strlen(CMD_MARVELL));

    /* Insert command */
    strncpy((char *) pos, (char *) cmd, strlen(cmd));
    pos += (strlen(cmd));

    /* Insert arguments */
    for (i = 0; i < num; i++) {
        strncpy((char *) pos, args[i], strlen(args[i]));
        pos += strlen(args[i]);
        if (i < (num - 1)) {
            strncpy((char *) pos, " ", strlen(" "));
            pos += 1;
        }
    }

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Get one line from the File
 *
 *  @param fp       File handler
 *  @param str      Storage location for data.
 *  @param size     Maximum number of characters to read.
 *  @param lineno   A pointer to return current line number
 *  @return         returns string or NULL
 */
char *
mlan_config_get_line(FILE * fp, t_s8 * str, t_s32 size, int *lineno)
{
    char *start, *end;
    int out, next_line;

    if (!fp || !str)
        return NULL;

    do {
      read_line:
        if (!fgets(str, size, fp))
            break;
        start = str;
        start[size - 1] = '\0';
        end = start + strlen(str);
        (*lineno)++;

        out = 1;
        while (out && (start < end)) {
            next_line = 0;
            /* Remove empty lines and lines starting with # */
            switch (start[0]) {
            case ' ':          /* White space */
            case '\t':         /* Tab */
                start++;
                break;
            case '#':
            case '\n':
            case '\0':
                next_line = 1;
                break;
            case '\r':
                if (start[1] == '\n')
                    next_line = 1;
                else
                    start++;
                break;
            default:
                out = 0;
                break;
            }
            if (next_line)
                goto read_line;
        }

        /* Remove # comments unless they are within a double quoted string.
           Remove trailing white space. */
        if ((end = strstr(start, "\""))) {
            if (!(end = strstr(end + 1, "\"")))
                end = start;
        } else
            end = start;

        if ((end = strstr(end + 1, "#")))
            *end-- = '\0';
        else
            end = start + strlen(start) - 1;

        out = 1;
        while (out && (start < end)) {
            switch (*end) {
            case ' ':          /* White space */
            case '\t':         /* Tab */
            case '\n':
            case '\r':
                *end = '\0';
                end--;
                break;
            default:
                out = 0;
                break;
            }
        }

        if (start == '\0')
            continue;

        return start;
    } while (1);

    return NULL;
}

/** 
 *  @brief Converts colon separated MAC address to hex value
 *
 *  @param mac      A pointer to the colon separated MAC string
 *  @param raw      A pointer to the hex data buffer
 *  @return         MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 *                  MAC_BROADCAST   - if broadcast mac
 *                  MAC_MULTICAST   - if multicast mac
 */
static int
mac2raw(char *mac, t_u8 * raw)
{
    unsigned int temp_raw[ETH_ALEN];
    int num_tokens = 0;
    int i;

    if (strlen(mac) != ((2 * ETH_ALEN) + (ETH_ALEN - 1))) {
        return MLAN_STATUS_FAILURE;
    }
    num_tokens = sscanf(mac, "%2x:%2x:%2x:%2x:%2x:%2x",
                        temp_raw + 0, temp_raw + 1, temp_raw + 2, temp_raw + 3,
                        temp_raw + 4, temp_raw + 5);
    if (num_tokens != ETH_ALEN) {
        return MLAN_STATUS_FAILURE;
    }
    for (i = 0; i < num_tokens; i++)
        raw[i] = (t_u8) temp_raw[i];

    if (memcmp(raw, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0) {
        return MAC_BROADCAST;
    } else if (raw[0] & 0x01) {
        return MAC_MULTICAST;
    }
    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief          Parse function for a configuration line  
 *
 *  @param s        Storage buffer for data
 *  @param size     Maximum size of data
 *  @param stream   File stream pointer
 *  @param line     Pointer to current line within the file
 *  @param _pos     Output string or NULL
 *  @return         String or NULL
 */
static char *
config_get_line(char *s, int size, FILE * stream, int *line, char **_pos)
{
    *_pos = mlan_config_get_line(stream, s, size, line);
    return *_pos;
}

/** 
 *  @brief Parses a command line
 *
 *  @param line     The line to parse
 *  @param args     Pointer to the argument buffer to be filled in
 *  @return         Number of arguments in the line or EOF
 */
static int
parse_line(char *line, char *args[])
{
    int arg_num = 0;
    int is_start = 0;
    int is_quote = 0;
    int length = 0;
    int i = 0;

    arg_num = 0;
    length = strlen(line);
    /* Process line */

    /* Find number of arguments */
    is_start = 0;
    is_quote = 0;
    for (i = 0; i < length; i++) {
        /* Ignore leading spaces */
        if (is_start == 0) {
            if (line[i] == ' ') {
                continue;
            } else if (line[i] == '\t') {
                continue;
            } else if (line[i] == '\n') {
                break;
            } else {
                is_start = 1;
                args[arg_num] = &line[i];
                arg_num++;
            }
        }
        if (is_start == 1) {
            /* Ignore comments */
            if (line[i] == '#') {
                if (is_quote == 0) {
                    line[i] = '\0';
                    arg_num--;
                }
                break;
            }
            /* Separate by '=' */
            if (line[i] == '=') {
                line[i] = '\0';
                is_start = 0;
                continue;
            }
            /* Separate by ',' */
            if (line[i] == ',') {
                line[i] = '\0';
                is_start = 0;
                continue;
            }
            /* Change ',' to ' ', but not inside quotes */
            if ((line[i] == ',') && (is_quote == 0)) {
                line[i] = ' ';
                continue;
            }
        }
        /* Remove newlines */
        if (line[i] == '\n') {
            line[i] = '\0';
        }
        /* Check for quotes */
        if (line[i] == '"') {
            is_quote = (is_quote == 1) ? 0 : 1;
            continue;
        }
        if (((line[i] == ' ') || (line[i] == '\t')) && (is_quote == 0)) {
            line[i] = '\0';
            is_start = 0;
            continue;
        }
    }
    return arg_num;
}

/**
 *  @brief Process version
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_version(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: version fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Version string received: %s\n", cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process band configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_bandcfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    int i;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_bandcfg *bandcfg = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: bandcfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    bandcfg = (struct eth_priv_bandcfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Band Configuration:\n");
        printf("  Infra Band: %d (", (int) bandcfg->config_bands);
        for (i = 0; i < 7; i++) {
            if ((bandcfg->config_bands >> i) & 0x1)
                printf(" %s", band[i]);
        }
        printf(" )\n");
        printf("  Adhoc Start Band: %d (", (int) bandcfg->adhoc_start_band);
        for (i = 0; i < 7; i++) {
            if ((bandcfg->adhoc_start_band >> i) & 0x1)
                printf(" %s", band[i]);
        }
        printf(" )\n");
        printf("  Adhoc Start Channel: %d\n", (int) bandcfg->adhoc_channel);
        printf("  Adhoc Band Width: %d (%s)\n",
               (int) bandcfg->adhoc_chan_bandwidth,
               adhoc_bw[bandcfg->adhoc_chan_bandwidth]);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief get hostcmd data
 *
 *  @param ln           A pointer to line number
 *  @param buf          A pointer to hostcmd data
 *  @param size         A pointer to the return size of hostcmd buffer
 *  @return             MLAN_STATUS_SUCCESS
 */
static int
mlan_get_hostcmd_data(FILE * fp, int *ln, t_u8 * buf, t_u16 * size)
{
    t_s32 errors = 0, i;
    t_s8 line[256], *pos, *pos1, *pos2, *pos3;
    t_u16 len;

    while ((pos = mlan_config_get_line(fp, line, sizeof(line), ln))) {
        (*ln)++;
        if (strcmp(pos, "}") == 0) {
            break;
        }

        pos1 = strchr(pos, ':');
        if (pos1 == NULL) {
            printf("Line %d: Invalid hostcmd line '%s'\n", *ln, pos);
            errors++;
            continue;
        }
        *pos1++ = '\0';

        pos2 = strchr(pos1, '=');
        if (pos2 == NULL) {
            printf("Line %d: Invalid hostcmd line '%s'\n", *ln, pos);
            errors++;
            continue;
        }
        *pos2++ = '\0';

        len = a2hex_or_atoi(pos1);
        if (len < 1 || len > BUFFER_LENGTH) {
            printf("Line %d: Invalid hostcmd line '%s'\n", *ln, pos);
            errors++;
            continue;
        }

        *size += len;

        if (*pos2 == '"') {
            pos2++;
            if ((pos3 = strchr(pos2, '"')) == NULL) {
                printf("Line %d: invalid quotation '%s'\n", *ln, pos);
                errors++;
                continue;
            }
            *pos3 = '\0';
            memset(buf, 0, len);
            memmove(buf, pos2, MIN(strlen(pos2), len));
            buf += len;
        } else if (*pos2 == '\'') {
            pos2++;
            if ((pos3 = strchr(pos2, '\'')) == NULL) {
                printf("Line %d: invalid quotation '%s'\n", *ln, pos);
                errors++;
                continue;
            }
            *pos3 = ',';
            for (i = 0; i < len; i++) {
                if ((pos3 = strchr(pos2, ',')) != NULL) {
                    *pos3 = '\0';
                    *buf++ = (t_u8) a2hex_or_atoi(pos2);
                    pos2 = pos3 + 1;
                } else
                    *buf++ = 0;
            }
        } else if (*pos2 == '{') {
            t_u16 tlvlen = 0, tmp_tlvlen;
            mlan_get_hostcmd_data(fp, ln, buf + len, &tlvlen);
            tmp_tlvlen = tlvlen;
            while (len--) {
                *buf++ = (t_u8) (tmp_tlvlen & 0xff);
                tmp_tlvlen >>= 8;
            }
            *size += tlvlen;
            buf += tlvlen;
        } else {
            t_u32 value = a2hex_or_atoi(pos2);
            while (len--) {
                *buf++ = (t_u8) (value & 0xff);
                value >>= 8;
            }
        }
    }
    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Prepare host-command buffer
 *  @param fp       File handler
 *  @param cmd_name Command name
 *  @param buf      A pointer to comand buffer
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
prepare_host_cmd_buffer(FILE * fp, char *cmd_name, t_u8 * buf)
{
    t_s8 line[256], cmdname[256], *pos, cmdcode[10];
    HostCmd_DS_GEN *hostcmd;
    t_u32 hostcmd_size = 0;
    int ln = 0;
    int cmdname_found = 0, cmdcode_found = 0;

    hostcmd = (HostCmd_DS_GEN *) (buf + sizeof(t_u32));
    hostcmd->command = 0xffff;

    snprintf(cmdname, sizeof(cmdname), "%s={", cmd_name);
    cmdname_found = 0;
    while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
        if (strcmp(pos, cmdname) == 0) {
            cmdname_found = 1;
            snprintf(cmdcode, sizeof(cmdcode), "CmdCode=");
            cmdcode_found = 0;
            while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
                if (strncmp(pos, cmdcode, strlen(cmdcode)) == 0) {
                    cmdcode_found = 1;
                    hostcmd->command = a2hex_or_atoi(pos + strlen(cmdcode));
                    hostcmd->size = S_DS_GEN;
                    mlan_get_hostcmd_data(fp, &ln,
                                          buf + sizeof(t_u32) + hostcmd->size,
                                          &hostcmd->size);
                    break;
                }
            }
            if (!cmdcode_found) {
                fprintf(stderr, "mlanutl: CmdCode not found in conf file\n");
                return MLAN_STATUS_FAILURE;
            }
            break;
        }
    }

    if (!cmdname_found) {
        fprintf(stderr, "mlanutl: cmdname '%s' is not found in conf file\n",
                cmd_name);
        return MLAN_STATUS_FAILURE;
    }

    hostcmd->seq_num = 0;
    hostcmd->result = 0;
    hostcmd->command = cpu_to_le16(hostcmd->command);
    hostcmd->size = cpu_to_le16(hostcmd->size);

    hostcmd_size = (t_u32) (hostcmd->size);
    memcpy(buf, (t_u8 *) & hostcmd_size, sizeof(t_u32));

    return MLAN_STATUS_SUCCESS;
}

#if defined(UAP_SUPPORT)
/**
 *  @brief Prints a MAC address in colon separated form from raw data
 *
 *  @param raw      A pointer to the hex data buffer
 *  @return         N/A
 */
void
print_mac(t_u8 * raw)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x", (unsigned int) raw[0],
           (unsigned int) raw[1], (unsigned int) raw[2], (unsigned int) raw[3],
           (unsigned int) raw[4], (unsigned int) raw[5]);
    return;
}
#endif

/**
 *  @brief Process host_cmd response
 *  @param buf      A pointer to the response buffer
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
process_host_cmd_resp(char *cmd_name, t_u8 * buf)
{
    t_u32 hostcmd_size = 0;
    HostCmd_DS_GEN *hostcmd = NULL;
    int ret = MLAN_STATUS_SUCCESS;

    buf += strlen(CMD_MARVELL) + strlen(cmd_name);
    memcpy((t_u8 *) & hostcmd_size, buf, sizeof(t_u32));
    buf += sizeof(t_u32);

    hostcmd = (HostCmd_DS_GEN *) buf;
    hostcmd->command = le16_to_cpu(hostcmd->command);
    hostcmd->size = le16_to_cpu(hostcmd->size);
    hostcmd->seq_num = le16_to_cpu(hostcmd->seq_num);
    hostcmd->result = le16_to_cpu(hostcmd->result);

    hostcmd->command &= ~HostCmd_RET_BIT;
    if (!hostcmd->result) {
        switch (hostcmd->command) {
        case HostCmd_CMD_CFG_DATA:
            {
                HostCmd_DS_802_11_CFG_DATA *pstcfgData =
                    (HostCmd_DS_802_11_CFG_DATA *) (buf + S_DS_GEN);
                pstcfgData->data_len = le16_to_cpu(pstcfgData->data_len);
                pstcfgData->action = le16_to_cpu(pstcfgData->action);

                if (pstcfgData->action == HostCmd_ACT_GEN_GET) {
                    hexdump("cfgdata", pstcfgData->data, pstcfgData->data_len,
                            ' ');
                }
                break;
            }
        case HostCmd_CMD_802_11_TPC_ADAPT_REQ:
            {
                mlan_ioctl_11h_tpc_resp *tpcIoctlResp =
                    (mlan_ioctl_11h_tpc_resp *) (buf + S_DS_GEN);
                if (tpcIoctlResp->status_code == 0) {
                    printf
                        ("tpcrequest:  txPower(%d), linkMargin(%d), rssi(%d)\n",
                         tpcIoctlResp->tx_power, tpcIoctlResp->link_margin,
                         tpcIoctlResp->rssi);
                } else {
                    printf("tpcrequest:  failure, status = %d\n",
                           tpcIoctlResp->status_code);
                }
                break;
            }
        case HostCmd_CMD_802_11_CRYPTO:
            {
                t_u16 alg =
                    le16_to_cpu((t_u16) * (buf + S_DS_GEN + sizeof(t_u16)));
                if (alg != CIPHER_TEST_AES_CCM) {
                    HostCmd_DS_802_11_CRYPTO *cmd =
                        (HostCmd_DS_802_11_CRYPTO *) (buf + S_DS_GEN);
                    cmd->encdec = le16_to_cpu(cmd->encdec);
                    cmd->algorithm = le16_to_cpu(cmd->algorithm);
                    cmd->key_IV_length = le16_to_cpu(cmd->key_IV_length);
                    cmd->key_length = le16_to_cpu(cmd->key_length);
                    cmd->data.header.type = le16_to_cpu(cmd->data.header.type);
                    cmd->data.header.len = le16_to_cpu(cmd->data.header.len);

                    printf("crypto_result: encdec=%d algorithm=%d,KeyIVLen=%d,"
                           " KeyLen=%d,dataLen=%d\n",
                           cmd->encdec, cmd->algorithm, cmd->key_IV_length,
                           cmd->key_length, cmd->data.header.len);
                    hexdump("KeyIV", cmd->keyIV, cmd->key_IV_length, ' ');
                    hexdump("Key", cmd->key, cmd->key_length, ' ');
                    hexdump("Data", cmd->data.data, cmd->data.header.len, ' ');
                } else {
                    HostCmd_DS_802_11_CRYPTO_AES_CCM *cmd_aes_ccm =
                        (HostCmd_DS_802_11_CRYPTO_AES_CCM *) (buf + S_DS_GEN);

                    cmd_aes_ccm->encdec = le16_to_cpu(cmd_aes_ccm->encdec);
                    cmd_aes_ccm->algorithm
                        = le16_to_cpu(cmd_aes_ccm->algorithm);
                    cmd_aes_ccm->key_length
                        = le16_to_cpu(cmd_aes_ccm->key_length);
                    cmd_aes_ccm->nonce_length
                        = le16_to_cpu(cmd_aes_ccm->nonce_length);
                    cmd_aes_ccm->AAD_length
                        = le16_to_cpu(cmd_aes_ccm->AAD_length);
                    cmd_aes_ccm->data.header.type
                        = le16_to_cpu(cmd_aes_ccm->data.header.type);
                    cmd_aes_ccm->data.header.len
                        = le16_to_cpu(cmd_aes_ccm->data.header.len);

                    printf("crypto_result: encdec=%d algorithm=%d, KeyLen=%d,"
                           " NonceLen=%d,AADLen=%d,dataLen=%d\n",
                           cmd_aes_ccm->encdec,
                           cmd_aes_ccm->algorithm,
                           cmd_aes_ccm->key_length,
                           cmd_aes_ccm->nonce_length,
                           cmd_aes_ccm->AAD_length,
                           cmd_aes_ccm->data.header.len);

                    hexdump("Key", cmd_aes_ccm->key, cmd_aes_ccm->key_length,
                            ' ');
                    hexdump("Nonce", cmd_aes_ccm->nonce,
                            cmd_aes_ccm->nonce_length, ' ');
                    hexdump("AAD", cmd_aes_ccm->AAD, cmd_aes_ccm->AAD_length,
                            ' ');
                    hexdump("Data", cmd_aes_ccm->data.data,
                            cmd_aes_ccm->data.header.len, ' ');
                }
                break;
            }
        case HostCmd_CMD_802_11_AUTO_TX:
            {
                HostCmd_DS_802_11_AUTO_TX *at =
                    (HostCmd_DS_802_11_AUTO_TX *) (buf + S_DS_GEN);

                if (le16_to_cpu(at->action) == HostCmd_ACT_GEN_GET) {
                    if (S_DS_GEN + sizeof(at->action) == hostcmd->size) {
                        printf("auto_tx not configured\n");

                    } else {
                        MrvlIEtypesHeader_t *header = &at->auto_tx.header;

                        header->type = le16_to_cpu(header->type);
                        header->len = le16_to_cpu(header->len);

                        if ((S_DS_GEN + sizeof(at->action)
                             + sizeof(MrvlIEtypesHeader_t)
                             + header->len == hostcmd->size) &&
                            (header->type == TLV_TYPE_AUTO_TX)) {

                            AutoTx_MacFrame_t *atmf
                                = &at->auto_tx.auto_tx_mac_frame;

                            printf("Interval: %d second(s)\n",
                                   le16_to_cpu(atmf->interval));
                            printf("Priority: %#x\n", atmf->priority);
                            printf("Frame Length: %d\n",
                                   le16_to_cpu(atmf->frame_len));
                            printf("Dest Mac Address: "
                                   "%02x:%02x:%02x:%02x:%02x:%02x\n",
                                   atmf->dest_mac_addr[0],
                                   atmf->dest_mac_addr[1],
                                   atmf->dest_mac_addr[2],
                                   atmf->dest_mac_addr[3],
                                   atmf->dest_mac_addr[4],
                                   atmf->dest_mac_addr[5]);
                            printf("Src Mac Address: "
                                   "%02x:%02x:%02x:%02x:%02x:%02x\n",
                                   atmf->src_mac_addr[0],
                                   atmf->src_mac_addr[1],
                                   atmf->src_mac_addr[2],
                                   atmf->src_mac_addr[3],
                                   atmf->src_mac_addr[4],
                                   atmf->src_mac_addr[5]);

                            hexdump("Frame Payload", atmf->payload,
                                    le16_to_cpu(atmf->frame_len)
                                    - MLAN_MAC_ADDR_LENGTH * 2, ' ');
                        } else {
                            printf("incorrect auto_tx command response\n");
                        }
                    }
                }
                break;
            }
        case HostCmd_CMD_802_11_SUBSCRIBE_EVENT:
            {
                HostCmd_DS_802_11_SUBSCRIBE_EVENT *se =
                    (HostCmd_DS_802_11_SUBSCRIBE_EVENT *) (buf + S_DS_GEN);
                if (le16_to_cpu(se->action) == HostCmd_ACT_GEN_GET) {
                    int len =
                        S_DS_GEN + sizeof(HostCmd_DS_802_11_SUBSCRIBE_EVENT);
                    printf("\nEvent\t\tValue\tFreq\tsubscribed\n\n");
                    while (len < hostcmd->size) {
                        MrvlIEtypesHeader_t *header =
                            (MrvlIEtypesHeader_t *) (buf + len);
                        switch (le16_to_cpu(header->type)) {
                        case TLV_TYPE_RSSI_LOW:
                            {
                                MrvlIEtypes_RssiThreshold_t *low_rssi =
                                    (MrvlIEtypes_RssiThreshold_t *) (buf + len);
                                printf("Beacon Low RSSI\t%d\t%d\t%s\n",
                                       low_rssi->RSSI_value,
                                       low_rssi->RSSI_freq,
                                       (le16_to_cpu(se->events) & 0x0001) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_SNR_LOW:
                            {
                                MrvlIEtypes_SnrThreshold_t *low_snr =
                                    (MrvlIEtypes_SnrThreshold_t *) (buf + len);
                                printf("Beacon Low SNR\t%d\t%d\t%s\n",
                                       low_snr->SNR_value,
                                       low_snr->SNR_freq,
                                       (le16_to_cpu(se->events) & 0x0002) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_FAILCOUNT:
                            {
                                MrvlIEtypes_FailureCount_t *failure_count =
                                    (MrvlIEtypes_FailureCount_t *) (buf + len);
                                printf("Failure Count\t%d\t%d\t%s\n",
                                       failure_count->fail_value,
                                       failure_count->fail_freq,
                                       (le16_to_cpu(se->events) & 0x0004) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_BCNMISS:
                            {
                                MrvlIEtypes_BeaconsMissed_t *bcn_missed =
                                    (MrvlIEtypes_BeaconsMissed_t *) (buf + len);
                                printf("Beacon Missed\t%d\tN/A\t%s\n",
                                       bcn_missed->beacon_missed,
                                       (le16_to_cpu(se->events) & 0x0008) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_RSSI_HIGH:
                            {
                                MrvlIEtypes_RssiThreshold_t *high_rssi =
                                    (MrvlIEtypes_RssiThreshold_t *) (buf + len);
                                printf("Bcn High RSSI\t%d\t%d\t%s\n",
                                       high_rssi->RSSI_value,
                                       high_rssi->RSSI_freq,
                                       (le16_to_cpu(se->events) & 0x0010) ?
                                       "yes" : "no");
                                break;
                            }

                        case TLV_TYPE_SNR_HIGH:
                            {
                                MrvlIEtypes_SnrThreshold_t *high_snr =
                                    (MrvlIEtypes_SnrThreshold_t *) (buf + len);
                                printf("Beacon High SNR\t%d\t%d\t%s\n",
                                       high_snr->SNR_value,
                                       high_snr->SNR_freq,
                                       (le16_to_cpu(se->events) & 0x0020) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_RSSI_LOW_DATA:
                            {
                                MrvlIEtypes_RssiThreshold_t *low_rssi =
                                    (MrvlIEtypes_RssiThreshold_t *) (buf + len);
                                printf("Data Low RSSI\t%d\t%d\t%s\n",
                                       low_rssi->RSSI_value,
                                       low_rssi->RSSI_freq,
                                       (le16_to_cpu(se->events) & 0x0040) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_SNR_LOW_DATA:
                            {
                                MrvlIEtypes_SnrThreshold_t *low_snr =
                                    (MrvlIEtypes_SnrThreshold_t *) (buf + len);
                                printf("Data Low SNR\t%d\t%d\t%s\n",
                                       low_snr->SNR_value,
                                       low_snr->SNR_freq,
                                       (le16_to_cpu(se->events) & 0x0080) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_RSSI_HIGH_DATA:
                            {
                                MrvlIEtypes_RssiThreshold_t *high_rssi =
                                    (MrvlIEtypes_RssiThreshold_t *) (buf + len);
                                printf("Data High RSSI\t%d\t%d\t%s\n",
                                       high_rssi->RSSI_value,
                                       high_rssi->RSSI_freq,
                                       (le16_to_cpu(se->events) & 0x0100) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_SNR_HIGH_DATA:
                            {
                                MrvlIEtypes_SnrThreshold_t *high_snr =
                                    (MrvlIEtypes_SnrThreshold_t *) (buf + len);
                                printf("Data High SNR\t%d\t%d\t%s\n",
                                       high_snr->SNR_value,
                                       high_snr->SNR_freq,
                                       (le16_to_cpu(se->events) & 0x0200) ?
                                       "yes" : "no");
                                break;
                            }
                        case TLV_TYPE_LINK_QUALITY:
                            {
                                MrvlIEtypes_LinkQuality_t *link_qual =
                                    (MrvlIEtypes_LinkQuality_t *) (buf + len);
                                printf("Link Quality Parameters:\n");
                                printf("------------------------\n");
                                printf("Link Quality Event Subscribed\t%s\n",
                                       (le16_to_cpu(se->events) & 0x0400) ?
                                       "yes" : "no");
                                printf("Link SNR Threshold   = %d\n",
                                       le16_to_cpu(link_qual->link_SNR_thrs));
                                printf("Link SNR Frequency   = %d\n",
                                       le16_to_cpu(link_qual->link_SNR_freq));
                                printf("Min Rate Value       = %d\n",
                                       le16_to_cpu(link_qual->min_rate_val));
                                printf("Min Rate Frequency   = %d\n",
                                       le16_to_cpu(link_qual->min_rate_freq));
                                printf("Tx Latency Value     = %d\n",
                                       le32_to_cpu(link_qual->tx_latency_val));
                                printf("Tx Latency Threshold = %d\n",
                                       le32_to_cpu(link_qual->tx_latency_thrs));

                                break;
                            }
                        case TLV_TYPE_PRE_BEACON_LOST:
                            {
                                MrvlIEtypes_PreBeaconLost_t *pre_bcn_lost =
                                    (MrvlIEtypes_PreBeaconLost_t *) (buf + len);
                                printf("------------------------\n");
                                printf("Pre-Beacon Lost Event Subscribed\t%s\n",
                                       (le16_to_cpu(se->events) & 0x0800) ?
                                       "yes" : "no");
                                printf("Pre-Beacon Lost: %d\n",
                                       pre_bcn_lost->pre_beacon_lost);
                                break;
                            }
                        default:
                            printf("Unknown subscribed event TLV Type=%#x,"
                                   " Len=%d\n",
                                   le16_to_cpu(header->type),
                                   le16_to_cpu(header->len));
                            break;
                        }

                        len += (sizeof(MrvlIEtypesHeader_t)
                                + le16_to_cpu(header->len));
                    }
                }
                break;
            }
        case HostCmd_CMD_MAC_REG_ACCESS:
        case HostCmd_CMD_BBP_REG_ACCESS:
        case HostCmd_CMD_RF_REG_ACCESS:
        case HostCmd_CMD_CAU_REG_ACCESS:
            {
                HostCmd_DS_REG *preg = (HostCmd_DS_REG *) (buf + S_DS_GEN);
                preg->action = le16_to_cpu(preg->action);
                if (preg->action == HostCmd_ACT_GEN_GET) {
                    preg->value = le32_to_cpu(preg->value);
                    printf("value = 0x%08x\n", preg->value);
                }
                break;
            }
        case HostCmd_CMD_MEM_ACCESS:
            {
                HostCmd_DS_MEM *pmem = (HostCmd_DS_MEM *) (buf + S_DS_GEN);
                pmem->action = le16_to_cpu(pmem->action);
                if (pmem->action == HostCmd_ACT_GEN_GET) {
                    pmem->value = le32_to_cpu(pmem->value);
                    printf("value = 0x%08x\n", pmem->value);
                }
                break;
            }
        default:
            printf("HOSTCMD_RESP: CmdCode=%#04x, Size=%#04x,"
                   " SeqNum=%#04x, Result=%#04x\n",
                   hostcmd->command, hostcmd->size,
                   hostcmd->seq_num, hostcmd->result);
            hexdump("payload",
                    (t_void *) (buf + S_DS_GEN), hostcmd->size - S_DS_GEN, ' ');
            break;
        }
    } else {
        printf("HOSTCMD failed: CmdCode=%#04x, Size=%#04x,"
               " SeqNum=%#04x, Result=%#04x\n",
               hostcmd->command, hostcmd->size,
               hostcmd->seq_num, hostcmd->result);
    }
    return ret;
}

/**
 *  @brief Process hostcmd command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_hostcmd(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    FILE *fp = NULL;
    t_s8 cmdname[256];

    if (argc < 5) {
        printf("Error: invalid no of arguments\n");
        printf("Syntax: ./mlanutl mlanX hostcmd <hostcmd.conf> <cmdname>\n");
        return MLAN_STATUS_FAILURE;
    }

    if ((fp = fopen(argv[3], "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s\n", argv[3]);
        return MLAN_STATUS_FAILURE;
    }

    snprintf(cmdname, sizeof(cmdname), "%s", argv[4]);

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Prepare the hostcmd buffer */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (MLAN_STATUS_FAILURE ==
        prepare_host_cmd_buffer(fp, cmdname,
                                buffer + strlen(CMD_MARVELL) +
                                strlen(argv[2]))) {
        fclose(fp);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }
    fclose(fp);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: hostcmd fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    process_host_cmd_resp(argv[2], cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process HT Tx configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_httxcfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    t_u32 *data = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: httxcfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (argc == 3) {
        /* Get result */
        data = (t_u32 *) cmd->buf;
        printf("HT Tx cfg: \n");
        printf("    BG band:  0x%08x\n", data[0]);
        printf("     A band:  0x%08x\n", data[1]);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process HT capability configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_htcapinfo(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_htcapinfo *ht_cap = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: htcapinfo fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 3) {
        ht_cap = (struct eth_priv_htcapinfo *) cmd->buf;
        printf("HT cap info: \n");
        printf("    BG band:  0x%08x\n", ht_cap->ht_cap_info_bg);
        printf("     A band:  0x%08x\n", ht_cap->ht_cap_info_a);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process HT Add BA parameters
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_addbapara(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_addba *addba = NULL;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: addbapara fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (argc == 3) {
        /* Get */
        addba = (struct eth_priv_addba *) cmd->buf;
        printf("Add BA configuration: \n");
        printf("    Time out : %d\n", addba->time_out);
        printf("    TX window: %d\n", addba->tx_win_size);
        printf("    RX window: %d\n", addba->rx_win_size);
        printf("    TX AMSDU : %d\n", addba->tx_amsdu);
        printf("    RX AMSDU : %d\n", addba->rx_amsdu);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process Aggregation priority table parameters
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_aggrpriotbl(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    int i;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: aggrpriotbl fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (argc == 3) {
        /* Get */
        printf("Aggregation priority table cfg: \n");
        printf("    TID      AMPDU      AMSDU \n");
        for (i = 0; i < MAX_NUM_TID; i++) {
            printf("     %d        %3d        %3d \n",
                   i, cmd->buf[2 * i], cmd->buf[2 * i + 1]);
        }
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process HT Add BA reject configurations
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_addbareject(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    int i;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: addbareject fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (argc == 3) {
        /* Get */
        printf("Add BA reject configuration: \n");
        printf("    TID      Reject \n");
        for (i = 0; i < MAX_NUM_TID; i++) {
            printf("     %d        %d\n", i, cmd->buf[i]);
        }
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process HT Del BA command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_delba(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: delba fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process reject addba req command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_rejectaddbareq(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: rejectaddbareq fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Reject addba req command response: %s\n", cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

static t_s8 *rate_format[3] = { "LG", "HT", "VHT" };

static t_s8 *lg_rate[] = { "1 Mbps", "2 Mbps", "5.5 Mbps", "11 Mbps",
    "6 Mbps", "9 Mbps", "12 Mbps", "18 Mbps",
    "24 Mbps", "36 Mbps", "48 Mbps", "54 Mbps"
};

/**
 *  @brief Process Get data rate
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_datarate(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_data_rate *datarate = NULL;
    struct ifreq ifr;
    t_s8 *bw[] = { "20 MHz", "40 MHz", "80 MHz", "160 MHz" };

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: getdatarate fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    datarate = (struct eth_priv_data_rate *) (cmd->buf);
    printf("Data Rate:\n");
    printf("  TX: \n");
    if (datarate->tx_data_rate < 12) {
        printf("    Type: %s\n", rate_format[0]);
        /* LG */
        printf("    Rate: %s\n", lg_rate[datarate->tx_data_rate]);
    } else {
        /* HT */
        printf("    Type: %s\n", rate_format[1]);
        if (datarate->tx_bw <= 2)
            printf("    BW:   %s\n", bw[datarate->tx_bw]);
        if (datarate->tx_gi == 0)
            printf("    GI:   Long\n");
        else
            printf("    GI:   Short\n");
        printf("    MCS:  MCS %d\n", (int) (datarate->tx_data_rate - 12));
    }

    printf("  RX: \n");
    if (datarate->rx_data_rate < 12) {
        printf("    Type: %s\n", rate_format[0]);
        /* LG */
        printf("    Rate: %s\n", lg_rate[datarate->rx_data_rate]);
    } else {
        /* HT */
        printf("    Type: %s\n", rate_format[1]);
        if (datarate->rx_bw <= 2)
            printf("    BW:   %s\n", bw[datarate->rx_bw]);
        if (datarate->rx_gi == 0)
            printf("    GI:   Long\n");
        else
            printf("    GI:   Short\n");
        printf("    MCS:  MCS %d\n", (int) (datarate->rx_data_rate - 12));
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process tx rate configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_txratecfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_tx_rate_cfg *txratecfg = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: txratecfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    txratecfg = (struct eth_priv_tx_rate_cfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Tx Rate Configuration: \n");
        /* format */
        if (txratecfg->rate_format == 0xFF) {
            printf("    Type:       0xFF (Auto)\n");
        } else if (txratecfg->rate_format <= 2) {
            printf("    Type:       %d (%s)\n", txratecfg->rate_format,
                   rate_format[txratecfg->rate_format]);
            if (txratecfg->rate_format == 0)
                printf("    Rate Index: %d (%s)\n", txratecfg->rate_index,
                       lg_rate[txratecfg->rate_index]);
            else if (txratecfg->rate_format >= 1)
                printf("    MCS Index:  %d\n", (int) txratecfg->rate_index);
        } else {
            printf("    Unknown rate format.\n");
        }
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process esuppmode command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_esuppmode(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_esuppmode_cfg *esuppmodecfg = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: esuppmode fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    esuppmodecfg = (struct eth_priv_esuppmode_cfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Esupplicant Mode Configuration: \n");
        /* RSN mode */
        printf("    RSN mode:         0x%x ( ", esuppmodecfg->rsn_mode);
        if (esuppmodecfg->rsn_mode & MBIT(0))
            printf("No-RSN ");
        if (esuppmodecfg->rsn_mode & MBIT(3))
            printf("WPA ");
        if (esuppmodecfg->rsn_mode & MBIT(4))
            printf("WPA-None ");
        if (esuppmodecfg->rsn_mode & MBIT(5))
            printf("WPA2 ");
        printf(")\n");
        /* Pairwise cipher */
        printf("    Pairwise cipher:  0x%x ( ", esuppmodecfg->pairwise_cipher);
        if (esuppmodecfg->pairwise_cipher & MBIT(2))
            printf("TKIP ");
        if (esuppmodecfg->pairwise_cipher & MBIT(3))
            printf("AES ");
        printf(")\n");
        /* Group cipher */
        printf("    Group cipher:     0x%x ( ", esuppmodecfg->group_cipher);
        if (esuppmodecfg->group_cipher & MBIT(2))
            printf("TKIP ");
        if (esuppmodecfg->group_cipher & MBIT(3))
            printf("AES ");
        printf(")\n");
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process passphrase command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_passphrase(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (argc >= 4) {
        strcpy((char *) (buffer + strlen(CMD_MARVELL) + strlen(argv[2])),
               argv[3]);
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: passphrase fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Passphrase Configuration: %s\n", (char *) (cmd->buf));

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process deauth command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_deauth(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (argc >= 4) {
        strcpy((char *) (buffer + strlen(CMD_MARVELL) + strlen(argv[2])),
               argv[3]);
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: deauth fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

#ifdef UAP_SUPPORT
/**
 *  @brief Process getstalist command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_getstalist(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_getstalist *list = NULL;
    int i = 0, rssi = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: getstalist fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }
    list =
        (struct eth_priv_getstalist *) (buffer + strlen(CMD_MARVELL) +
                                        strlen(argv[2]));

    printf("Number of STA = %d\n\n", list->sta_count);

    for (i = 0; i < list->sta_count; i++) {
        printf("STA %d information:\n", i + 1);
        printf("=====================\n");
        printf("MAC Address: ");
        print_mac(list->client_info[i].mac_address);
        printf("\nPower mfg status: %s\n",
               (list->client_info[i].power_mfg_status ==
                0) ? "active" : "power save");

        /** On some platform, s8 is same as unsigned char*/
        rssi = (int) list->client_info[i].rssi;
        if (rssi > 0x7f)
            rssi = -(256 - rssi);
        printf("Rssi : %d dBm\n\n", rssi);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}
#endif

#if defined(WIFI_DIRECT_SUPPORT)
#if defined(STA_SUPPORT) && defined(UAP_SUPPORT)
/**
 *  @brief Process BSS role command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_bssrole(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: bssrole fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 3) {
        /* GET operation */
        printf("BSS role: %d\n", buffer[0]);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}
#endif
#endif

#ifdef STA_SUPPORT
/**
 *  @brief Helper function for process_getscantable_idx
 *
 *  @param pbuf     A pointer to the buffer
 *  @param buf_len  Buffer length
 *
 *  @return         NA
 *
 */
static void
dump_scan_elems(const t_u8 * pbuf, uint buf_len)
{
    uint idx;
    uint marker = 2 + pbuf[1];

    for (idx = 0; idx < buf_len; idx++) {
        if (idx % 0x10 == 0) {
            printf("\n%04x: ", idx);
        }

        if (idx == marker) {
            printf("|");
            marker = idx + pbuf[idx + 1] + 2;
        } else {
            printf(" ");
        }

        printf("%02x ", pbuf[idx]);
    }

    printf("\n");
}

/**
 *  @brief Helper function for process_getscantable_idx
 *  Find next element
 *
 *  @param pp_ie_out    Pointer of a IEEEtypes_Generic_t structure pointer
 *  @param p_buf_left   Integer pointer, which contains the number of left p_buf
 *
 *  @return             MLAN_STATUS_SUCCESS on success, otherwise MLAN_STATUS_FAILURE
 */
static int
scantable_elem_next(IEEEtypes_Generic_t ** pp_ie_out, int *p_buf_left)
{
    IEEEtypes_Generic_t *pie_gen;
    t_u8 *p_next;

    if (*p_buf_left < 2) {
        return MLAN_STATUS_FAILURE;
    }

    pie_gen = *pp_ie_out;

    p_next = (t_u8 *) pie_gen + (pie_gen->ieee_hdr.len
                                 + sizeof(pie_gen->ieee_hdr));
    *p_buf_left -= (p_next - (t_u8 *) pie_gen);

    *pp_ie_out = (IEEEtypes_Generic_t *) p_next;

    if (*p_buf_left <= 0) {
        return MLAN_STATUS_FAILURE;
    }

    return MLAN_STATUS_SUCCESS;
}

 /**
  *  @brief Helper function for process_getscantable_idx
  *         scantable find element
  *
  *  @param ie_buf       Pointer of the IE buffer
  *  @param ie_buf_len   IE buffer length
  *  @param ie_type      IE type
  *  @param ppie_out     Pointer to the IEEEtypes_Generic_t structure pointer
  *  @return             MLAN_STATUS_SUCCESS on success, otherwise MLAN_STATUS_FAILURE
  */
static int
scantable_find_elem(t_u8 * ie_buf,
                    unsigned int ie_buf_len,
                    IEEEtypes_ElementId_e ie_type,
                    IEEEtypes_Generic_t ** ppie_out)
{
    int found;
    unsigned int ie_buf_left;

    ie_buf_left = ie_buf_len;

    found = FALSE;

    *ppie_out = (IEEEtypes_Generic_t *) ie_buf;

    do {
        found = ((*ppie_out)->ieee_hdr.element_id == ie_type);

    } while (!found &&
             (scantable_elem_next(ppie_out, (int *) &ie_buf_left) == 0));

    if (!found) {
        *ppie_out = NULL;
    }

    return (found ? MLAN_STATUS_SUCCESS : MLAN_STATUS_FAILURE);
}

 /**
  *  @brief Helper function for process_getscantable_idx
  *         It gets SSID from IE
  *
  *  @param ie_buf       IE buffer
  *  @param ie_buf_len   IE buffer length
  *  @param pssid        SSID
  *  @param ssid_buf_max Size of SSID
  *  @return             MLAN_STATUS_SUCCESS on success, otherwise MLAN_STATUS_FAILURE
  */
static int
scantable_get_ssid_from_ie(t_u8 * ie_buf,
                           unsigned int ie_buf_len,
                           t_u8 * pssid, unsigned int ssid_buf_max)
{
    int retval;
    IEEEtypes_Generic_t *pie_gen;

    retval = scantable_find_elem(ie_buf, ie_buf_len, SSID, &pie_gen);

    memcpy(pssid, pie_gen->data, MIN(pie_gen->ieee_hdr.len, ssid_buf_max));

    return retval;
}

/**
 *  @brief Display detailed information for a specific scan table entry
 *
 *  @param prsp_info_req    Scan table entry request structure
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
process_getscantable_idx(char *cmd_name,
                         wlan_ioctl_get_scan_table_info * prsp_info_req)
{
    int ret = 0;
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    t_u8 *pcurrent;
    char ssid[33];
    t_u16 tmp_cap;
    t_u8 tsf[8];
    t_u16 beacon_interval;
    t_u16 cap_info;
    wlan_ioctl_get_scan_table_info *prsp_info;

    wlan_get_scan_table_fixed fixed_fields;
    t_u32 fixed_field_length;
    t_u32 bss_info_length;

    memset(ssid, 0x00, sizeof(ssid));

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, cmd_name, 0, NULL);

    prsp_info =
        (wlan_ioctl_get_scan_table_info *) (buffer + strlen(CMD_MARVELL) +
                                            strlen(cmd_name));

    memcpy(prsp_info, prsp_info_req, sizeof(wlan_ioctl_get_scan_table_info));

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* 
     * Set up and execute the ioctl call
     */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        if (errno == EAGAIN) {
            ret = -EAGAIN;
        } else {
            perror("mlanutl");
            fprintf(stderr, "mlanutl: getscantable fail\n");
            ret = MLAN_STATUS_FAILURE;
        }
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return ret;
    }

    prsp_info = (wlan_ioctl_get_scan_table_info *) buffer;
    if (prsp_info->scan_number == 0) {
        printf("mlanutl: getscantable ioctl - index out of range\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return -EINVAL;
    }

    pcurrent = prsp_info->scan_table_entry_buf;

    memcpy((t_u8 *) & fixed_field_length,
           (t_u8 *) pcurrent, sizeof(fixed_field_length));
    pcurrent += sizeof(fixed_field_length);

    memcpy((t_u8 *) & bss_info_length,
           (t_u8 *) pcurrent, sizeof(bss_info_length));
    pcurrent += sizeof(bss_info_length);

    memcpy((t_u8 *) & fixed_fields, (t_u8 *) pcurrent, sizeof(fixed_fields));
    pcurrent += fixed_field_length;

    /* Time stamp is 8 byte long */
    memcpy(tsf, pcurrent, sizeof(tsf));
    pcurrent += sizeof(tsf);
    bss_info_length -= sizeof(tsf);

    /* Beacon interval is 2 byte long */
    memcpy(&beacon_interval, pcurrent, sizeof(beacon_interval));
    /* Endian convert needed here */
    beacon_interval = le16_to_cpu(beacon_interval);
    pcurrent += sizeof(beacon_interval);
    bss_info_length -= sizeof(beacon_interval);

    /* Capability information is 2 byte long */
    memcpy(&cap_info, pcurrent, sizeof(cap_info));
    /* Endian convert needed here */
    cap_info = le16_to_cpu(cap_info);
    pcurrent += sizeof(cap_info);
    bss_info_length -= sizeof(cap_info);

    scantable_get_ssid_from_ie(pcurrent,
                               bss_info_length, (t_u8 *) ssid, sizeof(ssid));

    printf("\n*** [%s], %02x:%02x:%02x:%02x:%02x:%2x\n",
           ssid,
           fixed_fields.bssid[0],
           fixed_fields.bssid[1],
           fixed_fields.bssid[2],
           fixed_fields.bssid[3], fixed_fields.bssid[4], fixed_fields.bssid[5]);
    memcpy(&tmp_cap, &cap_info, sizeof(tmp_cap));
    printf("Channel = %d, SS = %d, CapInfo = 0x%04x, BcnIntvl = %d\n",
           fixed_fields.channel,
           255 - fixed_fields.rssi, tmp_cap, beacon_interval);

    printf("TSF Values: AP(0x%02x%02x%02x%02x%02x%02x%02x%02x), ",
           tsf[7], tsf[6], tsf[5], tsf[4], tsf[3], tsf[2], tsf[1], tsf[0]);

    printf("Network(0x%016llx)\n", fixed_fields.network_tsf);
    printf("\n");
    printf("Element Data (%d bytes)\n", (int) bss_info_length);
    printf("------------");
    dump_scan_elems(pcurrent, bss_info_length);
    printf("\n");

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process getscantable command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_getscantable(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    unsigned int scan_start;
    int idx, ret = 0;

    t_u8 *pcurrent;
    t_u8 *pnext;
    IEEEtypes_ElementId_e *pelement_id;
    t_u8 *pelement_len;
    int ssid_idx;
    t_u8 *pbyte;
    char ssid[33];
    int ssid_len = 0;

    IEEEtypes_CapInfo_t cap_info;
    t_u16 tmp_cap;
    t_u8 tsf[8];
    t_u16 beacon_interval;

    IEEEtypes_VendorSpecific_t *pwpa_ie;
    const t_u8 wpa_oui[4] = { 0x00, 0x50, 0xf2, 0x01 };

    IEEEtypes_WmmParameter_t *pwmm_ie;
    const t_u8 wmm_oui[4] = { 0x00, 0x50, 0xf2, 0x02 };
    IEEEtypes_VendorSpecific_t *pwps_ie;
    const t_u8 wps_oui[4] = { 0x00, 0x50, 0xf2, 0x04 };
    char wmm_cap;
    char wps_cap;
    char dot11k_cap;
    char dot11r_cap;
    char priv_cap;
    char ht_cap;

    int displayed_info;

    wlan_ioctl_get_scan_table_info rsp_info_req;
    wlan_ioctl_get_scan_table_info *prsp_info;

    wlan_get_scan_table_fixed fixed_fields;
    t_u32 fixed_field_length;
    t_u32 bss_info_length;

    memset(&cap_info, 0x00, sizeof(cap_info));

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    if (argc > 3 && (strcmp(argv[3], "tsf") != 0)
        && (strcmp(argv[3], "help") != 0)) {

        idx = strtol(argv[3], NULL, 10);

        if (idx >= 0) {
            rsp_info_req.scan_number = idx;
            ret = process_getscantable_idx(argv[2], &rsp_info_req);
            if (buffer)
                free(buffer);
            if (cmd)
                free(cmd);
            return ret;
        }
    }

    displayed_info = FALSE;
    scan_start = 1;

    do {
        prepare_buffer(buffer, argv[2], 0, NULL);
        prsp_info =
            (wlan_ioctl_get_scan_table_info *) (buffer + strlen(CMD_MARVELL) +
                                                strlen(argv[2]));

        prsp_info->scan_number = scan_start;

        /* 
         * Set up and execute the ioctl call
         */
        memset(&ifr, 0, sizeof(struct ifreq));
        strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
        ifr.ifr_ifru.ifru_data = (void *) cmd;

        if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
            if (errno == EAGAIN) {
                ret = -EAGAIN;
            } else {
                perror("mlanutl");
                fprintf(stderr, "mlanutl: getscantable fail\n");
                ret = MLAN_STATUS_FAILURE;
            }
            if (cmd)
                free(cmd);
            if (buffer)
                free(buffer);
            return ret;
        }

        prsp_info = (wlan_ioctl_get_scan_table_info *) buffer;
        pcurrent = 0;
        pnext = prsp_info->scan_table_entry_buf;

        if (scan_start == 1) {
            printf("---------------------------------------");
            printf("---------------------------------------\n");
            printf("# | ch  | ss  |       bssid       |   cap    |   SSID \n");
            printf("---------------------------------------");
            printf("---------------------------------------\n");
        }

        for (idx = 0; (unsigned int) idx < prsp_info->scan_number; idx++) {

            /* 
             * Set pcurrent to pnext in case pad bytes are at the end
             *   of the last IE we processed.
             */
            pcurrent = pnext;

            memcpy((t_u8 *) & fixed_field_length,
                   (t_u8 *) pcurrent, sizeof(fixed_field_length));
            pcurrent += sizeof(fixed_field_length);

            memcpy((t_u8 *) & bss_info_length,
                   (t_u8 *) pcurrent, sizeof(bss_info_length));
            pcurrent += sizeof(bss_info_length);

            memcpy((t_u8 *) & fixed_fields,
                   (t_u8 *) pcurrent, sizeof(fixed_fields));
            pcurrent += fixed_field_length;

            /* Set next to be the start of the next scan entry */
            pnext = pcurrent + bss_info_length;

            printf("%02u| %03d | %03d | %02x:%02x:%02x:%02x:%02x:%02x |",
                   scan_start + idx,
                   fixed_fields.channel,
                   255 - fixed_fields.rssi,
                   fixed_fields.bssid[0],
                   fixed_fields.bssid[1],
                   fixed_fields.bssid[2],
                   fixed_fields.bssid[3],
                   fixed_fields.bssid[4], fixed_fields.bssid[5]);

            displayed_info = TRUE;

            if (bss_info_length >=
                (sizeof(tsf) + sizeof(beacon_interval) + sizeof(cap_info))) {
                /* Time stamp is 8 byte long */
                memcpy(tsf, pcurrent, sizeof(tsf));
                pcurrent += sizeof(tsf);
                bss_info_length -= sizeof(tsf);

                /* Beacon interval is 2 byte long */
                memcpy(&beacon_interval, pcurrent, sizeof(beacon_interval));
                /* Endian convert needed here */
                beacon_interval = le16_to_cpu(beacon_interval);
                pcurrent += sizeof(beacon_interval);
                bss_info_length -= sizeof(beacon_interval);

                /* Capability information is 2 byte long */
                memcpy(&tmp_cap, pcurrent, sizeof(tmp_cap));
                /* Endian convert needed here */
                tmp_cap = le16_to_cpu(tmp_cap);
                memcpy(&cap_info, &tmp_cap, sizeof(cap_info));
                pcurrent += sizeof(cap_info);
                bss_info_length -= sizeof(cap_info);
            }

            wmm_cap = ' ';      /* M (WMM), C (WMM-Call Admission Control) */
            wps_cap = ' ';      /* "S" */
            dot11k_cap = ' ';   /* "K" */
            dot11r_cap = ' ';   /* "R" */
            ht_cap = ' ';       /* "N" */

            /* "P" for Privacy (WEP) since "W" is WPA, and "2" is RSN/WPA2 */
            priv_cap = cap_info.privacy ? 'P' : ' ';

            memset(ssid, 0, MRVDRV_MAX_SSID_LENGTH + 1);
            ssid_len = 0;
            while (bss_info_length >= 2) {
                pelement_id = (IEEEtypes_ElementId_e *) pcurrent;
                pelement_len = pcurrent + 1;
                pcurrent += 2;

                switch (*pelement_id) {

                case SSID:
                    if (*pelement_len &&
                        *pelement_len <= MRVDRV_MAX_SSID_LENGTH) {
                        memcpy(ssid, pcurrent, *pelement_len);
                        ssid_len = *pelement_len;
                    }
                    break;

                case WPA_IE:
                    pwpa_ie = (IEEEtypes_VendorSpecific_t *) pelement_id;
                    if ((memcmp
                         (pwpa_ie->vend_hdr.oui, wpa_oui,
                          sizeof(pwpa_ie->vend_hdr.oui)) == 0)
                        && (pwpa_ie->vend_hdr.oui_type == wpa_oui[3])) {
                        /* WPA IE found, 'W' for WPA */
                        priv_cap = 'W';
                    } else {
                        pwmm_ie = (IEEEtypes_WmmParameter_t *) pelement_id;
                        if ((memcmp(pwmm_ie->vend_hdr.oui,
                                    wmm_oui,
                                    sizeof(pwmm_ie->vend_hdr.oui)) == 0)
                            && (pwmm_ie->vend_hdr.oui_type == wmm_oui[3])) {
                            /* Check the subtype: 1 == parameter, 0 == info */
                            if ((pwmm_ie->vend_hdr.oui_subtype == 1)
                                && pwmm_ie->ac_params[WMM_AC_VO].aci_aifsn.acm) {
                                /* Call admission on VO; 'C' for CAC */
                                wmm_cap = 'C';
                            } else {
                                /* No CAC; 'M' for uh, WMM */
                                wmm_cap = 'M';
                            }
                        } else {
                            pwps_ie =
                                (IEEEtypes_VendorSpecific_t *) pelement_id;
                            if ((memcmp
                                 (pwps_ie->vend_hdr.oui, wps_oui,
                                  sizeof(pwps_ie->vend_hdr.oui)) == 0)
                                && (pwps_ie->vend_hdr.oui_type == wps_oui[3])) {
                                wps_cap = 'S';
                            }
                        }
                    }
                    break;

                case RSN_IE:
                    /* RSN IE found; '2' for WPA2 (RSN) */
                    priv_cap = '2';
                    break;
                case HT_CAPABILITY:
                    ht_cap = 'N';
                    break;
                default:
                    break;
                }

                pcurrent += *pelement_len;
                bss_info_length -= (2 + *pelement_len);
            }
            /* "A" for Adhoc "I" for Infrastructure, "D" for DFS (Spectrum
               Mgmt) */
            printf(" %c%c%c%c%c%c%c%c | ", cap_info.ibss ? 'A' : 'I', priv_cap, /* P 
                                                                                   (WEP), 
                                                                                   W 
                                                                                   (WPA), 
                                                                                   2 
                                                                                   (WPA2) 
                                                                                 */
                   cap_info.spectrum_mgmt ? 'D' : ' ', wmm_cap, /* M (WMM), C
                                                                   (WMM-Call
                                                                   Admission
                                                                   Control) */
                   dot11k_cap,  /* K */
                   dot11r_cap,  /* R */
                   wps_cap,     /* S */
                   ht_cap);     /* N (11n) or A (11ac) */

            /* Print out the ssid or the hex values if non-printable */
            for (ssid_idx = 0; ssid_idx < ssid_len; ssid_idx++) {
                if (isprint(ssid[ssid_idx])) {
                    printf("%c", ssid[ssid_idx]);
                } else {
                    printf("\\%02x", ssid[ssid_idx]);
                }
            }

            printf("\n");

            if (argc > 3 && strcmp(argv[3], "tsf") == 0) {
                /* TSF is a u64, some formatted printing libs have trouble
                   printing long longs, so cast and dump as bytes */
                pbyte = (t_u8 *) & fixed_fields.network_tsf;
                printf("    TSF=%02x%02x%02x%02x%02x%02x%02x%02x\n",
                       pbyte[7], pbyte[6], pbyte[5], pbyte[4],
                       pbyte[3], pbyte[2], pbyte[1], pbyte[0]);
            }
        }

        scan_start += prsp_info->scan_number;

    } while (prsp_info->scan_number);

    if (displayed_info == TRUE) {
        if (argc > 3 && strcmp(argv[3], "help") == 0) {
            printf("\n\n"
                   "Capability Legend (Not all may be supported)\n"
                   "-----------------\n"
                   " I [ Infrastructure ]\n"
                   " A [ Ad-hoc ]\n"
                   " W [ WPA IE ]\n"
                   " 2 [ WPA2/RSN IE ]\n"
                   " M [ WMM IE ]\n"
                   " C [ Call Admission Control - WMM IE, VO ACM set ]\n"
                   " D [ Spectrum Management - DFS (11h) ]\n"
                   " K [ 11k ]\n"
                   " R [ 11r ]\n" " S [ WPS ]\n" " N [ HT (11n) ]\n" "\n\n");
        }
    } else {
        printf("< No Scan Results >\n");
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Prepare setuserscan command buffer
 *  @param buffer   Command buffer to be filled
 *  @param cmd      Command id
 *  @param num      Number of arguments
 *  @param args     Arguments list
 *  @return         MLAN_STATUS_SUCCESS
 */
static int
prepare_setuserscan_buffer(t_u8 * buffer, t_s8 * cmd, t_u32 num, char *args[])
{
    wlan_ioctl_user_scan_cfg *scan_req = NULL;
    t_u8 *pos = NULL;
    int arg_idx = 0;
    int num_ssid = 0;
    char *parg_tok = NULL;
    char *pchan_tok = NULL;
    char *parg_cookie = NULL;
    char *pchan_cookie = NULL;
    int chan_parse_idx = 0;
    int chan_cmd_idx = 0;
    char chan_scratch[MAX_CHAN_SCRATCH];
    char *pscratch = NULL;
    int tmp_idx = 0;
    int scan_time = 0;
    int is_radio_set = 0;
    unsigned int mac[ETH_ALEN];

    memset(buffer, 0, BUFFER_LENGTH);

    /* Flag it for our use */
    pos = buffer;
    strncpy((char *) pos, CMD_MARVELL, strlen(CMD_MARVELL));
    pos += (strlen(CMD_MARVELL));

    /* Insert command */
    strncpy((char *) pos, (char *) cmd, strlen(cmd));
    pos += (strlen(cmd));

    /* Insert arguments */
    scan_req = (wlan_ioctl_user_scan_cfg *) pos;

    for (arg_idx = 0; arg_idx < num; arg_idx++) {
        if (strncmp(args[arg_idx], "ssid=", strlen("ssid=")) == 0) {
            /* "ssid" token string handler */
            if (num_ssid < MRVDRV_MAX_SSID_LIST_LENGTH) {
                strncpy(scan_req->ssid_list[num_ssid].ssid,
                        args[arg_idx] + strlen("ssid="),
                        sizeof(scan_req->ssid_list[num_ssid].ssid));

                scan_req->ssid_list[num_ssid].max_len = 0;

                num_ssid++;
            }
        } else if (strncmp(args[arg_idx], "bssid=", strlen("bssid=")) == 0) {
            /* "bssid" token string handler */
            sscanf(args[arg_idx] + strlen("bssid="), "%2x:%2x:%2x:%2x:%2x:%2x",
                   mac + 0, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);

            for (tmp_idx = 0;
                 (unsigned int) tmp_idx < NELEMENTS(mac); tmp_idx++) {
                scan_req->specific_bssid[tmp_idx] = (t_u8) mac[tmp_idx];
            }
        } else if (strncmp(args[arg_idx], "chan=", strlen("chan=")) == 0) {
            /* "chan" token string handler */
            parg_tok = args[arg_idx] + strlen("chan=");

            if (strlen(parg_tok) > MAX_CHAN_SCRATCH) {
                printf("Error: Specified channels exceeds max limit\n");
                return MLAN_STATUS_FAILURE;
            }
            is_radio_set = FALSE;

            while ((parg_tok = strtok_r(parg_tok, ",", &parg_cookie)) != NULL) {

                memset(chan_scratch, 0x00, sizeof(chan_scratch));
                pscratch = chan_scratch;

                for (chan_parse_idx = 0;
                     (unsigned int) chan_parse_idx < strlen(parg_tok);
                     chan_parse_idx++) {
                    if (isalpha(*(parg_tok + chan_parse_idx))) {
                        *pscratch++ = ' ';
                    }

                    *pscratch++ = *(parg_tok + chan_parse_idx);
                }
                *pscratch = 0;
                parg_tok = NULL;

                pchan_tok = chan_scratch;

                while ((pchan_tok = strtok_r(pchan_tok, " ",
                                             &pchan_cookie)) != NULL) {
                    if (isdigit(*pchan_tok)) {
                        scan_req->chan_list[chan_cmd_idx].chan_number
                            = atoi(pchan_tok);
                        if (scan_req->chan_list[chan_cmd_idx].chan_number >
                            MAX_CHAN_BG_BAND)
                            scan_req->chan_list[chan_cmd_idx].radio_type = 1;
                    } else {
                        switch (toupper(*pchan_tok)) {
                        case 'A':
                            scan_req->chan_list[chan_cmd_idx].radio_type = 1;
                            is_radio_set = TRUE;
                            break;
                        case 'B':
                        case 'G':
                            scan_req->chan_list[chan_cmd_idx].radio_type = 0;
                            is_radio_set = TRUE;
                            break;
                        case 'N':
                            break;
                        case 'P':
                            scan_req->chan_list[chan_cmd_idx].scan_type =
                                MLAN_SCAN_TYPE_PASSIVE;
                            break;
                        default:
                            printf("Error: Band type not supported!\n");
                            return -EOPNOTSUPP;
                        }
                        if (!chan_cmd_idx &&
                            !scan_req->chan_list[chan_cmd_idx].chan_number &&
                            is_radio_set)
                            scan_req->chan_list[chan_cmd_idx].radio_type |=
                                BAND_SPECIFIED;
                    }
                    pchan_tok = NULL;
                }
                chan_cmd_idx++;
            }
        } else if (strncmp(args[arg_idx], "keep=", strlen("keep=")) == 0) {
            /* "keep" token string handler */
            scan_req->keep_previous_scan =
                atoi(args[arg_idx] + strlen("keep="));
        } else if (strncmp(args[arg_idx], "dur=", strlen("dur=")) == 0) {
            /* "dur" token string handler */
            scan_time = atoi(args[arg_idx] + strlen("dur="));
            scan_req->chan_list[0].scan_time = scan_time;

        } else if (strncmp(args[arg_idx], "wc=", strlen("wc=")) == 0) {

            if (num_ssid < MRVDRV_MAX_SSID_LIST_LENGTH) {
                /* "wc" token string handler */
                pscratch = strrchr(args[arg_idx], ',');

                if (pscratch) {
                    *pscratch = 0;
                    pscratch++;

                    if (isdigit(*pscratch)) {
                        scan_req->ssid_list[num_ssid].max_len = atoi(pscratch);
                    } else {
                        scan_req->ssid_list[num_ssid].max_len = *pscratch;
                    }
                } else {
                    /* Standard wildcard matching */
                    scan_req->ssid_list[num_ssid].max_len = 0xFF;
                }

                strncpy(scan_req->ssid_list[num_ssid].ssid,
                        args[arg_idx] + strlen("wc="),
                        sizeof(scan_req->ssid_list[num_ssid].ssid));

                num_ssid++;
            }
        } else if (strncmp(args[arg_idx], "probes=", strlen("probes=")) == 0) {
            /* "probes" token string handler */
            scan_req->num_probes = atoi(args[arg_idx] + strlen("probes="));
            if (scan_req->num_probes > MAX_PROBES) {
                fprintf(stderr, "Invalid probes (> %d)\n", MAX_PROBES);
                return -EOPNOTSUPP;
            }
        } else if (strncmp(args[arg_idx], "type=", strlen("type=")) == 0) {
            /* "type" token string handler */
            scan_req->bss_mode = atoi(args[arg_idx] + strlen("type="));
            switch (scan_req->bss_mode) {
            case MLAN_SCAN_MODE_BSS:
            case MLAN_SCAN_MODE_IBSS:
                break;
            case MLAN_SCAN_MODE_ANY:
            default:
                /* Set any unknown types to ANY */
                scan_req->bss_mode = MLAN_SCAN_MODE_ANY;
                break;
            }
        }
    }

    /* Update all the channels to have the same scan time */
    for (tmp_idx = 1; tmp_idx < chan_cmd_idx; tmp_idx++) {
        scan_req->chan_list[tmp_idx].scan_time = scan_time;
    }

    pos += sizeof(wlan_ioctl_user_scan_cfg);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process setuserscan command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_setuserscan(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    int status = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_setuserscan_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: setuserscan fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    do {
        argv[2] = "getscantable";
        status = process_getscantable(0, argv);
    } while (status == -EAGAIN);

    return MLAN_STATUS_SUCCESS;
}
#endif

/**
 *  @brief Process deep sleep configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_deepsleep(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: deepsleep fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Deepsleep command response: %s\n", cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process ipaddr command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_ipaddr(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (argc >= 4) {
        strcpy((char *) (buffer + strlen(CMD_MARVELL) + strlen(argv[2])),
               argv[3]);
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: ipaddr fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("IP address Configuration: %s\n", (char *) (cmd->buf));

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process otpuserdata command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_otpuserdata(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    if (argc < 4) {
        printf("ERR:No argument\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: otpuserdata fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    hexdump("OTP user data: ", cmd->buf,
            MIN(cmd->used_len, a2hex_or_atoi(argv[3])), ' ');

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process countrycode setting
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_countrycode(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct eth_priv_countrycode *countrycode = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (argc >= 4) {
        strcpy((char *) (buffer + strlen(CMD_MARVELL) + strlen(argv[2])),
               argv[3]);
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: countrycode fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    countrycode = (struct eth_priv_countrycode *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Country code: %s\n", countrycode->country_code);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process TCP ACK enhancement configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_tcpackenh(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: tcpackenh fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("TCP Ack enhancement: ");
    if (cmd->buf[0])
        printf("enabled.\n");
    else
        printf("disabled.\n");

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

#ifdef REASSOCIATION
/**
 *  @brief Process asynced essid setting
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_assocessid(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);
    if (argc >= 4) {
        strcpy((char *) (buffer + strlen(CMD_MARVELL) + strlen(argv[2])),
               argv[3]);
    }

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: assocessid fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Set Asynced ESSID: %s\n", (char *) (cmd->buf));

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;

}
#endif

#ifdef STA_SUPPORT
/**
 *  @brief Process listen interval configuration
 *  @param argc   number of arguments
 *  @param argv   A pointer to arguments array    
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_listeninterval(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: listen interval fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 3)
        printf("Listen interval command response: %s\n", cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}
#endif

#ifdef DEBUG_LEVEL1
/**
 *  @brief Process driver debug configuration
 *  @param argc   number of arguments
 *  @param argv   A pointer to arguments array    
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_drvdbg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    t_u32 drvdbg;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: drvdbg config fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 3) {
        memcpy(&drvdbg, cmd->buf, sizeof(drvdbg));
        printf("drvdbg: 0x%08x\n", drvdbg);
#ifdef DEBUG_LEVEL2
        printf("MINFO  (%08x) %s\n", MINFO, (drvdbg & MINFO) ? "X" : "");
        printf("MWARN  (%08x) %s\n", MWARN, (drvdbg & MWARN) ? "X" : "");
        printf("MENTRY (%08x) %s\n", MENTRY, (drvdbg & MENTRY) ? "X" : "");
#endif
        printf("MIF_D  (%08x) %s\n", MIF_D, (drvdbg & MIF_D) ? "X" : "");
        printf("MFW_D  (%08x) %s\n", MFW_D, (drvdbg & MFW_D) ? "X" : "");
        printf("MEVT_D (%08x) %s\n", MEVT_D, (drvdbg & MEVT_D) ? "X" : "");
        printf("MCMD_D (%08x) %s\n", MCMD_D, (drvdbg & MCMD_D) ? "X" : "");
        printf("MDAT_D (%08x) %s\n", MDAT_D, (drvdbg & MDAT_D) ? "X" : "");
        printf("MIOCTL (%08x) %s\n", MIOCTL, (drvdbg & MIOCTL) ? "X" : "");
        printf("MINTR  (%08x) %s\n", MINTR, (drvdbg & MINTR) ? "X" : "");
        printf("MEVENT (%08x) %s\n", MEVENT, (drvdbg & MEVENT) ? "X" : "");
        printf("MCMND  (%08x) %s\n", MCMND, (drvdbg & MCMND) ? "X" : "");
        printf("MDATA  (%08x) %s\n", MDATA, (drvdbg & MDATA) ? "X" : "");
        printf("MERROR (%08x) %s\n", MERROR, (drvdbg & MERROR) ? "X" : "");
        printf("MFATAL (%08x) %s\n", MFATAL, (drvdbg & MFATAL) ? "X" : "");
        printf("MMSG   (%08x) %s\n", MMSG, (drvdbg & MMSG) ? "X" : "");

    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}
#endif

/**
 *  @brief Process hscfg configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_hscfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_hs_cfg *hscfg;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: hscfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    hscfg = (struct eth_priv_hs_cfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("HS Configuration:\n");
        printf("  Conditions: %d\n", (int) hscfg->conditions);
        printf("  GPIO: %d\n", (int) hscfg->gpio);
        printf("  GAP: %d\n", (int) hscfg->gap);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process hssetpara configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_hssetpara(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: hssetpara fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process wakeup reason
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_wakeupresaon(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: get wakeup reason fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    printf("Get wakeup reason response: %s\n", cmd->buf);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process scancfg configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_scancfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_scan_cfg *scancfg;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: scancfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    scancfg = (struct eth_priv_scan_cfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Scan Configuration:\n");
        printf("    Scan Type:              %d (%s)\n", scancfg->scan_type,
               (scancfg->scan_type == 1) ? "Active" : (scancfg->scan_type ==
                                                       2) ? "Passive" : "");
        printf("    Scan Mode:              %d (%s)\n", scancfg->scan_mode,
               (scancfg->scan_mode == 1) ? "BSS" : (scancfg->scan_mode ==
                                                    2) ? "IBSS" : (scancfg->
                                                                   scan_mode ==
                                                                   3) ? "Any" :
               "");
        printf("    Scan Probes:            %d (%s)\n", scancfg->scan_probe,
               "per channel");
        printf("    Specific Scan Time:     %d ms\n",
               scancfg->scan_time.specific_scan_time);
        printf("    Active Scan Time:       %d ms\n",
               scancfg->scan_time.active_scan_time);
        printf("    Passive Scan Time:      %d ms\n",
               scancfg->scan_time.passive_scan_time);
        printf("    Extended Scan Support:  %d (%s)\n", scancfg->ext_scan,
               (scancfg->ext_scan == 0) ? "No" : (scancfg->ext_scan ==
                                                  1) ? "Yes" : "");
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Process warmreset command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_warmreset(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    /* The argument being a string, this requires special handling */
    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: warmreset fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Process txpowercfg command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_txpowercfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_power_cfg_ext *power_ext = NULL;
    struct eth_priv_power_group *power_group = NULL;
    int i = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: txpowercfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    power_ext = (struct eth_priv_power_cfg_ext *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Tx Power Configurations:\n");
        power_group = (struct eth_priv_power_group *) (power_ext->power_data);
        for (i = 0; i < power_ext->len / 5; i++) {
            printf("    Power Group %d: \n", i);
            printf("        first rate index: %3d\n",
                   power_group->first_rate_ind);
            printf("        last rate index:  %3d\n",
                   power_group->last_rate_ind);
            printf("        minimum power:    %3d dBm\n",
                   power_group->power_min);
            printf("        maximum power:    %3d dBm\n",
                   power_group->power_max);
            printf("        power step:       %3d\n", power_group->power_step);
            printf("\n");
            power_group++;
        }
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Process pscfg command
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_pscfg(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_ds_ps_cfg *ps_cfg = NULL;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: pscfg fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    ps_cfg = (struct eth_priv_ds_ps_cfg *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("PS Configurations:\n");
        printf("%d", (int) ps_cfg->ps_null_interval);
        printf("  %d", (int) ps_cfg->multiple_dtim_interval);
        printf("  %d", (int) ps_cfg->listen_interval);
        printf("  %d  ", (int) ps_cfg->adhoc_awake_period);
        printf("  %d", (int) ps_cfg->bcn_miss_timeout);
        printf("  %d", (int) ps_cfg->delay_to_ps);
        printf("  %d", (int) ps_cfg->ps_mode);
        printf("\n");
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process sleeppd configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_sleeppd(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    int sleeppd = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: sleeppd fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    sleeppd = *(int *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Sleep Period: %d ms\n", sleeppd);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process tx control configuration
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_txcontrol(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    t_u32 txcontrol = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: txcontrol fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    txcontrol = *(t_u32 *) (cmd->buf);
    if (argc == 3) {
        /* GET operation */
        printf("Tx control: 0x%x\n", txcontrol);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/** custom IE, auto mask value */
#define	CUSTOM_IE_AUTO_MASK	0xffff

/**
 * @brief Show usage information for the customie
 * command
 *
 * $return         N/A
 **/
void
print_custom_ie_usage(void)
{
    printf("\nUsage : customie [INDEX] [MASK] [IEBuffer]");
    printf("\n         empty - Get all IE settings\n");
    printf("\n         INDEX:  0 - Get/Set IE index 0 setting");
    printf("\n                 1 - Get/Set IE index 1 setting");
    printf("\n                 2 - Get/Set IE index 2 setting");
    printf("\n                 3 - Get/Set IE index 3 setting");
    printf("\n                 .                             ");
    printf("\n                 .                             ");
    printf("\n                 .                             ");
    printf("\n                -1 - Append/Delete IE automatically");
    printf
        ("\n                     Delete will delete the IE from the matching IE buffer");
    printf
        ("\n                     Append will append the IE to the buffer with the same mask");
    printf
        ("\n         MASK :  Management subtype mask value as per bit defintions");
    printf("\n              :  Bit 0 - Association request.");
    printf("\n              :  Bit 1 - Association response.");
    printf("\n              :  Bit 2 - Reassociation request.");
    printf("\n              :  Bit 3 - Reassociation response.");
    printf("\n              :  Bit 4 - Probe request.");
    printf("\n              :  Bit 5 - Probe response.");
    printf("\n              :  Bit 8 - Beacon.");
    printf("\n         MASK :  MASK = 0 to clear the mask and the IE buffer");
    printf("\n         IEBuffer :  IE Buffer in hex (max 256 bytes)\n\n");
    return;
}

/** 
 * @brief Converts a string to hex value
 *
 * @param str      A pointer to the string
 * @param raw      A pointer to the raw data buffer
 * @return         Number of bytes read
 **/
int
string2raw(char *str, unsigned char *raw)
{
    int len = (strlen(str) + 1) / 2;

    do {
        if (!isxdigit(*str)) {
            return -1;
        }
        *str = toupper(*str);
        *raw = CHAR2INT(*str) << 4;
        ++str;
        *str = toupper(*str);
        if (*str == '\0')
            break;
        *raw |= CHAR2INT(*str);
        ++raw;
    } while (*++str != '\0');
    return len;
}

/**
 * @brief Creates a hostcmd request for custom IE settings
 * and sends to the driver
 *
 * Usage: "customie [INDEX] [MASK] [IEBuffer]"
 * 
 * Options: INDEX :      0 - Get/Delete IE index 0 setting
 *                       1 - Get/Delete IE index 1 setting
 *                       2 - Get/Delete IE index 2 setting
 *                       3 - Get/Delete IE index 3 setting
 *                       .
 *                       .
 *                       .
 *                      -1 - Append IE at the IE buffer with same MASK
 *          MASK  :      Management subtype mask value
 *          IEBuffer:    IE Buffer in hex
 *   					       empty - Get all IE settings
 * 
 * @param argc     Number of arguments
 * @param argv     Pointer to the arguments
 * @return         N/A
 **/
static int
process_customie(int argc, char *argv[])
{
    eth_priv_ds_misc_custom_ie *tlv = NULL;
    tlvbuf_max_mgmt_ie *max_mgmt_ie_tlv = NULL;
    t_u16 mgmt_subtype_mask = 0;
    custom_ie *ie_ptr = NULL;
    int ie_buf_len = 0, ie_len = 0, i = 0;
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }
    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* mlanutl mlan0 customie idx flag buf */
    if (argc > 6) {
        printf("ERR:Too many arguments.\n");
        print_custom_ie_usage();
        return MLAN_STATUS_FAILURE;
    }
    /* Error checks and initialize the command length */
    if (argc > 3) {
        if (((IS_HEX_OR_DIGIT(argv[3]) == MLAN_STATUS_FAILURE) &&
             (atoi(argv[3]) != -1)) || (atoi(argv[3]) < -1)) {
            printf("ERR:Illegal index %s\n", argv[3]);
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
    }
    switch (argc) {
    case 3:
        break;
    case 4:
        if (atoi(argv[3]) < 0) {
            printf
                ("ERR:Illegal index %s. Must be either greater than or equal to 0 for Get Operation \n",
                 argv[3]);
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        break;
    case 5:
        if (MLAN_STATUS_FAILURE == ishexstring(argv[4]) ||
            A2HEXDECIMAL(argv[4]) != 0) {
            printf("ERR: Mask value should be 0 to clear IEBuffers.\n");
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        if (atoi(argv[3]) == -1) {
            printf("ERR: You must provide buffer for automatic deletion.\n");
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        break;
    case 6:
        /* This is to check negative numbers and special symbols */
        if (MLAN_STATUS_FAILURE == IS_HEX_OR_DIGIT(argv[4])) {
            printf("ERR:Mask value must be 0 or hex digits\n");
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        /* If above check is passed and mask is not hex, then it must be 0 */
        if ((ISDIGIT(argv[4]) == MLAN_STATUS_SUCCESS) && atoi(argv[4])) {
            printf("ERR:Mask value must be 0 or hex digits\n ");
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        if (MLAN_STATUS_FAILURE == ishexstring(argv[5])) {
            printf("ERR:Only hex digits are allowed\n");
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        ie_buf_len = strlen(argv[5]);
        if (!strncasecmp("0x", argv[5], 2)) {
            ie_len = (ie_buf_len - 2 + 1) / 2;
            argv[5] += 2;
        } else
            ie_len = (ie_buf_len + 1) / 2;
        if (ie_len > MAX_IE_BUFFER_LEN) {
            printf("ERR:Incorrect IE length %d\n", ie_buf_len);
            print_custom_ie_usage();
            return MLAN_STATUS_FAILURE;
        }
        mgmt_subtype_mask = (t_u16) A2HEXDECIMAL(argv[4]);
        break;
    }
    /* Initialize the command buffer */
    tlv =
        (eth_priv_ds_misc_custom_ie *) (buffer + strlen(CMD_MARVELL) +
                                        strlen(argv[2]));
    tlv->type = MRVL_MGMT_IE_LIST_TLV_ID;
    if (argc == 3 || argc == 4) {
        if (argc == 3)
            tlv->len = 0;
        else {
            tlv->len = sizeof(t_u16);
            ie_ptr = (custom_ie *) (tlv->ie_data);
            ie_ptr->ie_index = (t_u16) (atoi(argv[3]));
        }
    } else {
        /* Locate headers */
        ie_ptr = (custom_ie *) (tlv->ie_data);
        /* Set TLV fields */
        tlv->len = sizeof(custom_ie) + ie_len;
        ie_ptr->ie_index = atoi(argv[3]);
        ie_ptr->mgmt_subtype_mask = mgmt_subtype_mask;
        ie_ptr->ie_length = ie_len;
        if (argc == 6)
            string2raw(argv[5], ie_ptr->ie_buffer);
    }
    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;
    /* Perform ioctl */
    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("ioctl[CUSTOM_IE_CFG]");
        printf("ERR:Command sending failed!\n");
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }
    /* Print response */
    if (argc > 4) {
        printf("Custom IE setting successful\n");
    } else {
        printf("Querying custom IE successful\n");
        ie_len = tlv->len;
        ie_ptr = (custom_ie *) (tlv->ie_data);
        while (ie_len >= sizeof(custom_ie)) {
            printf("Index [%d]\n", ie_ptr->ie_index);
            if (ie_ptr->ie_length)
                printf("Management Subtype Mask = 0x%02x\n",
                       (ie_ptr->mgmt_subtype_mask) == 0 ?
                       CUSTOM_IE_AUTO_MASK : (ie_ptr->mgmt_subtype_mask));
            else
                printf("Management Subtype Mask = 0x%02x\n",
                       (ie_ptr->mgmt_subtype_mask));
            hexdump("IE Buffer", (void *) ie_ptr->ie_buffer,
                    ie_ptr->ie_length, ' ');
            ie_len -= sizeof(custom_ie) + ie_ptr->ie_length;
            ie_ptr = (custom_ie *) ((t_u8 *) ie_ptr + sizeof(custom_ie) +
                                    ie_ptr->ie_length);
        }
    }
    max_mgmt_ie_tlv = (tlvbuf_max_mgmt_ie *) ((t_u8 *) tlv +
                                              sizeof(eth_priv_ds_misc_custom_ie)
                                              + tlv->len);
    if (max_mgmt_ie_tlv) {
        if (max_mgmt_ie_tlv->type == MRVL_MAX_MGMT_IE_TLV_ID) {
            for (i = 0; i < max_mgmt_ie_tlv->count; i++) {
                printf("buf%d_size = %d\n", i,
                       max_mgmt_ie_tlv->info[i].buf_size);
                printf("number of buffers = %d\n",
                       max_mgmt_ie_tlv->info[i].buf_count);
                printf("\n");
            }
        }
    }
    if (buffer)
        free(buffer);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process regrdwr command
 *
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return       MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_regrdwr(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_ds_reg_rw *reg = NULL;

    if (argc < 5 || argc > 6) {
        printf("Error: invalid no of arguments\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: regrdwr fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    reg = (struct eth_priv_ds_reg_rw *) (cmd->buf);
    if (argc == 5) {
        /* GET operation */
        printf("Value = 0x%x\n", reg->value);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process rdeeprom command
 *
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return       MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_rdeeprom(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_ds_read_eeprom *eeprom = NULL;
    int i = 0;

    if (argc != 5) {
        printf("Error: invalid no of arguments\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: rdeeprom fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    eeprom = (struct eth_priv_ds_read_eeprom *) (cmd->buf);
    if (argc == 5) {
        /* GET operation */
        printf("Value:\n");
        for (i = 0; i < MIN(MAX_EEPROM_DATA, eeprom->byte_count); i++)
            printf(" %02x", eeprom->value[i]);
        printf("\n");
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process memrdwr command
 *
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return       MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_memrdwr(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    struct eth_priv_ds_mem_rw *mem = NULL;

    if (argc < 4 || argc > 5) {
        printf("Error: invalid no of arguments\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: memrdwr fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    mem = (struct eth_priv_ds_mem_rw *) (cmd->buf);
    if (argc == 4) {
        /* GET operation */
        printf("Value = 0x%x\n", mem->value);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Process sdcmd52rw command
 *
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return       MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_sdcmd52rw(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    if (argc < 5 || argc > 6) {
        printf("Error: invalid no of arguments\n");
        return MLAN_STATUS_FAILURE;
    }

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: sdcmd52rw fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 5) {
        /* GET operation */
        printf("Value = 0x%x\n", (int) (*cmd->buf));
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

#define STACK_NBYTES            	100     /**< Number of bytes in stack */
#define MAX_BYTESEQ 		       	6       /**< Maximum byte sequence */
#define TYPE_DNUM           		1 /**< decimal number */
#define TYPE_BYTESEQ        		2 /**< byte sequence */
#define MAX_OPERAND         		0x40    /**< Maximum operands */
#define TYPE_EQ         		(MAX_OPERAND+1) /**< byte comparison:    == operator */
#define TYPE_EQ_DNUM    		(MAX_OPERAND+2) /**< decimal comparison: =d operator */
#define TYPE_EQ_BIT     		(MAX_OPERAND+3) /**< bit comparison:     =b operator */
#define TYPE_AND        		(MAX_OPERAND+4) /**< && operator */
#define TYPE_OR         		(MAX_OPERAND+5) /**< || operator */

typedef struct
{
    t_u16 sp;                         /**< Stack pointer */
    t_u8 byte[STACK_NBYTES];          /**< Stack */
} mstack_t;

typedef struct
{
    t_u8 type;                    /**< Type */
    t_u8 reserve[3];       /**< so 4-byte align val array */
    /* byte sequence is the largest among all the operands and operators. */
    /* byte sequence format: 1 byte of num of bytes, then variable num bytes */
    t_u8 val[MAX_BYTESEQ + 1];    /**< Value */
} op_t;

/** 
 *  @brief push data to stack
 *  
 *  @param s			a pointer to mstack_t structure
 *  
 *  @param nbytes		number of byte to push to stack  
 *  
 *  @param val			a pointer to data buffer	
 *  
 *  @return			TRUE-- sucess , FALSE -- fail
 *  			
 */
static int
push_n(mstack_t * s, t_u8 nbytes, t_u8 * val)
{
    if ((s->sp + nbytes) < STACK_NBYTES) {
        memcpy((void *) (s->byte + s->sp), (const void *) val, (size_t) nbytes);
        s->sp += nbytes;
        /* printf("push: n %d sp %d\n", nbytes, s->sp); */
        return TRUE;
    } else                      /* stack full */
        return FALSE;
}

/** 
 *  @brief push data to stack
 *  
 *  @param s			a pointer to mstack_t structure
 *  
 *  @param op			a pointer to op_t structure  
 *  
 *  @return			TRUE-- sucess , FALSE -- fail
 *  			
 */
static int
push(mstack_t * s, op_t * op)
{
    t_u8 nbytes;
    switch (op->type) {
    case TYPE_DNUM:
        if (push_n(s, 4, op->val))
            return (push_n(s, 1, &op->type));
        return FALSE;
    case TYPE_BYTESEQ:
        nbytes = op->val[0];
        if (push_n(s, nbytes, op->val + 1) &&
            push_n(s, 1, op->val) && push_n(s, 1, &op->type))
            return TRUE;
        return FALSE;
    default:
        return (push_n(s, 1, &op->type));
    }
}

/** 
 *  @brief parse RPN string
 *  
 *  @param s			a pointer to Null-terminated string to scan. 
 *  
 *  @param first_time		a pointer to return first_time  
 *  
 *  @return			A pointer to the last token found in string.   
 *  				NULL is returned when there are no more tokens to be found. 
 *  			
 */
static char *
getop(char *s, int *first_time)
{
    const char delim[] = " \t\n";
    char *p;
    if (*first_time) {
        p = strtok(s, delim);
        *first_time = FALSE;
    } else {
        p = strtok(NULL, delim);
    }
    return (p);
}

/** 
 *  @brief Verify hex digit.
 *  
 *  @param c			input ascii char 
 *  @param h			a pointer to return integer value of the digit char. 
 *  @return			TURE -- c is hex digit, FALSE -- c is not hex digit.
 */
static int
ishexdigit(char c, t_u8 * h)
{
    if (c >= '0' && c <= '9') {
        *h = c - '0';
        return (TRUE);
    } else if (c >= 'a' && c <= 'f') {
        *h = c - 'a' + 10;
        return (TRUE);
    } else if (c >= 'A' && c <= 'F') {
        *h = c - 'A' + 10;
        return (TRUE);
    }
    return (FALSE);
}

/** 
 *  @brief convert hex string to integer.
 *  
 *  @param s			A pointer to hex string, string length up to 2 digits. 
 *  @return			integer value.
 */
static t_u8
hex_atoi(char *s)
{
    int i;
    t_u8 digit;                 /* digital value */
    t_u8 t = 0;                 /* total value */

    for (i = 0, t = 0; ishexdigit(s[i], &digit) && i < 2; i++)
        t = 16 * t + digit;
    return (t);
}

/** 
 *  @brief Parse byte sequence in hex format string to a byte sequence.
 *  
 *  @param opstr		A pointer to byte sequence in hex format string, with ':' as delimiter between two byte. 
 *  @param val			A pointer to return byte sequence string
 *  @return			NA
 */
static void
parse_hex(char *opstr, t_u8 * val)
{
    char delim = ':';
    char *p;
    char *q;
    t_u8 i;

    /* +1 is for skipping over the preceding h character. */
    p = opstr + 1;

    /* First byte */
    val[1] = hex_atoi(p++);

    /* Parse subsequent bytes. */
    /* Each byte is preceded by the : character. */
    for (i = 1; *p; i++) {
        q = strchr(p, delim);
        if (!q)
            break;
        p = q + 1;
        val[i + 1] = hex_atoi(p);
    }
    /* Set num of bytes */
    val[0] = i;
}

/** 
 *  @brief str2bin, convert RPN string to binary format
 *  
 *  @param str			A pointer to rpn string
 *  @param stack		A pointer to mstack_t structure
 *  @return			MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
str2bin(char *str, mstack_t * stack)
{
    int first_time;
    char *opstr;
    op_t op;                    /* operator/operand */
    int dnum;
    int ret = MLAN_STATUS_SUCCESS;

    memset(stack, 0, sizeof(mstack_t));
    first_time = TRUE;
    while ((opstr = getop(str, &first_time)) != NULL) {
        if (isdigit(*opstr)) {
            op.type = TYPE_DNUM;
            dnum = cpu_to_le32(atoi(opstr));
            memcpy((t_u8 *) op.val, &dnum, sizeof(dnum));
            if (!push(stack, &op)) {
                printf("push decimal number failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (*opstr == 'h') {
            op.type = TYPE_BYTESEQ;
            parse_hex(opstr, op.val);
            if (!push(stack, &op)) {
                printf("push byte sequence failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (!strcmp(opstr, "==")) {
            op.type = TYPE_EQ;
            if (!push(stack, &op)) {
                printf("push byte cmp operator failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (!strcmp(opstr, "=d")) {
            op.type = TYPE_EQ_DNUM;
            if (!push(stack, &op)) {
                printf("push decimal cmp operator failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (!strcmp(opstr, "=b")) {
            op.type = TYPE_EQ_BIT;
            if (!push(stack, &op)) {
                printf("push bit cmp operator failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (!strcmp(opstr, "&&")) {
            op.type = TYPE_AND;
            if (!push(stack, &op)) {
                printf("push AND operator failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else if (!strcmp(opstr, "||")) {
            op.type = TYPE_OR;
            if (!push(stack, &op)) {
                printf("push OR operator failed\n");
                ret = MLAN_STATUS_FAILURE;
                break;
            }
        } else {
            printf("Unknown operand\n");
            ret = MLAN_STATUS_FAILURE;
            break;
        }
    }
    return ret;
}

#define FILTER_BYTESEQ 		TYPE_EQ /**< byte sequence */
#define FILTER_DNUM    		TYPE_EQ_DNUM /**< decimal number */
#define FILTER_BITSEQ		TYPE_EQ_BIT /**< bit sequence */
#define FILTER_TEST		FILTER_BITSEQ+1 /**< test */

#define NAME_TYPE		1           /**< Field name 'type' */
#define NAME_PATTERN		2       /**< Field name 'pattern' */
#define NAME_OFFSET		3           /**< Field name 'offset' */
#define NAME_NUMBYTE		4       /**< Field name 'numbyte' */
#define NAME_REPEAT		5           /**< Field name 'repeat' */
#define NAME_BYTE		6           /**< Field name 'byte' */
#define NAME_MASK		7           /**< Field name 'mask' */
#define NAME_DEST		8           /**< Field name 'dest' */

static struct mef_fields
{
    t_s8 *name;
              /**< Name */
    t_s8 nameid;/**< Name Id. */
} mef_fields[] = {
    {
    "type", NAME_TYPE}, {
    "pattern", NAME_PATTERN}, {
    "offset", NAME_OFFSET}, {
    "numbyte", NAME_NUMBYTE}, {
    "repeat", NAME_REPEAT}, {
    "byte", NAME_BYTE}, {
    "mask", NAME_MASK}, {
    "dest", NAME_DEST}
};

/** 
 *  @brief get filter data
 *  
 *  @param fp			A pointer to file stream
 *  @param ln			A pointer to line number
 *  @param buf			A pointer to hostcmd data
 *  @param size			A pointer to the return size of hostcmd buffer
 *  @return			MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
mlan_get_filter_data(FILE * fp, int *ln, t_u8 * buf, t_u16 * size)
{
    t_s32 errors = 0, i;
    t_s8 line[256], *pos = NULL, *pos1 = NULL;
    t_u16 type = 0;
    t_u32 pattern = 0;
    t_u16 repeat = 0;
    t_u16 offset = 0;
    t_s8 byte_seq[50];
    t_s8 mask_seq[50];
    t_u16 numbyte = 0;
    t_s8 type_find = 0;
    t_s8 pattern_find = 0;
    t_s8 offset_find = 0;
    t_s8 numbyte_find = 0;
    t_s8 repeat_find = 0;
    t_s8 byte_find = 0;
    t_s8 mask_find = 0;
    t_s8 dest_find = 0;
    t_s8 dest_seq[50];

    *size = 0;
    while ((pos = mlan_config_get_line(fp, line, sizeof(line), ln))) {
        if (strcmp(pos, "}") == 0) {
            break;
        }
        pos1 = strchr(pos, '=');
        if (pos1 == NULL) {
            printf("Line %d: Invalid mef_filter line '%s'\n", *ln, pos);
            errors++;
            continue;
        }
        *pos1++ = '\0';
        for (i = 0; (t_u32) i < NELEMENTS(mef_fields); i++) {
            if (strncmp(pos, mef_fields[i].name, strlen(mef_fields[i].name)) ==
                0) {
                switch (mef_fields[i].nameid) {
                case NAME_TYPE:
                    type = a2hex_or_atoi(pos1);
                    if ((type != FILTER_DNUM) && (type != FILTER_BYTESEQ)
                        && (type != FILTER_BITSEQ) && (type != FILTER_TEST)) {
                        printf("Invalid filter type:%d\n", type);
                        return MLAN_STATUS_FAILURE;
                    }
                    type_find = 1;
                    break;
                case NAME_PATTERN:
                    pattern = a2hex_or_atoi(pos1);
                    pattern_find = 1;
                    break;
                case NAME_OFFSET:
                    offset = a2hex_or_atoi(pos1);
                    offset_find = 1;
                    break;
                case NAME_NUMBYTE:
                    numbyte = a2hex_or_atoi(pos1);
                    numbyte_find = 1;
                    break;
                case NAME_REPEAT:
                    repeat = a2hex_or_atoi(pos1);
                    repeat_find = 1;
                    break;
                case NAME_BYTE:
                    memset(byte_seq, 0, sizeof(byte_seq));
                    strncpy(byte_seq, pos1, (sizeof(byte_seq) - 1));
                    byte_find = 1;
                    break;
                case NAME_MASK:
                    memset(mask_seq, 0, sizeof(mask_seq));
                    strncpy(mask_seq, pos1, (sizeof(mask_seq) - 1));
                    mask_find = 1;
                    break;
                case NAME_DEST:
                    memset(dest_seq, 0, sizeof(dest_seq));
                    strncpy(dest_seq, pos1, (sizeof(dest_seq) - 1));
                    dest_find = 1;
                    break;
                }
                break;
            }
        }
        if (i == NELEMENTS(mef_fields)) {
            printf("Line %d: unknown mef field '%s'.\n", *line, pos);
            errors++;
        }
    }
    if (type_find == 0) {
        printf("Can not find filter type\n");
        return MLAN_STATUS_FAILURE;
    }
    switch (type) {
    case FILTER_DNUM:
        if (!pattern_find || !offset_find || !numbyte_find) {
            printf
                ("Missing field for FILTER_DNUM: pattern=%d,offset=%d,numbyte=%d\n",
                 pattern_find, offset_find, numbyte_find);
            return MLAN_STATUS_FAILURE;
        }
        memset(line, 0, sizeof(line));
        snprintf(line, sizeof(line), "%d %d %d =d ", pattern, offset, numbyte);
        break;
    case FILTER_BYTESEQ:
        if (!byte_find || !offset_find || !repeat_find) {
            printf
                ("Missing field for FILTER_BYTESEQ: byte=%d,offset=%d,repeat=%d\n",
                 byte_find, offset_find, repeat_find);
            return MLAN_STATUS_FAILURE;
        }
        memset(line, 0, sizeof(line));
        snprintf(line, sizeof(line), "%d h%s %d == ", repeat, byte_seq, offset);
        break;
    case FILTER_BITSEQ:
        if (!byte_find || !offset_find || !mask_find) {
            printf
                ("Missing field for FILTER_BITSEQ: byte=%d,offset=%d,mask_find=%d\n",
                 byte_find, offset_find, mask_find);
            return MLAN_STATUS_FAILURE;
        }
        if (strlen(byte_seq) != strlen(mask_seq)) {
            printf("byte string's length is different with mask's length!\n");
            return MLAN_STATUS_FAILURE;
        }
        memset(line, 0, sizeof(line));
        snprintf(line, sizeof(line), "h%s %d h%s =b ", byte_seq, offset,
                 mask_seq);
        break;
    case FILTER_TEST:
        if (!byte_find || !offset_find || !repeat_find || !dest_find) {
            printf
                ("Missing field for FILTER_TEST: byte=%d,offset=%d,repeat=%d,dest=%d\n",
                 byte_find, offset_find, repeat_find, dest_find);
            return MLAN_STATUS_FAILURE;
        }
        memset(line, 0, sizeof(line));
        snprintf(line, sizeof(line), "h%s %d h%s %d ", dest_seq, repeat,
                 byte_seq, offset);
        break;
    }
    memcpy(buf, line, strlen(line));
    *size = strlen(line);
    return MLAN_STATUS_SUCCESS;
}

#define NAME_MODE	1       /**< Field name 'mode' */
#define NAME_ACTION	2       /**< Field name 'action' */
#define NAME_FILTER_NUM	3   /**< Field name 'filter_num' */
#define NAME_RPN	4       /**< Field name 'RPN' */
static struct mef_entry_fields
{
    t_s8 *name;
              /**< Name */
    t_s8 nameid;/**< Name id */
} mef_entry_fields[] = {
    {
    "mode", NAME_MODE}, {
    "action", NAME_ACTION}, {
    "filter_num", NAME_FILTER_NUM}, {
"RPN", NAME_RPN},};

typedef struct _MEF_ENTRY
{
    /** Mode */
    t_u8 Mode;
    /** Size */
    t_u8 Action;
    /** Size of expression */
    t_u16 ExprSize;
} MEF_ENTRY;

/** 
 *  @brief get mef_entry data
 *  
 *  @param fp			A pointer to file stream
 *  @param ln			A pointer to line number
 *  @param buf			A pointer to hostcmd data
 *  @param size			A pointer to the return size of hostcmd buffer
 *  @return			MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
mlan_get_mef_entry_data(FILE * fp, int *ln, t_u8 * buf, t_u16 * size)
{
    t_s8 line[256], *pos = NULL, *pos1 = NULL;
    t_u8 mode, action, filter_num = 0;
    t_s8 rpn[256];
    t_s8 mode_find = 0;
    t_s8 action_find = 0;
    t_s8 filter_num_find = 0;
    t_s8 rpn_find = 0;
    t_s8 rpn_str[256];
    int rpn_len = 0;
    t_s8 filter_name[50];
    t_s8 name_found = 0;
    t_u16 len = 0;
    int i;
    int first_time = TRUE;
    char *opstr = NULL;
    t_s8 filter_action[10];
    t_s32 errors = 0;
    MEF_ENTRY *pMefEntry = (MEF_ENTRY *) buf;
    mstack_t stack;
    while ((pos = mlan_config_get_line(fp, line, sizeof(line), ln))) {
        if (strcmp(pos, "}") == 0) {
            break;
        }
        pos1 = strchr(pos, '=');
        if (pos1 == NULL) {
            printf("Line %d: Invalid mef_entry line '%s'\n", *ln, pos);
            errors++;
            continue;
        }
        *pos1++ = '\0';
        if (!mode_find || !action_find || !filter_num_find || !rpn_find) {
            for (i = 0; (unsigned int) i < NELEMENTS(mef_entry_fields); i++) {
                if (strncmp
                    (pos, mef_entry_fields[i].name,
                     strlen(mef_entry_fields[i].name)) == 0) {
                    switch (mef_entry_fields[i].nameid) {
                    case NAME_MODE:
                        mode = a2hex_or_atoi(pos1);
                        if (mode & ~0x7) {
                            printf("invalid mode=%d\n", mode);
                            return MLAN_STATUS_FAILURE;
                        }
                        pMefEntry->Mode = mode;
                        mode_find = 1;
                        break;
                    case NAME_ACTION:
                        action = a2hex_or_atoi(pos1);
                        if (action & ~0xff) {
                            printf("invalid action=%d\n", action);
                            return MLAN_STATUS_FAILURE;
                        }
                        pMefEntry->Action = action;
                        action_find = 1;
                        break;
                    case NAME_FILTER_NUM:
                        filter_num = a2hex_or_atoi(pos1);
                        filter_num_find = 1;
                        break;
                    case NAME_RPN:
                        memset(rpn, 0, sizeof(rpn));
                        strncpy(rpn, pos1, (sizeof(rpn) - 1));
                        rpn_find = 1;
                        break;
                    }
                    break;
                }
            }
            if (i == NELEMENTS(mef_fields)) {
                printf("Line %d: unknown mef_entry field '%s'.\n", *line, pos);
                return MLAN_STATUS_FAILURE;
            }
        }
        if (mode_find && action_find && filter_num_find && rpn_find) {
            for (i = 0; i < filter_num; i++) {
                opstr = getop(rpn, &first_time);
                if (opstr == NULL)
                    break;
                snprintf(filter_name, sizeof(filter_name), "%s={", opstr);
                name_found = 0;
                while ((pos = mlan_config_get_line(fp, line, sizeof(line), ln))) {
                    if (strncmp(pos, filter_name, strlen(filter_name)) == 0) {
                        name_found = 1;
                        break;
                    }
                }
                if (!name_found) {
                    fprintf(stderr, "mlanutl: %s not found in file\n",
                            filter_name);
                    return MLAN_STATUS_FAILURE;
                }
                if (MLAN_STATUS_FAILURE ==
                    mlan_get_filter_data(fp, ln, (t_u8 *) (rpn_str + rpn_len),
                                         &len))
                    break;
                rpn_len += len;
                if (i > 0) {
                    memcpy(rpn_str + rpn_len, filter_action,
                           strlen(filter_action));
                    rpn_len += strlen(filter_action);
                }
                opstr = getop(rpn, &first_time);
                if (opstr == NULL)
                    break;
                memset(filter_action, 0, sizeof(filter_action));
                snprintf(filter_action, sizeof(filter_action), "%s ", opstr);
            }
            /* Remove the last space */
            if (rpn_len > 0) {
                rpn_len--;
                rpn_str[rpn_len] = 0;
            }
            if (MLAN_STATUS_FAILURE == str2bin(rpn_str, &stack)) {
                printf("Fail on str2bin!\n");
                return MLAN_STATUS_FAILURE;
            }
            *size = sizeof(MEF_ENTRY);
            pMefEntry->ExprSize = cpu_to_le16(stack.sp);
            memmove(buf + sizeof(MEF_ENTRY), stack.byte, stack.sp);
            *size += stack.sp;
            break;
        } else if (mode_find && action_find && filter_num_find &&
                   (filter_num == 0)) {
            pMefEntry->ExprSize = 0;
            *size = sizeof(MEF_ENTRY);
            break;
        }
    }
    return MLAN_STATUS_SUCCESS;
}

#define MEFCFG_CMDCODE	0x009a

/**
 *  @brief Process mefcfg command
 *  @param argc     number of arguments
 *  @param argv     A pointer to arguments array    
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_mefcfg(int argc, char *argv[])
{
    t_s8 line[256], cmdname[256], *pos = NULL;
    int cmdname_found = 0, name_found = 0;
    int ln = 0;
    int ret = MLAN_STATUS_SUCCESS;
    int i;
    t_u8 *buffer = NULL;
    t_u16 len;
    HostCmd_DS_MEF_CFG *mefcmd = NULL;
    HostCmd_DS_GEN *hostcmd = NULL;
    FILE *fp = NULL;
    t_u32 cmd_len = 0, cmd_header_len;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    if (argc < 4) {
        printf("Error: invalid no of arguments\n");
        printf("Syntax: ./mlanutl mlan0 mefcfg <mef.conf>\n");
        exit(1);
    }

    cmd_header_len = strlen(CMD_MARVELL) + strlen("HOSTCMD");
    cmd_len = sizeof(HostCmd_DS_GEN) + sizeof(HostCmd_DS_MEF_CFG);
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (buffer == NULL) {
        fclose(fp);
        fprintf(stderr, "Cannot alloc memory\n");
        exit(1);
    }
    memset(buffer, 0, BUFFER_LENGTH);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;
    /* buf = MRVL_CMD<cmd> */
    prepare_buffer(buffer, HOSTCMD, 0, NULL);

    /* buf = MRVL_CMD<cmd><hostcmd_size><HostCmd_DS_GEN> */
    hostcmd = (HostCmd_DS_GEN *) (buffer + cmd_header_len + sizeof(t_u32));
    hostcmd->command = cpu_to_le16(MEFCFG_CMDCODE);
    hostcmd->seq_num = 0;
    hostcmd->result = 0;
    /* buf = MRVL_CMD<cmd><hostcmd_size><HostCmd_DS_GEN><HostCmd_DS_MEF_CFG> */
    mefcmd =
        (HostCmd_DS_MEF_CFG *) (buffer + cmd_header_len + sizeof(t_u32) +
                                S_DS_GEN);

/* Host Command Population */
    snprintf(cmdname, sizeof(cmdname), "%s={", argv[2]);
    cmdname_found = 0;
    if ((fp = fopen(argv[3], "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s\n", argv[4]);
        exit(1);
    }

    while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
        if (strcmp(pos, cmdname) == 0) {
            cmdname_found = 1;
            snprintf(cmdname, sizeof(cmdname), "Criteria=");
            name_found = 0;
            while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
                if (strncmp(pos, cmdname, strlen(cmdname)) == 0) {
                    name_found = 1;
                    mefcmd->Criteria = a2hex_or_atoi(pos + strlen(cmdname));
                    break;
                }
            }
            if (!name_found) {
                fprintf(stderr, "mlanutl: criteria not found in file '%s'\n",
                        argv[3]);
                break;
            }
            snprintf(cmdname, sizeof(cmdname), "NumEntries=");
            name_found = 0;
            while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
                if (strncmp(pos, cmdname, strlen(cmdname)) == 0) {
                    name_found = 1;
                    mefcmd->NumEntries = a2hex_or_atoi(pos + strlen(cmdname));
                    break;
                }
            }
            if (!name_found) {
                fprintf(stderr, "mlanutl: NumEntries not found in file '%s'\n",
                        argv[3]);
                break;
            }
            for (i = 0; i < mefcmd->NumEntries; i++) {
                snprintf(cmdname, sizeof(cmdname), "mef_entry_%d={", i);
                name_found = 0;
                while ((pos =
                        mlan_config_get_line(fp, line, sizeof(line), &ln))) {
                    if (strncmp(pos, cmdname, strlen(cmdname)) == 0) {
                        name_found = 1;
                        break;
                    }
                }
                if (!name_found) {
                    fprintf(stderr, "mlanutl: %s not found in file '%s'\n",
                            cmdname, argv[3]);
                    break;
                }
                if (MLAN_STATUS_FAILURE ==
                    mlan_get_mef_entry_data(fp, &ln, (t_u8 *) hostcmd + cmd_len,
                                            &len)) {
                    ret = MLAN_STATUS_FAILURE;
                    break;
                }
                cmd_len += len;
            }
            break;
        }
    }
    fclose(fp);
    /* buf = MRVL_CMD<cmd><hostcmd_size> */
    memcpy(buffer + cmd_header_len, (t_u8 *) & cmd_len, sizeof(t_u32));

    if (!cmdname_found)
        fprintf(stderr, "mlanutl: cmdname '%s' not found in file '%s'\n",
                argv[4], argv[3]);

    if (!cmdname_found || !name_found) {
        ret = MLAN_STATUS_FAILURE;
        goto mef_exit;
    }
    hostcmd->size = cpu_to_le16(cmd_len);
    mefcmd->Criteria = cpu_to_le32(mefcmd->Criteria);
    mefcmd->NumEntries = cpu_to_le16(mefcmd->NumEntries);
    hexdump("mefcfg", buffer + cmd_header_len, cmd_len, ' ');

    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;
    /* Perform ioctl */
    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("ioctl[MEF_CFG]");
        printf("ERR:Command sending failed!\n");
        if (buffer)
            free(buffer);
        if (cmd)
            free(cmd);
        return MLAN_STATUS_FAILURE;
    }

    ret = process_host_cmd_resp(HOSTCMD, buffer);

  mef_exit:
    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);
    return ret;

}

#ifdef STA_SUPPORT
/** 
 *  @brief Prepare ARP filter buffer 
 *  @param fp		File handler
 *  @param buf		A pointer to the buffer    
 *  @param length	A pointer to the length of buffer     
 *  @return      	MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
prepare_arp_filter_buffer(FILE * fp, t_u8 * buf, t_u16 * length,
                          int cmd_header_len)
{
    t_s8 line[256], *pos = NULL;
    int ln = 0;
    int ret = MLAN_STATUS_SUCCESS;
    int arpfilter_found = 0;

    memset(buf, 0, BUFFER_LENGTH - cmd_header_len - sizeof(t_u32));
    while ((pos = mlan_config_get_line(fp, line, sizeof(line), &ln))) {
        if (strcmp(pos, "arpfilter={") == 0) {
            arpfilter_found = 1;
            mlan_get_hostcmd_data(fp, &ln, buf, length);
            break;
        }
    }
    if (!arpfilter_found) {
        fprintf(stderr, "mlanutl: 'arpfilter' not found in conf file");
        ret = MLAN_STATUS_FAILURE;
    }
    return ret;
}

/** 
 *  @brief Process arpfilter
 *  @param argc   number of arguments
 *  @param argv   A pointer to arguments array    
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_arpfilter(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    t_u16 length = 0;
    int ret = MLAN_STATUS_SUCCESS;
    FILE *fp = NULL;
    int cmd_header_len = 0;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    if (argc < 4) {
        printf("Error: invalid no of arguments\n");
        printf("Syntax: ./mlanutl mlanX arpfilter <arpfilter.conf>\n");
        exit(1);
    }

    cmd_header_len = strlen(CMD_MARVELL) + strlen(argv[2]);

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (buffer == NULL) {
        printf("Error: allocate memory for arpfilter failed\n");
        fclose(fp);
        return -ENOMEM;
    }
    memset(buffer, 0, BUFFER_LENGTH);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;
    /* buf = MRVL_CMD<cmd> */
    prepare_buffer(buffer, argv[2], 0, NULL);

    /* Reading the configuration file */
    if ((fp = fopen(argv[3], "r")) == NULL) {
        fprintf(stderr, "Cannot open file %s\n", argv[3]);
        ret = MLAN_STATUS_FAILURE;
        goto arp_exit;
    }
    ret =
        prepare_arp_filter_buffer(fp, buffer + cmd_header_len + sizeof(t_u32),
                                  &length, cmd_header_len);
    fclose(fp);

    if (ret == MLAN_STATUS_FAILURE)
        goto arp_exit;

    /* buf = MRVL_CMD<cmd><hostcmd_size> */
    memcpy(buffer + cmd_header_len, (t_u8 *) & length, sizeof(t_u32));

    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;
    /* Perform ioctl */
    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("ioctl[ARP_FILTER]");
        printf("ERR:Command sending failed!\n");
        if (buffer)
            free(buffer);
        if (cmd)
            free(cmd);
        return MLAN_STATUS_FAILURE;
    }

  arp_exit:
    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return ret;
}
#endif /* STA_SUPPORT */

/** 
 *  @brief parse hex data
 *  @param fp       File handler
 *  @param dst      A pointer to receive hex data
 *  @return         length of hex data
 */
int
fparse_for_hex(FILE * fp, t_u8 * dst)
{
    t_s8 *ptr = NULL;
    t_u8 *dptr = NULL;
    t_s8 buf[256];

    dptr = dst;
    while (fgets(buf, sizeof(buf), fp)) {
        ptr = buf;

        while (*ptr) {
            /* skip leading spaces */
            while (*ptr && (isspace(*ptr) || *ptr == '\t'))
                ptr++;

            /* skip blank lines and lines beginning with '#' */
            if (*ptr == '\0' || *ptr == '#')
                break;

            if (isxdigit(*ptr)) {
                ptr = convert2hex(ptr, dptr++);
            } else {
                /* Invalid character on data line */
                ptr++;
            }
        }
    }

    return (dptr - dst);
}

/** Config data header length */
#define CFG_DATA_HEADER_LEN 6

/** 
 *  @brief Prepare cfg-data buffer 
 *  @param argc     number of arguments
 *  @param argv     A pointer to arguments array    
 *  @param fp       File handler
 *  @param buf      A pointer to comand buffer    
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
prepare_cfg_data_buffer(int argc, char *argv[], FILE * fp, t_u8 * buf,
                        int cmd_header_len)
{
    int ln = 0, type;
    HostCmd_DS_GEN *hostcmd = NULL;
    HostCmd_DS_802_11_CFG_DATA *pcfg_data = NULL;

    memset(buf, 0, BUFFER_LENGTH - cmd_header_len - sizeof(t_u32));
    hostcmd = (HostCmd_DS_GEN *) buf;
    hostcmd->command = cpu_to_le16(HostCmd_CMD_CFG_DATA);
    pcfg_data = (HostCmd_DS_802_11_CFG_DATA *) (buf + S_DS_GEN);
    pcfg_data->action = (argc == 4) ? HostCmd_ACT_GEN_GET : HostCmd_ACT_GEN_SET;
    type = atoi(argv[3]);
    if ((type < 1) || (type > 2)) {
        fprintf(stderr, "mlanutl: Invalid register type\n");
        return MLAN_STATUS_FAILURE;
    } else {
        pcfg_data->type = type;
    }
    if (argc == 5) {
        ln = fparse_for_hex(fp, pcfg_data->data);
    }
    pcfg_data->data_len = ln;
    hostcmd->size =
        cpu_to_le16(pcfg_data->data_len + S_DS_GEN + CFG_DATA_HEADER_LEN);
    pcfg_data->data_len = cpu_to_le16(pcfg_data->data_len);
    pcfg_data->type = cpu_to_le16(pcfg_data->type);
    pcfg_data->action = cpu_to_le16(pcfg_data->action);

    hostcmd->seq_num = 0;
    hostcmd->result = 0;
    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Process cfgdata 
 *  @param argc     number of arguments
 *  @param argv     A pointer to arguments array    
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_cfgdata(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    HostCmd_DS_GEN *hostcmd = NULL;
    int ret = MLAN_STATUS_SUCCESS;
    FILE *fp = NULL;
    int cmd_header_len = 0;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    if (argc < 4 || argc > 5) {
        printf("Error: invalid no of arguments\n");
        printf("Syntax: ./mlanutl mlanX cfgdata <register type> <filename>\n");
        exit(1);
    }

    if (argc == 5) {
        if ((fp = fopen(argv[4], "r")) == NULL) {
            fprintf(stderr, "Cannot open file %s\n", argv[3]);
            exit(1);
        }
    }

    cmd_header_len = strlen(CMD_MARVELL) + strlen(HOSTCMD);

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (buffer == NULL) {
        printf("Error: allocate memory for hostcmd failed\n");
        if (argc == 5)
            fclose(fp);
        return -ENOMEM;
    }
    memset(buffer, 0, BUFFER_LENGTH);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }
    memset(cmd, 0, sizeof(struct eth_priv_cmd));

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;
    /* buf = MRVL_CMD<cmd> */
    prepare_buffer(buffer, HOSTCMD, 0, NULL);

    /* buf = MRVL_CMD<cmd><hostcmd_size><HostCmd_DS_GEN> */
    hostcmd = (HostCmd_DS_GEN *) (buffer + cmd_header_len + sizeof(t_u32));

    ret =
        prepare_cfg_data_buffer(argc, argv, fp, (t_u8 *) hostcmd,
                                cmd_header_len);
    if (argc == 5)
        fclose(fp);

    if (ret == MLAN_STATUS_FAILURE)
        goto _exit_;

    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;
    /* Perform ioctl */
    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("ioctl[CFG_DATA]");
        printf("ERR:Command sending failed!\n");
        if (buffer)
            free(buffer);
        if (cmd)
            free(cmd);
        return MLAN_STATUS_FAILURE;
    }
    ret = process_host_cmd_resp(HOSTCMD, buffer);

  _exit_:
    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return ret;
}

/** 
 *  @brief Process transmission of mgmt frames 
 *  @param argc   number of arguments
 *  @param argv   A pointer to arguments array    
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_mgmtframetx(int argc, char *argv[])
{
    struct ifreq ifr;
    char *line = NULL;
    FILE *config_file = NULL;
    int li = 0, arg_num = 0, ret = 0, i = 0;
    char *args[100], *pos = NULL, mac_addr[20];
    t_u8 peer_mac[ETH_ALEN];
    t_u16 data_len = 0, subtype = 0;
    eth_priv_mgmt_frame_tx *pmgmt_frame = NULL;
    t_u8 *buffer = NULL;
    pkt_header *hdr = NULL;

    /* Check arguments */
    if (argc != 4) {
        printf("ERR:Incorrect number of arguments.\n");
        printf
            ("Syntax: ./mlanutl mlanX mgmtframetx <config/mgmt_frame.conf>\n");
        exit(1);
    }

    data_len = sizeof(eth_priv_mgmt_frame_tx);

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate memory!\n");
        goto done;
    }
    memset(buffer, 0, BUFFER_LENGTH);
    hdr = (pkt_header *) buffer;
    pmgmt_frame = (eth_priv_mgmt_frame_tx *) (buffer + sizeof(pkt_header));

    /* Check if file exists */
    config_file = fopen(argv[3], "r");
    if (config_file == NULL) {
        printf("\nERR:Config file can not open.\n");
        goto done;
    }
    line = (char *) malloc(MAX_CONFIG_LINE);
    if (!line) {
        printf("ERR:Cannot allocate memory for line\n");
        goto done;
    }
    memset(line, 0, MAX_CONFIG_LINE);

    /* Parse file and process */
    while (config_get_line(line, MAX_CONFIG_LINE, config_file, &li, &pos)) {
        arg_num = parse_line(line, args);
        if (strcmp(args[0], "PktSubType") == 0) {
            subtype = (t_u16) A2HEXDECIMAL(args[1]);
            pmgmt_frame->frm_ctl |= subtype << 4;
        } else if (strncmp(args[0], "Addr", 4) == 0) {
            strncpy(mac_addr, args[1], 20);
            if ((ret = mac2raw(mac_addr, peer_mac)) != MLAN_STATUS_SUCCESS) {
                printf("ERR: %s Address \n",
                       ret == MLAN_STATUS_FAILURE ? "Invalid MAC" : ret ==
                       MAC_BROADCAST ? "Broadcast" : "Multicast");
                goto done;
            }
            i = atoi(args[0] + 4);
            switch (i) {
            case 1:
                memcpy(pmgmt_frame->addr1, peer_mac, ETH_ALEN);
                break;
            case 2:
                memcpy(pmgmt_frame->addr2, peer_mac, ETH_ALEN);
                break;
            case 3:
                memcpy(pmgmt_frame->addr3, peer_mac, ETH_ALEN);
                break;
            case 4:
                memcpy(pmgmt_frame->addr4, peer_mac, ETH_ALEN);
                break;
            }
        } else if (strcmp(args[0], "Data") == 0) {
            for (i = 0; i < arg_num - 1; i++)
                pmgmt_frame->payload[i] = (t_u8) A2HEXDECIMAL(args[i + 1]);
            data_len += arg_num - 1;
        }
    }
    pmgmt_frame->frm_len = data_len - sizeof(pmgmt_frame->frm_len);
#define MRVL_PKT_TYPE_MGMT_FRAME 0xE5
    hdr->pkt_len = data_len;
    hdr->TxPktType = MRVL_PKT_TYPE_MGMT_FRAME;
    hdr->TxControl = 0;
    hexdump("Frame Tx", buffer, data_len + sizeof(pkt_header), ' ');
    /* Send collective command */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) buffer;

    /* Perform ioctl */
    if (ioctl(sockfd, FRAME_TX_IOCTL, &ifr)) {
        perror("");
        printf("ERR:Could not send management frame.\n");
    } else {
        printf("Mgmt Frame sucessfully sent.\n");
    }

  done:
    if (config_file)
        fclose(config_file);
    if (buffer)
        free(buffer);
    if (line)
        free(line);
    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief set/get management frame passthrough
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_mgmt_frame_passthrough(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    t_u32 mask = 0;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], (argc - 3), &argv[3]);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: htcapinfo fail\n");
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    mask = *(t_u32 *) (cmd->buf);
    if (argc == 3)
        printf("Registed Management Frame Mask: 0x%x\n", mask);

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/**
 *  @brief Send a WMM AC Queue configuration command to get/set/default params
 *
 *  Configure or get the parameters of a WMM AC queue. The command takes
 *    an optional Queue Id as a last parameter.  Without the queue id, all
 *    queues will be acted upon.
 *  
 *  mlanutl mlanX qconfig set msdu <lifetime in TUs> [Queue Id: 0-3]
 *  mlanutl mlanX qconfig get [Queue Id: 0-3]
 *  mlanutl mlanX qconfig def [Queue Id: 0-3]
 *
 *  @param argc     number of arguments
 *  @param argv     A pointer to arguments array    
 *
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_qconfig(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;
    wlan_ioctl_wmm_queue_config_t queue_config_cmd;
    mlan_wmm_ac_e ac_idx;
    mlan_wmm_ac_e ac_idx_start;
    mlan_wmm_ac_e ac_idx_stop;
    int cmd_header_len = 0;

    const char *ac_str_tbl[] = { "BK", "BE", "VI", "VO" };

    if (argc < 4) {
        fprintf(stderr, "Invalid number of parameters!\n");
        return -EINVAL;
    }

    cmd_header_len = strlen(CMD_MARVELL) + strlen(argv[2]);

    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (buffer == NULL) {
        printf("Error: allocate memory for qconfig failed\n");
        return -ENOMEM;
    }
    memset(buffer, 0, BUFFER_LENGTH);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;
    /* buf = MRVL_CMD<cmd> */
    prepare_buffer(buffer, argv[2], 0, NULL);

    memset(&queue_config_cmd, 0x00, sizeof(&queue_config_cmd));
    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if (strcmp(argv[3], "get") == 0) {
        /* 3 4 5 */
        /* qconfig get [qid] */
        if (argc == 4) {
            ac_idx_start = WMM_AC_BK;
            ac_idx_stop = WMM_AC_VO;
        } else if (argc == 5) {
            if (atoi(argv[4]) < WMM_AC_BK || atoi(argv[4]) > WMM_AC_VO) {
                fprintf(stderr, "ERROR: Invalid Queue ID!\n");
                return -EINVAL;
            }
            ac_idx_start = atoi(argv[4]);
            ac_idx_stop = ac_idx_start;
        } else {
            fprintf(stderr, "Invalid number of parameters!\n");
            return -EINVAL;
        }
        queue_config_cmd.action = WMM_QUEUE_CONFIG_ACTION_GET;

        for (ac_idx = ac_idx_start; ac_idx <= ac_idx_stop; ac_idx++) {
            queue_config_cmd.accessCategory = ac_idx;
            memcpy(buffer + cmd_header_len, (t_u8 *) & queue_config_cmd,
                   sizeof(queue_config_cmd));
            if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
                perror("qconfig ioctl");
            } else {
                memcpy((t_u8 *) & queue_config_cmd, buffer + cmd_header_len,
                       sizeof(queue_config_cmd));
                printf("qconfig %s(%d): MSDU Lifetime GET = 0x%04x (%d)\n",
                       ac_str_tbl[ac_idx], ac_idx,
                       queue_config_cmd.msduLifetimeExpiry,
                       queue_config_cmd.msduLifetimeExpiry);
            }
        }
    } else if (strcmp(argv[3], "set") == 0) {
        if ((argc >= 5) && strcmp(argv[4], "msdu") == 0) {
            /* 3 4 5 6 7 */
            /* qconfig set msdu <value> [qid] */
            if (argc == 6) {
                ac_idx_start = WMM_AC_BK;
                ac_idx_stop = WMM_AC_VO;
            } else if (argc == 7) {
                if (atoi(argv[6]) < WMM_AC_BK || atoi(argv[6]) > WMM_AC_VO) {
                    fprintf(stderr, "ERROR: Invalid Queue ID!\n");
                    return -EINVAL;
                }
                ac_idx_start = atoi(argv[6]);
                ac_idx_stop = ac_idx_start;
            } else {
                fprintf(stderr, "Invalid number of parameters!\n");
                return -EINVAL;
            }
            queue_config_cmd.action = WMM_QUEUE_CONFIG_ACTION_SET;
            queue_config_cmd.msduLifetimeExpiry = atoi(argv[5]);

            for (ac_idx = ac_idx_start; ac_idx <= ac_idx_stop; ac_idx++) {
                queue_config_cmd.accessCategory = ac_idx;
                memcpy(buffer + cmd_header_len, (t_u8 *) & queue_config_cmd,
                       sizeof(queue_config_cmd));
                if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
                    perror("qconfig ioctl");
                } else {
                    memcpy((t_u8 *) & queue_config_cmd, buffer + cmd_header_len,
                           sizeof(queue_config_cmd));
                    printf("qconfig %s(%d): MSDU Lifetime SET = 0x%04x (%d)\n",
                           ac_str_tbl[ac_idx], ac_idx,
                           queue_config_cmd.msduLifetimeExpiry,
                           queue_config_cmd.msduLifetimeExpiry);
                }
            }
        } else {
            /* Only MSDU Lifetime provisioning accepted for now */
            fprintf(stderr, "Invalid set parameter: s/b [msdu]\n");
            return -EINVAL;
        }
    } else if (strncmp(argv[3], "def", strlen("def")) == 0) {
        /* 3 4 5 */
        /* qconfig def [qid] */
        if (argc == 4) {
            ac_idx_start = WMM_AC_BK;
            ac_idx_stop = WMM_AC_VO;
        } else if (argc == 5) {
            if (atoi(argv[4]) < WMM_AC_BK || atoi(argv[4]) > WMM_AC_VO) {
                fprintf(stderr, "ERROR: Invalid Queue ID!\n");
                return -EINVAL;
            }
            ac_idx_start = atoi(argv[4]);
            ac_idx_stop = ac_idx_start;
        } else {
            fprintf(stderr, "Invalid number of parameters!\n");
            return -EINVAL;
        }
        queue_config_cmd.action = WMM_QUEUE_CONFIG_ACTION_DEFAULT;

        for (ac_idx = ac_idx_start; ac_idx <= ac_idx_stop; ac_idx++) {
            queue_config_cmd.accessCategory = ac_idx;
            memcpy(buffer + cmd_header_len, (t_u8 *) & queue_config_cmd,
                   sizeof(queue_config_cmd));
            if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
                perror("qconfig ioctl");
            } else {
                memcpy((t_u8 *) & queue_config_cmd, buffer + cmd_header_len,
                       sizeof(queue_config_cmd));
                printf("qconfig %s(%d): MSDU Lifetime DEFAULT = 0x%04x (%d)\n",
                       ac_str_tbl[ac_idx], ac_idx,
                       queue_config_cmd.msduLifetimeExpiry,
                       queue_config_cmd.msduLifetimeExpiry);
            }
        }
    } else {
        fprintf(stderr, "Invalid qconfig command; s/b [set, get, default]\n");
        return -EINVAL;
    }

    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief Process generic commands
 *  @param argc   Number of arguments
 *  @param argv   A pointer to arguments array
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
static int
process_generic(int argc, char *argv[])
{
    t_u8 *buffer = NULL;
    struct eth_priv_cmd *cmd = NULL;
    struct ifreq ifr;

    /* Initialize buffer */
    buffer = (t_u8 *) malloc(BUFFER_LENGTH);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return MLAN_STATUS_FAILURE;
    }

    prepare_buffer(buffer, argv[2], 0, NULL);

    cmd = (struct eth_priv_cmd *) malloc(sizeof(struct eth_priv_cmd));
    if (!cmd) {
        printf("ERR:Cannot allocate buffer for command!\n");
        free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Fill up buffer */
    cmd->buf = buffer;
    cmd->used_len = 0;
    cmd->total_len = BUFFER_LENGTH;

    /* Perform IOCTL */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;

    if ((ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) < 0) {
        perror("mlanutl");
        fprintf(stderr, "mlanutl: %s fail\n", argv[2]);
        if (cmd)
            free(cmd);
        if (buffer)
            free(buffer);
        return MLAN_STATUS_FAILURE;
    }

    /* Process result */
    if (argc == 3) {
        /* GET operation */
        printf("%s command response received: %s\n", argv[2], buffer);
    }

    if (buffer)
        free(buffer);
    if (cmd)
        free(cmd);

    return MLAN_STATUS_SUCCESS;
}

/********************************************************
        Global Functions
********************************************************/

/**
 *  @brief Entry function for mlanutl
 *  @param argc     Number of arguments
 *  @param argv     A pointer to arguments array
 *  @return         MLAN_STATUS_SUCCESS--success, otherwise--fail
 */
int
main(int argc, char *argv[])
{
    int ret = MLAN_STATUS_SUCCESS;

    if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
        fprintf(stdout, "Marvell mlanutl version %s\n", MLANUTL_VER);
        exit(0);
    }
    if (argc < 3) {
        fprintf(stderr, "Invalid number of parameters!\n");
        display_usage();
        exit(1);
    }

    strncpy(dev_name, argv[1], IFNAMSIZ - 1);

    /* 
     * Create a socket
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "mlanutl: Cannot open socket.\n");
        exit(1);
    }

    ret = process_command(argc, argv);

    if (ret == MLAN_STATUS_FAILURE) {
        ret = process_generic(argc, argv);
    }

    if (ret) {
        fprintf(stderr, "Invalid command specified!\n");
        display_usage();
        ret = 1;
    }

    close(sockfd);
    return ret;
}
