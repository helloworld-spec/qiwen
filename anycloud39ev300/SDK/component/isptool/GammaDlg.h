#if !defined(AFX_GAMMADLG_H__AE1A01B9_1CB1_4774_9B99_7CB0CF27EE5B__INCLUDED_)
#define AFX_GAMMADLG_H__AE1A01B9_1CB1_4774_9B99_7CB0CF27EE5B__INCLUDED_
#include "basepage.h"
#include "CurveDlg.h"	// Added by ClassView
#include "GammaRgbDlg.h"	// Added by ClassView
#include "GammaLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GammaDlg.h : header file


/////////////////////////////////////////////////////////////////////////////
// CGammaDlg dialog

class CGammaDlg : public CDialog, public CBasePage
{
// Construction
public:
	CGammaLargeDlg m_LargeDlg;
	CGammaRgbDlg m_RgbDlg;
	CCurveDlg m_CurveDlg;
	CGammaDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGammaDlg)
	enum { IDD = IDD_DIALOG_GAMMA };
	CButton	m_GammaCheck;
	//}}AFX_DATA
	int SetGammaEnable(BOOL bEnable);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	BOOL IsSetApart();
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGammaDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CGammaDlg)
	afx_msg void OnButton1();
	afx_msg void OnCheckEnable();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bGammaEnable;
	BOOL m_bSetApart;	//r°¢g°¢b∑÷±…Ë÷√
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMMADLG_H__AE1A01B9_1CB1_4774_9B99_7CB0CF27EE5B__INCLUDED_)
