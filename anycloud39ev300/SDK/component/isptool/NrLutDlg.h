#if !defined(AFX_NRLUTDLG_H__9238144D_52DE_44E1_AE95_4FC7C3F565F0__INCLUDED_)
#define AFX_NRLUTDLG_H__9238144D_52DE_44E1_AE95_4FC7C3F565F0__INCLUDED_

#include "CurveDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NrLutDlg.h : header file
//
#define NR_LUT_LEVEL_NUM	17
/////////////////////////////////////////////////////////////////////////////
// CNrLutDlg dialog

class CNrLutDlg : public CDialog
{
// Construction
public:
	CCurveDlg m_CurveDlg;
	CNrLutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNrLutDlg)
	enum { IDD = IDD_DIALOG_NR_LUT };
		// NOTE: the ClassWizard will add data members here
	int m_lutmode;
	//}}AFX_DATA
	unsigned short m_lc_lut[11][NR_LUT_LEVEL_NUM]; 
	vector<CPoint> m_lut_keyPts[10];


	void ChangeCurve(void);
	void SaveKeyPts(void);
	void SetKeyPts(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNrLutDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNrLutDlg)
		afx_msg void OnClose();
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio0();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	afx_msg void OnRadio10();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NRLUTDLG_H__9238144D_52DE_44E1_AE95_4FC7C3F565F0__INCLUDED_)
