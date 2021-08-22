#if !defined(AFX_RAWLUTLARGEDLG_H__C1C684AA_DA7A_421B_A0C5_7E98A0C54C79__INCLUDED_)
#define AFX_RAWLUTLARGEDLG_H__C1C684AA_DA7A_421B_A0C5_7E98A0C54C79__INCLUDED_

#include "CurveLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RawLutLargeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRawLutLargeDlg dialog

class CRawLutLargeDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveLargeDlg m_CurveDlg;
	CRawLutLargeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRawLutLargeDlg)
	enum { IDD = IDD_DIALOG_RAWLUTLARGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	int SetGammaEnable(BOOL bEnable);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRawLutLargeDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRawLutLargeDlg)
		// NOTE: the ClassWizard will add member functions here
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

#endif // !defined(AFX_RAWLUTLARGEDLG_H__C1C684AA_DA7A_421B_A0C5_7E98A0C54C79__INCLUDED_)
