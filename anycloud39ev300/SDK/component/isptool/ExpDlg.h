#if !defined(AFX_EXPDLG_H__A6BA5840_D9BF_4B9A_9660_AE5334A7FED4__INCLUDED_)
#define AFX_EXPDLG_H__A6BA5840_D9BF_4B9A_9660_AE5334A7FED4__INCLUDED_
#include "basepage.h"
#include "MEDlg.h"	// Added by ClassView
#include "isp_struct.h"
#include "AEOtherDlg.h"	// Added by ClassView
#include "isp_struct_v2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExpDlg.h : header file
//
 
/////////////////////////////////////////////////////////////////////////////
// CExpDlg dialog

class CExpDlg : public CDialog, public CBasePage
{
// Construction
public:
	CAEOtherDlg m_AEOtherDlg;
	CMEDlg m_MEDlg;
	CExpDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_DIALOG_EXP };
	CSpinButtonCtrl	m_target_lumiance;
	CSpinButtonCtrl	m_stable_range;
	CSpinButtonCtrl	m_step;
	CSpinButtonCtrl	m_a_gain_min;
	CSpinButtonCtrl	m_a_gain_max;
	CSpinButtonCtrl	m_isp_d_gain_min;
	CSpinButtonCtrl	m_isp_d_gain_max;
	CSpinButtonCtrl	m_d_gain_min;
	CSpinButtonCtrl	m_d_gain_max;
	CSpinButtonCtrl	m_exp_time_min;
	CSpinButtonCtrl	m_exp_time_max;
	int		m_raw_hist_enable;
	int		m_rgb_hist_enable;
	int		m_yuv_hist_enable;
	int		m_exp_mode;
	UINT	m_exp_time_max_value;
	UINT	m_a_gain_max_value;
	UINT	m_d_gain_max_value;
	UINT	m_isp_d_gain_max_value;
	//}}AFX_DATA

	AK_ISP_INIT_EXP m_Exp;


	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_EXP *struct_new, AK_ISP_INIT_EXP_V2* struct_v2); 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExpDlg)
	afx_msg void OnButtonManualExp();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	afx_msg void OnButtonOther();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPDLG_H__A6BA5840_D9BF_4B9A_9660_AE5334A7FED4__INCLUDED_)
