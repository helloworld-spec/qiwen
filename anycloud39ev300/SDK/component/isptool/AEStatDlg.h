#if !defined(AFX_AESTATDLG_H__F55EE9DD_18BD_472A_8B56_D2D5BB7D8C78__INCLUDED_)
#define AFX_AESTATDLG_H__F55EE9DD_18BD_472A_8B56_D2D5BB7D8C78__INCLUDED_
#include "basepage.h"
#include "anyka_types.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AEStatDlg.h : header file
//

//#define JG_PLATFORM


typedef  struct ak_isp_ae_run_info
{
	T_U8   current_calc_avg_lumi;				//现在的计算出的亮度值
	T_U8   current_calc_avg_compensation_lumi;	 //经过曝光补偿后的亮度值

	T_U8   current_darked_flag;					//白天黑夜的标记

	T_S32  current_a_gain;					   //模拟增益的值
	T_S32  current_d_gain;					  //数字增益的值
	T_S32  current_isp_d_gain;				  //isp数字增益的值
	T_S32  current_exp_time;				//曝光时间的值

	T_U32  current_a_gain_step; 			//现在的模拟增益的步长
	T_U32  current_d_gain_step; 			//数字增益的步长
	T_U32  current_isp_d_gain_step;		   //isp数字增益的步长
	T_U32  current_exp_time_step;		   //曝光时间的步长
}AK_ISP_AE_RUN_INFO;

typedef struct ak_isp_raw_hist_stat_info
{

	T_U32	raw_g_hist[256];
	T_U32	raw_g_total;
}AK_ISP_RAW_HIST_STAT_INFO;

typedef struct ak_isp_rgb_hist_stat_info
{ 
	T_U32	rgb_hist[256];
	T_U32	rgb_total;
}AK_ISP_RGB_HIST_STAT_INFO;

typedef struct	ak_isp_yuv_hist_stat_info
{
	T_U32	 y_hist[256];
	T_U32	 y_total;
}AK_ISP_YUV_HIST_STAT_INFO;


typedef struct  ak_isp_ae_stat_info
{

	AK_ISP_AE_RUN_INFO			run_info;       
	AK_ISP_RAW_HIST_STAT_INFO	raw_hist;      
	AK_ISP_RGB_HIST_STAT_INFO	rgb_hist;    
	AK_ISP_YUV_HIST_STAT_INFO	yuv_hist; 
#ifdef JG_PLATFORM
	T_U32						scene_mode;
#endif
}AK_ISP_AE_STAT_INFO;

/////////////////////////////////////////////////////////////////////////////
// CAEStatDlg dialog

class CAEStatDlg : public CDialog, public CBasePage
{
// Construction
public:
	CAEStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAEStatDlg();
// Dialog Data
	//{{AFX_DATA(CAEStatDlg)
	enum { IDD = IDD_DIALOG_AE_STAT };
	int		m_hist_id;
	int		m_hist_mode;
	//}}AFX_DATA

	UINT m_timer;
	AK_ISP_AE_STAT_INFO m_AEStat;
	CRect m_HistWindowRect;
	CRect m_HistRect;
	CRect m_HistBarRect;
	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;
	CBitmap m_GrayBarBitmap;

	HANDLE m_semaphore;
	bool m_bInit;
	AK_ISP_INIT_EXP m_Exp;
	
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void SetDataValue(void);
	void DrawHistoGram(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAEStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAEStatDlg)
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AESTATDLG_H__F55EE9DD_18BD_472A_8B56_D2D5BB7D8C78__INCLUDED_)
