#if !defined(AFX_GBDLG_H__EB0AECE2_31EE_41C1_9A95_42140FC6ACBC__INCLUDED_)
#define AFX_GBDLG_H__EB0AECE2_31EE_41C1_9A95_42140FC6ACBC__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GBDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGBDlg dialog

class CGBDlg : public CDialog, public CBasePage
{
// Construction
public:
	CGBDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGBDlg)
	enum { IDD = IDD_DIALOG_GB };
	CSliderCtrl	m_en_th_slider;
	CSliderCtrl	m_kstep_slider;
	CSliderCtrl	m_threshold_slider;
	CSpinButtonCtrl	m_en_th;
	CSpinButtonCtrl	m_kstep;
	CSpinButtonCtrl	m_threshold;
	int		m_mode;
	int		m_envi;
	int		m_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_GB m_Gb;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGBDlg)
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
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
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

#endif // !defined(AFX_GBDLG_H__EB0AECE2_31EE_41C1_9A95_42140FC6ACBC__INCLUDED_)
