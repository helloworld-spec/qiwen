#if !defined(AFX_FCSLINKAGEDLG_H__5B43415D_9683_4002_9402_FF5F7E02253D__INCLUDED_)
#define AFX_FCSLINKAGEDLG_H__5B43415D_9683_4002_9402_FF5F7E02253D__INCLUDED_
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FcsLinkageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFcsLinkageDlg dialog

class CFcsLinkageDlg : public CDialog
{
// Construction
public:
	CFcsLinkageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFcsLinkageDlg)
	enum { IDD = IDD_DIALOG_FCS_LINKAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	int m_Enable[9];
	int m_uv_Enable[9];
	CSpinButtonCtrl	m_th[9];
	CSpinButtonCtrl	m_gain_slop[9];
	CSpinButtonCtrl	m_uv_nr_th[9];

	AK_ISP_FCS	m_linkage[9];

	void GetDataValue(void);
	void SetDataValue(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFcsLinkageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFcsLinkageDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio39();
	afx_msg void OnRadio72();
	afx_msg void OnRadio19();
	afx_msg void OnRadio20();
	afx_msg void OnRadio73();
	afx_msg void OnRadio74();
	afx_msg void OnRadio29();
	afx_msg void OnRadio30();
	afx_msg void OnRadio75();
	afx_msg void OnRadio76();
	afx_msg void OnRadio77();
	afx_msg void OnRadio78();
	afx_msg void OnRadio79();
	afx_msg void OnRadio80();
	afx_msg void OnRadio81();
	afx_msg void OnRadio82();
	afx_msg void OnRadio83();
	afx_msg void OnRadio84();
	afx_msg void OnRadio85();
	afx_msg void OnRadio86();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FCSLINKAGEDLG_H__5B43415D_9683_4002_9402_FF5F7E02253D__INCLUDED_)
