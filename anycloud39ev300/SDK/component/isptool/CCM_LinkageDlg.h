#if !defined(AFX_CCM_LINKAGEDLG_H__A6420CBA_AAD1_455E_A7B0_9AB30E0220EB__INCLUDED_)
#define AFX_CCM_LINKAGEDLG_H__A6420CBA_AAD1_455E_A7B0_9AB30E0220EB__INCLUDED_

#include "isp_struct.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCM_LinkageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCM_LinkageDlg dialog

class CCCM_LinkageDlg : public CDialog
{
// Construction
public:
	CCCM_LinkageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCM_LinkageDlg)
	enum { IDD = IDD_DIALOG_CCM_LINKAGE };
	CSliderCtrl	m_slider_ccm_linkage_A_C11;
	CSliderCtrl	m_slider_ccm_linkage_A_C12;
	CSliderCtrl	m_slider_ccm_linkage_A_C13;
	CSliderCtrl	m_slider_ccm_linkage_A_C21;
	CSliderCtrl	m_slider_ccm_linkage_A_C22;
	CSliderCtrl	m_slider_ccm_linkage_A_C23;
	CSliderCtrl	m_slider_ccm_linkage_A_C31;
	CSliderCtrl	m_slider_ccm_linkage_A_C32;
	CSliderCtrl	m_slider_ccm_linkage_A_C33;
	CSliderCtrl	m_slider_ccm_linkage_D50_C11;
	CSliderCtrl	m_slider_ccm_linkage_D50_C12;
	CSliderCtrl	m_slider_ccm_linkage_D50_C13;
	CSliderCtrl	m_slider_ccm_linkage_D50_C21;
	CSliderCtrl	m_slider_ccm_linkage_D50_C22;
	CSliderCtrl	m_slider_ccm_linkage_D50_C23;
	CSliderCtrl	m_slider_ccm_linkage_D50_C31;
	CSliderCtrl	m_slider_ccm_linkage_D50_C32;
	CSliderCtrl	m_slider_ccm_linkage_D50_C33;
	CSliderCtrl	m_slider_ccm_linkage_D65_C11;
	CSliderCtrl	m_slider_ccm_linkage_D65_C12;
	CSliderCtrl	m_slider_ccm_linkage_D65_C13;
	CSliderCtrl	m_slider_ccm_linkage_D65_C21;
	CSliderCtrl	m_slider_ccm_linkage_D65_C22;
	CSliderCtrl	m_slider_ccm_linkage_D65_C23;
	CSliderCtrl	m_slider_ccm_linkage_D65_C31;
	CSliderCtrl	m_slider_ccm_linkage_D65_C32;
	CSliderCtrl	m_slider_ccm_linkage_D65_C33;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C11;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C12;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C13;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C21;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C22;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C23;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C31;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C32;
	CSliderCtrl	m_slider_ccm_linkage_TL84_C33;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C11;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C12;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C13;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C21;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C22;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C23;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C31;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C32;
	CSpinButtonCtrl	m_spin_ccm_linkage_A_C33;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C11;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C12;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C13;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C21;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C22;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C23;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C31;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C32;
	CSpinButtonCtrl	m_spin_ccm_linkage_D50_C33;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C11;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C12;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C13;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C21;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C22;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C23;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C31;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C32;
	CSpinButtonCtrl	m_spin_ccm_linkage_D65_C33;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C11;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C12;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C13;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C21;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C22;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C23;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C31;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C32;
	CSpinButtonCtrl	m_spin_ccm_linkage_TL84_C33;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCM_LinkageDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

	AK_ISP_CCM	 m_Isp_ccm_linkage[4];

	UINT m_timer;
	

	void SetDataValue(void);
	void GetDataValue(void);
	int GetPageInfoSt_CCMlinkage(void * pPageInfoSt, int & nStLen);
	int SetPageInfoSt_CCMlinkage(void * pPageInfoSt, int nStLen);


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCCM_LinkageDlg)
	afx_msg void OnKillfocusEditCcmLinkageAC11();
	afx_msg void OnKillfocusEditCcmLinkageAC12();
	afx_msg void OnKillfocusEditCcmLinkageAC13();
	afx_msg void OnKillfocusEditCcmLinkageAC21();
	afx_msg void OnKillfocusEditCcmLinkageAC22();
	afx_msg void OnKillfocusEditCcmLinkageAC23();
	afx_msg void OnKillfocusEditCcmLinkageAC31();
	afx_msg void OnKillfocusEditCcmLinkageAC32();
	afx_msg void OnKillfocusEditCcmLinkageAC33();
	afx_msg void OnKillfocusEditCcmLinkageD50C11();
	afx_msg void OnKillfocusEditCcmLinkageD50C12();
	afx_msg void OnKillfocusEditCcmLinkageD50C13();
	afx_msg void OnKillfocusEditCcmLinkageD50C21();
	afx_msg void OnKillfocusEditCcmLinkageD50C22();
	afx_msg void OnKillfocusEditCcmLinkageD50C23();
	afx_msg void OnKillfocusEditCcmLinkageD50C31();
	afx_msg void OnKillfocusEditCcmLinkageD50C32();
	afx_msg void OnKillfocusEditCcmLinkageD50C33();
	afx_msg void OnKillfocusEditCcmLinkageD65C11();
	afx_msg void OnKillfocusEditCcmLinkageD65C12();
	afx_msg void OnKillfocusEditCcmLinkageD65C13();
	afx_msg void OnKillfocusEditCcmLinkageD65C21();
	afx_msg void OnKillfocusEditCcmLinkageD65C22();
	afx_msg void OnKillfocusEditCcmLinkageD65C23();
	afx_msg void OnKillfocusEditCcmLinkageD65C31();
	afx_msg void OnKillfocusEditCcmLinkageD65C32();
	afx_msg void OnKillfocusEditCcmLinkageD65C33();
	afx_msg void OnKillfocusEditCcmLinkageTl84C11();
	afx_msg void OnKillfocusEditCcmLinkageTl84C12();
	afx_msg void OnKillfocusEditCcmLinkageTl84C13();
	afx_msg void OnKillfocusEditCcmLinkageTl84C21();
	afx_msg void OnKillfocusEditCcmLinkageTl84C22();
	afx_msg void OnKillfocusEditCcmLinkageTl84C23();
	afx_msg void OnKillfocusEditCcmLinkageTl84C31();
	afx_msg void OnKillfocusEditCcmLinkageTl84C32();
	afx_msg void OnKillfocusEditCcmLinkageTl84C33();
	afx_msg void OnCustomdrawSliderCcmLinkageAC11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageAC33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD50C33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageD65C33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderCcmLinkageTl84C33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageAC33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD50C33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageD65C33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCcmLinkageTl84C33(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnRadioCcmLinkageADisable();
	afx_msg void OnRadioCcmLinkageAEnable();
	afx_msg void OnRadioCcmLinkageD50Disable3();
	afx_msg void OnRadioCcmLinkageD50Enable3();
	afx_msg void OnRadioCcmLinkageD65Disable4();
	afx_msg void OnRadioCcmLinkageD65Enable4();
	afx_msg void OnRadioCcmLinkageTl84Disable2();
	afx_msg void OnRadioCcmLinkageTl84Enable2();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCM_LINKAGEDLG_H__A6420CBA_AAD1_455E_A7B0_9AB30E0220EB__INCLUDED_)
