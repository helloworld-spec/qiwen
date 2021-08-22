#!/bin/sh

# Current Script Path
PWD=`dirname $0`
LOGFILE=${PWD}/Libraries_Version.csv

# $1=KEYWORD $2=FILE
function scan_ver ()
{
    grep "$2" "$1" | grep -E "define|char" | awk -F "[\"\"]" '{print $2}'
}

# $1=LIB $2=KEYWORD $3=FILE
function match_ver () {

	a=`scan_ver "$2" "$3"`;
	if [ a.length > 0 ]; then
		# Match the indicated library
		if [ -e $1 ]; then
			b=`grep "$a" "$1" `;
			if [[ $b =~ "Binary file" && $b =~ "matches" ]]; then
				echo $a
			elif [[ $b =~ "$a" ]]; then
				echo $a
			else
				echo "<verion mismatch>"
			fi
		else
			echo "<file not found>"
		fi
	else
		echo "<verion not found>"
	fi

}

# $1=LIB $2=KEYWORD $3=FILE
function dump_ver () {

	a=`match_ver "$1" "$2" "$3"`;
	key=`basename "$1"`
	
	printf "%32s = %s\r\n" "${key}" "$a"
	echo "${key}, $a" >> ${LOGFILE}
	
}

echo "" > ${LOGFILE}

### ai
dump_ver ${PWD}/platform/libplat/lib/libplat_ai.a       "${PWD}/platform/libplat/src/ai/ak_ai.c"            "ai_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_ai.so      "${PWD}/platform/libplat/src/ai/ak_ai.c"            "ai_version"
### ao   
dump_ver ${PWD}/platform/libplat/lib/libplat_ao.a       "${PWD}/platform/libplat/src/ao/ak_ao.c"            "ao_version"          
dump_ver ${PWD}/platform/libplat/lib/libplat_ao.so      "${PWD}/platform/libplat/src/ao/ak_ao.c"            "ao_version" 
### common   
dump_ver ${PWD}/platform/libplat/lib/libplat_common.a   "${PWD}/platform/libplat/src/common/ak_common.c"    "common_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_common.so  "${PWD}/platform/libplat/src/common/ak_common.c"    "common_version"
### drv_ir   
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.a      "${PWD}/platform/libplat/src/drv/ak_drv_ir.c"       "drv_ir_version"      
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.so     "${PWD}/platform/libplat/src/drv/ak_drv_ir.c"       "drv_ir_version"
### drv_key
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.a      "${PWD}/platform/libplat/src/drv/ak_drv_key.c"      "drv_key_version"      
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.so     "${PWD}/platform/libplat/src/drv/ak_drv_key.c"      "drv_key_version"
### drv_ptz
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.a      "${PWD}/platform/libplat/src/drv/ak_drv_ptz.c"      "drv_ptz_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.so     "${PWD}/platform/libplat/src/drv/ak_drv_ptz.c"      "drv_ptz_version"
### drv_wdt
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.a      "${PWD}/platform/libplat/src/drv/ak_drv_wdt.c"      "drv_wdt_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_drv.so     "${PWD}/platform/libplat/src/drv/ak_drv_wdt.c"      "drv_wdt_version"
### its
dump_ver ${PWD}/platform/libplat/lib/libplat_its.a      "${PWD}/platform/libplat/src/its/ak_its.c"          "its_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_its.so     "${PWD}/platform/libplat/src/its/ak_its.c"          "its_version"
### thread
dump_ver ${PWD}/platform/libplat/lib/libplat_thread.a   "${PWD}/platform/libplat/src/thread/ak_thread.c"    "thread_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_thread.so  "${PWD}/platform/libplat/src/thread/ak_thread.c"    "thread_version"
### vi
dump_ver ${PWD}/platform/libplat/lib/libplat_vi.a       "${PWD}/platform/libplat/src/vi/ak_vi.c"            "vi_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_vi.so      "${PWD}/platform/libplat/src/vi/ak_vi.c"            "vi_version"
### vpss
dump_ver ${PWD}/platform/libplat/lib/libplat_vpss.a     "${PWD}/platform/libplat/src/vpss/ak_vpss_public.c"        "vpss_version"
dump_ver ${PWD}/platform/libplat/lib/libplat_vpss.so    "${PWD}/platform/libplat/src/vpss/ak_vpss_public.c"        "vpss_version"
### adec
dump_ver ${PWD}/platform/libmpi/lib/libmpi_adec.a       "${PWD}/platform/libmpi/src/adec/ak_adec.c"         "adec_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_adec.so      "${PWD}/platform/libmpi/src/adec/ak_adec.c"         "adec_version"
### aenc
dump_ver ${PWD}/platform/libmpi/lib/libmpi_aenc.a       "${PWD}/platform/libmpi/src/aenc/ak_aenc.c"         "aenc_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_aenc.so      "${PWD}/platform/libmpi/src/aenc/ak_aenc.c"         "aenc_version"
### md(hardware)
dump_ver ${PWD}/platform/libmpi/lib/libmpi_md.a         "${PWD}/platform/libmpi/src/md/ak_md.c"             "md_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_md.so        "${PWD}/platform/libmpi/src/md/ak_md.c"             "md_version"
### muxer
dump_ver ${PWD}/platform/libmpi/lib/libmpi_muxer.a      "${PWD}/platform/libmpi/src/muxer/ak_mux.c"         "muxer_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_muxer.so     "${PWD}/platform/libmpi/src/muxer/ak_mux.c"         "muxer_version"
### osd
dump_ver ${PWD}/platform/libmpi/lib/libmpi_osd.a        "${PWD}/platform/libmpi/src/osd/ak_osd.c"           "osd_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_osd.so       "${PWD}/platform/libmpi/src/osd/ak_osd.c"           "osd_version"
### venc
dump_ver ${PWD}/platform/libmpi/lib/libmpi_venc.a       "${PWD}/platform/libmpi/src/venc/ak_venc.c"         "venc_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_venc.so      "${PWD}/platform/libmpi/src/venc/ak_venc.c"         "venc_version"
### aed
dump_ver ${PWD}/platform/libmpi/lib/libmpi_aed.a        "${PWD}/platform/libmpi/src/aed/ak_aed.c"           "aed_version"
dump_ver ${PWD}/platform/libmpi/lib/libmpi_aed.so       "${PWD}/platform/libmpi/src/aed/ak_aed.c"           "aed_version"
### alarm
dump_ver ${PWD}/platform/libapp/lib/libapp_alarm.a      "${PWD}/platform/libapp/src/alarm/ak_alarm.c"       "alarm_version"
dump_ver ${PWD}/platform/libapp/lib/libapp_alarm.so     "${PWD}/platform/libapp/src/alarm/ak_alarm.c"       "alarm_version"
### ini
dump_ver ${PWD}/platform/libapp/lib/libapp_ini.a        "${PWD}/platform/libapp/src/ini/ak_ini.c"           "ini_version"
dump_ver ${PWD}/platform/libapp/lib/libapp_ini.so       "${PWD}/platform/libapp/src/ini/ak_ini.c"           "ini_version"
### dvr
dump_ver ${PWD}/platform/libapp/lib/libapp_dvr.a        "${PWD}/platform/libapp/src/dvr/ak_dvr_record.c"    "dvr_version"
dump_ver ${PWD}/platform/libapp/lib/libapp_dvr.so       "${PWD}/platform/libapp/src/dvr/ak_dvr_record.c"    "dvr_version"
### net
dump_ver ${PWD}/platform/libapp/lib/libapp_net.a        "${PWD}/platform/libapp/src/net/ak_net.c"           "net_version"
dump_ver ${PWD}/platform/libapp/lib/libapp_net.so       "${PWD}/platform/libapp/src/net/ak_net.c"           "net_version"
### osd_ex
dump_ver ${PWD}/platform/libapp/lib/libapp_osd_ex.a     "${PWD}/platform/libapp/src/osd_ex/ak_osd_ex.c"      "osd_ex_version"
dump_ver ${PWD}/platform/libapp/lib/libapp_osd_ex.so    "${PWD}/platform/libapp/src/osd_ex/ak_osd_ex.c"      "osd_ex_version"
### rtsp
dump_ver ${PWD}/platform/libapp/lib/libapp_rtsp.a       "${PWD}/platform/libapp/src/rtsp/ak_rtsp.c"      "rtsp_version" 
dump_ver ${PWD}/platform/libapp/lib/libapp_rtsp.so      "${PWD}/platform/libapp/src/rtsp/ak_rtsp.c"      "rtsp_version"      

### Video Encoder
dump_ver ${PWD}/platform/libmpi/lib/libakv_encode.so    "${PWD}/platform/libmpi/src/include/akv_interface.h"      "VIDEO_LIB_VERSION"  
dump_ver ${PWD}/platform/libmpi/lib/libakv_encode.a     "${PWD}/platform/libmpi/src/include/akv_interface.h"      "VIDEO_LIB_VERSION"  

### Audio Codec
dump_ver ${PWD}/platform/libmpi/lib/libakaudiocodec.so       "${PWD}/platform/libmpi/src/include/sdcodec.h"      "AUDIOCODEC_VERSION_STRING"  
dump_ver ${PWD}/platform/libmpi/lib/libakaudiocodec.a        "${PWD}/platform/libmpi/src/include/sdcodec.h"      "AUDIOCODEC_VERSION_STRING"  

### Media Lib
dump_ver ${PWD}/platform/libmpi/lib/libakmedia.so       "${PWD}/platform/libmpi/src/include/medialib_struct.h"      "MEDIA_LIB_VERSION"  
dump_ver ${PWD}/platform/libmpi/lib/libakmedia.a        "${PWD}/platform/libmpi/src/include/medialib_struct.h"      "MEDIA_LIB_VERSION"  

### SD Filter
dump_ver ${PWD}/platform/libplat/lib/libakaudiofilter.so       "${PWD}/platform/libplat/include_inner/sdfilter.h"      "AUDIO_FILTER_VERSION_STRING"  
dump_ver ${PWD}/platform/libplat/lib/libakaudiofilter.a        "${PWD}/platform/libplat/include_inner/sdfilter.h"      "AUDIO_FILTER_VERSION_STRING"  
dump_ver ${PWD}/kernel/lib/libakaudiofilter_kern.a              "${PWD}/platform/libplat/include_inner/sdfilter.h"      "AUDIO_FILTER_VERSION_STRING"  

                         
                         
### 软件ipc_main
dump_ver ${PWD}/platform/apps/akipc/main/ipc_main.c     "${PWD}/platform/apps/akipc/main/ipc_main.c"     "AK_VERSION_SOFTWARE" 
### uboot

                                                                                  

