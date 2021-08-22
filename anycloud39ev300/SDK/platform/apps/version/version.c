#include <stdio.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_its.h"
#include "ak_drv_ir.h"
#include "ak_drv_irled.h"
#include "ak_drv_key.h"
#include "ak_drv_ptz.h"
#include "ak_drv_wdt.h"
#include "ak_ipc_srv.h"
#include "ak_tw.h"

#include "ak_adec.h"
#include "ak_aenc.h"
#include "ak_venc.h"
#include "ak_md.h"
#include "ak_aed.h"
#include "ak_osd.h"
#include "ak_mux.h"

#include "ak_ini.h"
#include "ak_dvr_record.h"
#include "ak_alarm.h"
#include "ak_net.h"
#include "ak_osd_ex.h"
#include "ak_rtsp.h"

int main(int argc, char **argv)
{
	ak_print_normal("\n****************************************\n");
	ak_print_normal("cloud39EV300 PDK V1 lib version info\n");
	ak_print_normal("%s %s\n\n", __DATE__, __TIME__);

	/* plat level */
	ak_print_normal("%s\n", ak_common_get_version());
	ak_print_normal("%s\n", ak_thread_get_version());
	ak_print_normal("%s\n", ak_ai_get_version());
	ak_print_normal("%s\n", ak_ao_get_version());
	ak_print_normal("%s\n", ak_vi_get_version());
	ak_print_normal("%s\n", ak_vpss_get_version());
	ak_print_normal("%s\n", ak_its_get_version());
	ak_print_normal("%s\n", ak_drv_ir_get_version());
	ak_print_normal("%s\n", ak_drv_irled_get_version());
	ak_print_normal("%s\n", ak_drv_key_get_version());
	ak_print_normal("%s\n", ak_drv_ptz_get_version());
	ak_print_normal("%s\n", ak_drv_wdt_get_version());
	ak_print_normal("%s\n", ak_thread_get_version());
	ak_print_normal("%s\n", ak_ipcsrv_get_version());
	ak_print_normal("%s\n", ak_tw_get_version());
	ak_print_normal("\n");

	/* mpi level */
	ak_print_normal("%s\n", ak_adec_get_version());
	ak_print_normal("%s\n", ak_aenc_get_version());
	ak_print_normal("%s\n", ak_venc_get_version());
	ak_print_normal("%s\n", ak_md_get_version());
	ak_print_normal("%s\n", ak_aed_get_version());
	ak_print_normal("%s\n", ak_osd_get_version());
	ak_print_normal("%s\n", ak_muxer_get_version());
	ak_print_normal("\n");

	/* libapp level */
	ak_print_normal("%s\n", ak_alarm_get_version());
	ak_print_normal("%s\n", ak_dvr_record_get_version());
	ak_print_normal("%s\n", ak_ini_get_version());
	ak_print_normal("%s\n", ak_net_get_version());
	ak_print_normal("%s\n", ak_osd_ex_get_version());
	ak_print_normal("%s\n", ak_rtsp_get_version());

	ak_print_normal("****************************************\n\n");
	return 0;
}
