#if !defined(AFX_EDGEDLG_H__3E73BDD3_FB60_4447_B0A3_C9157B0517BD__INCLUDED_)
#define AFX_EDGEDLG_H__3E73BDD3_FB60_4447_B0A3_C9157B0517BD__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EdgeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEdgeDlg dialog

class CEdgeDlg : public CDialog, public CBasePage
{
// Construction
public:
	CEdgeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEdgeDlg)
	enum { IDD = IDD_DIALOG_EDGE };
	CSliderCtrl	m_th_slider;
	CSliderCtrl	m_max_len_slider;
	CSliderCtrl	m_gain_th_slider;
	CSliderCtrl	m_gain_slop_slider;
	CSpinButtonCtrl	m_th;
	CSpinButtonCtrl	m_max_len;
	CSpinButtonCtrl	m_gain_th;
	CSpinButtonCtrl	m_gain_slop;
	int		m_mode;
	int		m_envi;
	int		m_Enable;
	int		m_c_edge_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_EDGE m_Edge;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEdgeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEdgeDlg)
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
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
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

#endif // !defined(AFX_EDGEDLG_H__3E73BDD3_FB60_4447_B0A3_C9157B0517BD__INCLUDED_)
