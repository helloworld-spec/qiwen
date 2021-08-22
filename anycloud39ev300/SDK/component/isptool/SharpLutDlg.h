#if !defined(AFX_SHARPLUTDLG_H__4E61FA2D_0694_45FA_BD80_D3A93B51528C__INCLUDED_)
#define AFX_SHARPLUTDLG_H__4E61FA2D_0694_45FA_BD80_D3A93B51528C__INCLUDED_

#include "LineDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SharpLutDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSharpLutDlg dialog

class CSharpLutDlg : public CDialog
{
// Construction
public:
	CLineDlg m_HfLutDlg;
	CLineDlg m_MfLutDlg;
	CSharpLutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSharpLutDlg)
	enum { IDD = IDD_DIALOG_SHARP_LUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSharpLutDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSharpLutDlg)
	afx_msg void OnClose();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHARPLUTDLG_H__4E61FA2D_0694_45FA_BD80_D3A93B51528C__INCLUDED_)
