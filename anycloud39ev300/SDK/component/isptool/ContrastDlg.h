#if !defined(AFX_CONTRASTDLG_H__C5AA8A51_4C7C_4F8A_A789_F1F19C4CC813__INCLUDED_)
#define AFX_CONTRASTDLG_H__C5AA8A51_4C7C_4F8A_A789_F1F19C4CC813__INCLUDED_

#include "basepage.h"
#include "isp_struct.h"
#include "NetCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContrastDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContrastDlg dialog

class CContrastDlg : public CDialog, public CBasePage
{
// Construction
public:
	CContrastDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CContrastDlg)
	enum { IDD = IDD_DIALOG_CONTRAST };
	CSpinButtonCtrl	m_spin_y_shift;
	CSpinButtonCtrl	m_spin_y_constrast;
	CSpinButtonCtrl	m_area;
	CSpinButtonCtrl	m_rate;
	CSpinButtonCtrl	m_shiftmax;
	
	CSliderCtrl	m_slider_y_shift;
	CSliderCtrl	m_slider_y_constrast;
	CSliderCtrl	m_area_slider;
	CSliderCtrl	m_rate_slider;
	CSliderCtrl	m_shiftmax_slider;
	

	int		m_mode;
	int		m_envi;
	//}}AFX_DATA
	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContrastDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	AK_ISP_INIT_CONTRAST	m_Isp_contrast;



	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void Clean(void);
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CContrastDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonConstrastGet();
	afx_msg void OnButtonConstrastSet();
	afx_msg void OnKillfocusEditYConstrast();
	afx_msg void OnKillfocusEditYShift();
	afx_msg void OnCustomdrawSliderYConstrast(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderYShift(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinYConstrast(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinYShift(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
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

	void EnableLinkageRadio(bool bEnable);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTRASTDLG_H__C5AA8A51_4C7C_4F8A_A789_F1F19C4CC813__INCLUDED_)
