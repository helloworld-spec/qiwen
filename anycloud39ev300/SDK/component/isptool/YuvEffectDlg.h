#if !defined(AFX_YUVEFFECTDLG_H__667F3E22_6A18_4A76_9597_D93D4E954F61__INCLUDED_)
#define AFX_YUVEFFECTDLG_H__667F3E22_6A18_4A76_9597_D93D4E954F61__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// YuvEffectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CYuvEffectDlg dialog

class CYuvEffectDlg : public CDialog, public CBasePage
{
// Construction
public:
	CYuvEffectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CYuvEffectDlg)
	enum { IDD = IDD_DIALOG_YUVEFFECT };
	CSliderCtrl	m_uv_b_slider;
	CSpinButtonCtrl	m_uv_b;
	CSliderCtrl	m_uv_a_slider;
	CSpinButtonCtrl	m_uv_a;
	CSliderCtrl	m_y_b_slider;
	CSpinButtonCtrl	m_y_b;
	CSliderCtrl	m_y_a_slider;
	CSpinButtonCtrl	m_y_a;
	int		m_dark_margin_enable;

	
	//}}AFX_DATA
	AK_ISP_INIT_EFFECT m_Effect;
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYuvEffectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CYuvEffectDlg)
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
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
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_YUVEFFECTDLG_H__667F3E22_6A18_4A76_9597_D93D4E954F61__INCLUDED_)
