#if !defined(AFX_AWB_G_WEIGHTDLG_H__44563E43_6D91_4626_9593_B281CADCFF51__INCLUDED_)
#define AFX_AWB_G_WEIGHTDLG_H__44563E43_6D91_4626_9593_B281CADCFF51__INCLUDED_

#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AWB_G_weightDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAWB_G_weightDlg dialog

class CAWB_G_weightDlg : public CDialog
{
// Construction
public:
	CAWB_G_weightDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAWB_G_weightDlg)
	enum { IDD = IDD_DIALOG_AWB_G_WEIGHT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAWB_G_weightDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

public:
	AK_ISP_AWB_ATTR  m_isp_awb_G_weight;


	void GetDataValue(void);
	void SetDataValue(void);
	int SetPageInfoSt_G_weight(void * pPageInfoSt, int nStLen);
	int GetPageInfoSt_G_weight(void * pPageInfoSt, int & nStLen);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAWB_G_weightDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AWB_G_WEIGHTDLG_H__44563E43_6D91_4626_9593_B281CADCFF51__INCLUDED_)
