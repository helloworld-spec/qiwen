#if !defined(AFX_LSCCOEFDLG_H__E65B7678_69CB_4204_91D3_BE48431AFB69__INCLUDED_)
#define AFX_LSCCOEFDLG_H__E65B7678_69CB_4204_91D3_BE48431AFB69__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LSCCOEFDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLSCCOEFDlg dialog

class CLSCCOEFDlg : public CDialog, public CBasePage
{
// Construction
public:
	CLSCCOEFDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLSCCOEFDlg)
	enum { IDD = IDD_DIALOG_LSC_COEF };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	CSpinButtonCtrl	m_r_b[10];
	CSpinButtonCtrl	m_r_c[10];
	CSpinButtonCtrl	m_gr_b[10];
	CSpinButtonCtrl	m_gr_c[10];
	CSpinButtonCtrl	m_gb_b[10];
	CSpinButtonCtrl	m_gb_c[10];
	CSpinButtonCtrl	m_b_b[10];
	CSpinButtonCtrl	m_b_c[10];

	lens_coef	m_r_coef;
	lens_coef	m_gr_coef;
	lens_coef	m_gb_coef;
	lens_coef	m_b_coef;

	void GetDataValue(void);
	void SetDataValue(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLSCCOEFDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLSCCOEFDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LSCCOEFDLG_H__E65B7678_69CB_4204_91D3_BE48431AFB69__INCLUDED_)
