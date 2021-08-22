#if !defined(AFX_CHUEDLG_H__FA153565_3B74_4977_90A2_56AA2393A073__INCLUDED_)
#define AFX_CHUEDLG_H__FA153565_3B74_4977_90A2_56AA2393A073__INCLUDED_

#include "isp_struct.h"
#include "basepage.h"
#include "HUE_ImgDlg.h"
#include "HUE_CALCDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CHUEDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCHUEDlg dialog

class CCHUEDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCHUEDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCHUEDlg)
	enum { IDD = IDD_DIALOG_HUE };
	int		m_mode;
	int		m_envi;
	int		m_Enable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCHUEDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CHUE_ImgDlg m_HUE_img;
	CHUE_CALCDlg m_HUE_calc;

	AK_ISP_INIT_HUE m_hue;


	void EnableLinkageRadio(bool bEnable);
	BOOL check_data_init(AK_ISP_HUE *hue);
	void line_data_init(AK_ISP_HUE *hue);
	void SetDataValue(bool bFromStruct);
	void GetDataValue(bool bToStruct);
	int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	int SetPageInfoSt(void * pPageInfoSt, int nStLen);
protected:

	// Generated message map functions
	//{{AFX_MSG(CCHUEDlg)
	afx_msg void OnButtonHueImg();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg BOOL OnInitDialog();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnButtonHueCalc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHUEDLG_H__FA153565_3B74_4977_90A2_56AA2393A073__INCLUDED_)
