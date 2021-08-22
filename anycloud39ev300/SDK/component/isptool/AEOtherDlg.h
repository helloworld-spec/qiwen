#if !defined(AFX_AEOTHERDLG_H__5591B8E4_313C_4431_A219_318BCD20BC0E__INCLUDED_)
#define AFX_AEOTHERDLG_H__5591B8E4_313C_4431_A219_318BCD20BC0E__INCLUDED_

#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AEOtherDlg.h : header file
//
typedef struct ak_isp_ae_other
{
	T_U32	envi_gain_range[10][2];
	T_U32	hist_weight[16];        //曝光计算权重    [0 ,16]
    T_U32	OE_suppress_en;    //过曝抑制使能
    T_U32	OE_detect_scope; //[0,255]    过曝检测范围
    T_U32	OE_rate_max; //[0, 255]    过曝检测系数最大值
    T_U32	OE_rate_min; //[0, 255]    过曝检测系数最小值
}AK_ISP_AE_OTHER;

/////////////////////////////////////////////////////////////////////////////
// CAEOtherDlg dialog

class CAEOtherDlg : public CDialog
{
// Construction
public:
	CAEOtherDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAEOtherDlg)
	enum { IDD = IDD_DIALOG_AE_OTHER };
	CSliderCtrl	m_detect_scope_slider;
	CSliderCtrl	m_rate_max_slider;
	CSliderCtrl	m_rate_min_slider;
	CSpinButtonCtrl	m_detect_scope;
	CSpinButtonCtrl	m_rate_max;
	CSpinButtonCtrl	m_rate_min;
	int		m_Enable;
	//}}AFX_DATA

	UINT m_range[10][2];
	UINT m_hist_wt[16];

	AK_ISP_AE_OTHER m_AEOther;

	void GetDataValue(void);
	void SetDataValue(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAEOtherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAEOtherDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit154();
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin10(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit155();
	afx_msg void OnCustomdrawSlider21(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin12(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit144();
	afx_msg void OnCustomdrawSlider22(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AEOTHERDLG_H__5591B8E4_313C_4431_A219_318BCD20BC0E__INCLUDED_)
