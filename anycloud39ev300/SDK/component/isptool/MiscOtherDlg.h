#if !defined(AFX_MISCOTHERDLG_H__FF95C1C2_EC2D_48FB_8D9A_E462E33A3DD0__INCLUDED_)
#define AFX_MISCOTHERDLG_H__FF95C1C2_EC2D_48FB_8D9A_E462E33A3DD0__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiscOtherDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMiscOtherDlg dialog

class CMiscOtherDlg : public CDialog, public CBasePage
{
// Construction
public:
	CMiscOtherDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMiscOtherDlg)
	enum { IDD = IDD_DIALOG_MISC_OTHER };
		// NOTE: the ClassWizard will add data members here
	CSpinButtonCtrl	m_mipi_cnt_time;
	int		m_2frm_merge_en;
	int		m_mipi_line_end_sel;
	int		m_mipi_line_end_cnt_en_cfg;

	AK_ISP_MISC_ATTR m_Misc_attr;
	//}}AFX_DATA

	void GetDataValue(void);
	void SetDataValue(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiscOtherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMiscOtherDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISCOTHERDLG_H__FF95C1C2_EC2D_48FB_8D9A_E462E33A3DD0__INCLUDED_)
