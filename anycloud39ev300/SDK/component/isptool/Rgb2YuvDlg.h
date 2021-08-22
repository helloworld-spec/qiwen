#if !defined(AFX_RGB2YUVDLG_H__2EF3A674_E585_464C_BA8E_5F66C55F5EEB__INCLUDED_)
#define AFX_RGB2YUVDLG_H__2EF3A674_E585_464C_BA8E_5F66C55F5EEB__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Rgb2YuvDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRgb2YuvDlg dialog

class CRgb2YuvDlg : public CDialog, public CBasePage
{
// Construction
public:
	CRgb2YuvDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRgb2YuvDlg)
	enum { IDD = IDD_DIALOG_RGB2YUV };
	int		m_mode;
	//}}AFX_DATA

	AK_ISP_INIT_RGB2YUV m_Rgb2Yuv;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRgb2YuvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRgb2YuvDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RGB2YUVDLG_H__2EF3A674_E585_464C_BA8E_5F66C55F5EEB__INCLUDED_)
