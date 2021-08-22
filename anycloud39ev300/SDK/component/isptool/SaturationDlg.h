#if !defined(AFX_SATURATIONDLG_H__B429F5EE_A350_4BDE_8E6E_8B08E1F001AF__INCLUDED_)
#define AFX_SATURATIONDLG_H__B429F5EE_A350_4BDE_8E6E_8B08E1F001AF__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaturationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaturationDlg dialog

class CSaturationDlg : public CDialog, public CBasePage
{
// Construction
public:
	CSaturationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSaturationDlg)
	enum { IDD = IDD_DIALOG_SATURATION };
	CSliderCtrl	m_th1_slider;
	CSliderCtrl	m_th2_slider;
	CSliderCtrl	m_th3_slider;
	CSliderCtrl	m_th4_slider;
	CSliderCtrl	m_scale1_slider;
	CSliderCtrl	m_scale2_slider;
	CSliderCtrl	m_scale3_slider;
	CSpinButtonCtrl	m_th1;
	CSpinButtonCtrl	m_th2;
	CSpinButtonCtrl	m_th3;
	CSpinButtonCtrl	m_th4;
	CSpinButtonCtrl	m_scale1;
	CSpinButtonCtrl	m_scale2;
	CSpinButtonCtrl	m_scale3;
	int		m_mode;
	int		m_envi;
	int		m_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_SATURATION m_Sat;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
	void CheckData(AK_ISP_INIT_SATURATION *p_Sat);
	int Convert_v2_data(AK_ISP_INIT_SATURATION*struct_new, AK_ISP_INIT_SATURATION_V2* struct_v2); 
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaturationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);
	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaturationDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin87(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnCustomdrawSlider21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin150(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit7();
	afx_msg void OnCustomdrawSlider7(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio12();
	afx_msg void OnRadio13();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SATURATIONDLG_H__B429F5EE_A350_4BDE_8E6E_8B08E1F001AF__INCLUDED_)
