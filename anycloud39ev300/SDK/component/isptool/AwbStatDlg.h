#if !defined(AFX_AWBSTATDLG_H__2D941D5E_617C_4965_9B82_86D8D3A05623__INCLUDED_)
#define AFX_AWBSTATDLG_H__2D941D5E_617C_4965_9B82_86D8D3A05623__INCLUDED_
#include "basepage.h"
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AwbStatDlg.h : header file
//

typedef  struct ak_isp_awb_stat_info 
{
	//白平衡统计结果
	T_U32 total_R[10];
	T_U32 total_G[10];
	T_U32 total_B[10];
	T_U32 total_cnt[10];
	//经由自动白平衡算法算出的白平衡增益值
	T_U16  r_gain;
	T_U16  g_gain;
	T_U16  b_gain;
	T_S16  r_offset;
	T_S16  g_offset;
	T_S16  b_offset;
	T_U16   current_colortemp_index;	 //环境色温标记，是参数随环境变化的色温指标。
	T_U16   colortemp_stable_cnt[10];         //每一种色温稳定的帧数计数

}AK_ISP_AWB_STAT_INFO;

/////////////////////////////////////////////////////////////////////////////
// CAwbStatDlg dialog

class CAwbStatDlg : public CDialog, public CBasePage
{
// Construction
public:
	CAwbStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAwbStatDlg();
// Dialog Data
	//{{AFX_DATA(CAwbStatDlg)
	enum { IDD = IDD_DIALOG_AWB_STAT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	UINT m_timer;
	AK_ISP_AWB_STAT_INFO m_AWBStat;
	HANDLE m_semaphore;
	bool m_bInit;
	
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void SetDataValue(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAwbStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAwbStatDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AWBSTATDLG_H__2D941D5E_617C_4965_9B82_86D8D3A05623__INCLUDED_)
