#if !defined(AFX_HUE_IMGDLG_H__2008321D_0127_43B7_B9F5_5A3AB0BA0EF8__INCLUDED_)
#define AFX_HUE_IMGDLG_H__2008321D_0127_43B7_B9F5_5A3AB0BA0EF8__INCLUDED_

#include "isp_struct.h"
#include "basepage.h"
#include "HUE_lineDlg.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HUE_ImgDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHUE_ImgDlg dialog

class CHUE_ImgDlg : public CDialog, public CBasePage
{
// Construction
public:
	CHUE_ImgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHUE_ImgDlg)
	enum { IDD = IDD_DIALOG_HUE_IMG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHUE_ImgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CHUE_lineDlg m_HUE_lineDlg;
protected:

	// Generated message map functions
	//{{AFX_MSG(CHUE_ImgDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUE_IMGDLG_H__2008321D_0127_43B7_B9F5_5A3AB0BA0EF8__INCLUDED_)
