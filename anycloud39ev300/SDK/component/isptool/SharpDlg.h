#if !defined(AFX_SHARPDLG_H__F84173FA_0C2E_4A2E_B40B_B3DF54D75D03__INCLUDED_)
#define AFX_SHARPDLG_H__F84173FA_0C2E_4A2E_B40B_B3DF54D75D03__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"
#include "SharpLutDlg.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SharpDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSharpDlg dialog

class CSharpDlg : public CDialog, public CBasePage
{
// Construction
public:
	CSharpLutDlg m_LutDlg;
	CSharpDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSharpDlg)
	enum { IDD = IDD_DIALOG_SHARP };
	CSliderCtrl	m_m_k_slider;
	CSliderCtrl	m_m_shift_slider;
	CSliderCtrl	m_h_k_slider;
	CSliderCtrl	m_h_shift_slider;
	CSliderCtrl	m_strength_slider;
	CSpinButtonCtrl	m_m_k;
	CSpinButtonCtrl	m_m_shift;
	CSpinButtonCtrl	m_h_k;
	CSpinButtonCtrl	m_h_shift;
	CSpinButtonCtrl	m_strength;
	int		m_mode;
	int		m_envi;
	int		m_ysharp_Enable;
	int		m_skin_det_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_SHARP m_Sharp;

	vector<CPoint> m_Mf_keyPts[10];
	vector<CPoint> m_Hf_keyPts[10];

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_SHARP *struct_new, AK_ISP_INIT_SHARP_V2* struct_v2); 
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSharpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);
	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSharpDlg)
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
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio12();
	afx_msg void OnRadio13();
	afx_msg void OnRadio14();
	afx_msg void OnRadio15();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	afx_msg void OnButtonLut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHARPDLG_H__F84173FA_0C2E_4A2E_B40B_B3DF54D75D03__INCLUDED_)
