#include "anyka_types.h"
#include "ak_isp_drv.h"
#include "isp_struct.h"
#include "ispdrv_modules_interface.h"

#ifndef BURNTOOL

//#define ISM_DEBUG_DUMP

/**
 * @brief set isp module configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] module_id isp module id
 * @param[in] buf memory for this module
 * @return bool
 * @retval true if set successfully
 * @retval false if set unsuccessfully
 */
bool ispdrv_set_module(unsigned long module_id, unsigned char *buf)
{
	bool ret = true;

	switch (module_id) {
		case ISP_BB:
			{
				AK_ISP_INIT_BLC *p = (AK_ISP_INIT_BLC *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_blc);
					printf("\n ISP_BB:\n");
					for (i = 0; i < sizeof(p->p_blc); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set BB param!\n");
				ak_isp_vp_set_blc_attr(&p->p_blc);
			}
			break;

		case ISP_LSC:
			{
				AK_ISP_INIT_LSC *p = (AK_ISP_INIT_LSC *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->lsc);
					printf("\n ISP_LSC:\n");
					for (i = 0; i < sizeof(p->lsc); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set LSC param!\n");
				ak_isp_vp_set_lsc_attr(&p->lsc);
			}
			break;

		case ISP_RAW_LUT:
			{
				AK_ISP_INIT_RAW_LUT *p = (AK_ISP_INIT_RAW_LUT *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->raw_lut_p);
					printf("\n ISP_RAW_LUT:\n");
					for (i = 0; i < sizeof(p->raw_lut_p); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set RAW GAMMA param!\n");
				ak_isp_vp_set_raw_lut_attr(&p->raw_lut_p);
			}
			break;

		case ISP_NR:
			{
				int i;
				AK_ISP_INIT_NR *p = (AK_ISP_INIT_NR *)buf;
				#if 0
				printf("nr1_mode:%d,nr1_enable:%d\n", p->p_nr1.nr1_mode, p->p_nr1.manual_nr1.nr1_enable);
				for (i = 0; i < 9; i++)
					printf("enable-%d:%d\n", i, p->p_nr1.linkage_nr1[i].nr1_enable);
				printf("\n");
				printf("nr2_mode:%d,nr2_enable:%d\n", p->p_nr2.nr2_mode, p->p_nr2.manual_nr2.nr2_enable);
				for (i = 0; i < 9; i++)
					printf("enable-%d:%d\n", i, p->p_nr2.linkage_nr2[i].nr2_enable);
				printf("\n");
				#endif
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_nr1);
					printf("\n ISP_NR p_nr1:\n");
					for (i = 0; i < sizeof(p->p_nr1); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_nr2);
					printf("\n ISP_NR p_nr2:\n");
					for (i = 0; i < sizeof(p->p_nr2); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set NR param!\n");
				ak_isp_vp_set_nr1_attr(&p->p_nr1);
				ak_isp_vp_set_nr2_attr(&p->p_nr2);
			}
			break;

		case ISP_3DNR:
			{
				AK_ISP_INIT_3DNR *p = (AK_ISP_INIT_3DNR *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_3d_nr);
					printf("\n ISP_3DNR p_3d_nr:\n");
					for (i = 0; i < sizeof(p->p_3d_nr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_3d_nr_ex);
					printf("\n ISP_NR p_3d_nr_ex:\n");
					for (i = 0; i < sizeof(p->p_3d_nr_ex); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set 3DNR param!\n");
				ak_isp_vp_set_3d_nr_attr(&p->p_3d_nr);
			}
			break;

		case ISP_GB:
			{
				AK_ISP_INIT_GB *p = (AK_ISP_INIT_GB *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_gb);
					printf("\n ISP_GB p_gb:\n");
					for (i = 0; i < sizeof(p->p_gb); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set GB param!\n");
				ak_isp_vp_set_gb_attr(&p->p_gb);
			}
			break;

		case ISP_DEMO:
			{
				AK_ISP_INIT_DEMO *p = (AK_ISP_INIT_DEMO *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_demo_attr);
					printf("\n ISP_DEMO p_demo_attr:\n");
					for (i = 0; i < sizeof(p->p_demo_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set DEMO param!\n");
				ak_isp_vp_set_demo_attr(&p->p_demo_attr);
			}
			break;

		case ISP_GAMMA:
			{
				AK_ISP_INIT_GAMMA *p = (AK_ISP_INIT_GAMMA *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_gamma_attr);
					printf("\n ISP_GAMMA p_gamma_attr:\n");
					for (i = 0; i < sizeof(p->p_gamma_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set GAMMA param!\n");
				ak_isp_vp_set_rgb_gamma_attr(&p->p_gamma_attr);
			}
			break;

		case ISP_CCM:
			{
				AK_ISP_INIT_CCM *p = (AK_ISP_INIT_CCM *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_ccm);
					printf("\n ISP_CCM p_ccm:\n");
					for (i = 0; i < sizeof(p->p_ccm); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_ccm_ex);
					printf("\n ISP_CCM p_ccm_ex:\n");
					for (i = 0; i < sizeof(p->p_ccm_ex); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_ccm_fine);
					printf("\n ISP_CCM p_ccm_fine:\n");
					for (i = 0; i < sizeof(p->p_ccm_fine); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_cw);
					printf("\n ISP_CCM p_cw:\n");
					for (i = 0; i < sizeof(p->p_cw); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set CCM param!\n");
				ak_isp_vp_set_ccm_attr(&p->p_ccm);
			}
			break;

		case ISP_FCS:
			{
				AK_ISP_INIT_FCS *p = (AK_ISP_INIT_FCS *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_fcs);
					printf("\n ISP_FCS p_fcs:\n");
					for (i = 0; i < sizeof(p->p_fcs); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set FCS param!\n");
				ak_isp_vp_set_fcs_attr(&p->p_fcs);
			}
			break;

		case ISP_WDR:
			{
				AK_ISP_INIT_WDR *p = (AK_ISP_INIT_WDR *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_wdr_attr);
					printf("\n ISP_WDR p_wdr_attr:\n");
					for (i = 0; i < sizeof(p->p_wdr_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_wdr_ex);
					printf("\n ISP_WDR p_wdr_ex:\n");
					for (i = 0; i < sizeof(p->p_wdr_ex); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set WDR param!\n");
				ak_isp_vp_set_wdr_attr(&p->p_wdr_attr);
			}
			break;
		case ISP_SHARP:
			{
				AK_ISP_INIT_SHARP *p = (AK_ISP_INIT_SHARP *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_sharp_attr);
					printf("\n ISP_SHARP p_sharp_attr:\n");
					for (i = 0; i < sizeof(p->p_sharp_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_sharp_ex_attr);
					printf("\n ISP_SHARP p_sharp_ex_attr:\n");
					for (i = 0; i < sizeof(p->p_sharp_ex_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set SHARP param!\n");
				ak_isp_vp_set_sharp_attr(&p->p_sharp_attr);
				ak_isp_vp_set_sharp_ex_attr(&p->p_sharp_ex_attr);
			}
			break;

		case ISP_SATURATION:
			{
				AK_ISP_INIT_SATURATION *p = (AK_ISP_INIT_SATURATION *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_se_attr);
					printf("\n ISP_SATURATION p_se_attr:\n");
					for (i = 0; i < sizeof(p->p_se_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set SATURATION param!\n");
				ak_isp_vp_set_saturation_attr(&p->p_se_attr);
			}
			break;

		case ISP_CONSTRAST:
			{
				AK_ISP_INIT_CONTRAST *p = (AK_ISP_INIT_CONTRAST *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_contrast);
					printf("\n ISP_CONSTRAST p_contrast:\n");
					for (i = 0; i < sizeof(p->p_contrast); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set CONSTRAST param!\n");
				ak_isp_vp_set_contrast_attr(&p->p_contrast);
			}
			break;

		case ISP_YUVEFFECT:
			{
				AK_ISP_INIT_EFFECT *p = (AK_ISP_INIT_EFFECT *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_isp_effect);
					printf("\n ISP_YUVEFFECT p_isp_effect:\n");
					for (i = 0; i < sizeof(p->p_isp_effect); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set YUV EFFECT param!\n");
				ak_isp_vp_set_effect_attr(&p->p_isp_effect);
			}
			break;

		case ISP_RGB2YUV:
			{
				AK_ISP_INIT_RGB2YUV *p = (AK_ISP_INIT_RGB2YUV *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_rgb2yuv);
					printf("\n ISP_RGB2YUV p_rgb2yuv:\n");
					for (i = 0; i < sizeof(p->p_rgb2yuv); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set RGB2YUV param!\n");
				ak_isp_vp_set_rgb2yuv_attr(&p->p_rgb2yuv);
			}
			break;

		case ISP_DPC:
			{
				AK_ISP_INIT_DPC *p = (AK_ISP_INIT_DPC *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_ddpc);
					printf("\n ISP_DPC p_ddpc:\n");
					for (i = 0; i < sizeof(p->p_ddpc); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_sdpc);
					printf("\n ISP_DPC p_sdpc:\n");
					for (i = 0; i < sizeof(p->p_sdpc); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set DPC param!\n");
				ak_isp_vp_set_dpc_attr(&p->p_ddpc);
				ak_isp_vp_set_sdpc_attr(&p->p_sdpc);// ispsdk in linux not call this function
			}
			break;

		case ISP_WEIGHT:
			{
				AK_ISP_INIT_WEIGHT *p = (AK_ISP_INIT_WEIGHT *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_weight);
					printf("\n ISP_WEIGHT p_weight:\n");
					for (i = 0; i < sizeof(p->p_weight); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set WEITHT param!\n");
				ak_isp_vp_set_zone_weight(&p->p_weight);
			}
			break;

		case ISP_AF:
			{
				AK_ISP_INIT_AF *p = (AK_ISP_INIT_AF *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_af_attr);
					printf("\n ISP_AF p_af_attr:\n");
					for (i = 0; i < sizeof(p->p_af_attr); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set AF param!\n");
				ak_isp_vp_set_af_attr(&p->p_af_attr);
			}
			break;

		case ISP_WB:
			{
				AK_ISP_INIT_WB *p = (AK_ISP_INIT_WB *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->wb_type);
					printf("\n ISP_WB wb_type:\n");
					for (i = 0; i < sizeof(p->wb_type); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_mwb);
					printf("\n ISP_WB p_mwb:\n");
					for (i = 0; i < sizeof(p->p_mwb); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_awb);
					printf("\n ISP_WB p_awb:\n");
					for (i = 0; i < sizeof(p->p_awb); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_awb_default);
					printf("\n ISP_WB p_awb_default:\n");
					for (i = 0; i < sizeof(p->p_awb_default); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set WB param!\n");
				ak_isp_vp_set_wb_type(&p->wb_type);
				ak_isp_vp_set_mwb_attr(&p->p_mwb);
				ak_isp_vp_set_awb_attr(&p->p_awb);
				ak_isp_vp_set_awb_ex_attr(&p->p_awb_ex);
			}
			break;

		case ISP_EXP:
			{
				AK_ISP_INIT_EXP *p = (AK_ISP_INIT_EXP *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_raw_hist);
					printf("\n ISP_EXP p_raw_hist:\n");
					for (i = 0; i < sizeof(p->p_raw_hist); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_rgb_hist);
					printf("\n ISP_EXP p_rgb_hist:\n");
					for (i = 0; i < sizeof(p->p_rgb_hist); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_yuv_hist);
					printf("\n ISP_EXP p_yuv_hist:\n");
					for (i = 0; i < sizeof(p->p_yuv_hist); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_exp_type);
					printf("\n ISP_EXP p_exp_type:\n");
					for (i = 0; i < sizeof(p->p_exp_type); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_me);
					printf("\n ISP_EXP p_me:\n");
					for (i = 0; i < sizeof(p->p_me); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_ae);
					printf("\n ISP_EXP p_ae:\n");
					for (i = 0; i < sizeof(p->p_ae); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_raw_hist_attr(&p->p_raw_hist);
				ak_isp_vp_set_rgb_hist_attr(&p->p_rgb_hist);
				ak_isp_vp_set_yuv_hist_attr(&p->p_yuv_hist);
				ak_isp_vp_set_exp_type(&p->p_exp_type);
				ak_isp_vp_set_frame_rate(&p->p_frame_rate);
				ak_isp_vp_set_ae_attr(&p->p_ae);
			}
			break;

		case ISP_MISC:
			{
				AK_ISP_INIT_MISC *p = (AK_ISP_INIT_MISC *)buf;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					unsigned char *ptr = (unsigned char *)(&p->p_misc);
					printf("\n ISP_MISC p_misc:\n");
					for (i = 0; i < sizeof(p->p_misc); i++) {
						printf("%02x ", ptr[i]);
						if ((i > 0) && (i % 16 == 0))
							printf("\n");
					}
				}
				#endif
				//anyka_print("set MISC param!\n");
				ak_isp_vo_set_misc_attr(&p->p_misc);
			}
			break;

		case ISP_Y_GAMMA:
			{
				AK_ISP_INIT_Y_GAMMA *p = (AK_ISP_INIT_Y_GAMMA*)buf;
				//anyka_print("set y gamma param!\n");
				ak_isp_vp_set_y_gamma_attr(&p->p_gamma_attr);
			}
			break;
		
		case ISP_HUE:
			{
				AK_ISP_INIT_HUE *p = (AK_ISP_INIT_HUE *)buf;
				//anyka_print("set hue param!\n");
				ak_isp_vp_set_hue_attr(&p->p_hue);
			}
			break;


		case ISP_SENSOR:
			{
				AK_ISP_INIT_SENSOR *p = (AK_ISP_INIT_SENSOR*)buf;
				AK_ISP_SENSOR_INIT_PARA sensor_para;
				sensor_para.num = p->length / sizeof(AK_ISP_SENSOR_REG_INFO);
				sensor_para.reg_info = (void *)&p->p_sensor;
				#ifdef ISM_DEBUG_DUMP
				{
					int i;
					printf("\n ISP_SENSOR:\n");
					for (i = 0; i < sensor_para.num; i++) {
						printf("%04x=%04x\n", sensor_para.reg_info[i].reg_addr, sensor_para.reg_info[i].value);
					}
				}
				#endif
				ak_isp_sensor_cb_init(&sensor_para);
			}
			break;

		/*SUB MODULES*/
		case ISP_BB_SUB_BLC:
			{
				AK_ISP_BLC_ATTR *p = (AK_ISP_BLC_ATTR *)buf;
				//anyka_print("set BB param!\n");
				ak_isp_vp_set_blc_attr(p);
			}
			break;

		case ISP_LSC_SUB_LSC:
			{
				AK_ISP_LSC_ATTR *p = (AK_ISP_LSC_ATTR *)buf;
				//anyka_print("set LSC param!\n");
				ak_isp_vp_set_lsc_attr(p);
			}
			break;

		case ISP_RAW_LUT_SUB_LUT:
			{
				AK_ISP_RAW_LUT_ATTR *p = (AK_ISP_RAW_LUT_ATTR *)buf;
				//anyka_print("set RAW GAMMA param!\n");
				ak_isp_vp_set_raw_lut_attr(p);
			}
			break;

		case ISP_NR_SUB_NR1:
			{
				AK_ISP_NR1_ATTR *p = (AK_ISP_NR1_ATTR *)buf;
				//anyka_print("set NR param!\n");
				ak_isp_vp_set_nr1_attr(p);
			}
			break;

		case ISP_NR_SUB_NR2:
			{
				AK_ISP_NR2_ATTR *p = (AK_ISP_NR2_ATTR *)buf;
				//anyka_print("set NR param!\n");
				ak_isp_vp_set_nr2_attr(p);
			}
			break;


		case ISP_3DNR_SUB_3DNR:
			{
				AK_ISP_3D_NR_ATTR *p = (AK_ISP_3D_NR_ATTR *)buf;
				//anyka_print("set 3DNR param!\n");
				ak_isp_vp_set_3d_nr_attr(p);
			}
			break;

		case ISP_GB_SUB_GB:
			{
				AK_ISP_GB_ATTR *p = (AK_ISP_GB_ATTR *)buf;
				//anyka_print("set GB param!\n");
				ak_isp_vp_set_gb_attr(p);
			}
			break;

		case ISP_DEMO_SUB_DEMO:
			{
				AK_ISP_DEMO_ATTR *p = (AK_ISP_DEMO_ATTR *)buf;
				//anyka_print("set DEMO param!\n");
				ak_isp_vp_set_demo_attr(p);
			}
			break;

		case ISP_GAMMA_SUB_GAMMA:
			{
				AK_ISP_RGB_GAMMA_ATTR *p = (AK_ISP_RGB_GAMMA_ATTR *)buf;
				//anyka_print("set GAMMA param!\n");
				ak_isp_vp_set_rgb_gamma_attr(p);
			}
			break;

		case ISP_CCM_SUB_CCM:
			{
				AK_ISP_CCM_ATTR *p = (AK_ISP_CCM_ATTR *)buf;
				//anyka_print("set CCM param!\n");
				ak_isp_vp_set_ccm_attr(p);
			}
			break;

		case ISP_FCS_SUB_FCS:
			{
				AK_ISP_FCS_ATTR *p = (AK_ISP_FCS_ATTR *)buf;
				//anyka_print("set FCS param!\n");
				ak_isp_vp_set_fcs_attr(p);
			}
			break;

		case ISP_WDR_SUB_WDR:
			{
				AK_ISP_WDR_ATTR *p = (AK_ISP_WDR_ATTR *)buf;
				//anyka_print("set WDR param!\n");
				ak_isp_vp_set_wdr_attr(p);
			}
			break;

		case ISP_SHARP_SUB_SHARP:
			{
				AK_ISP_SHARP_ATTR *p = (AK_ISP_SHARP_ATTR *)buf;
				//anyka_print("set SHARP param!\n");
				ak_isp_vp_set_sharp_attr(p);
			}
			break;

		case ISP_SHARP_SUB_SHARP_EX:
			{
				AK_ISP_SHARP_EX_ATTR *p = (AK_ISP_SHARP_EX_ATTR *)buf;
				//anyka_print("set SHARP param!\n");
				ak_isp_vp_set_sharp_ex_attr(p);
			}
			break;

		case ISP_SATURATION_SUB_SATURATION:
			{
				AK_ISP_SATURATION_ATTR *p = (AK_ISP_SATURATION_ATTR *)buf;
				//anyka_print("set SATURATION param!\n");
				ak_isp_vp_set_saturation_attr(p);
			}
			break;

		case ISP_CONSTRAST_SUB_CONSTRAST:
			{
				AK_ISP_CONTRAST_ATTR *p = (AK_ISP_CONTRAST_ATTR *)buf;
				//anyka_print("set CONSTRAST param!\n");
				ak_isp_vp_set_contrast_attr(p);
			}
			break;

		case ISP_YUVEFFECT_SUB_YUVEFFECT:
			{
				AK_ISP_EFFECT_ATTR *p = (AK_ISP_EFFECT_ATTR *)buf;
				//anyka_print("set YUV EFFECT param!\n");
				ak_isp_vp_set_effect_attr(p);
			}
			break;

		case ISP_RGB2YUV_SUB_RGB2YUV:
			{
				AK_ISP_RGB2YUV_ATTR *p = (AK_ISP_RGB2YUV_ATTR *)buf;
				//anyka_print("set RGB2YUV param!\n");
				ak_isp_vp_set_rgb2yuv_attr(p);
			}
			break;

		case ISP_DPC_SUB_DPC:
			{
				AK_ISP_DDPC_ATTR *p = (AK_ISP_DDPC_ATTR *)buf;
				//anyka_print("set DPC param!\n");
				ak_isp_vp_set_dpc_attr(p);
			}
			break;

		case ISP_DPC_SUB_SDPC:
			{
				AK_ISP_SDPC_ATTR *p = (AK_ISP_SDPC_ATTR *)buf;
				//anyka_print("set DPC param!\n");
				ak_isp_vp_set_sdpc_attr(p);// ispsdk in linux not call this function
			}
			break;

		case ISP_WEIGHT_SUB_WEIGHT:
			{
				AK_ISP_WEIGHT_ATTR *p = (AK_ISP_WEIGHT_ATTR *)buf;
				//anyka_print("set WEITHT param!\n");
				ak_isp_vp_set_zone_weight(p);
			}
			break;

		case ISP_AF_SUB_AF:
			{
				AK_ISP_AF_ATTR *p = (AK_ISP_AF_ATTR *)buf;
				//anyka_print("set AF param!\n");
				ak_isp_vp_set_af_attr(p);
			}
			break;

		case ISP_WB_SUB_WB:
			{
				AK_ISP_WB_TYPE_ATTR *p = (AK_ISP_WB_TYPE_ATTR *)buf;
				//anyka_print("set WB param!\n");
				ak_isp_vp_set_wb_type(p);
			}
			break;

		case ISP_WB_SUB_MWB:
			{
				AK_ISP_MWB_ATTR *p = (AK_ISP_MWB_ATTR *)buf;
				//anyka_print("set WB param!\n");
				ak_isp_vp_set_mwb_attr(p);
			}
			break;

		case ISP_WB_SUB_AWB:
			{
				AK_ISP_AWB_ATTR *p = (AK_ISP_AWB_ATTR *)buf;
				//anyka_print("set WB param!\n");
				ak_isp_vp_set_awb_attr(p);
			}
			break;

		case ISP_WB_SUB_AWB_EX:
			{
				AK_ISP_AWB_EX_ATTR *p = (AK_ISP_AWB_EX_ATTR *)buf;
				//anyka_print("set WB param!\n");
				ak_isp_vp_set_awb_ex_attr(p);
			}
			break;



		case ISP_EXP_SUB_RAW_HIST:
			{
				AK_ISP_RAW_HIST_ATTR *p = (AK_ISP_RAW_HIST_ATTR *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_raw_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_RGB_HIST:
			{
				AK_ISP_RGB_HIST_ATTR *p = (AK_ISP_RGB_HIST_ATTR *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_rgb_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_YUV_HIST:
			{
				AK_ISP_YUV_HIST_ATTR *p = (AK_ISP_YUV_HIST_ATTR *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_yuv_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_EXP_TYPE:
			{
				AK_ISP_EXP_TYPE *p = (AK_ISP_EXP_TYPE *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_exp_type(p);
			}
			break;
		case ISP_EXP_SUB_FRAME_RATE:
			{
				AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_frame_rate(p);
			}
			break;

		case ISP_EXP_SUB_AE:
			{
				AK_ISP_AE_ATTR *p = (AK_ISP_AE_ATTR *)buf;
				//anyka_print("set EXP param!\n");
				ak_isp_vp_set_ae_attr(p);
			}
			break;

		case ISP_MISC_SUB_MISC:
			{
				AK_ISP_MISC_ATTR *p = (AK_ISP_MISC_ATTR *)buf;
				//anyka_print("set MISC param!\n");
				ak_isp_vo_set_misc_attr(p);
			}
			break;

		case ISP_Y_GAMMA_SUB_Y_GAMMA:
			{
				AK_ISP_Y_GAMMA_ATTR *p = (AK_ISP_Y_GAMMA_ATTR*)buf;
				//anyka_print("set y gamma param!\n");
				ak_isp_vp_set_y_gamma_attr(p);
			}
			break;
		
		case ISP_HUE_SUB_HUE:
			{
				AK_ISP_HUE_ATTR *p = (AK_ISP_HUE_ATTR *)buf;
				//anyka_print("set hue param!\n");
				ak_isp_vp_set_hue_attr(p);
			}
			break;

		default:
			ret = false;
			break;
	}

	return ret;
}

/**
 * @brief get isp module configuration
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[in] module_id isp module id
 * @param[in] buf memory for this module
 * @return bool
 * @retval true if get successfully
 * @retval false if get unsuccessfully
 */
bool ispdrv_get_module(unsigned long module_id, unsigned char *buf)
{
	bool ret = true;

	switch (module_id) {
		case ISP_BB:
			{
				AK_ISP_INIT_BLC *p = (AK_ISP_INIT_BLC *)buf;
				//anyka_print("get BB param!\n");
				ak_isp_vp_get_blc_attr(&p->p_blc);
			}
			break;

		case ISP_LSC:
			{
				AK_ISP_INIT_LSC *p = (AK_ISP_INIT_LSC *)buf;
				//anyka_print("get LSC param!\n");
				ak_isp_vp_get_lsc_attr(&p->lsc);
			}
			break;

		case ISP_RAW_LUT:
			{
				AK_ISP_INIT_RAW_LUT *p = (AK_ISP_INIT_RAW_LUT *)buf;
				//anyka_print("get RAW GAMMA param!\n");
				ak_isp_vp_get_raw_lut_attr(&p->raw_lut_p);
			}
			break;

		case ISP_NR:
			{
				AK_ISP_INIT_NR *p = (AK_ISP_INIT_NR *)buf;
				//anyka_print("get NR param!\n");
				ak_isp_vp_get_nr1_attr(&p->p_nr1);
				ak_isp_vp_get_nr2_attr(&p->p_nr2);
			}
			break;

		case ISP_3DNR:
			{
				AK_ISP_INIT_3DNR *p = (AK_ISP_INIT_3DNR *)buf;
				//anyka_print("get 3DNR param!\n");
				ak_isp_vp_get_3d_nr_attr(&p->p_3d_nr);
			}
			break;

		case ISP_GB:
			{
				AK_ISP_INIT_GB *p = (AK_ISP_INIT_GB *)buf;
				//anyka_print("get GB param!\n");
				ak_isp_vp_get_gb_attr(&p->p_gb);
			}
			break;

		case ISP_DEMO:
			{
				AK_ISP_INIT_DEMO *p = (AK_ISP_INIT_DEMO *)buf;
				//anyka_print("get DEMO param!\n");
				ak_isp_vp_get_demo_attr(&p->p_demo_attr);
			}
			break;

		case ISP_GAMMA:
			{
				AK_ISP_INIT_GAMMA *p = (AK_ISP_INIT_GAMMA *)buf;
				//anyka_print("get GAMMA param!\n");
				ak_isp_vp_get_rgb_gamma_attr(&p->p_gamma_attr);
			}
			break;

		case ISP_CCM:
			{
				AK_ISP_INIT_CCM *p = (AK_ISP_INIT_CCM *)buf;
				//anyka_print("get CCM param!\n");
				ak_isp_vp_get_ccm_attr(&p->p_ccm);
			}
			break;

		case ISP_FCS:
			{
				AK_ISP_INIT_FCS *p = (AK_ISP_INIT_FCS *)buf;
				//anyka_print("get FCS param!\n");
				ak_isp_vp_get_fcs_attr(&p->p_fcs);
			}
			break;

		case ISP_WDR:
			{
				AK_ISP_INIT_WDR *p = (AK_ISP_INIT_WDR *)buf;
				//anyka_print("get WDR param!\n");
				ak_isp_vp_get_wdr_attr(&p->p_wdr_attr);
			}
			break;

		case ISP_SHARP:
			{
				AK_ISP_INIT_SHARP *p = (AK_ISP_INIT_SHARP *)buf;
				//anyka_print("get SHARP param!\n");
				ak_isp_vp_get_sharp_attr(&p->p_sharp_attr);
				ak_isp_vp_get_sharp_ex_attr(&p->p_sharp_ex_attr);
			}
			break;

		case ISP_SATURATION:
			{
				AK_ISP_INIT_SATURATION *p = (AK_ISP_INIT_SATURATION *)buf;
				//anyka_print("get SATURATION param!\n");
				ak_isp_vp_get_saturation_attr(&p->p_se_attr);
			}
			break;

		case ISP_CONSTRAST:
			{
				AK_ISP_INIT_CONTRAST *p = (AK_ISP_INIT_CONTRAST *)buf;
				//anyka_print("get CONSTRAST param!\n");
				ak_isp_vp_get_contrast_attr(&p->p_contrast);
			}
			break;

		case ISP_YUVEFFECT:
			{
				AK_ISP_INIT_EFFECT *p = (AK_ISP_INIT_EFFECT *)buf;
				//anyka_print("get YUV EFFECT param!\n");
				ak_isp_vp_get_effect_attr(&p->p_isp_effect);
			}
			break;

		case ISP_RGB2YUV:
			{
				AK_ISP_INIT_RGB2YUV *p = (AK_ISP_INIT_RGB2YUV *)buf;
				//anyka_print("get RGB2YUV param!\n");
				ak_isp_vp_get_rgb2yuv_attr(&p->p_rgb2yuv);
			}
			break;

		case ISP_DPC:
			{
				AK_ISP_INIT_DPC *p = (AK_ISP_INIT_DPC *)buf;
				//anyka_print("get DPC param!\n");
				ak_isp_vp_get_dpc_attr(&p->p_ddpc);
				ak_isp_vp_get_sdpc_attr(&p->p_sdpc);// ispsdk in linux not call this function
			}
			break;

		case ISP_WEIGHT:
			{
				AK_ISP_INIT_WEIGHT *p = (AK_ISP_INIT_WEIGHT *)buf;
				//anyka_print("get WEITHT param!\n");
				ak_isp_vp_get_zone_weight(&p->p_weight);
			}
			break;

		case ISP_AF:
			{
				AK_ISP_INIT_AF *p = (AK_ISP_INIT_AF *)buf;
				//anyka_print("get AF param!\n");
				ak_isp_vp_get_af_attr(&p->p_af_attr);
			}
			break;

		case ISP_WB:
			{
				AK_ISP_INIT_WB *p = (AK_ISP_INIT_WB *)buf;
				//anyka_print("get WB param!\n");
				ak_isp_vp_get_wb_type(&p->wb_type);
				ak_isp_vp_get_mwb_attr(&p->p_mwb);
				ak_isp_vp_get_awb_attr(&p->p_awb);
				ak_isp_vp_get_awb_ex_attr(&p->p_awb_ex);
			}
			break;

		case ISP_EXP:
			{
				AK_ISP_INIT_EXP *p = (AK_ISP_INIT_EXP *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_raw_hist_attr(&p->p_raw_hist);
				ak_isp_vp_get_rgb_hist_attr(&p->p_rgb_hist);
				ak_isp_vp_get_yuv_hist_attr(&p->p_yuv_hist);
				ak_isp_vp_get_exp_type(&p->p_exp_type);
				ak_isp_vp_get_frame_rate(&p->p_frame_rate);
				ak_isp_vp_get_ae_attr(&p->p_ae);
			}
			break;

		case ISP_MISC:
			{
				AK_ISP_INIT_MISC *p = (AK_ISP_INIT_MISC *)buf;
				//anyka_print("get MISC param!\n");
				ak_isp_vo_get_misc_attr(&p->p_misc);
			}
			break;

		case ISP_Y_GAMMA:
			{
				AK_ISP_INIT_Y_GAMMA *p = (AK_ISP_INIT_Y_GAMMA*)buf;
				
				ak_isp_vp_get_y_gamma_attr(&p->p_gamma_attr);
			}
			break;
		
		case ISP_HUE:
			{
				AK_ISP_INIT_HUE *p = (AK_ISP_INIT_HUE*)buf;
				
				ret = ak_isp_vp_get_hue_attr(&p->p_hue);
			}
			break;

		case ISP_SENSOR:
			break;

		case ISP_BB_SUB_BLC:
			{
				AK_ISP_BLC_ATTR *p = (AK_ISP_BLC_ATTR *)buf;
				//anyka_print("get BB param!\n");
				ak_isp_vp_get_blc_attr(p);
			}
			break;

		case ISP_LSC_SUB_LSC:
			{
				AK_ISP_LSC_ATTR *p = (AK_ISP_LSC_ATTR *)buf;
				//anyka_print("get LSC param!\n");
				ak_isp_vp_get_lsc_attr(p);
			}
			break;

		case ISP_RAW_LUT_SUB_LUT:
			{
				AK_ISP_RAW_LUT_ATTR *p = (AK_ISP_RAW_LUT_ATTR *)buf;
				//anyka_print("get RAW GAMMA param!\n");
				ak_isp_vp_get_raw_lut_attr(p);
			}
			break;

		case ISP_NR_SUB_NR1:
			{
				AK_ISP_NR1_ATTR *p = (AK_ISP_NR1_ATTR *)buf;
				//anyka_print("get NR param!\n");
				ak_isp_vp_get_nr1_attr(p);
			}
			break;

		case ISP_NR_SUB_NR2:
			{
				AK_ISP_NR2_ATTR *p = (AK_ISP_NR2_ATTR *)buf;
				//anyka_print("get NR param!\n");
				ak_isp_vp_get_nr2_attr(p);
			}
			break;

		case ISP_3DNR_SUB_3DNR:
			{
				AK_ISP_3D_NR_ATTR *p = (AK_ISP_3D_NR_ATTR *)buf;
				//anyka_print("get 3DNR param!\n");
				ak_isp_vp_get_3d_nr_attr(p);
			}
			break;

		case ISP_GB_SUB_GB:
			{
				AK_ISP_GB_ATTR *p = (AK_ISP_GB_ATTR *)buf;
				//anyka_print("get GB param!\n");
				ak_isp_vp_get_gb_attr(p);
			}
			break;

		case ISP_DEMO_SUB_DEMO:
			{
				AK_ISP_DEMO_ATTR *p = (AK_ISP_DEMO_ATTR *)buf;
				//anyka_print("get DEMO param!\n");
				ak_isp_vp_get_demo_attr(p);
			}
			break;

		case ISP_GAMMA_SUB_GAMMA:
			{
				AK_ISP_RGB_GAMMA_ATTR *p = (AK_ISP_RGB_GAMMA_ATTR *)buf;
				//anyka_print("get GAMMA param!\n");
				ak_isp_vp_get_rgb_gamma_attr(p);
			}
			break;

		case ISP_CCM_SUB_CCM:
			{
				AK_ISP_CCM_ATTR *p = (AK_ISP_CCM_ATTR *)buf;
				//anyka_print("get CCM param!\n");
				ak_isp_vp_get_ccm_attr(p);
			}
			break;

		case ISP_FCS_SUB_FCS:
			{
				AK_ISP_FCS_ATTR *p = (AK_ISP_FCS_ATTR *)buf;
				//anyka_print("get FCS param!\n");
				ak_isp_vp_get_fcs_attr(p);
			}
			break;

		case ISP_WDR_SUB_WDR:
			{
				AK_ISP_WDR_ATTR *p = (AK_ISP_WDR_ATTR *)buf;
				//anyka_print("get WDR param!\n");
				ak_isp_vp_get_wdr_attr(p);
			}
			break;

		case ISP_SHARP_SUB_SHARP:
			{
				AK_ISP_SHARP_ATTR *p = (AK_ISP_SHARP_ATTR *)buf;
				//anyka_print("get SHARP param!\n");
				ak_isp_vp_get_sharp_attr(p);
			}
			break;

		case ISP_SHARP_SUB_SHARP_EX:
			{
				AK_ISP_SHARP_EX_ATTR *p = (AK_ISP_SHARP_EX_ATTR *)buf;
				//anyka_print("get SHARP param!\n");
				ak_isp_vp_get_sharp_ex_attr(p);
			}
			break;

		case ISP_SATURATION_SUB_SATURATION:
			{
				AK_ISP_SATURATION_ATTR *p = (AK_ISP_SATURATION_ATTR *)buf;
				//anyka_print("get SATURATION param!\n");
				ak_isp_vp_get_saturation_attr(p);
			}
			break;

		case ISP_CONSTRAST_SUB_CONSTRAST:
			{
				AK_ISP_CONTRAST_ATTR *p = (AK_ISP_CONTRAST_ATTR *)buf;
				//anyka_print("get CONSTRAST param!\n");
				ak_isp_vp_get_contrast_attr(p);
			}
			break;

		case ISP_YUVEFFECT_SUB_YUVEFFECT:
			{
				AK_ISP_EFFECT_ATTR *p = (AK_ISP_EFFECT_ATTR *)buf;
				//anyka_print("get YUV EFFECT param!\n");
				ak_isp_vp_get_effect_attr(p);
			}
			break;

		case ISP_RGB2YUV_SUB_RGB2YUV:
			{
				AK_ISP_RGB2YUV_ATTR *p = (AK_ISP_RGB2YUV_ATTR *)buf;
				//anyka_print("get RGB2YUV param!\n");
				ak_isp_vp_get_rgb2yuv_attr(p);
			}
			break;

		case ISP_DPC_SUB_DPC:
			{
				AK_ISP_DDPC_ATTR *p = (AK_ISP_DDPC_ATTR *)buf;
				//anyka_print("get DPC param!\n");
				ak_isp_vp_get_dpc_attr(p);
			}
			break;

		case ISP_DPC_SUB_SDPC:
			{
				AK_ISP_SDPC_ATTR *p = (AK_ISP_SDPC_ATTR *)buf;
				//anyka_print("get DPC param!\n");
				ak_isp_vp_get_sdpc_attr(p);// ispsdk in linux not call this function
			}
			break;

		case ISP_WEIGHT_SUB_WEIGHT:
			{
				AK_ISP_WEIGHT_ATTR *p = (AK_ISP_WEIGHT_ATTR *)buf;
				//anyka_print("get WEITHT param!\n");
				ak_isp_vp_get_zone_weight(p);
			}
			break;

		case ISP_AF_SUB_AF:
			{
				AK_ISP_AF_ATTR *p = (AK_ISP_AF_ATTR *)buf;
				//anyka_print("get AF param!\n");
				ak_isp_vp_get_af_attr(p);
			}
			break;

		case ISP_WB_SUB_WB:
			{
				AK_ISP_INIT_WB *p = (AK_ISP_INIT_WB *)buf;
				//anyka_print("get WB param!\n");
				ak_isp_vp_get_wb_type(&p->wb_type);
			}
			break;

		case ISP_WB_SUB_MWB:
			{
				AK_ISP_MWB_ATTR *p = (AK_ISP_MWB_ATTR *)buf;
				//anyka_print("get WB param!\n");
				ak_isp_vp_get_mwb_attr(p);
			}
			break;

		case ISP_WB_SUB_AWB:
			{
				AK_ISP_AWB_ATTR *p = (AK_ISP_AWB_ATTR *)buf;
				//anyka_print("get WB param!\n");
				ak_isp_vp_get_awb_attr(p);
			}
			break;

		case ISP_WB_SUB_AWB_EX:
			{
				AK_ISP_AWB_EX_ATTR *p = (AK_ISP_AWB_EX_ATTR *)buf;
				//anyka_print("get WB param!\n");
				ak_isp_vp_get_awb_ex_attr(p);
			}
			break;

		case ISP_EXP_SUB_RAW_HIST:
			{
				AK_ISP_RAW_HIST_ATTR *p = (AK_ISP_RAW_HIST_ATTR *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_raw_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_RGB_HIST:
			{
				AK_ISP_RGB_HIST_ATTR *p = (AK_ISP_RGB_HIST_ATTR *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_rgb_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_YUV_HIST:
			{
				AK_ISP_YUV_HIST_ATTR *p = (AK_ISP_YUV_HIST_ATTR *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_yuv_hist_attr(p);
			}
			break;

		case ISP_EXP_SUB_EXP_TYPE:
			{
				AK_ISP_EXP_TYPE *p = (AK_ISP_EXP_TYPE *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_exp_type(p);
			}
			break;

		case ISP_EXP_SUB_FRAME_RATE:
			{
				AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_frame_rate(p);
			}
			break;

		case ISP_EXP_SUB_AE:
			{
				AK_ISP_AE_ATTR *p = (AK_ISP_AE_ATTR *)buf;
				//anyka_print("get EXP param!\n");
				ak_isp_vp_get_ae_attr(p);
			}
			break;

		case ISP_MISC_SUB_MISC:
			{
				AK_ISP_MISC_ATTR *p = (AK_ISP_MISC_ATTR *)buf;
				//anyka_print("get MISC param!\n");
				ak_isp_vo_get_misc_attr(p);
			}
			break;

		case ISP_Y_GAMMA_SUB_Y_GAMMA:
			{
				AK_ISP_Y_GAMMA_ATTR *p = (AK_ISP_Y_GAMMA_ATTR*)buf;
				//anyka_print("set y gamma param!\n");
				ak_isp_vp_get_y_gamma_attr(p);
			}
			break;
		
		case ISP_HUE_SUB_HUE:
			{
				AK_ISP_HUE_ATTR *p = (AK_ISP_HUE_ATTR *)buf;
				//anyka_print("set hue param!\n");
				ak_isp_vp_get_hue_attr(p);
			}
			break;

		case ISP_3DSTAT:
			{
				AK_ISP_3D_NR_STAT_INFO *p = (AK_ISP_3D_NR_STAT_INFO *)buf;
				ak_isp_vp_get_3d_nr_stat_info(p);
			}
			break;

		case ISP_AESTAT:
			{
				int size = 0;
				ak_isp_vp_get_ae_run_info((AK_ISP_AE_RUN_INFO *)(buf + size));
				size += sizeof(AK_ISP_AE_RUN_INFO);
				ak_isp_vp_get_raw_hist_stat_info((AK_ISP_RAW_HIST_STAT_INFO *)(buf + size));
				size += sizeof(AK_ISP_RAW_HIST_STAT_INFO);
				ak_isp_vp_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO *)(buf + size));
				size += sizeof(AK_ISP_RGB_HIST_STAT_INFO);
				ak_isp_vp_get_yuv_hist_stat_info((AK_ISP_YUV_HIST_STAT_INFO *)(buf + size));
				size += sizeof(AK_ISP_YUV_HIST_STAT_INFO);
			}
			break;
			
		case ISP_AFSTAT:
			ak_isp_vp_get_af_stat_info((AK_ISP_AF_STAT_INFO *)buf);
			break;
			
		case ISP_AWBSTAT:
			ak_isp_vp_get_awb_stat_info((AK_ISP_AWB_STAT_INFO *)buf);
			break;

		case ISP_AESTAT_SUB_AE:
			ak_isp_vp_get_ae_run_info((AK_ISP_AE_RUN_INFO *)buf);
			break;

		case ISP_AESTAT_SUB_RAW_HIST:
			ak_isp_vp_get_raw_hist_stat_info((AK_ISP_RAW_HIST_STAT_INFO *)buf);
			break;

		case ISP_AESTAT_SUB_RGB_HIST:
			ak_isp_vp_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO *)buf);
			break;
			
		case ISP_AESTAT_SUB_YUV_HIST:
			ak_isp_vp_get_yuv_hist_stat_info((AK_ISP_YUV_HIST_STAT_INFO *)buf);
			break;

		default:
			ret = false;
			break;
	}

	return ret;
}

/**
 * @brief pause isp
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true  successfully
 * @retval false  unsuccessfully
 */
bool ispdrv_isp_pause(void)
{
	return ak_isp_set_isp_capturing(0);
}
/**
 * @brief resume isp
 * @author ye_guohong   
 * @date 2016-10-19
 * @param[]
 * @return bool
 * @retval true  successfully
 * @retval false  unsuccessfully
 */
bool ispdrv_isp_resume(void)
{
	return ak_isp_set_isp_capturing(1);
}

#endif
