#if !defined(AFX_DEMOSAICDLG_H__C3BF9BCF_1EA6_4286_A2F0_500425CB6042__INCLUDED_)
#define AFX_DEMOSAICDLG_H__C3BF9BCF_1EA6_4286_A2F0_500425CB6042__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DemosaicDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDemosaicDlg dialog

class CDemosaicDlg : public CDialog, public CBasePage
{
// Construction
public:
	CDemosaicDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDemosaicDlg)
	enum { IDD = IDD_DIALOG_DEMOSAIC };
	CSliderCtrl	m_hf_th2_slider;
	CSpinButtonCtrl	m_hf_th2;
	CSliderCtrl	m_hf_th1_slider;
	CSpinButtonCtrl	m_hf_th1;
	CSliderCtrl	m_bg_th_slider;
	CSpinButtonCtrl	m_bg_th;
	CSliderCtrl	m_rg_th_slider;
	CSpinButtonCtrl	m_rg_th;
	CSliderCtrl	m_hv_th_slider;
	CSpinButtonCtrl	m_hv_th;
	//}}AFX_DATA

	AK_ISP_INIT_DEMO m_Demo;


	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemosaicDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDemosaicDlg)
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOSAICDLG_H__C3BF9BCF_1EA6_4286_A2F0_500425CB6042__INCLUDED_)
