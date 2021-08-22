#if !defined(AFX_WDRDLG_H__299F630D_17C7_41D9_8053_8E07EE5AB7A4__INCLUDED_)
#define AFX_WDRDLG_H__299F630D_17C7_41D9_8053_8E07EE5AB7A4__INCLUDED_
#include "basepage.h"
#include "WdrLutDlg.h"	// Added by ClassView
#include "isp_struct_v2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WdrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWdrDlg dialog

class CWdrDlg : public CDialog, public CBasePage
{
// Construction
public:
	CWdrLutDlg m_LutDlg;
	CWdrDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWdrDlg)
	enum { IDD = IDD_DIALOG_WDR };
	CSliderCtrl	m_gain_slider;
	CSpinButtonCtrl	m_gain;
	CSliderCtrl	m_yth2_slider;
	CSpinButtonCtrl	m_yth2;
	CSliderCtrl	m_yth1_slider;
	CSpinButtonCtrl	m_yth1;
	CSliderCtrl	m_uvlevel_slider;
	CSpinButtonCtrl	m_uv_level;
	CSliderCtrl	m_th2_slider;
	CSpinButtonCtrl	m_th2;
	CSliderCtrl	m_th1_slider;
	CSpinButtonCtrl	m_th1;
	CSliderCtrl	m_th3_slider;
	CSpinButtonCtrl	m_th3;
	CSliderCtrl	m_th4_slider;
	CSpinButtonCtrl	m_th4;
	CSliderCtrl	m_th5_slider;
	CSpinButtonCtrl	m_th5;
	int		m_WdrEnable;
	int		m_uvEnable;
	int		m_mode;
	int		m_envi;
	
	//}}AFX_DATA
	AK_ISP_INIT_WDR m_Wdr;
	
	vector<CPoint> m_area1_keyPts[10];
	vector<CPoint> m_area2_keyPts[10];
	vector<CPoint> m_area3_keyPts[10];
	vector<CPoint> m_area4_keyPts[10];
	vector<CPoint> m_area5_keyPts[10];
	vector<CPoint> m_area6_keyPts[10];

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
	void CheckData(AK_ISP_INIT_WDR* p_Wdr);
	int Convert_v2_data(AK_ISP_INIT_WDR* struct_new, AK_ISP_INIT_WDR_V2* struct_v2); 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWdrDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWdrDlg)
	afx_msg void OnButton1();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin20(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin160(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit145();
	afx_msg void OnCustomdrawSlider23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit159();
	afx_msg void OnCustomdrawSlider24(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit7();
	afx_msg void OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit8();
	afx_msg void OnDeltaposSpin8(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider10(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WDRDLG_H__299F630D_17C7_41D9_8053_8E07EE5AB7A4__INCLUDED_)
