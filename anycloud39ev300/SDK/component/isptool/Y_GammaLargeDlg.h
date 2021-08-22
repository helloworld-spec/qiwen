#if !defined(AFX_Y_GAMMALARGEDLG_H__8FEBAD63_4523_4A66_99D6_1A332F48C12D__INCLUDED_)
#define AFX_Y_GAMMALARGEDLG_H__8FEBAD63_4523_4A66_99D6_1A332F48C12D__INCLUDED_

#include "CurveLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Y_GammaLargeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CY_GammaLargeDlg dialog

class CY_GammaLargeDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveLargeDlg m_CurveDlg;
	CY_GammaLargeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CY_GammaLargeDlg)
	enum { IDD = IDD_DIALOG_Y_GAMMALARGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CY_GammaLargeDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CY_GammaLargeDlg)
	afx_msg void OnClose();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Y_GAMMALARGEDLG_H__8FEBAD63_4523_4A66_99D6_1A332F48C12D__INCLUDED_)
