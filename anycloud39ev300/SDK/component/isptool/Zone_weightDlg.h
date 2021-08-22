#if !defined(AFX_ZONE_WEIGHTDLG_H__E53CFB5E_27EB_4EEF_A242_D0A44B2ED249__INCLUDED_)
#define AFX_ZONE_WEIGHTDLG_H__E53CFB5E_27EB_4EEF_A242_D0A44B2ED249__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Zone_weightDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CZone_weightDlg dialog

class CZone_weightDlg : public CDialog, public CBasePage
{
// Construction
public:
	CZone_weightDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CZone_weightDlg)
	enum { IDD = IDD_DIALOG_ZONE_WEIGHT };
	//}}AFX_DATA

	short m_value[8][16];
	UINT m_timer;

	AK_ISP_INIT_WEIGHT m_Weight;
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZone_weightDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CZone_weightDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZONE_WEIGHTDLG_H__E53CFB5E_27EB_4EEF_A242_D0A44B2ED249__INCLUDED_)
