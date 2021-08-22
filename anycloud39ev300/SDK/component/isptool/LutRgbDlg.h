#if !defined(AFX_LUTRGBDLG_H__1F97E778_6DA2_4856_A1BD_12B154562F75__INCLUDED_)
#define AFX_LUTRGBDLG_H__1F97E778_6DA2_4856_A1BD_12B154562F75__INCLUDED_
#include "CurveDlg.h"	// Added by ClassView
#include "basepage.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LutRgbDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLutRgbDlg dialog

class CLutRgbDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveDlg m_RDlg;
	CCurveDlg m_GDlg;
	CCurveDlg m_BDlg;
	CLutRgbDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLutRgbDlg)
	enum { IDD = IDD_DIALOG_LUT_RGB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	virtual int SetPageInfoSt_level(void * pPageInfoSt, int nStLen);
	int SetGammaEnable(BOOL bEnable);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLutRgbDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLutRgbDlg)
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

#endif // !defined(AFX_LUTRGBDLG_H__1F97E778_6DA2_4856_A1BD_12B154562F75__INCLUDED_)
