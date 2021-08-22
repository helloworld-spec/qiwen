#if !defined(AFX_CCMDLG_H__C1C98D5C_2CEB_4DA1_A497_DB64933FE95F__INCLUDED_)
#define AFX_CCMDLG_H__C1C98D5C_2CEB_4DA1_A497_DB64933FE95F__INCLUDED_
#include "basepage.h"
#include "CCM_LinkageDlg.h"
#include "anyka_types.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"
#include "CCM_ImgDlg.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCMDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCCMDlg dialog

class CCCMDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCCMDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCMDlg)
	enum { IDD = IDD_DIALOG_CCM };
	CSpinButtonCtrl	m_spin_c33;
	CSpinButtonCtrl	m_spin_c32;
	CSpinButtonCtrl	m_spin_c31;
	CSpinButtonCtrl	m_spin_c23;
	CSpinButtonCtrl	m_spin_c22;
	CSpinButtonCtrl	m_spin_c21;
	CSpinButtonCtrl	m_spin_c13;
	CSpinButtonCtrl	m_spin_c12;
	CSpinButtonCtrl	m_spin_c11;
	CSliderCtrl	m_slider_c33;
	CSliderCtrl	m_slider_c32;
	CSliderCtrl	m_slider_c31;
	CSliderCtrl	m_slider_c23;
	CSliderCtrl	m_slider_c22;
	CSliderCtrl	m_slider_c21;
	CSliderCtrl	m_slider_c13;
	CSliderCtrl	m_slider_c12;
	CSliderCtrl	m_slider_c11;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCMDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CCCM_ImgDlg m_ImgDlg;
	CCCM_LinkageDlg  m_CCM_LinkageDlg;
	AK_ISP_INIT_CCM	 m_Isp_ccm;

	void GetDataValue(void);
	void SetDataValue(void);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void Clean(void);
	void SetConnectState(bool bConnect);
	
	int Convert_v2_data(AK_ISP_INIT_CCM* struct_new, AK_ISP_INIT_CCM_V2* struct_v2); 
	bool m_bConnect;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCCMDlg)
	afx_msg void OnButtonLinkageSet();
	afx_msg void OnRadioManual();
	afx_msg void OnRadio2();
	afx_msg void OnRadioLinkageEnable();
	afx_msg void OnRadioLinkageDisable();
	afx_msg void OnRadioManualEnable();
	afx_msg void OnRadioManualDisable();
	afx_msg void OnKillfocusEditC11();
	afx_msg void OnKillfocusEditC12();
	afx_msg void OnKillfocusEditC13();
	afx_msg void OnKillfocusEditC21();
	afx_msg void OnKillfocusEditC22();
	afx_msg void OnKillfocusEditC23();
	afx_msg void OnKillfocusEditC31();
	afx_msg void OnKillfocusEditC32();
	afx_msg void OnKillfocusEditC33();
	afx_msg void OnCustomdrawSliderC11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderC33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC11(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC13(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC23(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC31(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC32(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinC33(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonCcmRead();
	afx_msg void OnButtonCcmSave();
	afx_msg void OnButtonCcmGet();
	afx_msg void OnButtonCcmSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLinkage();
	afx_msg void OnButtonImgCalc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately bfore the previous line.

#endif // !defined(AFX_CCMDLG_H__C1C98D5C_2CEB_4DA1_A497_DB64933FE95F__INCLUDED_)
