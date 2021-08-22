#if !defined(AFX_AFDLG_H__B7A1DA65_B684_41E8_B654_F5AEA629EEAE__INCLUDED_)
#define AFX_AFDLG_H__B7A1DA65_B684_41E8_B654_F5AEA629EEAE__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#include "NetCtrl.h"
#include "isp_struct_v2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AFDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAFDlg dialog

class CAFDlg : public CDialog, public CBasePage
{
// Construction
public:
	CAFDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAFDlg)
	enum { IDD = IDD_DIALOG_AF };
	CSpinButtonCtrl	m_top_spin;
	CSpinButtonCtrl	m_th_spin;
	CSpinButtonCtrl	m_right_spin;
	CSpinButtonCtrl	m_left_spin;
	CSpinButtonCtrl	m_bottom_spin;
	CSliderCtrl	m_top_slider;
	CSliderCtrl	m_th_slider;
	CSliderCtrl	m_left_slider;
	CSliderCtrl	m_bottom_slider;
	CSliderCtrl	m_right_slider;

	CSpinButtonCtrl	m_top1_spin;
	CSpinButtonCtrl	m_right1_spin;
	CSpinButtonCtrl	m_left1_spin;
	CSpinButtonCtrl	m_bottom1_spin;
	CSliderCtrl	m_top1_slider;
	CSliderCtrl	m_left1_slider;
	CSliderCtrl	m_bottom1_slider;
	CSliderCtrl	m_right1_slider;

	CSpinButtonCtrl	m_top2_spin;
	CSpinButtonCtrl	m_right2_spin;
	CSpinButtonCtrl	m_left2_spin;
	CSpinButtonCtrl	m_bottom2_spin;
	CSliderCtrl	m_top2_slider;
	CSliderCtrl	m_left2_slider;
	CSliderCtrl	m_bottom2_slider;
	CSliderCtrl	m_right2_slider;

	CSpinButtonCtrl	m_top3_spin;
	CSpinButtonCtrl	m_right3_spin;
	CSpinButtonCtrl	m_left3_spin;
	CSpinButtonCtrl	m_bottom3_spin;
	CSliderCtrl	m_top3_slider;
	CSliderCtrl	m_left3_slider;
	CSliderCtrl	m_bottom3_slider;
	CSliderCtrl	m_right3_slider;

	CSpinButtonCtrl	m_top4_spin;
	CSpinButtonCtrl	m_right4_spin;
	CSpinButtonCtrl	m_left4_spin;
	CSpinButtonCtrl	m_bottom4_spin;
	CSliderCtrl	m_top4_slider;
	CSliderCtrl	m_left4_slider;
	CSliderCtrl	m_bottom4_slider;
	CSliderCtrl	m_right4_slider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAFDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
public:
	AK_ISP_INIT_AF			m_Isp_af;


	int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	void SetDataValue(void);
	void GetDataValue(void);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_AF *struct_new, AK_ISP_INIT_AF_V2* struct_v2); 

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAFDlg)
	afx_msg void OnButtonSet();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSave();
	afx_msg void OnButtonRead();
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomdrawSliderLeft(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderRight(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBottom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTh(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinBottom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinLeft(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRight(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTh(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditLeft();
	afx_msg void OnKillfocusEditRight();
	afx_msg void OnKillfocusEditTh();
	afx_msg void OnKillfocusEditTop();
	afx_msg void OnKillfocusEditBottom();

	afx_msg void OnCustomdrawSliderLeft1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderRight1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBottom1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTop1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinBottom1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinLeft1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRight1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTop1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditLeft1();
	afx_msg void OnKillfocusEditRight1();
	afx_msg void OnKillfocusEditTop1();
	afx_msg void OnKillfocusEditBottom1();

	afx_msg void OnCustomdrawSliderLeft2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderRight2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBottom2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTop2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinBottom2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinLeft2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRight2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTop2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditLeft2();
	afx_msg void OnKillfocusEditRight2();
	afx_msg void OnKillfocusEditTop2();
	afx_msg void OnKillfocusEditBottom2();

	afx_msg void OnCustomdrawSliderLeft3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderRight3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBottom3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTop3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinBottom3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinLeft3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRight3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTop3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditLeft3();
	afx_msg void OnKillfocusEditRight3();
	afx_msg void OnKillfocusEditTop3();
	afx_msg void OnKillfocusEditBottom3();

	afx_msg void OnCustomdrawSliderLeft4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderRight4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderBottom4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderTop4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinBottom4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinLeft4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRight4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinTop4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEditLeft4();
	afx_msg void OnKillfocusEditRight4();
	afx_msg void OnKillfocusEditTop4();
	afx_msg void OnKillfocusEditBottom4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AFDLG_H__B7A1DA65_B684_41E8_B654_F5AEA629EEAE__INCLUDED_)
