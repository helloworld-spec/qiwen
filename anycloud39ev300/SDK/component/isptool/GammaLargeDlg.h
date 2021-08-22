#if !defined(AFX_GAMMALARGEDLG_H__0E988AF7_9C73_4314_B44F_15D87DF50D0E__INCLUDED_)
#define AFX_GAMMALARGEDLG_H__0E988AF7_9C73_4314_B44F_15D87DF50D0E__INCLUDED_

#include "CurveLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GammaLargeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGammaLargeDlg dialog

class CGammaLargeDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveLargeDlg m_CurveDlg;
	CGammaLargeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGammaLargeDlg)
	enum { IDD = IDD_DIALOG_GAMMALARGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	int SetGammaEnable(BOOL bEnable);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGammaLargeDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGammaLargeDlg)
	afx_msg void OnClose();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bGammaEnable;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMMALARGEDLG_H__0E988AF7_9C73_4314_B44F_15D87DF50D0E__INCLUDED_)
