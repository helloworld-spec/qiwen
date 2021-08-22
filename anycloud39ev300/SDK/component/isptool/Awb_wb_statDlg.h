#if !defined(AFX_AWB_WB_STATDLG_H__CD2E1E5C_CFCB_4E45_95C3_8B66D3A535D3__INCLUDED_)
#define AFX_AWB_WB_STATDLG_H__CD2E1E5C_CFCB_4E45_95C3_8B66D3A535D3__INCLUDED_
#include "Awb_wb_statDlg.h"
#include "isp_struct.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Awb_wb_statDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAwb_wb_statDlg dialog

class CAwb_wb_statDlg : public CDialog
{
// Construction
public:
	CAwb_wb_statDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAwb_wb_statDlg)
	enum { IDD = IDD_DIALOG_AWB_WB_STAT };
	CSpinButtonCtrl	m_spin_awb_y_low;
	CSpinButtonCtrl	m_spin_awb_y_high;

	CSpinButtonCtrl	m_spin_awb_step;
	CSpinButtonCtrl	m_spin_awb_stable_cnt_thre;

	CSpinButtonCtrl	m_spin_awb_cnt_thre;

	UINT	m_cnt_thre;
	//}}AFX_DATA

	CSpinButtonCtrl	m_spin_awb_rb_low[10];
	CSpinButtonCtrl	m_spin_awb_rb_high[10];
	CSpinButtonCtrl	m_spin_awb_gr_low[10];
	CSpinButtonCtrl	m_spin_awb_gr_high[10];
	CSpinButtonCtrl	m_spin_awb_gb_low[10];
	CSpinButtonCtrl	m_spin_awb_gb_high[10];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAwb_wb_statDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

public:
	AK_ISP_AWB_ATTR  m_isp_awb_wb_stat;

	BOOL open_dlg_flag;
	

	void GetDataValue(void);
	void SetDataValue(void);
	int GetPageInfoSt_wb_stat(void * pPageInfoSt, int & nStLen);
	int SetPageInfoSt_wb_stat(void * pPageInfoSt, int nStLen);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAwb_wb_statDlg)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AWB_WB_STATDLG_H__CD2E1E5C_CFCB_4E45_95C3_8B66D3A535D3__INCLUDED_)
