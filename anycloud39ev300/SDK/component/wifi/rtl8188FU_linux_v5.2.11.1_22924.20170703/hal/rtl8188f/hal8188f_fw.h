
#ifdef CONFIG_RTL8188F
#ifndef _FW_HEADER_8188F_H
#define _FW_HEADER_8188F_H

#ifdef LOAD_FW_HEADER_FROM_DRIVER
#if (defined(CONFIG_AP_WOWLAN) || (DM_ODM_SUPPORT_TYPE & (ODM_AP)))
extern u8 array_mp_8188f_fw_ap[17900];
extern u32 array_length_mp_8188f_fw_ap;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN)) || (DM_ODM_SUPPORT_TYPE & (ODM_CE))
extern u8 array_mp_8188f_fw_nic[21020];
extern u32 array_length_mp_8188f_fw_nic;
extern u8 array_mp_8188f_fw_wowlan[22986];
extern u32 array_length_mp_8188f_fw_wowlan;
#endif
#endif /* end of LOAD_FW_HEADER_FROM_DRIVER */

#endif
#endif
