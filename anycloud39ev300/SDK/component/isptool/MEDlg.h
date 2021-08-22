#if !defined(AFX_MEDLG_H__47130D11_B258_44AB_9539_A8792C3AA6CD__INCLUDED_)
#define AFX_MEDLG_H__47130D11_B258_44AB_9539_A8792C3AA6CD__INCLUDED_
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MEDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMEDlg dialog

class CMEDlg : public CDialog
{
// Construction
public:
	CMEDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMEDlg)
	enum { IDD = IDD_DIALOG_ME };
	CSpinButtonCtrl	m_h_framerate;
	CSpinButtonCtrl	m_h_max_exp_time;
	CSpinButtonCtrl	m_h_to_l_gain;
	CSpinButtonCtrl	m_l_framerate;
	CSpinButtonCtrl	m_l_max_exp_time;
	CSpinButtonCtrl	m_l_to_h_gain;

	CSpinButtonCtrl	m_m_framerate;
	CSpinButtonCtrl	m_m_max_exp_time;
	CSpinButtonCtrl	m_m_to_l_gain;
	//}}AFX_DATA

	AK_ISP_FRAME_RATE_ATTR m_FrameRate;
	CString	m_title;
	void GetDataValue(void);
	void SetDataValue(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMEDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMEDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDLG_H__47130D11_B258_44AB_9539_A8792C3AA6CD__INCLUDED_)
