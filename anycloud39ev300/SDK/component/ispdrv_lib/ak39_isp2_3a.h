#ifndef _AK39_ISP2_3A_H
#define _AK39_ISP2_3A_H

#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))

/***************************************************************************/

enum ak_isp_wb_type {
    WB_OPS_TYPE_MANU=0,        //手动白平衡
    WB_OPS_TYPE_AUTO,          //自动白平衡
};

enum awb_stat_param_type
{
    STAT_NORMAL = 0,   //正常情况下的参数，
    STAT_ABNORMAL= 1,  //五组参数都失效的情况下的参数，
};

enum ae_type
{
   ISP_MANU_AE=0,    //manula exp type
   ISP_AUTO_AE,      //auto exp type
};

enum work_mode
{
   MODE_MANUAL=0,
   MODE_LINKAGE,
};

enum ae_status{
    AE_UNSTABLE=0,      //ae unstable
    AE_STABLE=1,         //ae stalble
};

/***************************************************************************/

/**
 * @brief: update white gain
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *wb_gain:  white balance param
 */
int ak_isp_updata_wb_gain(AK_ISP_WB_GAIN *wb_gain);

int _cmos_updata_d_gain(T_U32 d_gain);
int _cmos_updata_a_gain(T_U32 a_gain);
int _cmos_updata_exp_time(T_U32 exp_time);

/**
 * @brief: calc avg value
 * @author: xiepenghe
 * @date: 2016-5-06 
 */
int ak_isp_calc_avg(void);
int ae_compensation(void);
int _cmos_cam_exp_time_init(void);

int _cmos_cam_exp_time_init(void);
int _get_iso(void);

int _get_3dnr_stat(void);

/***************************************************************************/
#endif //#define _AK39_ISP2_3A_H
