#if !defined(AFX_WDRLUTDLG_H__AE763134_F683_426F_9D24_67F36DF4FDD5__INCLUDED_)
#define AFX_WDRLUTDLG_H__AE763134_F683_426F_9D24_67F36DF4FDD5__INCLUDED_

#include "CurveDlg.h"	// Added by ClassView
#include "basepage.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WdrLutDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWdrLutDlg dialog


#define WDR_LUT_LEVEL_NUM	65

class CWdrLutDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCurveDlg m_Area1Dlg;
	CCurveDlg m_Area2Dlg;
	CCurveDlg m_Area3Dlg;
	CCurveDlg m_Area4Dlg;
	CCurveDlg m_Area5Dlg;
	CCurveDlg m_Area6Dlg;
	CWdrLutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWdrLutDlg)
	enum { IDD = IDD_DIALOG_WDR_LUT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWdrLutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWdrLutDlg)
	afx_msg void OnClose();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDRLUTDLG_H__AE763134_F683_426F_9D24_67F36DF4FDD5__INCLUDED_)
