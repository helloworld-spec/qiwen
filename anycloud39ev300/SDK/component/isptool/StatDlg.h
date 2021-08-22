#if !defined(AFX_STATDLG_H__03503EB7_1268_42CC_819D_246ECC9C0B18__INCLUDED_)
#define AFX_STATDLG_H__03503EB7_1268_42CC_819D_246ECC9C0B18__INCLUDED_
#include "basepage.h"
#include "AwbStatDlg.h"	// Added by ClassView
#include "AEStatDlg.h"	// Added by ClassView
#include "AFStatDlg.h"	// Added by ClassView
#include "3DStatDlg.h"	// Added by ClassView
#include "ImgDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatDlg dialog

class CStatDlg : public CDialog, public CBasePage
{
// Construction
public:
	CImgDlg m_ImgDlg;
	C3DStatDlg m_3DStatDlg;
	CAFStatDlg m_AFStatDlg;
	CAEStatDlg m_AEStatDlg;
	CAwbStatDlg m_AwbStatDlg;
	CStatDlg(CWnd* pParent = NULL);   // standard constructor

	bool m_bConnect;

// Dialog Data
	//{{AFX_DATA(CStatDlg)
	enum { IDD = IDD_DIALOG_STAT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	void SetConnectState(bool bConnect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStatDlg)
	afx_msg void OnButtonAwb();
	afx_msg void OnButtonAe();
	afx_msg void OnButtonAf();
	afx_msg void OnButton3d();
	afx_msg void OnButtonGetYuvImg();
	afx_msg void OnButtonOpenYuvImg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATDLG_H__03503EB7_1268_42CC_819D_246ECC9C0B18__INCLUDED_)
