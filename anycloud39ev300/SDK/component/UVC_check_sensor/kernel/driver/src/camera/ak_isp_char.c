/** 
 * @file ak_isp_char.c
 * @brief provide interfaces for isp
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author yang_mengxia
 * @date 2017-04-18
 * @version 1.0
 */
#include <string.h>
#include "anyka_types.h"
#include "drv_api.h"
#include "ak_isp_drv.h"

#include "ak_isp_char.h"



//#define ISP_DEBUG
#ifdef ISP_DEBUG
#define ISP_PRINTF(stuff...)		printk(" ISP: " stuff)
#else
#define ISP_PRINTF(fmt, args...)	do{}while(0)
#endif 
#define isp_info(stuff...)		printk(" ISP: " stuff)

#define AKISP_REG_MEM_START	0x20000000
#define AKISP_REG_MEM_END	0x2000004c
#define RESOURCE_SIZE		(AKISP_REG_MEM_END - AKISP_REG_MEM_START + 1)

static int power_hz = 50;

static int  printk_blc(AK_ISP_BLC_ATTR  *blc_para)
{
   int i = 0;
   
   ISP_PRINTF("\n\n\n");
   ISP_PRINTF("blc_para->blc_mode=%d\n", blc_para->blc_mode);
   
   ISP_PRINTF("blc_para->m_blc.black_level_enable=%d\n", blc_para->m_blc.black_level_enable);
   ISP_PRINTF("blc_para->m_blc.bl_r_a=%d\n", blc_para->m_blc.bl_r_a);
   ISP_PRINTF("blc_para->m_blc.bl_r_offset=%d\n", blc_para->m_blc.bl_r_offset);
   ISP_PRINTF("blc_para->m_blc.bl_gr_a = %d\n", blc_para->m_blc.bl_gr_a);
   ISP_PRINTF("blc_para->m_blc.bl_gr_offset =%d\n", blc_para->m_blc.bl_gr_offset);
   ISP_PRINTF("blc_para->m_blc.bl_gb_a =%d\n", blc_para->m_blc.bl_gb_a);
   ISP_PRINTF("blc_para->m_blc.bl_gb_offset=%d", blc_para->m_blc.bl_gb_offset);
   ISP_PRINTF("blc_para->m_blc.bl_b_a=%d\n", blc_para->m_blc.bl_b_a);
   ISP_PRINTF("blc_para->m_blc.bl_b_offset=%d\n", blc_para->m_blc.bl_b_offset);

   for (i=0; i<9; i++)
   {
   		ISP_PRINTF("blc_para.linkage_blc[%d].black_level_enable=%d\n", i, blc_para->linkage_blc[i].black_level_enable);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_r_a=%d\n", i, blc_para->linkage_blc[i].bl_r_a);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_r_offset=%d\n", i, blc_para->linkage_blc[i].bl_r_offset);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_gr_a=%d\n", i, blc_para->linkage_blc[i].bl_gr_a);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_gr_offset=%d\n", i, blc_para->linkage_blc[i].bl_gr_offset);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_gb_a=%d\n", i, blc_para->linkage_blc[i].bl_gb_a);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_gb_offset=%d\n", i, blc_para->linkage_blc[i].bl_gb_offset);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_b_a=%d\n", i, blc_para->linkage_blc[i].bl_b_a);
		ISP_PRINTF("blc_para.linkage_blc[%d].bl_b_offset=%d\n", i, blc_para->linkage_blc[i].bl_b_offset);
   }
   
   return 0; 
}

static int  printk_gb(AK_ISP_GB_ATTR  *gb_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("gb_para->gb_mode = %d\n", gb_para->gb_mode);
	ISP_PRINTF("gb_para->manual_gb.gb_enable = %d\n", gb_para->manual_gb.gb_enable);
	ISP_PRINTF("gb_para->manual_gb.gb_en_th = %d\n", gb_para->manual_gb.gb_en_th);
	ISP_PRINTF("gb_para->manual_gb.gb_kstep=%d\n", gb_para->manual_gb.gb_kstep);
	ISP_PRINTF("gb_para->manual_gb.gb_threshold=%d\n",gb_para->manual_gb.gb_threshold);

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("gb_para.linkage_gb[%d].gb_enable=%d\n", i, gb_para->linkage_gb[i].gb_enable);
		ISP_PRINTF("gb_para.linkage_gb[%d].gb_en_th=%d\n", i, gb_para->linkage_gb[i].gb_en_th);
		ISP_PRINTF("gb_para.linkage_gb[%d].gb_kstep=%d\n", i, gb_para->linkage_gb[i].gb_kstep);
		ISP_PRINTF("gb_para.linkage_gb[%d].gb_threshold=%d\n", i, gb_para->linkage_gb[i].gb_threshold);
	}
	
	return 0;
}

static int printk_raw_lut(AK_ISP_RAW_LUT_ATTR *lut_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("lut_para->raw_gamma_enable=%d\n", lut_para->raw_gamma_enable);
	
	for (i=0; i<129; i++)
	{
		ISP_PRINTF("lut_para->raw_r[%d]=%d\n", i, lut_para->raw_r[i]);
		ISP_PRINTF("lut_para->raw_g[%d]=%d\n", i, lut_para->raw_g[i]);
		ISP_PRINTF("lut_para->raw_b[%d]=%d\n", i, lut_para->raw_b[i]);
	}
	
	return 0;
}

static int printk_nr1_para(AK_ISP_NR1_ATTR *nr1)
{
	int i = 0;
	int j = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("nr1->nr1_mode=%d\n", nr1->nr1_mode);
	ISP_PRINTF("nr1->manual_nr1.nr1_enable=%d\n", nr1->manual_nr1.nr1_enable);
	ISP_PRINTF("nr1->manual_nr1.nr1_calc_r_k=%d\n", nr1->manual_nr1.nr1_calc_r_k);
	ISP_PRINTF("nr1->manual_nr1.nr1_calc_g_k=%d\n", nr1->manual_nr1.nr1_calc_g_k);
	ISP_PRINTF("nr1->manual_nr1.nr1_calc_b_k=%d\n", nr1->manual_nr1.nr1_calc_b_k);
	
	for (i=0; i<17; i++)
	{
	   ISP_PRINTF("nr1->manual_nr1.nr1_weight_rtbl[%d]=%d\n", i, nr1->manual_nr1.nr1_weight_rtbl[i]);
	}
	
	for (i=0; i<17; i++)
	{
	   ISP_PRINTF("nr1->manual_nr1.nr1_weight_gtbl[%d]=%d\n", i, nr1->manual_nr1.nr1_weight_gtbl[i]);
	}
	
	for (i=0; i<17; i++)
	{
	   ISP_PRINTF("nr1->manual_nr1.nr1_weight_btbl[%d]=%d\n", i, nr1->manual_nr1.nr1_weight_btbl[i]);
	}
	
	for (i=0; i<17; i++)
	{
	   ISP_PRINTF("nr1->manual_nr1.nr1_lc_lut[%d]=%d\n", i, nr1->manual_nr1.nr1_lc_lut[i]);
	}
	
	for (j=0; j<9; j++)
	{
		ISP_PRINTF("nr1->linkage_nr1[%d].nr1_enable=%d\n", j, nr1->manual_nr1.nr1_enable);
	    ISP_PRINTF("nr1->linkage_nr1[%d]1.nr1_calc_r_k=%d\n", j, nr1->linkage_nr1[j].nr1_calc_r_k);
		ISP_PRINTF("nr1->linkage_nr1[%d].nr1_calc_g_k=%d\n", j, nr1->linkage_nr1[j].nr1_calc_g_k);
		ISP_PRINTF("nr1->linkage_nr1[%d].nr1_calc_b_k=%d\n", j, nr1->linkage_nr1[j].nr1_calc_b_k);
	    
		for (i=0; i<17; i++)
		{
		   ISP_PRINTF("nr1->linkage_nr1[%d].nr1_weight_rtbl[%d]=%d\n", j, i, nr1->linkage_nr1[j].nr1_weight_rtbl[i]);
		}

		//ISP_PRINTF("nr1->linkage_nr1[%d].nr1_calc_g_k=%d\n",j,nr1->linkage_nr1[j].nr1_calc_g_k);
		for (i=0; i<17; i++)
		{
		   ISP_PRINTF("nr1->linkage_nr1[%d].nr1_weight_gtbl[%d]=%d\n", j, i, nr1->linkage_nr1[j].nr1_weight_gtbl[i]);
		}

		//ISP_PRINTF("nr1->linkage_nr1[%d].nr1_calc_b_k=%d\n",j,nr1->linkage_nr1[j].nr1_calc_b_k);

		for (i=0; i<17; i++)
		{
		   ISP_PRINTF("nr1->linkage_nr1[%d].nr1_weight_btbl[%d]=%d\n", j, i, nr1->linkage_nr1[j].nr1_weight_btbl[i]);
		}
		
		for (i=0; i<17; i++)
		{
		   ISP_PRINTF("nr1->linkage_nr1[%d].nr1_lc_lut[%d]=%d\n", j, i, nr1->linkage_nr1[j].nr1_lc_lut[i]);
		}
	}

	return 0;
}

static int printk_demo_para(AK_ISP_DEMO_ATTR *demo_para)
{
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("demo_para->dm_bg_gain=%d\n", demo_para->dm_bg_gain);
	ISP_PRINTF("demo_para->dm_bg_thre=%d\n", demo_para->dm_bg_thre);
	ISP_PRINTF("demo_para->dm_gb_gain=%d\n", demo_para->dm_gb_gain);
	ISP_PRINTF("demo_para->dm_gr_gain=%d\n", demo_para->dm_gr_gain);
	ISP_PRINTF("demo_para->dm_hf_th1=%d\n", demo_para->dm_hf_th1);
	ISP_PRINTF("demo_para->dm_hf_th2=%d\n", demo_para->dm_hf_th2);
	ISP_PRINTF("demo_para->dm_HV_th=%d\n", demo_para->dm_HV_th);
	ISP_PRINTF("demo_para->dm_rg_gain=%d\n", demo_para->dm_rg_gain);
	ISP_PRINTF("demo_para->dm_rg_thre=%d\n", demo_para->dm_rg_thre);
	
	return 0;
}

static int printk_ccm_para(AK_ISP_CCM_ATTR *ccm_para)
{
    int i,j,k;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("ccm_para->cc_mode=%d\n", ccm_para->cc_mode);
	ISP_PRINTF("ccm_para->manual_ccm.cc_enable=%d\n", ccm_para->manual_ccm.cc_enable);
	ISP_PRINTF("ccm_para->manual_ccm.cc_cnoise_yth=%d\n", ccm_para->manual_ccm.cc_cnoise_yth);
	ISP_PRINTF("ccm_para->manual_ccm.cc_cnoise_gain=%d\n", ccm_para->manual_ccm.cc_cnoise_gain);
	ISP_PRINTF("ccm_para->manual_ccm.cc_cnoise_slop=%d\n", ccm_para->manual_ccm.cc_cnoise_slop);
	
	for (i=0; i<3; i++)
	{
		for (j=0; j<3; j++)
		{
		   ISP_PRINTF("ccm_para->manual_ccm.ccm[%d][%d]=%d\n", i, j, ccm_para->manual_ccm.ccm[i][j]);
		}
	}

	for (k=0; k<4; k++)
	{
		ISP_PRINTF("ccm_para->ccm[%d].cc_enable=%d\n", k, ccm_para->ccm[k].cc_enable);
		ISP_PRINTF("ccm_para->ccm[%d].cc_cnoise_yth=%d\n", k, ccm_para->ccm[k].cc_cnoise_yth);
		ISP_PRINTF("ccm_para->ccm[%d].cc_cnoise_gain=%d\n", k, ccm_para->ccm[k].cc_cnoise_gain);
		ISP_PRINTF("ccm_para->ccm[%d].cc_cnoise_slop=%d\n", k, ccm_para->ccm[k].cc_cnoise_slop);
		
		for (i=0; i<3; i++)
		{
			for (j=0; j<3; j++)
			{
			   ISP_PRINTF("ccm_para->ccm[%d].ccm[%d][%d]=%d\n", k, i, j, ccm_para->ccm[k].ccm[i][j]);
			}
		}
	}
		
	return 0;
}


static int printk_wdr_para(AK_ISP_WDR_ATTR *wdr_para)
{
	int i = 0, j = 0;
  
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("wdr_para->wdr_mode=%d\n", wdr_para->wdr_mode);
	
	ISP_PRINTF("wdr_para->manual_wdr.hdr_uv_adjust_level=%d\n", wdr_para->manual_wdr.hdr_uv_adjust_level);
	ISP_PRINTF("wdr_para->manual_wdr.hdr_cnoise_suppress_slop=%d\n", wdr_para->manual_wdr.hdr_cnoise_suppress_slop);
	ISP_PRINTF("wdr_para->manual_wdr.wdr_enable=%d\n", wdr_para->manual_wdr.wdr_enable);
	ISP_PRINTF("wdr_para->manual_wdr.hdr_uv_adjust_enable=%d\n", wdr_para->manual_wdr.hdr_uv_adjust_enable);
	ISP_PRINTF("wdr_para->manual_wdr.hdr_cnoise_suppress_yth1=%d\n", wdr_para->manual_wdr.hdr_cnoise_suppress_yth1);
	ISP_PRINTF("wdr_para->manual_wdr.hdr_cnoise_suppress_yth2=%d\n", wdr_para->manual_wdr.hdr_cnoise_suppress_yth2);
	ISP_PRINTF("wdr_para->manual_wdr.hdr_cnoise_suppress_gain=%d\n", wdr_para->manual_wdr.hdr_cnoise_suppress_gain);
  
	ISP_PRINTF("wdr_para->manual_wdr.wdr_th1=%d\n", wdr_para->manual_wdr.wdr_th1);
	ISP_PRINTF("wdr_para->manual_wdr.wdr_th2=%d\n", wdr_para->manual_wdr.wdr_th2);
	ISP_PRINTF("wdr_para->manual_wdr.wdr_th3=%d\n", wdr_para->manual_wdr.wdr_th3);
	ISP_PRINTF("wdr_para->manual_wdr.wdr_th4=%d\n", wdr_para->manual_wdr.wdr_th4);
	ISP_PRINTF("wdr_para->manual_wdr.wdr_th5=%d\n", wdr_para->manual_wdr.wdr_th5);
  
	for (i=0; i<65; i++)
	{
		ISP_PRINTF("wdr_para->manual_wdr.area_tb1[%d]=%d\n", i, wdr_para->manual_wdr.area_tb1[i]);
	}

	for (i=0; i<65; i++)
	{
		ISP_PRINTF("wdr_para->manual_wdr.area_tb2[%d]=%d\n", i, wdr_para->manual_wdr.area_tb2[i]);
	}

	for (i=0; i<65; i++)
	{
		ISP_PRINTF("wdr_para->manual_wdr.area_tb3[%d]=%d\n", i, wdr_para->manual_wdr.area_tb3[i]);
	}

	for (i=0; i<65; i++)
	{
		ISP_PRINTF("wdr_para->manual_wdr.area_tb4[%d]=%d\n", i, wdr_para->manual_wdr.area_tb4[i]);
	}

	for (i=0; i<65; i++)
	{
		ISP_PRINTF("wdr_para->manual_wdr.area_tb5[%d]=%d\n", i, wdr_para->manual_wdr.area_tb5[i]);
	}

	for (j=0; j<9; j++)
	{
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_uv_adjust_level=%d\n", j, wdr_para->linkage_wdr[j].hdr_uv_adjust_level);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_cnoise_suppress_slop=%d\n", j, wdr_para->linkage_wdr[j].hdr_cnoise_suppress_slop);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_enable=%d\n", j, wdr_para->linkage_wdr[j].wdr_enable);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_uv_adjust_enable=%d\n", j, wdr_para->linkage_wdr[j].hdr_uv_adjust_enable);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_cnoise_suppress_yth1=%d\n", j, wdr_para->linkage_wdr[j].hdr_cnoise_suppress_yth1);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_cnoise_suppress_yth2=%d\n", j, wdr_para->linkage_wdr[j].hdr_cnoise_suppress_yth2);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].hdr_cnoise_suppress_gain=%d\n", j, wdr_para->linkage_wdr[j].hdr_cnoise_suppress_gain);
	  
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_th1=%d\n", j, wdr_para->linkage_wdr[j].wdr_th1);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_th2=%d\n", j, wdr_para->linkage_wdr[j].wdr_th2);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_th3=%d\n", j, wdr_para->linkage_wdr[j].wdr_th3);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_th4=%d\n", j, wdr_para->linkage_wdr[j].wdr_th4);
		ISP_PRINTF("wdr_para->linkage_wdr[%d].wdr_th5=%d\n", j, wdr_para->linkage_wdr[j].wdr_th5);

		for (i=0; i<65; i++)
		{
			ISP_PRINTF("wdr_para->linkage_wdr[%d].area_tb1[%d]=%d\n", j, i, wdr_para->linkage_wdr[j].area_tb1[i]);
		}

		for (i=0; i<65; i++)
		{
			ISP_PRINTF("wdr_para->linkage_wdr[%d].area_tb2[%d]=%d\n", j, i, wdr_para->linkage_wdr[j].area_tb2[i]);
		}

		for (i=0; i<65; i++)
		{
			ISP_PRINTF("wdr_para->linkage_wdr[%d].area_tb3[%d]=%d\n", j, i, wdr_para->linkage_wdr[j].area_tb3[i]);
		}

		for (i=0; i<65; i++)
		{
			ISP_PRINTF("wdr_para->linkage_wdr[%d].area_tb4[%d]=%d\n", j, i, wdr_para->linkage_wdr[j].area_tb4[i]);
		}

		for (i=0; i<65; i++)
		{
			ISP_PRINTF("wdr_para->linkage_wdr[%d].area_tb5[%d]=%d\n", j, i, wdr_para->linkage_wdr[j].area_tb5[i]);
		}
	}

	return 0;
}

static int printk_nr2_para(AK_ISP_NR2_ATTR *nr2_para)
{
	int i = 0;
	int j = 0;
   
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("nr2_para->nr2_mode=%d\n", nr2_para->nr2_mode);
	ISP_PRINTF("nr2_para->manual_nr2.nr2_enable=%d\n", nr2_para->manual_nr2.nr2_enable);
	ISP_PRINTF("nr2_para->manual_nr2.nr2_calc_y_k=%d\n", nr2_para->manual_nr2.nr2_calc_y_k);
	ISP_PRINTF("nr2_para->manual_nr2.nr2_k=%d\n", nr2_para->manual_nr2.nr2_k);
	ISP_PRINTF("nr2_para->manual_nr2.y_dpc_enable=%d\n", nr2_para->manual_nr2.y_dpc_enable);
	ISP_PRINTF("nr2_para->manual_nr2.y_dpc_th=%d\n", nr2_para->manual_nr2.y_dpc_th);
	ISP_PRINTF("nr2_para->manual_nr2.y_black_dpc_enable=%d\n", nr2_para->manual_nr2.y_black_dpc_enable);
	ISP_PRINTF("nr2_para->manual_nr2.y_white_dpc_enable=%d\n", nr2_para->manual_nr2.y_white_dpc_enable);
   
	for (i=0; i<17; i++)
	{
		ISP_PRINTF("nr2_para->manual_nr2.nr2_weight_tbl[%d]=%d\n", i, nr2_para->manual_nr2.nr2_weight_tbl[i]);
	}

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("nr2_para->linkage_nr2[%d].nr2_enable=%d\n", i, nr2_para->linkage_nr2[i].nr2_enable);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].nr2_calc_y_k=%d\n", i, nr2_para->linkage_nr2[i].nr2_calc_y_k);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].nr2_k=%d\n", i, nr2_para->linkage_nr2[i].nr2_k);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].y_dpc_enable=%d\n", i, nr2_para->linkage_nr2[i].y_dpc_enable);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].y_dpc_th=%d\n", i, nr2_para->linkage_nr2[i].y_dpc_th);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].y_black_dpc_enable=%d\n", i, nr2_para->linkage_nr2[i].y_black_dpc_enable);
		ISP_PRINTF("nr2_para->linkage_nr2[%d].y_white_dpc_enable=%d\n", i, nr2_para->linkage_nr2[i].y_white_dpc_enable);
	  
		for (j=0; j<17; j++)
		{
			ISP_PRINTF("nr2_para->linkage_nr2[%d].nr2_weight_tbl[%d]=%d\n", i, j, nr2_para->linkage_nr2[i].nr2_weight_tbl[j]);
		}
	}
	
	return 0;
}

static int printk_rgb_gamma_para(AK_ISP_RGB_GAMMA_ATTR *gamma_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("gamma_para->rgb_gamma_enable=%d\n", gamma_para->rgb_gamma_enable);
	
	for (i=0; i<129; i++)
	{
		ISP_PRINTF("gamma_para->r_gamma[%d]=%d\n", i, gamma_para->r_gamma[i]);
	}
	
	for (i=0; i<129; i++)
	{
		ISP_PRINTF("gamma_para->g_gamma[%d]=%d\n", i, gamma_para->g_gamma[i]);
	}
	
	for (i=0; i<129; i++)
	{
		ISP_PRINTF("gamma_para->b_gamma[%d]=%d\n", i, gamma_para->b_gamma[i]);
	}
	
	return 0;
}

static int 	printk_fcs_para(AK_ISP_FCS_ATTR *fcs_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("fcs_para->fcs_mode=%d\n", fcs_para->fcs_mode);
	ISP_PRINTF("fcs_para->manual_fcs.fcs_enable=%d\n", fcs_para->manual_fcs.fcs_enable);
	ISP_PRINTF("fcs_para->manual_fcs.fcs_gain_slop=%d\n", fcs_para->manual_fcs.fcs_gain_slop);
	ISP_PRINTF("fcs_para->manual_fcs.fcs_th=%d\n", fcs_para->manual_fcs.fcs_th);
	ISP_PRINTF("fcs_para->manual_fcs.fcs_uv_nr_enable=%d\n", fcs_para->manual_fcs.fcs_uv_nr_enable);
	ISP_PRINTF("fcs_para->manual_fcs.fcs_uv_nr_th=%d\n", fcs_para->manual_fcs.fcs_uv_nr_th);
	
	for (i=0; i<9; i++)
	{
		ISP_PRINTF("fcs_para->linkage_fcs[%d].fcs_enable=%d\n", i, fcs_para->linkage_fcs[i].fcs_enable);
		ISP_PRINTF("fcs_para->linkage_fcs[%d].fcs_gain_slop=%d\n", i, fcs_para->linkage_fcs[i].fcs_gain_slop);
		ISP_PRINTF("fcs_para->linkage_fcs[%d].fcs_th=%d\n", i, fcs_para->linkage_fcs[i].fcs_th);
		ISP_PRINTF("fcs_para->linkage_fcs[%d].fcs_uv_nr_enable=%d\n", i, fcs_para->linkage_fcs[i].fcs_uv_nr_enable);
		ISP_PRINTF("fcs_para->linkage_fcs[%d].fcs_uv_nr_th=%d\n", i, fcs_para->linkage_fcs[i].fcs_uv_nr_th);
	}
	
	return 0;
}

//static int 	printk_fcs_para(AK_ISP_FCS_ATTR *fcs_para)
static int printk_contrast_para(AK_ISP_CONTRAST_ATTR *contrast)
{    
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("contrast->cc_mode=%d\n", contrast->cc_mode);
	ISP_PRINTF("contrast->manual_contrast.y_contrast=%d\n", contrast->manual_contrast.y_contrast);
	ISP_PRINTF("contrast->manual_contrast.y_shift=%d\n", contrast->manual_contrast.y_shift);

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("contrast->linkage_contrast[%d].dark_pixel_area=%d\n", i ,contrast->linkage_contrast[i].dark_pixel_area);
		ISP_PRINTF("contrast->linkage_contrast[%d].dark_pixel_rate=%d\n", i ,contrast->linkage_contrast[i].dark_pixel_rate);
		ISP_PRINTF("contrast->linkage_contrast[%d].shift_max=%d\n", i ,contrast->linkage_contrast[i].shift_max);
	}
	
	return 0;
}

static int printk_satu_para(AK_ISP_SATURATION_ATTR *satu_para)
{
    int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("satu_para->SE_mode=%d\n", satu_para->SE_mode);
	ISP_PRINTF("satu_para->manual_sat.SE_enable=%d\n", satu_para->manual_sat.SE_enable);
	ISP_PRINTF("satu_para->manual_sat.SE_th1=%d\n", satu_para->manual_sat.SE_th1);
	ISP_PRINTF("satu_para->manual_sat.SE_th2=%d\n", satu_para->manual_sat.SE_th2);
	ISP_PRINTF("satu_para->manual_sat.SE_th3=%d\n", satu_para->manual_sat.SE_th3);
	ISP_PRINTF("satu_para->manual_sat.SE_th4=%d\n", satu_para->manual_sat.SE_th4);
	ISP_PRINTF("satu_para->manual_sat.SE_scale_slop1=%d\n", satu_para->manual_sat.SE_scale_slop1);
	ISP_PRINTF("satu_para->manual_sat.SE_scale_slop2=%d\n", satu_para->manual_sat.SE_scale_slop2);
	ISP_PRINTF("satu_para->manual_sat.SE_scale1=%d\n", satu_para->manual_sat.SE_scale1);
	ISP_PRINTF("satu_para->manual_sat.SE_scale2=%d\n", satu_para->manual_sat.SE_scale2);
	ISP_PRINTF("satu_para->manual_sat.SE_scale3=%d\n", satu_para->manual_sat.SE_scale3);

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_enable=%d\n", i, satu_para->linkage_sat[i].SE_enable);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_th1=%d\n", i, satu_para->linkage_sat[i].SE_th1);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_th2=%d\n", i, satu_para->linkage_sat[i].SE_th2);	
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_th3=%d\n", i, satu_para->linkage_sat[i].SE_th3);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_th4=%d\n", i, satu_para->linkage_sat[i].SE_th4);	
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_scale_slop1=%d\n", i, satu_para->linkage_sat[i].SE_scale_slop1);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_scale_slop2=%d\n", i, satu_para->linkage_sat[i].SE_scale_slop2);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_scale1=%d\n", i, satu_para->linkage_sat[i].SE_scale1);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_scale2=%d\n", i, satu_para->linkage_sat[i].SE_scale2);
		ISP_PRINTF("satu_para->linkage_sat[%d].SE_scale3=%d\n", i, satu_para->linkage_sat[i].SE_scale3);
	}
	
	return 0;
}

static int printk_lsc(AK_ISP_LSC_ATTR *lsc_para)
{
	int i = 0;
   
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("lsc_para->enable=%d\n", lsc_para->enable);
	ISP_PRINTF("lsc_para->xref=%d\n", lsc_para->xref);
	ISP_PRINTF("lsc_para->yref=%d\n", lsc_para->yref);
	ISP_PRINTF("lsc_para->lsc_shift=%d\n", lsc_para->lsc_shift);
   
	for (i=0; i<10; i++)
	{
   		ISP_PRINTF("lsc_para->range[%d]=%d\n", i, lsc_para->range[i]);
	}
   
	for (i=0; i<10; i++)
	{
       ISP_PRINTF("lsc_para->lsc_r_coef.coef_b[%d]=%d\n", i, lsc_para->lsc_r_coef.coef_b[i]);
	   ISP_PRINTF("lsc_para->lsc_r_coef.coef_c[%d]=%d\n", i, lsc_para->lsc_r_coef.coef_c[i]);
	}
   
	for (i=0; i<10; i++)
	{
       ISP_PRINTF("lsc_para->lsc_gr_coef.coef_b[%d]=%d\n", i, lsc_para->lsc_gr_coef.coef_b[i]);
	   ISP_PRINTF("lsc_para->lsc_gr_coef.coef_c[%d]=%d\n", i, lsc_para->lsc_gr_coef.coef_c[i]);
	}

	for (i=0; i<10; i++)
	{
       ISP_PRINTF("lsc_para->lsc_gb_coef.coef_b[%d]=%d\n", i, lsc_para->lsc_gb_coef.coef_b[i]);
	   ISP_PRINTF("lsc_para->lsc_gb_coef.coef_c[%d]=%d\n", i, lsc_para->lsc_gb_coef.coef_c[i]);
	}

	for (i=0; i<10; i++)
	{
		ISP_PRINTF("lsc_para->lsc_b_coef.coef_b[%d]=%d\n", i, lsc_para->lsc_b_coef.coef_b[i]);
		ISP_PRINTF("lsc_para->lsc_b_coef.coef_c[%d]=%d\n", i, lsc_para->lsc_b_coef.coef_c[i]);
	}
	
	return 0;
}

static int printk_dpc(AK_ISP_DDPC_ATTR *dpc_para)
{   
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("dpc_para->ddpc_mode=%d\n", dpc_para->ddpc_mode);
	ISP_PRINTF("dpc_para->manual_ddpc.ddpc_enable=%d\n", dpc_para->manual_ddpc.ddpc_enable);
	ISP_PRINTF("dpc_para->manual_ddpc.ddpc_th=%d\n", dpc_para->manual_ddpc.ddpc_th);
	ISP_PRINTF("dpc_para->manual_ddpc.white_dpc_enable=%d\n", dpc_para->manual_ddpc.white_dpc_enable);
	ISP_PRINTF("dpc_para->manual_ddpc.black_dpc_enable=%d\n", dpc_para->manual_ddpc.black_dpc_enable);

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("dpc_para->linkage_ddpc[%d].ddpc_enable=%d\n", i, dpc_para->linkage_ddpc[i].ddpc_enable);
		ISP_PRINTF("dpc_para->linkage_ddpc[%d].ddpc_th=%d\n", i, dpc_para->linkage_ddpc[i].ddpc_th);
		ISP_PRINTF("dpc_para->linkage_ddpc[%d].white_dpc_enable=%d\n", i, dpc_para->linkage_ddpc[i].white_dpc_enable);
		ISP_PRINTF("dpc_para->linkage_ddpc[%d].black_dpc_enable=%d\n", i, dpc_para->linkage_ddpc[i].black_dpc_enable);
	}
	
	return 0;
}

static int printk_sharp_para(AK_ISP_SHARP_ATTR *sharp_para)
{
    int i = 0;
	int j = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("sharp_para.ysharp_mode=%d\n", sharp_para->ysharp_mode);
	ISP_PRINTF("sharp_para->manual_sharp_attr.ysharp_enable=%d\n", sharp_para->manual_sharp_attr.ysharp_enable);
	ISP_PRINTF("sharp_para->manual_sharp_attr.hf_hpf_k=%d\n", sharp_para->manual_sharp_attr.hf_hpf_k);
	ISP_PRINTF("sharp_para->manual_sharp_attr.hf_hpf_shift=%d\n", sharp_para->manual_sharp_attr.hf_hpf_shift);
	ISP_PRINTF("sharp_para->manual_sharp_attr.mf_hpf_k=%d\n", sharp_para->manual_sharp_attr.mf_hpf_k);
	ISP_PRINTF("sharp_para->manual_sharp_attr.mf_hpf_shift=%d\n", sharp_para->manual_sharp_attr.mf_hpf_shift);
	ISP_PRINTF("sharp_para->manual_sharp_attr.sharp_method=%d\n", sharp_para->manual_sharp_attr.sharp_method);
	ISP_PRINTF("sharp_para->manual_sharp_attr.sharp_skin_detect_enable=%d\n", sharp_para->manual_sharp_attr.sharp_skin_detect_enable);
	ISP_PRINTF("sharp_para->manual_sharp_attr.sharp_skin_gain_th=%d\n", sharp_para->manual_sharp_attr.sharp_skin_gain_th);
	ISP_PRINTF("sharp_para->manual_sharp_attr.sharp_skin_gain_weaken%d\n", sharp_para->manual_sharp_attr.sharp_skin_gain_weaken);
	
    for (i=0; i<256; i++)
    {
    	ISP_PRINTF("sharp_para->manual_sharp_attr.MF_HPF_LUT[%d]=%d\n", i, sharp_para->manual_sharp_attr.MF_HPF_LUT[i]);
    }
	
	for (i=0; i<256; i++)
    {
    	ISP_PRINTF("sharp_para->manual_sharp_attr.HF_HPF_LUT[%d]=%d\n", i, sharp_para->manual_sharp_attr.HF_HPF_LUT[i]);
    }
	
	for (j=0; j<9; j++)
	{
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].ysharp_enable=%d\n", j, sharp_para->linkage_sharp_attr[j].ysharp_enable);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].hf_hpf_k=%d\n", j, sharp_para->linkage_sharp_attr[j].hf_hpf_k);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].hf_hpf_shift=%d\n", j, sharp_para->linkage_sharp_attr[j].hf_hpf_shift);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].mf_hpf_k=%d\n", j, sharp_para->linkage_sharp_attr[j].mf_hpf_k);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].mf_hpf_shift=%d\n", j, sharp_para->linkage_sharp_attr[j].mf_hpf_shift);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].sharp_method=%d\n", j, sharp_para->linkage_sharp_attr[j].sharp_method);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].sharp_skin_detect_enable=%d\n", j, sharp_para->linkage_sharp_attr[j].sharp_skin_detect_enable);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].sharp_skin_gain_th=%d\n", j, sharp_para->linkage_sharp_attr[j].sharp_skin_gain_th);
		ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].sharp_skin_gain_weaken=%d\n", j, sharp_para->linkage_sharp_attr[j].sharp_skin_gain_weaken);
		
        for (i=0; i<256; i++)
        {
           ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].MF_HPF_LUT[%d]=%d\n", j, i, sharp_para->linkage_sharp_attr[j].MF_HPF_LUT[i]);
        }

		for (i=0; i<256; i++)
        {
           ISP_PRINTF("sharp_para->linkage_sharp_attr[%d].HF_HPF_LUT[%d]=%d\n", j, i, sharp_para->linkage_sharp_attr[j].HF_HPF_LUT[i]);
        }
	}
	
	return 0;	
}

static int printk_sharp_ex_para(AK_ISP_SHARP_EX_ATTR *sharp_ex_para)
{  
	int i = 0;
   
	ISP_PRINTF("\n\n\n");
   
	for (i=0; i<3; i++)
	{
		ISP_PRINTF("sharp_ex_para->hf_HPF[%d]=%d\n", i, sharp_ex_para->hf_HPF[i]);
   	}
	
	for (i=0; i<6; i++)
	{
		ISP_PRINTF("sharp_ex_para->mf_HPF[%d]=%d\n", i, sharp_ex_para->mf_HPF[i]);
	}
  
	ISP_PRINTF("sharp_ex_para->sharp_skin_max_th=%d\n", sharp_ex_para->sharp_skin_max_th);
	ISP_PRINTF("sharp_ex_para->sharp_skin_min_th=%d\n", sharp_ex_para->sharp_skin_min_th);
	ISP_PRINTF("sharp_ex_para->sharp_skin_y_max_th=%d\n", sharp_ex_para->sharp_skin_y_max_th);
	ISP_PRINTF("sharp_ex_para->sharp_skin_y_min_th=%d\n", sharp_ex_para->sharp_skin_y_min_th);
	ISP_PRINTF("sharp_ex_para->sharp_skin_v_max_th=%d\n", sharp_ex_para->sharp_skin_v_max_th);
	ISP_PRINTF("sharp_ex_para->sharp_skin_v_min_th=%d\n", sharp_ex_para->sharp_skin_v_min_th);
	
	return 0;
} 

static int printk_nr_3d_para(AK_ISP_3D_NR_ATTR *nr_3d_para)
{
    int i = 0, j = 0;
	
	ISP_PRINTF("\n\n\n");
	//printk("size =%d\n",sizeof(AK_ISP_3D_NR_ATTR));

	ISP_PRINTF("nr_3d_para->_3d_nr_mode=%d\n", nr_3d_para->_3d_nr_mode);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.uv_min_enable=%d\n", nr_3d_para->manual_3d_nr.uv_min_enable);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.tnr_y_enable=%d\n", nr_3d_para->manual_3d_nr.tnr_y_enable);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.tnr_uv_enable=%d\n", nr_3d_para->manual_3d_nr.tnr_uv_enable);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.updata_ref_y=%d\n", nr_3d_para->manual_3d_nr.updata_ref_y);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.updata_ref_uv=%d\n", nr_3d_para->manual_3d_nr.updata_ref_uv);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.tnr_refFrame_format=%d\n", nr_3d_para->manual_3d_nr.tnr_refFrame_format);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.y_2dnr_enable=%d\n", nr_3d_para->manual_3d_nr.y_2dnr_enable);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.uv_2dnr_enable=%d\n", nr_3d_para->manual_3d_nr.uv_2dnr_enable);
	
	ISP_PRINTF("nr_3d_para->manual_3d_nr.uvnr_k=%d\n", nr_3d_para->manual_3d_nr.uvnr_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.uvlp_k=%d\n", nr_3d_para->manual_3d_nr.uvlp_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_k=%d\n", nr_3d_para->manual_3d_nr.t_uv_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_minstep=%d\n", nr_3d_para->manual_3d_nr.t_uv_minstep);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_mf_th1=%d\n", nr_3d_para->manual_3d_nr.t_uv_mf_th1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_mf_th2=%d\n", nr_3d_para->manual_3d_nr.t_uv_mf_th2);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_diffth_k1=%d\n", nr_3d_para->manual_3d_nr.t_uv_diffth_k1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_diffth_k2=%d\n", nr_3d_para->manual_3d_nr.t_uv_diffth_k2);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_diffth_slop=%d\n", nr_3d_para->manual_3d_nr.t_uv_diffth_slop);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_mc_k=%d\n", nr_3d_para->manual_3d_nr.t_uv_mc_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_uv_ac_th=%d\n", nr_3d_para->manual_3d_nr.t_uv_ac_th);
	
	ISP_PRINTF("nr_3d_para->manual_3d_nr.ynr_calc_k=%d\n", nr_3d_para->manual_3d_nr.ynr_calc_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.ynr_k=%d\n", nr_3d_para->manual_3d_nr.ynr_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.ynr_diff_shift=%d\n", nr_3d_para->manual_3d_nr.ynr_diff_shift);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.ylp_k=%d\n", nr_3d_para->manual_3d_nr.ylp_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_th1=%d\n", nr_3d_para->manual_3d_nr.t_y_th1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_k1=%d\n", nr_3d_para->manual_3d_nr.t_y_k1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_k2=%d\n", nr_3d_para->manual_3d_nr.t_y_k2);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_kslop=%d\n", nr_3d_para->manual_3d_nr.t_y_kslop);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_minstep=%d\n", nr_3d_para->manual_3d_nr.t_y_minstep);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_mf_th1=%d\n", nr_3d_para->manual_3d_nr.t_y_mf_th1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_mf_th2=%d\n", nr_3d_para->manual_3d_nr.t_y_mf_th2);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_diffth_k1=%d\n", nr_3d_para->manual_3d_nr.t_y_diffth_k1);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_diffth_k2=%d\n", nr_3d_para->manual_3d_nr.t_y_diffth_k2);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_diffth_slop=%d\n", nr_3d_para->manual_3d_nr.t_y_diffth_slop);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_mc_k=%d\n", nr_3d_para->manual_3d_nr.t_y_mc_k);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.t_y_ac_th=%d\n", nr_3d_para->manual_3d_nr.t_y_ac_th);
	ISP_PRINTF("nr_3d_para->manual_3d_nr.md_th=%d\n", nr_3d_para->manual_3d_nr.md_th);
	
	for (j=0; j<17; j++)
	{
		ISP_PRINTF("nr_3d_para->manual_3d_nr.ynr_weight_tbl[%d]=%d\n", j, nr_3d_para->manual_3d_nr.ynr_weight_tbl[j]);
	}
	

	for (i=0; i<9; i++)
	{
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].uv_min_enable=%d\n", i, nr_3d_para->linkage_3d_nr[i].uv_min_enable);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].tnr_y_enable=%d\n", i, nr_3d_para->linkage_3d_nr[i].tnr_y_enable);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].tnr_uv_enable=%d\n", i, nr_3d_para->linkage_3d_nr[i].tnr_uv_enable);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].updata_ref_y=%d\n", i, nr_3d_para->linkage_3d_nr[i].updata_ref_y);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].updata_ref_uv=%d\n", i, nr_3d_para->linkage_3d_nr[i].updata_ref_uv);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].tnr_refFrame_format=%d\n", i, nr_3d_para->linkage_3d_nr[i].tnr_refFrame_format);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].y_2dnr_enable=%d\n", i, nr_3d_para->linkage_3d_nr[i].y_2dnr_enable);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].uv_2dnr_enable=%d\n", i, nr_3d_para->linkage_3d_nr[i].uv_2dnr_enable);
		
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].uvnr_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].uvnr_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].uvlp_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].uvlp_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_minstep=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_minstep);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_mf_th1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_mf_th1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_mf_th2=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_mf_th2);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_diffth_k1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_diffth_k1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_diffth_k2=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_diffth_k2);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_diffth_slop=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_diffth_slop);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_mc_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_mc_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_uv_ac_th=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_uv_ac_th);
		
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].ynr_calc_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].ynr_calc_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].ynr_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].ynr_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].ynr_diff_shift=%d\n", i, nr_3d_para->linkage_3d_nr[i].ynr_diff_shift);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].ylp_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].ylp_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_th1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_th1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_k1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_k1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_k2=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_k2);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_kslop=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_kslop);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_minstep=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_minstep);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_mf_th1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_mf_th1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_mf_th2=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_mf_th2);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_diffth_k1=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_diffth_k1);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_diffth_k2=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_diffth_k2);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_diffth_slop=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_diffth_slop);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_mc_k=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_mc_k);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].t_y_ac_th=%d\n", i, nr_3d_para->linkage_3d_nr[i].t_y_ac_th);
		ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].md_th=%d\n", i, nr_3d_para->linkage_3d_nr[i].md_th);
		
		for (j=0; j<17; j++)
		{
			ISP_PRINTF("nr_3d_para->linkage_3d_nr[%d].ynr_weight_tbl[%d]=%d\n", i, j, nr_3d_para->linkage_3d_nr[i].ynr_weight_tbl[j]);
		}
	}
	
	return 0;	
}

static int printk_rgb2yuv_para(AK_ISP_RGB2YUV_ATTR *rgb2yuv_para)
{
	ISP_PRINTF("rgb2yuv_para->mode=%d\n", rgb2yuv_para->mode);
	return 0;
}

static int printk_effect_para(AK_ISP_EFFECT_ATTR *effect_para)
{
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("effect_para->dark_margin_en=%d\n", effect_para->dark_margin_en);
	ISP_PRINTF("effect_para->y_a=%d\n", effect_para->y_a);
	ISP_PRINTF("effect_para->y_b=%d\n", effect_para->y_b);
	ISP_PRINTF("effect_para->uv_a=%d\n", effect_para->uv_a);
	ISP_PRINTF("effect_para->uv_b=%d\n", effect_para->uv_b);
	
	return 0;
}

static int printk_frame_rate_para(AK_ISP_FRAME_RATE_ATTR *frame_rate_para)
{   
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("frame_rate_para->hight_light_frame_rate=%d\n", frame_rate_para->hight_light_frame_rate);
	ISP_PRINTF("frame_rate_para->hight_light_max_exp_time=%d\n", frame_rate_para->hight_light_max_exp_time);
	ISP_PRINTF("frame_rate_para->hight_light_to_low_light_gain=%d\n", frame_rate_para->hight_light_to_low_light_gain);
	ISP_PRINTF("frame_rate_para->low_light_frame_rate=%d\n", frame_rate_para->low_light_frame_rate);
	ISP_PRINTF("frame_rate_para->low_light_max_exp_time=%d\n", frame_rate_para->low_light_max_exp_time);
	ISP_PRINTF("frame_rate_para->low_light_to_hight_light_gain=%d\n", frame_rate_para->low_light_to_hight_light_gain);
	
	return 0;
}

static int printk_ae_para(AK_ISP_AE_ATTR *ae_para)
{   
	int i = 0, j = 0;
	
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("ae_para->exp_time_max=%d\n", (int)ae_para->exp_time_max);
	ISP_PRINTF("ae_para->exp_time_min=%d\n", (int)ae_para->exp_time_min);
	ISP_PRINTF("ae_para->d_gain_max=%d\n", (int)ae_para->d_gain_max);
	ISP_PRINTF("ae_para->d_gain_min=%d\n", (int)ae_para->d_gain_min);
	ISP_PRINTF("ae_para->isp_d_gain_max=%d\n", (int)ae_para->isp_d_gain_max);
	ISP_PRINTF("ae_para->isp_d_gain_min=%d\n", (int)ae_para->isp_d_gain_min);
	ISP_PRINTF("ae_para->a_gain_max=%d\n", (int)ae_para->a_gain_max);
	ISP_PRINTF("ae_para->a_gain_min=%d\n", (int)ae_para->a_gain_min);
	ISP_PRINTF("ae_para->exp_step=%d\n", (int)ae_para->exp_step);
	ISP_PRINTF("ae_para->exp_stable_range=%d\n", (int)ae_para->exp_stable_range);	
	ISP_PRINTF("ae_para->target_lumiance=%d\n", (int)ae_para->target_lumiance);
	ISP_PRINTF("ae_para->OE_suppress_en=%d\n", (int)ae_para->OE_suppress_en);
	ISP_PRINTF("ae_para->OE_detect_scope=%d\n", (int)ae_para->OE_detect_scope);
	ISP_PRINTF("ae_para->OE_rate_max=%d\n", (int)ae_para->OE_rate_max);
	ISP_PRINTF("ae_para->OE_rate_min=%d\n", (int)ae_para->OE_rate_min);

	for (i=0; i<10; i++)
	{
		for (j=0; j<2; j++)
		{
			ISP_PRINTF("ae_para->envi_gain_range[%d][%d]=%d\n", i, j, (int)ae_para->envi_gain_range[i][j]);
		}
	}

	for (i=0; i<16; i++)
	{
		ISP_PRINTF("ae_para->hist_weight[%d]=%d\n", i, (int)ae_para->hist_weight[i]);
	}
	
	return 0;
}

static int printk_exp_type_para(AK_ISP_EXP_TYPE *exp_type_para)
{
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("exp_type_para->exp_type=%d\n", exp_type_para->exp_type);
	return 0;
}

static int printk_awb_para(AK_ISP_AWB_ATTR *awb_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("awb_para->auto_wb_step=%d\n", awb_para->auto_wb_step);
	ISP_PRINTF("awb_para->colortemp_stable_cnt_thresh=%d\n", awb_para->colortemp_stable_cnt_thresh);
	ISP_PRINTF("awb_para->total_cnt_thresh=%d\n", awb_para->total_cnt_thresh);
	ISP_PRINTF("awb_para->y_high=%d\n", awb_para->y_high);
	ISP_PRINTF("awb_para->y_low=%d\n", awb_para->y_low);
	ISP_PRINTF("awb_para->err_est=%d\n", awb_para->err_est);
	
	for(i=0; i<10; i++)
	{
		ISP_PRINTF("awb_para->gr_low[%d]=%d\n", i, awb_para->gr_low[i]);
		ISP_PRINTF("awb_para->gr_high[%d]=%d\n", i, awb_para->gr_high[i]);
		ISP_PRINTF("awb_para->gb_low[%d]=%d\n", i, awb_para->gb_low[i]);
		ISP_PRINTF("awb_para->gb_high[%d]=%d\n", i, awb_para->gb_high[i]);
		ISP_PRINTF("awb_para->rb_low[%d]=%d\n", i, awb_para->rb_low[i]);
		ISP_PRINTF("awb_para->rb_high[%d]=%d\n", i, awb_para->rb_high[i]);
	}
	
	for(i=0; i<16; i++)
	{
		ISP_PRINTF("awb_para->g_weight[%d]=%d\n", i, awb_para->g_weight[i]);
	}

	for(i=0; i<10; i++)
	{
		ISP_PRINTF("awb_para->colortemp_envi[%d]=%d\n", i, awb_para->colortemp_envi[i]);
	}
	
	return 0;
}

static int printk_awb_ex_para(AK_ISP_AWB_EX_ATTR *awb_ex_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("awb_ex_para->awb_ex_ctrl_enable=%d\n", awb_ex_para->awb_ex_ctrl_enable);
	
	for(i=0; i<10; i++)
	{
	  	ISP_PRINTF("awb_ex_para->awb_ctrl[%d].rgain_max=%d\n", i, awb_ex_para->awb_ctrl[i].rgain_max);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].rgain_min=%d\n", i, awb_ex_para->awb_ctrl[i].rgain_min);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].ggain_max=%d\n", i, awb_ex_para->awb_ctrl[i].ggain_max);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].ggain_min=%d\n", i, awb_ex_para->awb_ctrl[i].ggain_min);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].bgain_max=%d\n", i, awb_ex_para->awb_ctrl[i].bgain_max);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].bgain_min=%d\n", i, awb_ex_para->awb_ctrl[i].bgain_min);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].rgain_ex=%d\n", i, awb_ex_para->awb_ctrl[i].rgain_ex);
		ISP_PRINTF("awb_ex_para->awb_ctrl[%d].bgain_ex=%d\n", i, awb_ex_para->awb_ctrl[i].bgain_ex);
	}

	return 0;
}

static int printk_wb_type_para(AK_ISP_WB_TYPE_ATTR *wb_type_para)
{
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("wb_type_para->wb_type=%d\n",wb_type_para->wb_type);
	return 0;
}

static int printk_af_para(AK_ISP_AF_ATTR *af_para)
{   
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("af_para->af_th=%d\n", af_para->af_th);
	ISP_PRINTF("af_para->af_win0_left=%d\n", af_para->af_win0_left);
	ISP_PRINTF("af_para->af_win0_right=%d\n", af_para->af_win0_right);
	ISP_PRINTF("af_para->af_win0_top=%d\n", af_para->af_win0_top);
	ISP_PRINTF("af_para->af_win0_bottom=%d\n", af_para->af_win0_bottom);

	ISP_PRINTF("af_para->af_win1_left=%d\n", af_para->af_win1_left);
	ISP_PRINTF("af_para->af_win1_right=%d\n", af_para->af_win1_right);
	ISP_PRINTF("af_para->af_win1_top=%d\n", af_para->af_win1_top);
	ISP_PRINTF("af_para->af_win1_bottom=%d\n", af_para->af_win1_bottom);

	ISP_PRINTF("af_para->af_win2_left=%d\n", af_para->af_win2_left);
	ISP_PRINTF("af_para->af_win2_right=%d\n", af_para->af_win2_right);
	ISP_PRINTF("af_para->af_win2_top=%d\n", af_para->af_win2_top);
	ISP_PRINTF("af_para->af_win2_bottom=%d\n", af_para->af_win2_bottom);

	ISP_PRINTF("af_para->af_win3_left=%d\n", af_para->af_win3_left);
	ISP_PRINTF("af_para->af_win3_right=%d\n", af_para->af_win3_right);
	ISP_PRINTF("af_para->af_win3_top=%d\n", af_para->af_win3_top);
	ISP_PRINTF("af_para->af_win3_bottom=%d\n", af_para->af_win3_bottom);

	ISP_PRINTF("af_para->af_win4_left=%d\n", af_para->af_win4_left);
	ISP_PRINTF("af_para->af_win4_right=%d\n", af_para->af_win4_right);
	ISP_PRINTF("af_para->af_win4_top=%d\n", af_para->af_win4_top);
	ISP_PRINTF("af_para->af_win4_bottom=%d\n", af_para->af_win4_bottom);
	
    return 0;
}

static int printk_weight_para(AK_ISP_WEIGHT_ATTR *weight_para)
{
	int i = 0;
	int j = 0;
	
	ISP_PRINTF("\n\n\n");
	
	for (i=0; i<8; i++)
	{
		for (j=0; j<16; j++)
		{
			ISP_PRINTF("weight_para->zone_weight[%d][%d]=%d\n", i, j, weight_para->zone_weight[i][j]);
		}
	}
	
	return 0;	
}

static int printk_mwb_para(AK_ISP_MWB_ATTR *mwb_para)
{
	 ISP_PRINTF("\n\n\n");
	 ISP_PRINTF("mwb_para->r_gain=%d\n", mwb_para->r_gain);
	 ISP_PRINTF("mwb_para->g_gain=%d\n", mwb_para->g_gain);
	 ISP_PRINTF("mwb_para->b_gain=%d\n", mwb_para->b_gain);
	 ISP_PRINTF("mwb_para->r_offset=%d\n", mwb_para->r_offset);
	 ISP_PRINTF("mwb_para->g_offset=%d\n", mwb_para->g_offset);
	 ISP_PRINTF("mwb_para->b_offset=%d\n", mwb_para->b_offset);
	 
	 return 0;
}

static int printk_main_chan_mask_area_para(AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *mask_area_para)
{
    int i = 0;
	
	ISP_PRINTF("\n\n\n");
	
    for (i=0; i<4; i++)
    {
		ISP_PRINTF("main_chan_mask_area_para->mask_area[%d].enable=%d\n", i, mask_area_para->mask_area[i].enable);
		ISP_PRINTF("main_chan_mask_area_para->mask_area[%d].start_xpos=%d\n", i, mask_area_para->mask_area[i].start_xpos);
		ISP_PRINTF("main_chan_mask_area_para->mask_area[%d].start_ypos=%d\n", i, mask_area_para->mask_area[i].start_ypos);
		ISP_PRINTF("main_chan_mask_area_para->mask_area[%d].end_xpos=%d\n", i, mask_area_para->mask_area[i].end_xpos);
		ISP_PRINTF("main_chan_mask_area_para->mask_area[%d].end_ypos=%d\n", i, mask_area_para->mask_area[i].end_ypos);
    }
	
	return 0;
}

static int printk_sub_chan_mask_area_para(AK_ISP_SUB_CHAN_MASK_AREA_ATTR *mask_area_para)
{
    int i = 0;
	
	ISP_PRINTF("\n\n\n");
	
    for (i=0; i<4; i++)
    {
		ISP_PRINTF("sub_chan_mask_area_para->mask_area[%d].enable=%d\n", i, mask_area_para->mask_area[i].enable);
		ISP_PRINTF("sub_chan_mask_area_para->mask_area[%d].start_xpos=%d\n", i, mask_area_para->mask_area[i].start_xpos);
		ISP_PRINTF("sub_chan_mask_area_para->mask_area[%d].start_ypos=%d\n", i, mask_area_para->mask_area[i].start_ypos);
		ISP_PRINTF("sub_chan_mask_area_para->mask_area[%d].end_xpos=%d\n", i, mask_area_para->mask_area[i].end_xpos);
		ISP_PRINTF("sub_chan_mask_area_para->mask_area[%d].end_ypos=%d\n", i, mask_area_para->mask_area[i].end_ypos);
    }
	
	return 0;
}

static int printk_mask_color_para(AK_ISP_MASK_COLOR_ATTR *mask_color_para)
{   
    ISP_PRINTF("\n\n\n");
    ISP_PRINTF("mask_color_para->color_type=%d\n", mask_color_para->color_type);
	ISP_PRINTF("mask_color_para->mk_alpha=%d\n", mask_color_para->mk_alpha);
	ISP_PRINTF("mask_color_para->y_mk_color=%d\n", mask_color_para->y_mk_color);
	ISP_PRINTF("mask_color_para->u_mk_color=%d\n", mask_color_para->u_mk_color);
	ISP_PRINTF("mask_color_para->v_mk_color=%d\n", mask_color_para->v_mk_color);
	
	return 0;
}

static int printk_af_stat_para(AK_ISP_AF_STAT_INFO *af_stat_para)
{
	int i = 0;
	
	ISP_PRINTF("\n\n\n");

	for (i=0; i<5; i++)
	{
		ISP_PRINTF("af_stat_para->af_statics[%d]=%d\n", i, (int)af_stat_para->af_statics[i]);
	}
	
   return 0;
}

static int printk_awb_stat_info_para(AK_ISP_AWB_STAT_INFO *awb_stat_info)
{
   int i = 0;
   
   ISP_PRINTF("\n\n\n");
   ISP_PRINTF("awb_stat_info->r_gain=%d\n", awb_stat_info->r_gain);
   ISP_PRINTF("awb_stat_info->r_offset=%d\n", awb_stat_info->r_offset);
   ISP_PRINTF("awb_stat_info->g_gain=%d\n", awb_stat_info->g_gain);
   ISP_PRINTF("awb_stat_info->g_offset=%d\n", awb_stat_info->g_offset);
   ISP_PRINTF("awb_stat_info->b_gain=%d\n", awb_stat_info->b_gain);
   ISP_PRINTF("awb_stat_info->b_offset=%d\n", awb_stat_info->b_offset);

   ISP_PRINTF("awb_stat_info->current_colortemp_index=%d\n", awb_stat_info->current_colortemp_index);
  
   for(i=0; i<10; i++)
   {
       ISP_PRINTF("awb_stat_info->total_R[%d]=%d\n", i, (int)awb_stat_info->total_R[i]);
	   ISP_PRINTF("awb_stat_info->total_G[%d]=%d\n", i, (int)awb_stat_info->total_G[i]);
	   ISP_PRINTF("awb_stat_info->total_B[%d]=%d\n", i, (int)awb_stat_info->total_B[i]);
	   ISP_PRINTF("awb_stat_info->total_cnt[%d]=%d\n", i, (int)awb_stat_info->total_cnt[i]);
	   ISP_PRINTF("awb_stat_info->colortemp_stable_cnt[%d]=%d\n", i,(int)awb_stat_info->colortemp_stable_cnt[i]);
   }
   
   return 0;
}

static int printk_ae_run_para(AK_ISP_AE_RUN_INFO *ae_run_para)
{
   ISP_PRINTF("\n\n\n");
   ISP_PRINTF("ae_run_para->current_a_gain=%d\n", (int)ae_run_para->current_a_gain);
   ISP_PRINTF("ae_run_para->current_a_gain_step=%d\n", (int)ae_run_para->current_a_gain_step);
   ISP_PRINTF("ae_run_para->current_exp_time=%d\n", (int)ae_run_para->current_exp_time);
   ISP_PRINTF("ae_run_para->current_exp_time_step=%d\n", (int)ae_run_para->current_exp_time_step);
   ISP_PRINTF("ae_run_para->current_d_gain=%d\n", (int)ae_run_para->current_d_gain);
   ISP_PRINTF("ae_run_para->current_d_gain_step=%d\n", (int)ae_run_para->current_d_gain_step);
   ISP_PRINTF("ae_run_para->current_isp_d_gain=%d\n", (int)ae_run_para->current_isp_d_gain);
   ISP_PRINTF("ae_run_para->current_isp_d_gain_step=%d\n", (int)ae_run_para->current_isp_d_gain_step);
   ISP_PRINTF("ae_run_para->current_calc_avg_lumi=%d\n", ae_run_para->current_calc_avg_lumi);
   ISP_PRINTF("ae_run_para->current_calc_avg_compensation_lumi=%d\n", ae_run_para->current_calc_avg_compensation_lumi);
   ISP_PRINTF("ae_run_para->current_darked_flag=%d\n ", ae_run_para->current_darked_flag);
   
   return 0;
}

static int printk_raw_hist_stat_info(AK_ISP_RAW_HIST_STAT_INFO *raw_hist_stat_info)
{  
     int i = 0;
	 
	 ISP_PRINTF("\n\n\n");
	 ISP_PRINTF("raw_g_total=%d\n", (int)raw_hist_stat_info->raw_g_total);
	 
	 for (i=0; i<256; i++)
	 {
	 	ISP_PRINTF("raw_g_hist[%d] = %d\n", i, (int)raw_hist_stat_info->raw_g_hist[i]);
	 }
	 
	 return 0;
}

static int printk_rgb_hist_stat_info(AK_ISP_RGB_HIST_STAT_INFO *rgb_hist_stat_info)
{  
     int i = 0;
	 
	 ISP_PRINTF("\n\n\n");
	 ISP_PRINTF("rgb_total=%d\n", (int)rgb_hist_stat_info->rgb_total);
	 
	 for (i=0; i<256; i++)
	 {
	 	ISP_PRINTF("rgb_hist[%d] = %d\n", i, (int)rgb_hist_stat_info->rgb_hist[i]);
	 }
	 
	 return 0;
}

static int printk_yuv_hist_stat_info(AK_ISP_YUV_HIST_STAT_INFO *yuv_hist_stat_info)
{  
     int i = 0;
	 
	 ISP_PRINTF("\n\n\n");
	 ISP_PRINTF("y_total=%d\n", (int)yuv_hist_stat_info->y_total);
	 
	 for (i=0; i<256; i++)
	 {
	 	ISP_PRINTF("y_hist[%d] = %d\n", i, (int)yuv_hist_stat_info->y_hist[i]);
	 }
	 
	 return 0;
}

static int printk_raw_hist_para(AK_ISP_RAW_HIST_ATTR *raw_hist_para)
{
    ISP_PRINTF("\n\n\n");
    ISP_PRINTF("raw_hist_para->enable=%d\n", raw_hist_para->enable);
	return 0;
}

static int printk_rgb_hist_para(AK_ISP_RGB_HIST_ATTR *rgb_hist_para)
{
    ISP_PRINTF("\n\n\n");
    ISP_PRINTF("rgb_hist_para->enable=%d\n", rgb_hist_para->enable);
	return 0;
}

static int printk_yuv_hist_para(AK_ISP_YUV_HIST_ATTR *yuv_hist_para)
{
    ISP_PRINTF("\n\n\n");
    ISP_PRINTF("yuv_hist_para->enable=%d\n", yuv_hist_para->enable);
	return 0;
}

static int printk_3d_nr_ref(AK_ISP_3D_NR_REF_ATTR *ref_para)
{
    ISP_PRINTF("\n\n\n");
	ISP_PRINTF("ref_para->yaddr_3d=%d\n", (int)ref_para->yaddr_3d);
	ISP_PRINTF("ref_para->ysize_3d=%d\n", (int)ref_para->ysize_3d);
	ISP_PRINTF("ref_para->uaddr_3d=%d\n", (int)ref_para->uaddr_3d);
	ISP_PRINTF("ref_para->usize_3d=%d\n", (int)ref_para->usize_3d);
	ISP_PRINTF("ref_para->vaddr_3d=%d\n", (int)ref_para->vaddr_3d);
	ISP_PRINTF("ref_para->vsize_3d=%d\n", (int)ref_para->vsize_3d);
	
	return 0;
}

static int printk_3d_nr_stat_info(AK_ISP_3D_NR_STAT_INFO *nr_3d_stat_info)
{
	int i = 0, j = 0;
  
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("nr_3d_stat_info->MD_stat_max=%d\n", nr_3d_stat_info->MD_stat_max);
	ISP_PRINTF("nr_3d_stat_info->MD_level=%d\n", nr_3d_stat_info->MD_level);
  
	for(i=0; i<16; i++)
	{
		for(j=0; j<32; j++)
		{
			ISP_PRINTF("nr_3d_stat_info->MD_stat[%d][%d]=%d\n", i, j, nr_3d_stat_info->MD_stat[i][j]);
		}
	}
	
	return 0;
}


static int printk_misc_para(AK_ISP_MISC_ATTR *misc_para)
{
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("misc_para->hsyn_pol=%d\n", misc_para->hsyn_pol);
	ISP_PRINTF("misc_para->vsync_pol=%d\n", misc_para->vsync_pol);
	ISP_PRINTF("misc_para->pclk_pol=%d\n", misc_para->pclk_pol);
	ISP_PRINTF("misc_para->test_pattern_en=%d\n", misc_para->test_pattern_en);
	ISP_PRINTF("misc_para->test_pattern_cfg=%d\n", misc_para->test_pattern_cfg);
	ISP_PRINTF("misc_para->cfa_mode=%d\n", misc_para->cfa_mode);
	ISP_PRINTF("misc_para->inputdataw=%d\n", misc_para->inputdataw);
	ISP_PRINTF("misc_para->one_line_cycle=%d\n", misc_para->one_line_cycle);
	ISP_PRINTF("misc_para->hblank_cycle=%d\n", misc_para->hblank_cycle);
	ISP_PRINTF("misc_para->frame_start_delay_en=%d\n", misc_para->frame_start_delay_en);
	ISP_PRINTF("misc_para->frame_start_delay_num=%d\n", misc_para->frame_start_delay_num);
	ISP_PRINTF("misc_para->flip_en=%d\n", misc_para->flip_en);
	ISP_PRINTF("misc_para->mirror_en=%d\n", misc_para->mirror_en);

	return 0;
}


static int printk_y_gamma_para(AK_ISP_Y_GAMMA_ATTR *y_gamma_para)
{
	int i = 0; 
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("y_gamma_para->ygamma_uv_adjust_enable=%d\n", y_gamma_para->ygamma_uv_adjust_enable);
	ISP_PRINTF("y_gamma_para->ygamma_uv_adjust_level=%d\n", y_gamma_para->ygamma_uv_adjust_level);
	ISP_PRINTF("y_gamma_para->ygamma_cnoise_yth1=%d\n", y_gamma_para->ygamma_cnoise_yth1);
	ISP_PRINTF("y_gamma_para->ygamma_cnoise_yth2=%d\n", y_gamma_para->ygamma_cnoise_yth2);
	ISP_PRINTF("y_gamma_para->ygamma_cnoise_slop=%d\n", y_gamma_para->ygamma_cnoise_slop);
	ISP_PRINTF("y_gamma_para->ygamma_cnoise_gain=%d\n", y_gamma_para->ygamma_cnoise_gain);

	for (i=0; i<129; i++)
	{
		ISP_PRINTF("y_gamma_para->ygamma[%d]=%d\n", i, y_gamma_para->ygamma[i]);
	}

	return 0;
}


static int printk_hue_para(AK_ISP_HUE_ATTR *hue_para)
{
	int i = 0, j = 0;
	
	ISP_PRINTF("\n\n\n");
	ISP_PRINTF("hue_para->hue_mode=%d\n", hue_para->hue_mode);
	ISP_PRINTF("hue_para->manual_hue.hue_sat_en=%d\n", hue_para->manual_hue.hue_sat_en);

	for(i=0; i<65; i++)
	{
		ISP_PRINTF("hue_para->manual_hue.hue_lut_a[%d]=%d\n", i, hue_para->manual_hue.hue_lut_a[i]);
	}

	for(i=0; i<65; i++)
	{
		ISP_PRINTF("hue_para->manual_hue.hue_lut_b[%d]=%d\n", i, hue_para->manual_hue.hue_lut_b[i]);
	}

	for(i=0; i<65; i++)
	{
		ISP_PRINTF("hue_para->manual_hue.hue_lut_s[%d]=%d\n", i, hue_para->manual_hue.hue_lut_s[i]);
	}

	for(j=0; j<4; j++)
	{
		ISP_PRINTF("hue_para->hue[%d].hue_sat_en=%d\n", j, hue_para->hue[j].hue_sat_en);
		
		for(i=0; i<65; i++)
		{
			ISP_PRINTF("hue_para->hue[%d].hue_lut_a[%d]=%d\n", j, i, hue_para->hue[j].hue_lut_a[i]);
		}

		for(i=0; i<65; i++)
		{
			ISP_PRINTF("hue_para->hue[%d].hue_lut_b[%d]=%d\n", j, i, hue_para->hue[j].hue_lut_b[i]);
		}

		for(i=0; i<65; i++)
		{
			ISP_PRINTF("hue_para->hue[%d].hue_lut_s[%d]=%d\n", j, i, hue_para->hue[j].hue_lut_s[i]);
		}
	}
		
	return 0;
}

/**
 * @brief set isp user param for osd
 * @author yang_mengxia   
 * @date 2017-04-17
 * @param param[IN] user params
 * @return bool
 * @retval true if  successfully
 * @retval false if  unsuccessfully
 */
static int ak_isp_set_user_params_do(AK_ISP_USER_PARAM *param)
{
	int ret = 0;

	switch (param->id) 
	{
	case AK_ISP_USER_CID_SET_ZOOM:
		break;

	case AK_ISP_USER_CID_SET_SUB_CHANNEL:
		{
		struct isp_channel2_info *sub = (void *)param->data;
		ret = ak_isp_vo_set_sub_channel_scale(sub->width, sub->height);
		}
		break;

	case AK_ISP_USER_CID_SET_OCCLUSION:
		break;
	case AK_ISP_USER_CID_SET_OCCLUSION_COLOR:
		break;
	case AK_ISP_USER_CID_SET_GAMMA:
		break;
	case AK_ISP_USER_CID_SET_SATURATION:
		break;
	case AK_ISP_USER_CID_SET_BRIGHTNESS:
		break;
	case AK_ISP_USER_CID_SET_CONTRAST:
		break;
	case AK_ISP_USER_CID_SET_SHARPNESS:
		break;
	case AK_ISP_USER_CID_SET_POWER_LINE_FREQUENCY:
		break;
	case AK_ISP_USER_CID_SET_OSD_COLOR_TABLE_ATTR:
		{
		struct isp_osd_color_table_attr *color_table = (void *)param->data;

		ret = ak_isp_vpp_set_osd_color_table_attr((AK_ISP_OSD_COLOR_TABLE_ATTR *)color_table);

		}
		break;
	case AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_CONTEXT_ATTR:
		{
		struct isp_osd_context_attr *osd_context = (void *)param->data;
		AK_ISP_OSD_CONTEXT_ATTR *isp_osd_context = drv_malloc(sizeof(AK_ISP_OSD_CONTEXT_ATTR));
		if (!isp_osd_context) {
			printk("kzalloc for isp_osd_context failed\n");
			ret = -1;
			goto out;
		}

		isp_osd_context->osd_context_addr = (T_U32*)osd_context->osd_context_addr;

		isp_osd_context->osd_width = osd_context->osd_width;
		isp_osd_context->osd_height = osd_context->osd_height;
		isp_osd_context->start_xpos = osd_context->start_xpos;
		isp_osd_context->start_ypos = osd_context->start_ypos;
		isp_osd_context->alpha = osd_context->alpha;
		isp_osd_context->enable = osd_context->enable;
		//printk(KERN_ERR "%s %d osd width:%d, height:%d,start_xpos:%d,start_ypos:%d,alpha:%d\n",__func__, __LINE__,osd_context->osd_width, osd_context->osd_height,osd_context->start_xpos,osd_context->start_ypos,osd_context->alpha);
		ret = ak_isp_vpp_set_main_channel_osd_context_attr(isp_osd_context);

		drv_free(isp_osd_context);
		}
		break;
	case AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_CONTEXT_ATTR:
		{
		struct isp_osd_context_attr *osd_context = (void *)param->data;
		AK_ISP_OSD_CONTEXT_ATTR *isp_osd_context = drv_malloc(sizeof(AK_ISP_OSD_CONTEXT_ATTR));
		if (!isp_osd_context) {
			printk("kzalloc for isp_osd_context failed\n");
			ret = -1;
			goto out;
		}
		isp_osd_context->osd_context_addr = (T_U32*)osd_context->osd_context_addr;

		isp_osd_context->osd_width = osd_context->osd_width;
		isp_osd_context->osd_height = osd_context->osd_height;
		isp_osd_context->start_xpos = osd_context->start_xpos;
		isp_osd_context->start_ypos = osd_context->start_ypos;
		isp_osd_context->alpha = osd_context->alpha;
		isp_osd_context->enable = osd_context->enable;
		ret = ak_isp_vpp_set_sub_channel_osd_context_attr(isp_osd_context);


		drv_free(isp_osd_context);
		}
		break;
	case AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_MEM_ATTR:
		{
			
		struct isp_osd_mem_attr *osd_mem = (struct isp_osd_mem_attr *)param->data;

		AK_ISP_OSD_MEM_ATTR isp_osd_mem;

		isp_osd_mem.dma_paddr = osd_mem->dma_paddr;
		isp_osd_mem.dma_vaddr = osd_mem->dma_paddr;
		isp_osd_mem.size = osd_mem->size;			
		printk("%s %d paddr:%p size:%d \n",
				__func__, __LINE__, osd_mem->dma_paddr, osd_mem->size);

		ret = ak_isp_vpp_set_main_channel_osd_mem_attr(&isp_osd_mem);
		}
	
		break;
	case AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_MEM_ATTR:
		{
	
		struct isp_osd_mem_attr *osd_mem = (void *)param->data;

		AK_ISP_OSD_MEM_ATTR isp_osd_mem ;

		isp_osd_mem.dma_paddr = osd_mem->dma_paddr;

		isp_osd_mem.dma_vaddr = osd_mem->dma_paddr;
		isp_osd_mem.size = osd_mem->size;			
		printk("%s %d paddr:%p size:%d \n", __func__, __LINE__,
				osd_mem->dma_paddr,osd_mem->size);

		ret = ak_isp_vpp_set_sub_channel_osd_mem_attr(&isp_osd_mem);

		}
	
		break;
		
	default:
		ret = -1;
		break;
	}

	if(ret == 0)
		ret = true;
	else
		ret = false;
out:
	return ret;
}


long akisp_ioctl( unsigned int cmd, void* arg)
{
	long ret = 0;

	//mutex_lock(&priv->lock);
	
	switch(cmd) 
	{
	case AK_ISP_VP_SET_BLC:
		{
			AK_ISP_BLC_ATTR  *blc_para = (AK_ISP_BLC_ATTR *)arg;
			
			ak_isp_vp_set_blc_attr(blc_para);
			printk_blc(blc_para);
		}
		break;
	case AK_ISP_VP_GET_BLC:
		{
			AK_ISP_BLC_ATTR  *blc_para = (AK_ISP_BLC_ATTR *)arg;
			ak_isp_vp_get_blc_attr(blc_para);
			printk_blc(blc_para);
		}
		break;
	
	case AK_ISP_VP_SET_LSC:
		{
			AK_ISP_LSC_ATTR  *lsc_para = (AK_ISP_LSC_ATTR *)arg;
			ak_isp_vp_set_lsc_attr(lsc_para);
			printk_lsc(lsc_para);
		}
		break;
	case AK_ISP_VP_GET_LSC:
		{
			AK_ISP_LSC_ATTR  *lsc_para = (AK_ISP_LSC_ATTR *)arg;
			ak_isp_vp_get_lsc_attr(lsc_para);
			printk_lsc(lsc_para);
		}
		break;
		
	case AK_ISP_VP_SET_GB:
		{
			AK_ISP_GB_ATTR   *gb_para = (AK_ISP_GB_ATTR *)arg;
			ak_isp_vp_set_gb_attr(gb_para);
			printk_gb(gb_para);
		}
		break;
	case AK_ISP_VP_GET_GB:
		{
			AK_ISP_GB_ATTR   *gb_para = (AK_ISP_GB_ATTR *)arg;
			ak_isp_vp_get_gb_attr(gb_para);
			printk_gb(gb_para);
		}
		break;
		
	case AK_ISP_VP_SET_RAW_LUT:
		{
			AK_ISP_RAW_LUT_ATTR *raw_lut_para = (AK_ISP_RAW_LUT_ATTR *)arg;
			ak_isp_vp_set_raw_lut_attr(raw_lut_para);
			printk_raw_lut(raw_lut_para);
		}
		break;
	case AK_ISP_VP_GET_RAW_LUT:
		{
			AK_ISP_RAW_LUT_ATTR *raw_lut_para = (AK_ISP_RAW_LUT_ATTR *)arg;
			ak_isp_vp_get_raw_lut_attr(raw_lut_para);
			printk_raw_lut(raw_lut_para);
		}
		break;
		
	case AK_ISP_VP_SET_RAW_NR1:
		{
			AK_ISP_NR1_ATTR    *nr1_para = (AK_ISP_NR1_ATTR *)arg;

			ak_isp_vp_set_nr1_attr(nr1_para);
			printk_nr1_para(nr1_para);
		}
		break;
	case AK_ISP_VP_GET_RAW_NR1:
		{
			AK_ISP_NR1_ATTR    *nr1_para = (AK_ISP_NR1_ATTR *)arg;
			ak_isp_vp_get_nr1_attr(nr1_para);
			printk_nr1_para(nr1_para);
		}
		break;
	
	case AK_ISP_VP_SET_DEMO:
		{
			AK_ISP_DEMO_ATTR   *demo_para = (AK_ISP_DEMO_ATTR *)arg;
			
			ak_isp_vp_set_demo_attr(demo_para);
			printk_demo_para(demo_para);

		}
		break;
	case AK_ISP_VP_GET_DEMO:
		{
			AK_ISP_DEMO_ATTR   *demo_para = (AK_ISP_DEMO_ATTR *)arg;
		    ak_isp_vp_get_demo_attr(demo_para);
			printk_demo_para(demo_para);
		}
		break;
		
   case AK_ISP_SET_DPC:
   		{
	   		AK_ISP_DDPC_ATTR  *dpc_para = (AK_ISP_DDPC_ATTR *)arg;
			
			ak_isp_vp_set_dpc_attr(dpc_para);
			printk_dpc(dpc_para);

   		}
		break;
	case AK_ISP_GET_DPC:
		{
			AK_ISP_DDPC_ATTR  *dpc_para = (AK_ISP_DDPC_ATTR *)arg;
			ak_isp_vp_get_dpc_attr(dpc_para);
			printk_dpc(dpc_para);
		}
		break;
		
	case AK_ISP_SET_CCM:
		{
			AK_ISP_CCM_ATTR    *ccm_para = (AK_ISP_CCM_ATTR *)arg;
			ak_isp_vp_set_ccm_attr(ccm_para);
			printk_ccm_para(ccm_para);
		}
		break;
	case AK_ISP_GET_CCM:
		{
			AK_ISP_CCM_ATTR    *ccm_para = (AK_ISP_CCM_ATTR *)arg;
			ak_isp_vp_get_ccm_attr(ccm_para);
			printk_ccm_para(ccm_para);
		}
		break;

    case AK_ISP_SET_RGB_GAMMA:
		{
			AK_ISP_RGB_GAMMA_ATTR *rgb_gamma_para = (AK_ISP_RGB_GAMMA_ATTR *)arg;
			ak_isp_vp_set_rgb_gamma_attr(rgb_gamma_para);
			printk_rgb_gamma_para(rgb_gamma_para);
    	}
		break;
	case AK_ISP_GET_RGB_GAMMA:
		{
			AK_ISP_RGB_GAMMA_ATTR *rgb_gamma_para = (AK_ISP_RGB_GAMMA_ATTR *)arg;
			ak_isp_vp_get_rgb_gamma_attr(rgb_gamma_para);
			printk_rgb_gamma_para(rgb_gamma_para);
		}
		break;
	
	case AK_ISP_SET_WDR:
		{
			AK_ISP_WDR_ATTR       *wdr_para = (AK_ISP_WDR_ATTR *)arg;
			ak_isp_vp_set_wdr_attr(wdr_para);
			printk_wdr_para(wdr_para);
		}
		break;
	case AK_ISP_GET_WDR:
		{
			AK_ISP_WDR_ATTR       *wdr_para = (AK_ISP_WDR_ATTR *)arg;
			ak_isp_vp_get_wdr_attr(wdr_para);
			printk_wdr_para(wdr_para);
		}
		break;

	case AK_ISP_SET_SHARP:
		{
			AK_ISP_SHARP_ATTR   *sharp_para = (AK_ISP_SHARP_ATTR *)arg;
			ak_isp_vp_set_sharp_attr(sharp_para);
			printk_sharp_para(sharp_para);
		}
		break;
		
	case AK_ISP_GET_SHARP:
		{
			AK_ISP_SHARP_ATTR   *sharp_para = (AK_ISP_SHARP_ATTR *)arg;
			ak_isp_vp_get_sharp_attr(sharp_para);
			printk_sharp_para(sharp_para);
		}
		break;
		
	case AK_ISP_SET_SHARP_EX:
		{
			AK_ISP_SHARP_EX_ATTR *sharp_ex_para = (AK_ISP_SHARP_EX_ATTR *)arg;
			ak_isp_vp_set_sharp_ex_attr(sharp_ex_para);
			printk_sharp_ex_para(sharp_ex_para);
		}
		break;
	case AK_ISP_GET_SHARP_EX:
		{
			AK_ISP_SHARP_EX_ATTR *sharp_ex_para = (AK_ISP_SHARP_EX_ATTR *)arg;
			ak_isp_vp_get_sharp_ex_attr(sharp_ex_para);
			printk_sharp_ex_para(sharp_ex_para);
		}
		break;

	case AK_ISP_SET_Y_NR2:
		{
			AK_ISP_NR2_ATTR     *nr2_para = (AK_ISP_NR2_ATTR *)arg;

			ak_isp_vp_set_nr2_attr(nr2_para);
			printk_nr2_para(nr2_para);
		}
		break;
	case AK_ISP_GET_Y_NR2:
		{
			AK_ISP_NR2_ATTR     *nr2_para = (AK_ISP_NR2_ATTR *)arg;
			ak_isp_vp_get_nr2_attr(nr2_para);
			printk_nr2_para(nr2_para);
		}
		break;

	case AK_ISP_SET_3D_NR:
		{
			AK_ISP_3D_NR_ATTR    *nr_3d_para = (AK_ISP_3D_NR_ATTR *)arg;
			ak_isp_vp_set_3d_nr_attr(nr_3d_para);
			printk_nr_3d_para(nr_3d_para);
		}
		break;
	case AK_ISP_GET_3D_NR:
		{
			AK_ISP_3D_NR_ATTR    *nr_3d_para = (AK_ISP_3D_NR_ATTR *)arg;
			ak_isp_vp_get_3d_nr_attr(nr_3d_para);
			printk_nr_3d_para(nr_3d_para);
		}
		break;
		

	case AK_ISP_SET_3D_NR_REF:
		{
			AK_ISP_3D_NR_REF_ATTR *nr_3d_ref_para = (AK_ISP_3D_NR_REF_ATTR *)arg;
			ak_isp_vp_set_3d_nr_ref_addr(nr_3d_ref_para);
			printk_3d_nr_ref(nr_3d_ref_para);
		}
		break;
	case AK_ISP_GET_3D_NR_REF:
		{
			AK_ISP_3D_NR_REF_ATTR *nr_3d_ref_para = (AK_ISP_3D_NR_REF_ATTR *)arg;
			ak_isp_vp_get_3d_nr_ref_addr(nr_3d_ref_para);
			printk_3d_nr_ref(nr_3d_ref_para);
		}
		break;
	case AK_ISP_GET_3D_NR_STAT_INFO_INT:
		{
			AK_ISP_3D_NR_STAT_INFO  *nr_3d_stat_info = (AK_ISP_3D_NR_STAT_INFO *)arg;
			//printk("111\n");
			ak_isp_vp_get_3d_nr_stat_info(nr_3d_stat_info);
						//printk("222\n");
			printk_3d_nr_stat_info(nr_3d_stat_info);
						//printk("333\n");
		}
		break;
	case AK_ISP_GET_FCS:
		{
			AK_ISP_FCS_ATTR      *fcs_para = (AK_ISP_FCS_ATTR *)arg;
			ak_isp_vp_get_fcs_attr(fcs_para);
			printk_fcs_para(fcs_para);
		}
		break;
	case AK_ISP_SET_FCS:
		{
			AK_ISP_FCS_ATTR      *fcs_para = (AK_ISP_FCS_ATTR *)arg;
			ak_isp_vp_set_fcs_attr(fcs_para);
			printk_fcs_para(fcs_para);
		}
		break;
	
	case AK_ISP_SET_CONTRAST:
		{
			AK_ISP_CONTRAST_ATTR  *contrast_para = (AK_ISP_CONTRAST_ATTR *)arg;
			ak_isp_vp_set_contrast_attr(contrast_para);
			printk_contrast_para(contrast_para);
		}
		break;
	case AK_ISP_GET_CONTRAST:
		{
			AK_ISP_CONTRAST_ATTR  *contrast_para = (AK_ISP_CONTRAST_ATTR *)arg;
			ak_isp_vp_get_contrast_attr(contrast_para);
			printk_contrast_para(contrast_para);
		}
		break;


	case AK_ISP_SET_SAT:
		{
			AK_ISP_SATURATION_ATTR *satu_para = (AK_ISP_SATURATION_ATTR *)arg;
			ak_isp_vp_set_saturation_attr(satu_para);
			printk_satu_para(satu_para); 
		}
		break;
	case AK_ISP_GET_SAT:
		{
			AK_ISP_SATURATION_ATTR *satu_para = (AK_ISP_SATURATION_ATTR *)arg;
			ak_isp_vp_get_saturation_attr(satu_para);  
			printk_satu_para(satu_para); 
		}
		break;

	case AK_ISP_SET_RGB2YUV:
		{
			AK_ISP_RGB2YUV_ATTR  *rgb2yuv_para = (AK_ISP_RGB2YUV_ATTR *)arg;
			ak_isp_vp_set_rgb2yuv_attr(rgb2yuv_para);
			printk_rgb2yuv_para(rgb2yuv_para); 
		}
		break;
	case AK_ISP_GET_RGB2YUV:
		{
			AK_ISP_RGB2YUV_ATTR  *rgb2yuv_para = (AK_ISP_RGB2YUV_ATTR *)arg;
			ak_isp_vp_get_rgb2yuv_attr(rgb2yuv_para);  
			printk_rgb2yuv_para(rgb2yuv_para); 
		}
		break;
			
	case AK_ISP_SET_YUV_EFFECT:
		{
			AK_ISP_EFFECT_ATTR    *effect_para = (AK_ISP_EFFECT_ATTR *)arg;  
			ak_isp_vp_set_effect_attr(effect_para);
			printk_effect_para(effect_para); 
		}
		break;
	case AK_ISP_GET_YUV_EFFECT:
		{
			AK_ISP_EFFECT_ATTR    *effect_para= (AK_ISP_EFFECT_ATTR *)arg; 
			ak_isp_vp_get_effect_attr(effect_para);  
			printk_effect_para(effect_para); 
		}
		break;
		
	case AK_ISP_SET_RAW_HIST:
		{
			AK_ISP_RAW_HIST_ATTR       *raw_hist_para = (AK_ISP_RAW_HIST_ATTR *)arg;
			ak_isp_vp_set_raw_hist_attr(raw_hist_para);
			printk_raw_hist_para(raw_hist_para); 
		}
		break;
		
	case AK_ISP_GET_RAW_HIST:
		{
			AK_ISP_RAW_HIST_ATTR       *raw_hist_para = (AK_ISP_RAW_HIST_ATTR *)arg;
			ak_isp_vp_get_raw_hist_attr(raw_hist_para);
			printk_raw_hist_para(raw_hist_para); 
		}
	
		break;
	case AK_ISP_GET_RAW_HIST_STAT_INT:
		{
			AK_ISP_RAW_HIST_STAT_INFO  *raw_hist_stat_info = (AK_ISP_RAW_HIST_STAT_INFO *)arg;		
			
			ak_isp_vp_get_raw_hist_stat_info(raw_hist_stat_info);
			
			printk_raw_hist_stat_info(raw_hist_stat_info);

		}
		break;
		
	case AK_ISP_SET_RGB_HIST:
		{
			AK_ISP_RGB_HIST_ATTR       *rgb_hist_para = (AK_ISP_RGB_HIST_ATTR *)arg;  
			ak_isp_vp_set_rgb_hist_attr(rgb_hist_para);
			printk_rgb_hist_para(rgb_hist_para); 
		}
		break;
	
	case AK_ISP_GET_RGB_HIST:
		{
			AK_ISP_RGB_HIST_ATTR       *rgb_hist_para = (AK_ISP_RGB_HIST_ATTR *)arg;
			ak_isp_vp_get_rgb_hist_attr(rgb_hist_para);
			printk_rgb_hist_para(rgb_hist_para); 
		}
		break;
		
	case AK_ISP_GET_RGB_HIST_STAT_INT:
		{
			AK_ISP_RGB_HIST_STAT_INFO  *rgb_hist_stat_info = (AK_ISP_RGB_HIST_STAT_INFO *)arg;
			
			ak_isp_vp_get_rgb_hist_stat_info(rgb_hist_stat_info);
			
			printk_rgb_hist_stat_info(rgb_hist_stat_info);

		}
		break;
	case AK_ISP_SET_Y_HIST:
		{
			AK_ISP_YUV_HIST_ATTR       *yuv_hist_para = (AK_ISP_YUV_HIST_ATTR *)arg;  
			ak_isp_vp_set_yuv_hist_attr(yuv_hist_para);
			printk_yuv_hist_para(yuv_hist_para); 
		}
		break;
	case AK_ISP_GET_Y_HIST:
		{
			AK_ISP_YUV_HIST_ATTR       *yuv_hist_para = (AK_ISP_YUV_HIST_ATTR *)arg; 
			ak_isp_vp_get_yuv_hist_attr(yuv_hist_para); 
			printk_yuv_hist_para(yuv_hist_para); 
		}
		break;
	case AK_ISP_GET_Y_HIST_STAT_INT:
		{
			AK_ISP_YUV_HIST_STAT_INFO  *yuv_hist_stat_info = (AK_ISP_YUV_HIST_STAT_INFO *)arg; 
			ak_isp_vp_get_yuv_hist_stat_info(yuv_hist_stat_info); 
			printk_yuv_hist_stat_info(yuv_hist_stat_info);

		}
		break;

	case AK_ISP_SET_EXP_TYPE:
		{
			AK_ISP_EXP_TYPE       *exp_type_para = (AK_ISP_EXP_TYPE *)arg;  
			ak_isp_vp_set_exp_type(exp_type_para);
			printk_exp_type_para(exp_type_para); 
		}
		break;
	case AK_ISP_GET_EXP_TYPE:
		{
			AK_ISP_EXP_TYPE       *exp_type_para = (AK_ISP_EXP_TYPE *)arg;
		    ak_isp_vp_get_exp_type(exp_type_para);  
			printk_exp_type_para(exp_type_para); 
		}
		break;
		
	case AK_ISP_SET_FRAME_RATE:
		{
			AK_ISP_FRAME_RATE_ATTR        *frame_rate_para = (AK_ISP_FRAME_RATE_ATTR *)arg;  
			ak_isp_vp_set_frame_rate(frame_rate_para);
			printk_frame_rate_para(frame_rate_para); 
		}
		break;
	case AK_ISP_GET_FRAME_RATE:
		{
			AK_ISP_FRAME_RATE_ATTR        *frame_rate_para = (AK_ISP_FRAME_RATE_ATTR *)arg;  
			ak_isp_vp_get_frame_rate(frame_rate_para);  
			printk_frame_rate_para(frame_rate_para); 
		}
		break;
		
	case AK_ISP_SET_AE:
		{
			AK_ISP_AE_ATTR        *ae_para = (AK_ISP_AE_ATTR *)arg;  
			printk_ae_para(ae_para);
		    ak_isp_vp_set_ae_attr(ae_para); 
		}
		break;
	case AK_ISP_GET_AE:
		{
			AK_ISP_AE_ATTR        *ae_para = (AK_ISP_AE_ATTR *)arg; 
			ak_isp_vp_get_ae_attr(ae_para); 
			printk_ae_para(ae_para); 
		}
		break;

	case AK_ISP_GET_AE_RUN_INFO_INT:
		{
			AK_ISP_AE_RUN_INFO    *ae_run_para = (AK_ISP_AE_RUN_INFO*)arg; 
			ak_isp_vp_get_ae_run_info(ae_run_para); 
			printk_ae_run_para(ae_run_para);
		}
		break;
		
	case AK_ISP_SET_WB_TYPE:
		{
			AK_ISP_WB_TYPE_ATTR        *wb_type_para = (AK_ISP_WB_TYPE_ATTR *)arg;  
			ak_isp_vp_set_wb_type(wb_type_para);
			printk_wb_type_para(wb_type_para); 
		}
		break;
	case AK_ISP_GET_WB_TYPE:
		{
			AK_ISP_WB_TYPE_ATTR        *wb_type_para = (AK_ISP_WB_TYPE_ATTR *)arg;  
			ak_isp_vp_get_wb_type(wb_type_para);  
			printk_wb_type_para(wb_type_para); 
		}
		break;


	case AK_ISP_SET_AWB:
		{
			AK_ISP_AWB_ATTR            *awb_para = (AK_ISP_AWB_ATTR *)arg;  
			ak_isp_vp_set_awb_attr(awb_para);
			printk_awb_para(awb_para); 
		}
		break;
	case AK_ISP_GET_AWB:
		{
			AK_ISP_AWB_ATTR            *awb_para = (AK_ISP_AWB_ATTR *)arg;
			ak_isp_vp_get_awb_attr(awb_para);  
			printk_awb_para(awb_para); 
		}
		break;

	case AK_ISP_SET_AWB_EX:
		{
			AK_ISP_AWB_EX_ATTR            *awb_ex_para = (AK_ISP_AWB_EX_ATTR *)arg;  
			ak_isp_vp_set_awb_ex_attr(awb_ex_para);
			printk_awb_ex_para(awb_ex_para); 
		}
		break;
	case AK_ISP_GET_AWB_EX:
		{
			AK_ISP_AWB_EX_ATTR            *awb_ex_para = (AK_ISP_AWB_EX_ATTR *)arg;
			ak_isp_vp_get_awb_ex_attr(awb_ex_para); 
			printk_awb_ex_para(awb_ex_para); 
		}
		break;
		
	case AK_ISP_GET_AWB_STAT_INFO_INT:
		{
			AK_ISP_AWB_STAT_INFO        *awb_stat_info = (AK_ISP_AWB_STAT_INFO*)arg; 
			ak_isp_vp_get_awb_stat_info(awb_stat_info); 
			printk_awb_stat_info_para(awb_stat_info);
		}
		break;
		
	case  AK_ISP_SET_AF:
		{
			AK_ISP_AF_ATTR       *af_para = (AK_ISP_AF_ATTR*)arg;  
			ak_isp_vp_set_af_attr(af_para);
			printk_af_para(af_para); 
		}
		break;
	case  AK_ISP_GET_AF:
		{
			AK_ISP_AF_ATTR       *af_para = (AK_ISP_AF_ATTR*)arg;
			ak_isp_vp_get_af_attr(af_para);
			printk_af_para(af_para); 
		}
		break;

	case  AK_ISP_GET_AF_STAT_INT: 
		{
			AK_ISP_AF_STAT_INFO  *af_stat_para = (AK_ISP_AF_STAT_INFO*)arg; 
			ak_isp_vp_get_af_stat_info(af_stat_para); 
			printk_af_stat_para(af_stat_para);
		}
		break;
		
	case  AK_ISP_SET_MWB:
		{
			AK_ISP_MWB_ATTR            *mwb_para = (AK_ISP_MWB_ATTR*)arg;  
			ak_isp_vp_set_mwb_attr(mwb_para);
			printk_mwb_para(mwb_para); 
		}
		break;
	case  AK_ISP_GET_MWB:
		{
			AK_ISP_MWB_ATTR            *mwb_para = (AK_ISP_MWB_ATTR*)arg;  
			ak_isp_vp_get_mwb_attr(mwb_para);  
			printk_mwb_para(mwb_para); 
		}
		break;
		
	case  AK_ISP_SET_WEIGHT:
		{
			AK_ISP_WEIGHT_ATTR          *weight_para = (AK_ISP_WEIGHT_ATTR*)arg;
			ak_isp_vp_set_zone_weight(weight_para);
			printk_weight_para(weight_para); 
		}
		break;
		
	case  AK_ISP_GET_WEIGHT:
		{
			AK_ISP_WEIGHT_ATTR          *weight_para = (AK_ISP_WEIGHT_ATTR*)arg;
			ak_isp_vp_get_zone_weight(weight_para);
			printk_weight_para(weight_para); 
		}
		break;

	case  AK_ISP_SET_MAIN_CHAN_MASK_AREA:
		{
			AK_ISP_MAIN_CHAN_MASK_AREA_ATTR  *main_chan_mask_area_para = (AK_ISP_MAIN_CHAN_MASK_AREA_ATTR*)arg;  
			ak_isp_vpp_set_main_chan_mask_area_attr(main_chan_mask_area_para);
			printk_main_chan_mask_area_para(main_chan_mask_area_para); 
		}
		break;
	case  AK_ISP_GET_MAIN_CHAN_MASK_AREA:
		{
			AK_ISP_MAIN_CHAN_MASK_AREA_ATTR  *main_chan_mask_area_para = (AK_ISP_MAIN_CHAN_MASK_AREA_ATTR*)arg;
			ak_isp_vpp_get_main_chan_mask_area_attr(main_chan_mask_area_para);  
			printk_main_chan_mask_area_para(main_chan_mask_area_para); 
		}
		break;
	case  AK_ISP_SET_SUB_CHAN_MASK_AREA:
		{
			AK_ISP_SUB_CHAN_MASK_AREA_ATTR   *sub_chan_mask_area_para = (AK_ISP_SUB_CHAN_MASK_AREA_ATTR*)arg;  
			ak_isp_vpp_set_sub_chan_mask_area_attr(sub_chan_mask_area_para);
			printk_sub_chan_mask_area_para(sub_chan_mask_area_para); 
		}
		break;
		
	case  AK_ISP_GET_SUB_CHAN_MASK_AREA:
		{
			AK_ISP_SUB_CHAN_MASK_AREA_ATTR   *sub_chan_mask_area_para = (AK_ISP_SUB_CHAN_MASK_AREA_ATTR*)arg; 
			ak_isp_vpp_get_sub_chan_mask_area_attr(sub_chan_mask_area_para);  
			printk_sub_chan_mask_area_para(sub_chan_mask_area_para); 
		}
		break;
 
	case  AK_ISP_SET_MASK_COLOR:
		{
			AK_ISP_MASK_COLOR_ATTR       *mask_color_para = (AK_ISP_MASK_COLOR_ATTR*)arg;  
			ak_isp_vpp_set_mask_color(mask_color_para);
			printk_mask_color_para(mask_color_para); 
		}
		break;
	case  AK_ISP_GET_MASK_COLOR:
		{
			AK_ISP_MASK_COLOR_ATTR       *mask_color_para = (AK_ISP_MASK_COLOR_ATTR*)arg;
			ak_isp_vpp_get_mask_color(mask_color_para);  
			printk_mask_color_para(mask_color_para); 
		}
		break;
	case AK_ISP_SET_SENSOR_REG:
		{
			AK_ISP_SENSOR_REG_INFO* reg_info = (AK_ISP_SENSOR_REG_INFO *)arg;
			cam_sensor_write_reg(reg_info->reg_addr, reg_info->value);
		}
		break;
	case AK_ISP_GET_SENSOR_REG:
		{

			AK_ISP_SENSOR_REG_INFO* reg_info = (AK_ISP_SENSOR_REG_INFO *)arg;

			reg_info->value = cam_sensor_read_reg(reg_info->reg_addr);
		}
		break;

#if 0		

	case AK_ISP_INIT_SENSOR_DEV:
		{
			void *p_reg_info;
			AK_ISP_SENSOR_INIT_PARA sensor_para;
			AK_ISP_SENSOR_CB *sensor_cb = ak_sensor_get_sensor_cb();

			if (copy_from_user(&sensor_para, (void *)arg, sizeof(AK_ISP_SENSOR_INIT_PARA)))
			{
			    ret = -EFAULT;
			    goto fini;
			}
			p_reg_info = sensor_para.reg_info;
			sensor_para.reg_info = kmalloc(sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para.num, GFP_KERNEL);
			if (!sensor_para.reg_info) {
				ret = -ENOMEM;
				goto fini;
			} else {
				if (copy_from_user(sensor_para.reg_info, p_reg_info, sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para.num))
				{
					ret = -EFAULT;
					kfree(sensor_para.reg_info);
					goto fini;
				}
				sensor_cb->sensor_init_func(&sensor_para);
				kfree(sensor_para.reg_info);
			}
		}
		break;



	case AK_ISP_GET_SENSOR_ID:
		{
			int id;
			AK_ISP_SENSOR_CB *sensor_cb = ak_sensor_get_sensor_cb();

			id = sensor_cb->sensor_read_id_func();
			if (copy_to_user((void *)arg, &id, sizeof(int)))
			{
			    ret = -EFAULT;
			    goto fini;
			}
		}
		break;

#endif
	case AK_ISP_SET_ISP_CAPTURING:
		{
			int resume = *((int *)arg); 
			ret = ak_isp_set_isp_capturing(resume);
		}
		break;

	case AK_ISP_SET_USER_PARAMS:
		{
			AK_ISP_USER_PARAM *param = (AK_ISP_USER_PARAM *)arg; 
			ret = ak_isp_set_user_params_do(param);
		}
		break;

	case AK_ISP_SET_MISC_ATTR:
		{
			AK_ISP_MISC_ATTR *misc = (AK_ISP_MISC_ATTR *)arg; 
			ret = ak_isp_vo_set_misc_attr(misc);
			printk_misc_para(misc);
		}
		break;

	case AK_ISP_GET_MISC_ATTR:
		{
			AK_ISP_MISC_ATTR *misc = (AK_ISP_MISC_ATTR *)arg;
			ret = ak_isp_vo_get_misc_attr(misc); 
			printk_misc_para(misc);
		}
		break;

	case AK_ISP_SET_3D_NR_PHYADDR:
		{
			AK_ISP_3D_NR_REF_ATTR *p_ref = (AK_ISP_3D_NR_REF_ATTR *)arg; 
			ak_isp_vp_set_3d_nr_ref_addr(p_ref);
		}
		break;

	case AK_ISP_SET_Y_GAMMA:
		{
			AK_ISP_Y_GAMMA_ATTR *y_gamma = (AK_ISP_Y_GAMMA_ATTR *)arg;  
			ak_isp_vp_set_y_gamma_attr(y_gamma);
			printk_y_gamma_para(y_gamma); 
		}
		break;
	case AK_ISP_GET_Y_GAMMA:
		{
			AK_ISP_Y_GAMMA_ATTR *y_gamma = (AK_ISP_Y_GAMMA_ATTR *)arg;
			ak_isp_vp_get_y_gamma_attr(y_gamma);  
			printk_y_gamma_para(y_gamma); 
		}
		break;


	case AK_ISP_SET_HUE:
		{
			AK_ISP_HUE_ATTR *hue = (AK_ISP_HUE_ATTR *)arg;
			 
			ak_isp_vp_set_hue_attr(hue);
			printk_hue_para(hue); 
		}
		break;
	case AK_ISP_GET_HUE:
		{
			AK_ISP_HUE_ATTR *hue = (AK_ISP_HUE_ATTR *)arg;
			ak_isp_vp_get_hue_attr(hue);  
			printk_hue_para(hue); 
		}
		break;

	case AK_ISP_SET_FLIP_MIRROR:
		{
			struct isp_flip_mirror_info *info = (struct isp_flip_mirror_info *)arg;  
			ret = ak_isp_set_flip_mirror(info->flip_en, info->mirror_en);
		}
		break;
#if 0		

	case AK_ISP_SET_SENSOR_FPS:
		{
			int fps;
			AK_ISP_SENSOR_CB *sensor_cb;

			if (copy_from_user(&fps,(int *)arg, sizeof(int))) {
				printk("copy from user for fps failed\n");
				ret = -EFAULT;
			} else {
				sensor_cb = ak_sensor_get_sensor_cb();
				if (sensor_cb) {
					ret = sensor_cb->sensor_set_fps_func(fps);
				} else {
					printk(KERN_ERR "get sensor_cb failed int set sensor fps\n");
					ret = -ENODEV;
				}
			}
		}
		break;
	case AK_ISP_GET_SENSOR_FPS:
		{
			int fps;
			AK_ISP_SENSOR_CB *sensor_cb;

			sensor_cb = ak_sensor_get_sensor_cb();
			if (sensor_cb) {
				fps = sensor_cb->sensor_get_fps_func();
				if (copy_to_user((int *)arg, &fps, sizeof(int))) {
					printk("copy to user for fps failed\n");
					ret = -EFAULT;
				}
			} else {
				printk(KERN_ERR "get sensor_cb failed int set sensor fps\n");
				ret = -ENODEV;
			}
		}
		break;

#endif
		case AK_ISP_GET_WORK_SCENE:
		{

			ret = ak_isp_get_scene();
		}
		break;
		case AK_ISP_SET_MAIN_CHANNEL_SCALE:
		{
			AK_ISP_CHANNEL_SCALE *scale = (AK_ISP_CHANNEL_SCALE*)arg;
			printk("scale->width:%d,scale->height:%d.\n",scale->width, scale->height);
			ret = ak_isp_vo_set_main_channel_scale(scale->width, scale->height);
		}
		break;
		case AK_ISP_SET_SUB_CHANNEL_SCALE:
		{
			AK_ISP_CHANNEL_SCALE *sub_scale = (AK_ISP_CHANNEL_SCALE*)arg;
			ret = ak_isp_vo_set_sub_channel_scale(sub_scale->width, sub_scale->height);
		}
		break;
		case AK_ISP_SET_CROP:
		{
			AK_ISP_CROP *crop = (AK_ISP_CROP*)arg;
			ret = 	ak_isp_vi_set_crop(crop->left, crop->top, crop->width, crop->height);
		}
		break;
	default:
		printk("akisp: the ioctl is unknow.\n");
		ret = -1;
		break;
	}

fini:
	//mutex_unlock(&priv->lock);
	return ret;
}

/**
 * brief: get isp stat info data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_statinfo(enum isp_module_id module_id, void *buf, unsigned int *size)
{
	if (NULL == buf || NULL == size) {
		printk("param err\n");
		return -1;
	}

	switch (module_id) {
	case ISP_3DSTAT:
		//AK_ISP_get_3d_nr_stat_info((AK_ISP_3D_NR_STAT_INFO*)buf);
		akisp_ioctl( AK_ISP_GET_3D_NR_STAT_INFO_INT, (AK_ISP_3D_NR_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_3D_NR_STAT_INFO);
		break;
	case ISP_AESTAT:
		//AK_ISP_get_ae_run_info((AK_ISP_AE_RUN_INFO*)buf);
		akisp_ioctl( AK_ISP_GET_AE_RUN_INFO_INT, (AK_ISP_AE_RUN_INFO*)buf);
		*size = sizeof(AK_ISP_AE_RUN_INFO);
		//AK_ISP_get_raw_hist_stat_info((AK_ISP_RAW_HIST_STAT_INFO*)(buf + *size));
		akisp_ioctl( AK_ISP_GET_RAW_HIST_STAT_INT, (AK_ISP_RAW_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_RAW_HIST_STAT_INFO);
		//AK_ISP_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO*)(buf + *size));
		akisp_ioctl( AK_ISP_GET_RGB_HIST_STAT_INT, (AK_ISP_RGB_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_RGB_HIST_STAT_INFO);
		//AK_ISP_get_yuv_hist_stat_info((AK_ISP_YUV_HIST_STAT_INFO*)(buf + *size));
		akisp_ioctl( AK_ISP_GET_Y_HIST_STAT_INT, (AK_ISP_YUV_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_YUV_HIST_STAT_INFO);
		break;
	case ISP_AFSTAT:
		//AK_ISP_get_af_stat_info((AK_ISP_AF_STAT_INFO*)buf);
		akisp_ioctl( AK_ISP_GET_AF_STAT_INT, (AK_ISP_AF_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_AF_STAT_INFO);
		break;
	case ISP_AWBSTAT:
		//Ak_ISP_get_awb_stat_info((AK_ISP_AWB_STAT_INFO*)buf);
		akisp_ioctl( AK_ISP_GET_AWB_STAT_INFO_INT, (AK_ISP_AWB_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_AWB_STAT_INFO);
		break;
	case ISP_AESTAT_SUB_RGB_STAT:
		//AK_ISP_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO *)buf);
		akisp_ioctl( AK_ISP_GET_RGB_HIST_STAT_INT, (AK_ISP_RGB_HIST_STAT_INFO *)buf);
		break;
	default:
		return -1;
	}

	return 0;
}

int isp_get_hz(void)
{
	return power_hz;
}

/**
 * brief: set power hz
 * @value[IN]: power hz, 50 or 60
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_hz(int hz)
{
	AK_ISP_AE_ATTR ae_attr;
	int cur_fps = 0;

	if (akisp_ioctl(AK_ISP_GET_AE,&ae_attr))
		return -1;

	power_hz = hz;
	cur_fps = cam_get_sensor_fps();
	if (60 == power_hz) {
		if ((12 == cur_fps) || (13 == cur_fps))
			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1200;
		else
			ae_attr.exp_step = ae_attr.exp_time_max * cur_fps / 120;
	} else {
		if ((12 == cur_fps) || (13 == cur_fps))
			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1000;
		else
			ae_attr.exp_step = ae_attr.exp_time_max * cur_fps / 100;
	}

	if (akisp_ioctl(AK_ISP_SET_AE,&ae_attr))
		return -1;

	return 0;
}

void *isp_3D_NR_create(int width, int height)
{
	unsigned int _3dnr_bytes = width * height * 3;

	unsigned char *_3dnr_virt;
	AK_ISP_3D_NR_REF_ATTR m_3d_ref;


	_3dnr_virt = malloc(_3dnr_bytes);
	if (NULL == _3dnr_virt) {
		printk("%s, dmamalloc %d bytes for 3DNR fail\n", __func__, _3dnr_bytes);
	} else {
		//printk("%s, dmamalloc %d bytes for 3DNR ok\n", __func__, _3dnr_bytes);

		m_3d_ref.yaddr_3d = _3dnr_virt;
		m_3d_ref.ysize_3d = width*height*2;
		m_3d_ref.uaddr_3d = m_3d_ref.yaddr_3d + m_3d_ref.ysize_3d;
		m_3d_ref.usize_3d = width*height/4*2;
		m_3d_ref.vaddr_3d = m_3d_ref.uaddr_3d + m_3d_ref.usize_3d;
		m_3d_ref.vsize_3d = width*height/4*2;
		
		ak_isp_vp_set_3d_nr_ref_addr(&m_3d_ref);
	}
	return _3dnr_virt;
}