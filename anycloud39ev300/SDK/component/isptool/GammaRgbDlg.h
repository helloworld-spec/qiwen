#if !defined(AFX_GAMMARGBDLG_H__6843D7BE_1B7B_4B19_9C3E_1903C8ADC724__INCLUDED_)
#define AFX_GAMMARGBDLG_H__6843D7BE_1B7B_4B19_9C3E_1903C8ADC724__INCLUDED_
#include "CurveDlg.h"	// Added by ClassView
#include "basepage.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GammaRgbDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGammaRgbDlg dialog

class CGammaRgbDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveDlg m_RDlg;
	CCurveDlg m_GDlg;
	CCurveDlg m_BDlg;
	CGammaRgbDlg(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CGammaRgbDlg)
	enum { IDD = IDD_DIALOG_GAMMA_RGB };
	//}}AFX_DATA
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	virtual int SetPageInfoSt_level(void * pPageInfoSt, int nStLen);
	int SetGammaEnable(BOOL bEnable);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGammaRgbDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGammaRgbDlg)
	afx_msg void OnClose();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bGammaEnable;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMMARGBDLG_H__6843D7BE_1B7B_4B19_9C3E_1903C8ADC724__INCLUDED_)
