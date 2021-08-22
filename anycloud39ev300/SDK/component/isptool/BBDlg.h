#if !defined(AFX_BBDLG_H__2F963076_0C48_4766_8A45_ABEBC0ECC012__INCLUDED_)
#define AFX_BBDLG_H__2F963076_0C48_4766_8A45_ABEBC0ECC012__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BBDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBBDlg dialog

class CBBDlg : public CDialog, public CBasePage
{
// Construction
public:
	CBBDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBBDlg)
	enum { IDD = IDD_DIALOG_BB };
	CSliderCtrl	m_r_a_slider;
	CSliderCtrl	m_r_b_slider;
	CSliderCtrl	m_gr_a_slider;
	CSliderCtrl	m_gr_b_slider;
	CSliderCtrl	m_gb_a_slider;
	CSliderCtrl	m_gb_b_slider;
	CSliderCtrl	m_b_a_slider;
	CSliderCtrl	m_b_b_slider;
	CSpinButtonCtrl	m_r_a;
	CSpinButtonCtrl	m_r_b;
	CSpinButtonCtrl	m_gr_a;
	CSpinButtonCtrl	m_gr_b;
	CSpinButtonCtrl	m_gb_a;
	CSpinButtonCtrl	m_gb_b;
	CSpinButtonCtrl	m_b_a;
	CSpinButtonCtrl	m_b_b;
	int		m_mode;
	int		m_envi;
	int		m_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_BLC m_Blc;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBBDlg)
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
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin7(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit7();
	afx_msg void OnCustomdrawSlider7(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin8(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit8();
	afx_msg void OnCustomdrawSlider8(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio12();
	afx_msg void OnRadio13();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio33();
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

#endif // !defined(AFX_BBDLG_H__2F963076_0C48_4766_8A45_ABEBC0ECC012__INCLUDED_)
