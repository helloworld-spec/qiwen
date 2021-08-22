#if !defined(AFX_CCM_RGBVALDLG_H__B877C5A6_03E3_4AB7_8A4B_DCEA93CF2174__INCLUDED_)
#define AFX_CCM_RGBVALDLG_H__B877C5A6_03E3_4AB7_8A4B_DCEA93CF2174__INCLUDED_
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCM_RGBvalDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCM_RGBvalDlg dialog

class CCCM_RGBvalDlg : public CDialog
{
// Construction
public:
	CCCM_RGBvalDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCM_RGBvalDlg)
	enum { IDD = IDD_DIALOG_CCM_RGBVAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	
	BOOL first_show_flag;
	T_U8 R_in[4][6];
	T_U8 G_in[4][6];
	T_U8 B_in[4][6];

	T_U8 R_tar[4][6];
	T_U8 G_tar[4][6];
	T_U8 B_tar[4][6];

	T_U8 R_out[4][6];
	T_U8 G_out[4][6];
	T_U8 B_out[4][6];

	void SetDataValue();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCM_RGBvalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCCM_RGBvalDlg)
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCM_RGBVALDLG_H__B877C5A6_03E3_4AB7_8A4B_DCEA93CF2174__INCLUDED_)
