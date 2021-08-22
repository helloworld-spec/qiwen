#if !defined(AFX_WB_EX_ATTR_H__D99C28BE_139B_4C6C_85A2_C715F53868A3__INCLUDED_)
#define AFX_WB_EX_ATTR_H__D99C28BE_139B_4C6C_85A2_C715F53868A3__INCLUDED_

#include "isp_struct.h"
#include "basepage.h"



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WB_EX_Attr.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// WB_EX_Attr dialog

class WB_EX_Attr : public CDialog, public CBasePage
{
// Construction
public:
	WB_EX_Attr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(WB_EX_Attr)
	enum { IDD = IDD_DIALOG_WB_EX_ATTR };

	//}}AFX_DATA
	
	CSpinButtonCtrl	m_wb_ex_spin_rgain_min[10];
	CSpinButtonCtrl	m_wb_ex_spin_rgain_max[10];
	CSpinButtonCtrl	m_wb_ex_spin_rgain_ex[10];
	CSpinButtonCtrl	m_wb_ex_spin_ggain_min[10];
	CSpinButtonCtrl	m_wb_ex_spin_ggain_max[10];
	CSpinButtonCtrl	m_wb_ex_spin_bgain_min[10];
	CSpinButtonCtrl	m_wb_ex_spin_bgain_max[10];
	CSpinButtonCtrl	m_wb_ex_spin_bgain_ex[10];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WB_EX_Attr)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	AK_ISP_AWB_EX_ATTR     m_isp_awb_ex_attr;

	void SetDataValue(void);
	void GetDataValue(void);


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WB_EX_Attr)

	afx_msg void OnCheckWbExCtrlEnable();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WB_EX_ATTR_H__D99C28BE_139B_4C6C_85A2_C715F53868A3__INCLUDED_)
