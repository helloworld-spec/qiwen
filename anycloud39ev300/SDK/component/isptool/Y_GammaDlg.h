#if !defined(AFX_Y_GAMMADLG_H__AF6A19B0_EECA_402A_A0B2_C64A075D563F__INCLUDED_)
#define AFX_Y_GAMMADLG_H__AF6A19B0_EECA_402A_A0B2_C64A075D563F__INCLUDED_
#include "basepage.h"
#include "CurveDlg.h"	// Added by ClassView
#include "Y_GammaParmDlg.h"	// Added by ClassView
#include "Y_GammaLargeDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Y_GammaDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CY_GammaDlg dialog

class CY_GammaDlg : public CDialog, public CBasePage
{
// Construction
public:
	CY_GammaLargeDlg m_LargeDlg;
	CY_GammaParmDlg m_ParmDlg;
	CCurveDlg m_CurveDlg;
	CY_GammaDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CY_GammaDlg)
	enum { IDD = IDD_DIALOG_Y_GAMMA };

	//}}AFX_DATA

	AK_ISP_INIT_Y_GAMMA	m_y_gamma;
	T_U16 m_default_lut[129];
	
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void SetDataValue(void);
	void GetDataValue(void);
	void Clean(void);
	int GetDefaultData(AK_ISP_INIT_Y_GAMMA *struct_new);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CY_GammaDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CY_GammaDlg)
	afx_msg void OnButton1();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonSave();
	afx_msg void OnButtonRead();
	afx_msg void OnButton5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Y_GAMMADLG_H__AF6A19B0_EECA_402A_A0B2_C64A075D563F__INCLUDED_)
