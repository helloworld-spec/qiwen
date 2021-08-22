#if !defined(AFX_LSCDLG_H__E23F6462_40F8_4F87_9A75_E83553BE2347__INCLUDED_)
#define AFX_LSCDLG_H__E23F6462_40F8_4F87_9A75_E83553BE2347__INCLUDED_
#include "basepage.h"
#include "LSCCOEFDlg.h"	// Added by ClassView
#include "LscBrightnessCalcDlg.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LscDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLscDlg dialog

class CLscDlg : public CDialog, public CBasePage
{
// Construction
public:
	CLSCCOEFDlg m_LscCoefDlg;
	CLscBrightnessCalcDlg m_BrightnessCalcDlg;
	CLscDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLscDlg)
	enum { IDD = IDD_DIALOG_LSC };
	CSpinButtonCtrl	m_xref;
	CSpinButtonCtrl	m_yref;
	CSpinButtonCtrl	m_shift;
	CSpinButtonCtrl m_Strength_spin;

	int		m_Enable;
	//}}AFX_DATA
	CSpinButtonCtrl	m_range[10];
	AK_ISP_INIT_LSC m_Lsc;
	bool m_bConnect;
	int m_Strength;
	
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);
	void SetConnectState(bool bConnect);
	void refresh_data(void);
	void save_calc_result(void);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLscDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLscDlg)
	afx_msg void OnButtonLsc();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnButtonLscCalc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LSCDLG_H__E23F6462_40F8_4F87_9A75_E83553BE2347__INCLUDED_)
