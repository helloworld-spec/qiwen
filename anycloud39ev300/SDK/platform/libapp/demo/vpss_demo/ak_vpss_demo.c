#include <stdio.h>
#include <stdlib.h>

#include "ak_common.h"
#include "ak_vpss.h"


/**
 * show_vpss_menu: show vpss test main menu
 * @void
 * return: void
 * notes:
 */
static void show_vpss_menu(void)
{	
	/*
	* 0 - 3 , go in sub menu;
	* 4, exit vpss menu, it will exit this demo.
	*/
	ak_print_normal("\n--------------- vpss menu ---------------\n");
	ak_print_normal("\t 0. MD\n");
	ak_print_normal("\t 1. Mask\n");
	ak_print_normal("\t 2. OD\n");
	ak_print_normal("\t 3. Effect\n");
	ak_print_normal("\t 4. exit vpss menu\n");
	ak_print_normal("--------------- vpss menu end ---------------\n");
}

/**
 * show_vpss_md_menu: show md test menu
 * @void
 * return: void
 * notes:
 */
static void show_vpss_md_menu(void)
{	
	ak_print_normal("\n--------------- md menu ---------------\n");
	ak_print_normal("\t 0. md get stat\n");
	ak_print_normal("\t 1. exit md menu\n");
	ak_print_normal("--------------- md menu end ---------------\n");
}

/**
 * show_vpss_od_menu: show od test menu
 * @void
 * return: void
 * notes:
 */
static void show_vpss_od_menu(void)
{	
	ak_print_normal("\n--------------- od menu ---------------\n");
	ak_print_normal("\t 0. od get stat\n");
	ak_print_normal("\t 1. exit od menu\n");
	ak_print_normal("--------------- od menu end ---------------\n");
}

/**
 * show_vpss_mask_menu: show mask test menu
 * @void
 * return: void
 * notes:
 */
static void show_vpss_mask_menu(void)
{	
	ak_print_normal("\n--------------- mask menu ---------------\n");
	ak_print_normal("\t 0. mask set area\n");
	ak_print_normal("\t 1. mask get area\n");
	ak_print_normal("\t 2. mask set color\n");
	ak_print_normal("\t 3. mask get color\n");
	ak_print_normal("\t 4. exit mask menu\n");
	ak_print_normal("--------------- mask menu end ---------------\n");
}

/**
 * show_vpss_effect_menu: show effect test menu
 * @void
 * return: void
 * notes:
 */
static void show_vpss_effect_menu(void)
{	
	/*
	* 0 - 6 , set effect value;
	* 7 - 13, get effect value;
	* 14, exit effect menu.
	*/
	ak_print_normal("\n--------------- effect menu ---------------\n");
	ak_print_normal("\t 0. effect HUE, set value range: [-50, 50]\n");
	ak_print_normal("\t 1. effect brightness, set value range: [-50, 50]\n");
	ak_print_normal("\t 2. effect saturation, set value range: [-50, 50]\n");
	ak_print_normal("\t 3. effect contrast, set value range: [-50, 50]\n");
	ak_print_normal("\t 4. effect sharp, set value range: [-50, 50]\n");
	ak_print_normal("\t 5. effect wdr, set value range: [-50, 50]\n");
	ak_print_normal("\t 6. isp style ID, set value range: [0, 2]\n");
	ak_print_normal("\t 7. power Hz, set value range: 50 or 60\n");
	ak_print_normal("\t 8. effect HUE, get value\n");
	ak_print_normal("\t 9. effect brightness, get value\n");
	ak_print_normal("\t 10. effect saturation, get value\n");
	ak_print_normal("\t 11. effect contrast, get value\n");
	ak_print_normal("\t 12. effect sharp, get value\n");
	ak_print_normal("\t 13. effect wdr, get value\n");
	ak_print_normal("\t 14. isp style ID, get value\n");
	ak_print_normal("\t 15. power Hz, get value\n");
	ak_print_normal("\t 16. exit effect menu\n");
	ak_print_normal("--------------- effect menu end ---------------\n");
}

/**
 * text_vpss_md: md test
 * @void
 * return: void
 * notes:
 */
static void text_vpss_md(void)
{
	int menu_no = -1;
	int level_threshold;
	int blocks_threshold;
	int sensitivity;
	int ret;
	struct vpss_md_info md;
	
	while (1){

		show_vpss_md_menu();
		ak_print_normal("please select your function number:\n");
		if(scanf("%d", &menu_no) <= 0){
			getchar();
			continue;
		}
		while (getchar() != '\n');

		switch(menu_no){
		case 0:	//get stat
			ak_print_normal("please set Sensitivity[0,low; 1,middle; 2,high]:\n");
			if(scanf("%d", &sensitivity) <= 0){
				getchar();
				continue;
			}
			while (getchar() != '\n');

			switch (sensitivity)
			{
			case 0:	// low sensitivity
				level_threshold = 10000;
				blocks_threshold = 100;
				break;
			case 1:	// middle sensitivity
				level_threshold = 5000;
				blocks_threshold = 50;
				break;
			case 2:	// high sensitivity
				level_threshold = 2000;
				blocks_threshold = 10;
				break;
			default:
				ak_print_error("value error, sensitivity [0,low; 1,middle; 2,high]!\n");
				continue;
			}

			ret = ak_vpss_md_get_stat((const void *)1, &md);
			if (ret) {
				ak_print_error_ex("ak_md get fail\n");
			} else {
				int i, j;
				int md_cnt = 0;
				
				for(i=0; i<16; i++) {
					for(j=0; j<32; j++) {
						if(md.stat[i][j] > level_threshold)
							md_cnt++;
					}
				}

				if (md_cnt > blocks_threshold)
					ak_print_normal("############ md alarm! #############\n");
				else
					ak_print_normal("############ no md. ################\n");
			}
			break;
		case 1:	//exit
			return;
		default:
			ak_sleep_ms(500);
			break;
		}
		
		menu_no = -1;
	}
}

/**
 * text_vpss_od: od test, here we only get info and print
 * @void
 * return: void
 * notes:
 */
static void text_vpss_od(void)
{
	int menu_no = -1;
	int ret;
	struct vpss_od_info od;
	
	while (1){

		show_vpss_od_menu();
		ak_print_normal("please select your function number:\n");
		if(scanf("%d", &menu_no) <= 0){
			getchar();
			continue;
		}
		while (getchar() != '\n');

		switch(menu_no){
		case 0:	//get stat
			ret = ak_vpss_od_get((const void *)1, &od);
			if (ret == -1) {
				ak_print_error_ex("get od fail\n");
			} else {	//print af statics and ae rgb hist info
				int i;
				ak_print_normal("af_statics:\n");
				for (i = 0; i < VPSS_OD_AF_STATICS_MAX; i++)
					ak_print_normal("%08x ", od.af_statics[i]);
				ak_print_normal("\n");

				ak_print_normal("rgb_hist:\n");
				for (i = 0; i < VPSS_OD_RGB_HIST_MAX; i++)
					ak_print_normal("%08x ", od.rgb_hist[i]);
				ak_print_normal("\n");
			}
			break;
		case 1:	//exit
			return;
		default:
			ak_sleep_ms(500);
			break;
		}
		
		menu_no = -1;
	}
}

/**
 * text_vpss_mask: mask test
 * @void
 * return: void
 * notes:
 */
static void text_vpss_mask(void)
{
	int menu_no = -1;
	struct vpss_mask_area_info mask_area;
	struct vpss_mask_color_info color;
	int enable;
	int i;
	int ret;
	int type,alpha,y,u,v;
	
	while (1){

		show_vpss_mask_menu();
		ak_print_normal("please select your function number:\n");
		if(scanf("%d", &menu_no) <= 0){
			getchar();
			continue;
		}
		while (getchar() != '\n');

		switch(menu_no){
		case 0:	//set area
			ak_print_normal("please set mask enable 0 or 1:\n");
			if(scanf("%d", &enable) <= 0){
				getchar();
				continue;
			}
			while (getchar() != '\n');

			if ((enable != 1) && (enable != 0))
			{
				ak_print_error("value error, enable 0 or 1!\n");
				continue;
			}
				

			mask_area.main_mask[0].start_xpos = 0;
			mask_area.main_mask[0].end_xpos = 80;
			mask_area.main_mask[0].start_ypos = 0;
			mask_area.main_mask[0].end_ypos = 80;
			mask_area.main_mask[0].enable = enable;

			mask_area.main_mask[1].start_xpos = 40;
			mask_area.main_mask[1].end_xpos = 120;
			mask_area.main_mask[1].start_ypos = 600;
			mask_area.main_mask[1].end_ypos = 680;
			mask_area.main_mask[1].enable = enable;

			mask_area.main_mask[2].start_xpos = 1040;
			mask_area.main_mask[2].end_xpos = 1120;
			mask_area.main_mask[2].start_ypos = 600;
			mask_area.main_mask[2].end_ypos = 680;
			mask_area.main_mask[2].enable = enable;

			mask_area.main_mask[3].start_xpos = 1000;
			mask_area.main_mask[3].end_xpos = 1080;
			mask_area.main_mask[3].start_ypos = 50;
			mask_area.main_mask[3].end_ypos = 130;
			mask_area.main_mask[3].enable = enable;

			mask_area.sub_mask[0].start_xpos = 0;
			mask_area.sub_mask[0].end_xpos = 40;
			mask_area.sub_mask[0].start_ypos = 0;
			mask_area.sub_mask[0].end_ypos = 40;
			mask_area.sub_mask[0].enable = enable;

			mask_area.sub_mask[1].start_xpos = 20;
			mask_area.sub_mask[1].end_xpos = 60;
			mask_area.sub_mask[1].start_ypos = 300;
			mask_area.sub_mask[1].end_ypos = 340;
			mask_area.sub_mask[1].enable = enable;

			mask_area.sub_mask[2].start_xpos = 520;
			mask_area.sub_mask[2].end_xpos = 560;
			mask_area.sub_mask[2].start_ypos = 300;
			mask_area.sub_mask[2].end_ypos = 340;
			mask_area.sub_mask[2].enable = enable;

			mask_area.sub_mask[3].start_xpos = 500;
			mask_area.sub_mask[3].end_xpos = 540;
			mask_area.sub_mask[3].start_ypos = 26;
			mask_area.sub_mask[3].end_ypos = 66;
			mask_area.sub_mask[3].enable = enable;
			
			ret = ak_vpss_mask_set_area((const void *)1, &mask_area);
			if (ret) {
				ak_print_error_ex("set mask area failed\n");
			}
			break;
		case 1:	//get area
			ret = ak_vpss_mask_get_area((const void *)1, &mask_area);
			if (ret) {
			ak_print_error_ex("get mask area failed\n");
			}else {
				for (i=0; i<VPSS_MASK_AREA_MAX; i++)
				{
					ak_print_normal("main mask[%d]: x %d to %d, y %d to %d, enable %d\n", 
								i,
								mask_area.main_mask[i].start_xpos, 
								mask_area.main_mask[i].end_xpos,
								mask_area.main_mask[i].start_ypos,
								mask_area.main_mask[i].end_ypos,
								mask_area.main_mask[i].enable);
					ak_print_normal("sub mask[%d]: x %d to %d, y %d to %d, enable %d\n", 
								i,
								mask_area.sub_mask[i].start_xpos, 
								mask_area.sub_mask[i].end_xpos,
								mask_area.sub_mask[i].start_ypos,
								mask_area.sub_mask[i].end_ypos,
								mask_area.sub_mask[i].enable);
				}
			}
			break;
		case 2:	//set color
			ak_print_normal("please input color_type[0,3] mk_alpha[0,255] y_mk_color[0,255] u_mk_color[0,255] v_mk_color[0,255]:\n");
			if(scanf("%d %d %d %d %d", &type, &alpha, 
				&y, &u, &v) <= 0){
				getchar();
				continue;
			}
			while (getchar() != '\n');

			if ((type > VPSS_MASK_MOSAIC_BLACK_VIDEO) || (type < 0)
				|| (alpha > 255) || (alpha < 0)
				|| (y > 255) || (y < 0)
				|| (u > 255) || (u < 0)
				|| (v > 255) || (v < 0))
			{
				ak_print_error("value error, color_type[0,3] mk_alpha[0,255] y_mk_color[0,255] u_mk_color[0,255] v_mk_color[0,255]!\n");
				continue;
			}

			color.color_type = (unsigned char)type;
			color.mk_alpha = (unsigned char)alpha;
			color.y_mk_color = (unsigned char)y;
			color.u_mk_color = (unsigned char)u;
			color.v_mk_color = (unsigned char)v;
			
			ak_print_normal("your input value, color_type:%d, mk_alpha:%d, y_mk_color:%d, u_mk_color:%d, v_mk_color:%d\n", 
							color.color_type, 
							color.mk_alpha, 
							color.y_mk_color, 
							color.u_mk_color, 
							color.v_mk_color);
			
			ret = ak_vpss_mask_set_color((const void *)1, &color);

			if (ret) {
				ak_print_error_ex("set mask color failed\n");
			}
			break;
		case 3:	//get color
			ret = ak_vpss_mask_get_color((const void *)1, &color);
			if (ret) {
				ak_print_error_ex("get mask color failed\n");
			}else {
				ak_print_normal("color_type:%d, mk_alpha:%d, y_mk_color:%d, u_mk_color:%d, v_mk_color:%d\n",
							color.color_type, 
							color.mk_alpha, 
							color.y_mk_color, 
							color.u_mk_color, 
							color.v_mk_color);
			}
			break;
		case 4:	//exit
			return;
		default:
			ak_sleep_ms(500);
			break;
		}
		
		menu_no = -1;
	}
}

/**
 * text_vpss_effect: effect test
 * @void
 * return: void
 * notes:
 */
static void text_vpss_effect(void)
{
	int menu_no = -1;
	int value = -1;
	int ret = -1;

	while (1){
		show_vpss_effect_menu();
		ak_print_normal("please select your function number:\n");
		if(scanf("%d", &menu_no) <= 0){
			getchar();
			continue;
		}
		while (getchar() != '\n');

		switch(menu_no){
		case VPSS_EFFECT_HUE:			//set hue
		case VPSS_EFFECT_BRIGHTNESS:	//set brightness
		case VPSS_EFFECT_SATURATION:	//set saturation
		case VPSS_EFFECT_CONTRAST:		//set contrast
		case VPSS_EFFECT_SHARP:			//set sharp
		case VPSS_EFFECT_WDR:			//set wdr
		case VPSS_STYLE_ID:				//set style id
		case VPSS_POWER_HZ:				//set power hz
			ak_print_normal("please set your value:\n");
			if(scanf("%d", &value) <= 0){
				getchar();
				continue;
			}
			while (getchar() != '\n');
			
			if ((VPSS_STYLE_ID == menu_no) && ((value > 2) || (value < 0)))
			{
				ak_print_error("value error, style id [0, 2]!\n");
				continue;
			}
			
			ak_print_normal("your input value is: %d\n", value);
			
			ret = ak_vpss_effect_set((void *)1, menu_no, value);
			if (ret) {
				ak_print_error_ex("set effect failed\n");
			}
			break;
		case 8:		//get hue
		case 9:		//get brightness
		case 10:		//get saturation
		case 11:	//get contrast
		case 12:	//get sharp
		case 13:	//get wdr
		case 14:	//get style id
		case 15:	//get power hz
			ret = ak_vpss_effect_get((void *)1, menu_no - 8, &value);
			if (ret) {
				ak_print_error_ex("get effect failed\n");
			}else {
				ak_print_normal("the value is: %d\n", value);
			}
			break;
		case 16:	//exit
			return;
		default:
			ak_sleep_ms(500);
			break;
		}
		
		menu_no = -1;
		value = -1;
		
	}
}

/**
 * test_vpss: vpss test main function
 * @void
 * return: void
 * notes:
 */
static void test_vpss(void)
{
	int menu_no = -1;
	
	/* get user set input info */
	while (1) {
		show_vpss_menu();
		ak_print_normal("please select your function number:\n");
		if(scanf("%d", &menu_no) <= 0){
			getchar();
			continue;
		}
		while (getchar() != '\n');

		switch(menu_no){
		case 0:	//MD
			text_vpss_md();
			break;
		case 1:	//Mask
			text_vpss_mask();
			break;
		case 2:	//OD
			text_vpss_od();
			break;
		case 3:	//Effect
			text_vpss_effect();
			break;
		case 4:	//exit
			goto vpss_end;
		default:
			ak_sleep_ms(500);
			break;
		}

		menu_no = -1;
	}

vpss_end:
	ak_print_normal("%s stopped\n", __func__);
}


int main(int argc, char **argv)
{
	ak_print_normal("**********vpss demo begin************\n\n");

	/* 
	* this demo is running with anyka_ipc,
	* here we only init isp sdk , don't init vi 
	*/
	ak_vpss_isp_init_sdk();

	/* 
	* vpss test main function
	*/
	test_vpss();

	ak_vpss_isp_exit_sdk();

	ak_print_normal("**********vpss demo finish************\n\n");

	return 0;
}
