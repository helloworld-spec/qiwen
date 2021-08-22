#if !defined(AFX_RAWLUTDLG_H__A5B9B6EB_9C3B_416F_B8F2_07B5D9D4265A__INCLUDED_)
#define AFX_RAWLUTDLG_H__A5B9B6EB_9C3B_416F_B8F2_07B5D9D4265A__INCLUDED_
#include "basepage.h"
#include "CurveDlg.h"	// Added by ClassView
#include "LutRgbDlg.h"	// Added by ClassView
#include "RawLutLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RawlutDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CRawlutDlg dialog

class CRawlutDlg : public CDialog, public CBasePage
{
// Construction
public:
	CRawLutLargeDlg m_LargeDlg;
	CLutRgbDlg m_RgbDlg;
	CCurveDlg m_CurveDlg;

	CRawlutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRawlutDlg)
	enum { IDD = IDD_DIALOG_RAWLUT };
	CButton	m_GammaCheck;
	//}}AFX_DATA
	int SetGammaEnable(BOOL bEnable);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	BOOL IsSetApart();
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRawlutDlg)
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRawlutDlg)
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

#endif // !defined(AFX_RAWLUTDLG_H__A5B9B6EB_9C3B_416F_B8F2_07B5D9D4265A__INCLUDED_)
