/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/string.h> 
#include <net/cfg80211.h>
#ifdef SSV_MAC80211
#include "ssv_mac80211.h"
#else
#include <net/mac80211.h>
#endif
#include <linux/etherdevice.h>

#include "wapi_sms4.h"
#include "sec_wpi.h"
#include "sec.h"

//#define BIT(x) (0x1 << (x))

#define IWAPIELEMENT 68 /* WAPI Information Element       */
#define WID_WAPI_KEY    0x3033

/*****************************************************************************/
/* Global Variable Declarations                                              */
/*****************************************************************************/
u8 g_wapi_oui[3] = {0x00,0x14,0x72};

const u16 frame_cntl_mask = 0x8FC7; //little order,4, 5, 6, 11, 12, 13 should be set 0
const u16 seq_cntl_mask = 0x0F00; //little order,bit 4~15 should be set 0

struct lib80211_wpi_data {
	TRUTH_VALUE_T wapi_enable;
	TRUTH_VALUE_T wapi_key_ok;
	u8        wapi_version[2];

	u8        ap_address[ETH_ALEN];
	u8        key_index;
	u8        pn_key[WAPI_PN_LEN];
	u8        pmsk_key[3][WAPI_PN_LEN]; //psk or msk key
	u8        mic_key[3][WAPI_PN_LEN];
};

TRUTH_VALUE_T mget_wapi_key_ok(void *priv)
{
    struct lib80211_wpi_data *data = priv;
	//printk("mget_wapi_key_ok %s\n",(P_WAPI_MIB.wapi_key_ok == TV_TRUE) ? "TV_TRUE" : "TV_FALSE");
    return data->wapi_key_ok;
}

void mset_wapi_key_ok(TRUTH_VALUE_T val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	//printk("mset_wapi_key_ok %s\n",(val == TV_TRUE) ? "TV_TRUE" : "TV_FALSE");
    data->wapi_key_ok = val;
}

TRUTH_VALUE_T mget_wapi_enable(void *priv)
{
    struct lib80211_wpi_data *data = priv;
    return data->wapi_enable;
}

void mset_wapi_enable(TRUTH_VALUE_T val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	//printk("mset_wapi_enable %s\n",(val == TV_TRUE) ? "TV_TRUE" : "TV_FALSE");
    data->wapi_enable = val;
}

u8* mget_wapi_version(void *priv)
{
    struct lib80211_wpi_data *data = priv;
    return data->wapi_version;
}

void mset_wapi_version(u8* val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	memcpy(data->wapi_version,val,2);
}

u8* mget_wapi_address(void *priv)
{
    struct lib80211_wpi_data *data = priv;
	return data->ap_address; 
}

void mset_wapi_address(u8* val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	memcpy(data->ap_address,val, ETH_ALEN);
}

u8 mget_key_index(void *priv)
{
    struct lib80211_wpi_data *data = priv;
	return data->key_index; 
}

void mset_key_index(int index, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	if(index <= 3 )
	{
		data->key_index = index;
	}
}

u8* mget_pn_key(void *priv)
{
    struct lib80211_wpi_data *data = priv;
	return data->pn_key; 
}

void mset_pn_key(u8* val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	memcpy(data->pn_key,val,WAPI_PN_LEN);
}

u8 *inc_pn_key(void *priv)
{
    struct lib80211_wpi_data *data = priv;
	int i;
	
	data->pn_key[15] += 2;
	
	if( data->pn_key[15] == 0x00 )
	{
		for(i = 14 ; i >= 0 ; i--)
		{
			if( (data->pn_key[i] += 1) != 0x00 )
			{
				break;
			}
		}
	}

	return data->pn_key;
}

u8 *mget_pmsk_key(int index, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	return ( index >= 3 ) ? NULL : data->pmsk_key[index]; 
}

void mset_pmsk_key(int index,u8* val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	if(index < 3 )
	{
		memcpy(data->pmsk_key[index],val, WAPI_MIC_LEN);
	}
}	

u8* mget_mic_key(int index, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	return ( index >= 3 ) ? NULL : data->mic_key[index]; 
}

void mset_mic_key(int index,u8* val, void *priv)
{
    struct lib80211_wpi_data *data = priv;
	if(index < 3 )
	{
		memcpy(data->mic_key[index],val,WAPI_MIC_LEN);
	}
}	

/*****************************************************************************/
/*                                                                           */
/*  Function Name : init_wep                                                 */
/*                                                                           */
/*  Description   : This function initializes the WEP keys                   */
/*                                                                           */
/*  Inputs        : None                                                     */
/*                                                                           */
/*  Globals       : None                                                     */
/*                                                                           */
/*  Processing    : This function initializes the WEP keys                   */
/*                                                                           */
/*  Outputs       : None                                                     */
/*  Returns       : None                                                     */
/*  Issues        : None                                                     */
/*                                                                           */
/*****************************************************************************/

/* This function checks if the received OUI and the configured OUI are same  */
/* Returns TRUE is the OUI matches ; FALSE otherwise                         */
/**************************************************************************
struct ieee80211_hdr {
	__le16 frame_control;
	__le16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	__le16 seq_ctrl;
	u8 addr4[6];
} __attribute__ ((packed));
**************************************************************************/

/**************************************************************************/
/*                        wapi MSDU format                                */
/* ---------------------------------------------------------------------- */
/* |MAC Heager|keyID|Res|PN(iv)|PDU(Encryption)|MIC(Encryption)|FCS       */
/* ---------------------------------------------------------------------- */
/* |30/24     |1    |1  |16    |>=1            |16             |4  		  */
/* ---------------------------------------------------------------------- */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/*                        calc mic data format                            */
/* ---------------------------------------------------------------------- */
/* |frame cntl|addr1|addr2|seq cntl|addr3|addr4|qos|keyID|Res|Length      */
/* ---------------------------------------------------------------------- */
/* |2         |6    |6    |2       |6    |6    |2  |1    |1  |2(big order)*/
/* ---------------------------------------------------------------------- */
/*                                                                        */
/**************************************************************************/
u16 wlan_tx_wapi_encryption(u8 * header,
									   u8 * data,u16 data_len,
	                                   u8 * mic_pos, void *priv)
{
	int i = 0;
	u16 offset = 0;
	BOOL_T qos_in = BFALSE;
	BOOL_T valid_addr4 = BTRUE;
	
	u8 ptk_header[36] = {0};
	u16 ptk_headr_len = 32;
	u8 *p_ptk_header = ptk_header;
	//u8 *p_outputdata = ouput_buf;//get_eth_hdr_offset(ETHERNET_HOST_TYPE);
	u8 *data_mic = mic_pos;
    u8 *iv = NULL;
	u8 keyid = 0;
	
	keyid = mget_key_index(priv);

#ifdef MULTI_THREAD_ENCRYPT
    iv = kzalloc(WAPI_PN_LEN, GFP_KERNEL);
    memcpy(iv, data + WAPI_KEYID_LEN + WAPI_RESERVD_LEN, WAPI_PN_LEN);
    data_len -= WAPI_IV_LEN;
#else
	iv  = inc_pn_key(priv);
#endif

	*data    = keyid;
	*(data + 1) = 0x00;
	data += 2;
	
    //printk(KERN_ALERT "keyid=%x,iv=",keyid);
	for( i = 15 ; i >= 0 ; i-- ) {
	    //printk("%02x ",iv[i]);
		*data = iv[i];
		data++;
	}
    //printk("\n");

	/* save frame cntl */
	*p_ptk_header     = header[offset]   & (frame_cntl_mask >> 8);
	*(p_ptk_header + 1) = header[offset + 1] & (frame_cntl_mask & 0xFF);

	if(*p_ptk_header & 0x80) {
		qos_in = BTRUE;
		ptk_headr_len += 2;//add qos len 2 byte
	}

	//valid addr4 in case:ToDS==1 && FromDS==1
	if((*(p_ptk_header + 1) & 0x03 ) != 0x03) {
		valid_addr4 = BFALSE;
	}
	
	p_ptk_header += 2;
	offset += 2;

	/* jump over duration id*/
	offset += 2;
	
	/* save addr1 addr2 */
	memcpy(p_ptk_header, &header[offset], ADDID_LEN);
	p_ptk_header += ADDID_LEN;
	offset       += ADDID_LEN;

	/* save seq cntl */
	*p_ptk_header       = header[offset + ETH_ALEN]   & (seq_cntl_mask >> 8);
	*(p_ptk_header + 1) = header[offset + ETH_ALEN + 1] & (seq_cntl_mask & 0xFF);
	p_ptk_header += 2;

	/* save addr3 */
	memcpy(p_ptk_header, &header[offset], ETH_ALEN);
	p_ptk_header += ETH_ALEN;
	offset       += ETH_ALEN;

	//jump seq cntl
	offset += 2;
	
	/* save addr4 */
	if(valid_addr4) {
		memcpy(p_ptk_header, &header[offset], ETH_ALEN);
		p_ptk_header += ETH_ALEN;
		offset       += ETH_ALEN;
	}
	else {
		memset(p_ptk_header,0x00, ETH_ALEN);
		p_ptk_header += ETH_ALEN;
	}


	/* save qos */
	if(qos_in) {
		memcpy(p_ptk_header, &header[offset], 2);
		p_ptk_header += 2;
		offset       += 2;
	}

	/* save keyid */
	*p_ptk_header = keyid;
	p_ptk_header++;
	
	/* reserved */
	*p_ptk_header = 0x00;
	p_ptk_header++;

	/* save data len */
	*p_ptk_header     = (data_len >> 8);
	*(p_ptk_header+1) = data_len & 0xFF;

	/* calc mic*/
	WapiCryptoSms4Mic(iv,
	                  mget_mic_key(keyid, priv),
	                  ptk_header, ptk_headr_len, data, data_len, data_mic);

	/* add mic to data */
	//memcpy(data + data_len, data_mic, WAPI_MIC_LEN);
	data_len += WAPI_MIC_LEN;

	/* encryption data(inclue mic) & save keyid & iv */

	WapiCryptoSms4(iv,
				   mget_pmsk_key(keyid, priv),
				   data, data_len,
	               data);


#ifdef MULTI_THREAD_ENCRYPT
    kfree(iv);
#endif //MULTI_THREAD_ENCRYPT

	return data_len + WAPI_IV_LEN;

}

BOOL_T is_group(u8* addr)
{
    if((addr[0] & BIT(0)) != 0)
        return BTRUE;

    return BFALSE;
}

u16 wlan_rx_wapi_decryption(u8 * input_ptk,u16 header_len,u16 data_len,
	                                   u8 * output_buf, void *priv)
{
	u16 offset = 0;
	BOOL_T  qos_in = BFALSE;
	BOOL_T  valid_addr4 = BTRUE;
	BOOL_T  is_group_ptk = BFALSE;
	
	u8 ptk_header[36] = {0};
	u16 ptk_headr_len = 32;
	u8 * p_ptk_header = ptk_header;
	//u8 * p_outputdata = ouput_buf + get_eth_hdr_offset(ETHERNET_HOST_TYPE);
	u8 data_mic[WAPI_MIC_LEN] = {0};
	u8 calc_data_mic[WAPI_MIC_LEN] = {0};
	u8 iv[WAPI_PN_LEN] = {0};
	u8 keyid = {0};
	
	u16 ral_data_len = 0;
	u16 encryp_data_len = 0;

	int i = 0;

	/* save calc mic header*/
	/* save frame cntl */
	*p_ptk_header     = input_ptk[offset]   & (frame_cntl_mask >> 8);
	*(p_ptk_header+1) = input_ptk[offset+1] & (frame_cntl_mask & 0xFF);

    //Matt01, if packet header is qos, header_len += 2...
	if(*p_ptk_header & 0x80) {
		qos_in = BTRUE;
		ptk_headr_len += 2;//add qos len 2 byte
	}
	
	//valid addr4 in case:ToDS==1 && FromDS==1
	if((*(p_ptk_header+1) & 0x03 ) != 0x03) {
		valid_addr4 = BFALSE;
	}
	
	p_ptk_header += 2;
	offset       += 2;

	/* jump over duration id*/
	offset       += 2;
	
	/* save addr1 addr2 */
	memcpy(p_ptk_header, &input_ptk[offset], ADDID_LEN);
	is_group_ptk = is_group(p_ptk_header);
	p_ptk_header += ADDID_LEN;
	offset       += ADDID_LEN;

	/* save seq cntl */
	*p_ptk_header     = input_ptk[offset+6]   & (seq_cntl_mask >> 8);
	*(p_ptk_header+1) = input_ptk[offset+6+1] & (seq_cntl_mask & 0xFF);
	p_ptk_header += 2;

	/* save addr3 */
	memcpy(p_ptk_header, &input_ptk[offset], ETH_ALEN);
	p_ptk_header += ETH_ALEN;
	offset       += ETH_ALEN;

	//jump seq cntl
	offset += 2;
	
	/* save addr4 */
	if(valid_addr4) {
		memcpy(p_ptk_header, &input_ptk[offset], ETH_ALEN);
		p_ptk_header += ETH_ALEN;
		offset       += ETH_ALEN;
	}
	else {
		memset(p_ptk_header, 0x00, ETH_ALEN);
		p_ptk_header += ETH_ALEN;
	}
	
	/* save qos */
	if(qos_in) {
		memcpy(p_ptk_header,&input_ptk[offset], 2);
		p_ptk_header += 2;
		offset       += 2;

		//mac h/w offset 2 byte to multiple of 4
		//offset       += 2;
	}

	/* save keyid */
	*p_ptk_header = input_ptk[offset];
	keyid         = input_ptk[offset];
	p_ptk_header++;
	offset++;
	
	/* reserved */
	*p_ptk_header = input_ptk[offset];
	p_ptk_header++;
	offset++;

	/* save data len */
	encryp_data_len = data_len - WAPI_IV_LEN;
	ral_data_len    = data_len - WAPI_IV_LEN - WAPI_MIC_LEN;
	*p_ptk_header     = (ral_data_len >> 8);
	*(p_ptk_header+1) = ral_data_len & 0xFF;

	/* save calc mic header over*/
    //printk(KERN_ALERT "encryp_data_len=%d,ral_data_len=%d,offset=%d,ptk_headr_len=%d",encryp_data_len,ral_data_len,offset,ptk_headr_len);
	/* save iv */
    //printk(KERN_ALERT "is_group_ptk=%d,keyid=%x,save iv=",is_group_ptk,keyid);
	for( i = 15 ; i >= 0 ; i-- ) {
	    //printk("%02x ",input_ptk[offset]);
		iv[i] = input_ptk[offset];
		offset++;
	}
    //printk("\n");

	/* add adjust here,later...*/
	if(is_group_ptk) {
        //Matt01, if it's mtk then its pn(iv) should be a strictly increasing number.
        printk("%s, is_group_ptk\n", __func__);
	}
	else {
        //Matt01, if it's ptk then its pn(iv) should be an odd and a strictly increasing number.
		if( (iv[15] & 0x01) != 0x01 ) {
            printk(KERN_ALERT "decry pairwise error,iv[15]=%x\n", iv[15]);
			return 0;
		}
	}


	/* decryption */

	WapiCryptoSms4(iv,
				   mget_pmsk_key(keyid, priv),
				   (input_ptk + header_len + WAPI_IV_LEN), encryp_data_len,
	               output_buf);
	memcpy(data_mic, output_buf + ral_data_len, WAPI_MIC_LEN);

	/* calc mic */
	WapiCryptoSms4Mic(iv,
	                  mget_mic_key(keyid, priv),
	                  ptk_header, ptk_headr_len,
	                  (output_buf), ral_data_len,
	                  calc_data_mic);
	if( memcmp(calc_data_mic, data_mic, WAPI_MIC_LEN) != 0 ) {
        printk(KERN_ALERT "calc_data_mic != data_mic\n");
		return 0;
	}
	else {
		return ral_data_len;
	}

}

int lib80211_wpi_encrypt(struct sk_buff *mpdu, int hdr_len, void *priv)
//u16 encrypt_mpdu_of_wapi(struct sk_buff *mpdu)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)mpdu->data;
    u8 *pos, *mic_pos;
    int hdrlen = 0, len = 0, out_len = 0;
    u8 *pdata = NULL;
	
#ifdef MULTI_THREAD_ENCRYPT
	   //WAPI_PN_LEN already requested in ssv6xxx_skb_pre_encryt
	   u32 wapi_iv_icv_offset = WAPI_IV_ICV_OFFSET - WAPI_IV_LEN;
#else //MULTI_THREAD_ENCRYPT
		u32 wapi_iv_icv_offset = WAPI_IV_ICV_OFFSET;
#endif //MULTI_THREAD_ENCRYPT

    hdrlen = ieee80211_hdrlen(hdr->frame_control);
    pdata = (mpdu->data) + hdrlen;
    //printk("%s,skb->len=%d,hdrlen=%d, tx_fctl=%04x, tx_flag=%04x\n",__func__,mpdu->len,hdrlen, hdr->frame_control, info->flags);

    //printk("TX recv %pM: \n", hdr->addr1);    
    //printk("TX xmit %pM: \n", hdr->addr2);    

    if (mpdu->protocol != cpu_to_be16(0x88b4)) {
		
        if (WARN_ON(skb_headroom(mpdu) < wapi_iv_icv_offset)) {
            printk("[I] skb_headroom(skb) < %d\n", wapi_iv_icv_offset);
            return 0;
        }

        len = mpdu->len - hdrlen;
        pos = skb_push(mpdu, wapi_iv_icv_offset);

#ifdef MULTI_THREAD_ENCRYPT
		/*
			skb_push:      | wapi_iv_icv_offset | hdr | wapi iv | ptk data |
			after memmove: | hdr | wapi iv | ptk data | ... |
		*/
        memmove(pos, pos + wapi_iv_icv_offset, mpdu->len - WAPI_MIC_LEN);
#else
		/*
			skb_push:    | wapi_iv_icv_offset | hdr | ptk data |
			two memmove: | hdr | wapi iv(null data) | ptk data | ... |
		*/
		memmove(pos, pos + wapi_iv_icv_offset, hdrlen);
		memmove(pos + hdrlen + WAPI_IV_LEN, pos + wapi_iv_icv_offset + hdrlen, len);
#endif
        hdr = (struct ieee80211_hdr *)pos;
		pos += hdrlen;

		mic_pos = mpdu->data + mpdu->len - WAPI_MIC_LEN;
        out_len = wlan_tx_wapi_encryption((u8 *)hdr, pos, len, mic_pos, priv);
    }
    else {
        if (ieee80211_has_protected(hdr->frame_control))
            hdr->frame_control &= ~(cpu_to_le16(IEEE80211_FCTL_PROTECTED));
        printk("[I] send WAPI WAI data pkt\n");
    }

    return 1;
}

int lib80211_wpi_decrypt(struct sk_buff *rx_skb, int hdr_len, void *priv)
//u16 decrypt_mpdu_of_wapi(struct sk_buff *rx_skb)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(rx_skb->data);
    int hdrlen, len, dcry_len;
    char *pdata = NULL;
    int ret = 0;
 
    //printk("%s, rx_fctl=%04x, rxs_flag=%04x\n",__func__,hdr->frame_control, rxs->flag);
    hdrlen = ieee80211_hdrlen(hdr->frame_control);
    pdata = ((char*)(rx_skb->data)) + hdrlen;
    len = rx_skb->len - hdrlen;
	
    dcry_len = wlan_rx_wapi_decryption((u8 *)rx_skb->data, hdrlen, len, pdata, priv);
    if (dcry_len) {
		skb_trim(rx_skb, hdrlen + dcry_len);
        //memcpy(pdata, rx_pload, dcry_len);
        //rx_skb->len = hdrlen + dcry_len;
        //printk("dcry_len=%d,len=%d,hdrlen=%d,rx_skb->len=%d\n",dcry_len,len,hdrlen,rx_skb->len);
        hdr->frame_control &= ~(cpu_to_le16(IEEE80211_FCTL_PROTECTED));
        ret = dcry_len;
    }
    return ret;
}

void *lib80211_wpi_init(int key_idx)
{
    struct lib80211_wpi_data *priv;
    
	priv = kzalloc(sizeof(*priv), GFP_ATOMIC);

	if (priv == NULL) {
        printk("allocate lib80211_wpi_data failed\n");
		return NULL;
    }
	
    priv->key_index = key_idx;

    return priv;    
}

void lib80211_wpi_deinit(void *priv)
{
    if (priv) {
        printk("%s\n", __func__);
        kfree(priv);
        priv = NULL; 
    }
    else
        printk("%s, passing NULL lib80211_wpi_data?\n", __func__);
}

#ifdef MULTI_THREAD_ENCRYPT
/**********************************************************/
/*                     wapi pre encrpt format             */
/* -------------------------------------------------------*/
/* |MAC Heager|keyID|Res|PN(iv)|PTK(mpdu data)|FCS|       */
/* -------------------------------------------------------*/
/* |30/24	  |1    |1	|16	   | >=1          |4  |       */
/* -------------------------------------------------------*/
/*														  */
/**********************************************************/

int lib80211_wpi_encrypt_prepare(struct sk_buff *mpdu, int hdr_len, void *priv)
{
    struct lib80211_wpi_data *data = priv;
    //struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)mpdu->data;
    //struct SKB_info_st *skb_info = (struct SKB_info_st *)mpdu->head;
    //struct ieee80211_sta *sta = skb_info->sta;
    //int sta_idx;
    //struct ssv_sta_info *sta_info = NULL;
    u8 *pos = NULL;
    unsigned char *iv = NULL;

    if (mpdu->protocol != cpu_to_be16(0x88b4) && 
            (skb_headroom(mpdu) >= WAPI_IV_LEN)) {
        pos = skb_push(mpdu, WAPI_IV_LEN);
        memmove(pos, pos + WAPI_IV_LEN, hdr_len);
        pos += hdr_len;
        *pos = data->key_index;
        pos++;
        *pos = 0x00;
        pos++;
        iv = inc_pn_key(priv); 
        memcpy(pos, iv, WAPI_PN_LEN);
        return 0;
    }
    printk("%s, pass through\n", __func__);
    return 0; 
}
#endif

int lib80211_wpi_set_key(void *key, int len, u8 *seq, void *priv)
{
    struct lib80211_wpi_data *data = priv;
    int keyidx = data->key_index;
    u8 WapiASUEPNInitialValueSrc[16] = {
        0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36,
        0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36
    };

    /*************************************************************************/
    /*                        set_wapi_key wid Format                        */
    /* --------------------------------------------------------------------  */
    /* |PAIRWISE/group|key index|BSSID|RSC|key(rxtx + mic)|                  */
    /* --------------------------------------------------------------------  */
    /* | 1            |1        |6    |16 |32 |                              */
    /* --------------------------------------------------------------------  */
    /*																		 */
    /*************************************************************************/
    printk("%s\n", __func__);
    mset_key_index(keyidx, priv);
    mset_pn_key(WapiASUEPNInitialValueSrc, priv);
    mset_pmsk_key(keyidx, key, priv);
    mset_mic_key(keyidx, key + WAPI_PN_LEN, priv); //WAPI_PN_LEN offset to mic key location 

    if (seq) {
        memcpy(data->ap_address, (u8 *)seq, ETH_ALEN);
        printk("%s: set ap_address %pM\n", __func__, data->ap_address);
    }

	mset_wapi_key_ok(TV_TRUE, priv);
    return 0;    
}

static struct ssv_crypto_ops ssv_crypto_wpi = {
  	.name = "WPI",
	.init = lib80211_wpi_init,
	.deinit = lib80211_wpi_deinit,
	.encrypt_mpdu = lib80211_wpi_encrypt,
	.decrypt_mpdu = lib80211_wpi_decrypt,
	.encrypt_msdu = NULL,
	.decrypt_msdu = NULL,
	.set_tx_pn = NULL,
	.set_key = lib80211_wpi_set_key,
	.get_key = NULL,
	.print_stats = NULL,
	.extra_mpdu_prefix_len = WAPI_IV_LEN,
	.extra_mpdu_postfix_len = WAPI_MIC_LEN,
#ifdef MULTI_THREAD_ENCRYPT
	.encrypt_prepare = lib80211_wpi_encrypt_prepare,
    .decrypt_prepare = NULL,
#endif
};

struct ssv_crypto_ops *get_crypto_wpi_ops(void)
{
    return &ssv_crypto_wpi;
}
