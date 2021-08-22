#if !defined(AFX_FCSDLG_H__122E51B2_0FAC_45B7_A714_D6E8C3990DF4__INCLUDED_)
#define AFX_FCSDLG_H__122E51B2_0FAC_45B7_A714_D6E8C3990DF4__INCLUDED_
#include "basepage.h"
#include "FcsLinkageDlg.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FcsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFcsDlg dialog

class CFcsDlg : public CDialog, public CBasePage
{
// Construction
public:
	CFcsLinkageDlg m_FcsLinkageDlg;
	CFcsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFcsDlg)
	enum { IDD = IDD_DIALOG_FCS };
	CSliderCtrl	m_th_slider;
	CSliderCtrl	m_gain_slop_slider;
	CSliderCtrl	m_uv_nr_th_slider;
	CSpinButtonCtrl	m_th;
	CSpinButtonCtrl	m_gain_slop;
	CSpinButtonCtrl	m_uv_nr_th;
	int		m_Enable;
	int		m_uv_Enable;
	int		m_mode;
	//}}AFX_DATA

	AK_ISP_INIT_FCS m_Fcs;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFcsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFcsDlg)
	afx_msg void OnButtonLinkage();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FCSDLG_H__122E51B2_0FAC_45B7_A714_D6E8C3990DF4__INCLUDED_)
