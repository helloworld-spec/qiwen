#if !defined(AFX_Y_GAMMAPARMDLG_H__3E26322E_D99B_46C0_A81A_CF31ABAB7195__INCLUDED_)
#define AFX_Y_GAMMAPARMDLG_H__3E26322E_D99B_46C0_A81A_CF31ABAB7195__INCLUDED_
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Y_GammaParmDlg.h : header file
//
typedef struct ak_isp_y_gamma_parm
{
	T_U16    ygamma_uv_adjust_enable;
	T_U16    ygamma_uv_adjust_level;
	T_U16    ygamma_cnoise_yth1;   //Ygamma色差抑制门限值
	T_U16    ygamma_cnoise_yth2;   //Ygamma色差抑制门限值
	T_U16    ygamma_cnoise_slop;   
	T_U16    ygamma_cnoise_gain ;  //UV调整系数计算参数
}AK_ISP_Y_GAMMA_PARM;

/////////////////////////////////////////////////////////////////////////////
// CY_GammaParmDlg dialog

class CY_GammaParmDlg : public CDialog
{
// Construction
public:
	CY_GammaParmDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CY_GammaParmDlg)
	enum { IDD = IDD_DIALOG_Y_GAMMA_PARM };

	int m_uvad_en;
	CSpinButtonCtrl	m_level;
	CSpinButtonCtrl	m_yth1;
	CSpinButtonCtrl	m_yth2;
	CSpinButtonCtrl	m_slop;
	CSpinButtonCtrl	m_gain;

	CSliderCtrl		m_level_slider;
	CSliderCtrl		m_yth1_slider;
	CSliderCtrl		m_yth2_slider;
	CSliderCtrl		m_slop_slider;
	CSliderCtrl		m_gain_slider;
	//}}AFX_DATA

	AK_ISP_Y_GAMMA_PARM m_parm;

	void GetDataValue(char* pbuf, int* size);
	void SetDataValue(char* pbuf, int size);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CY_GammaParmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CY_GammaParmDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Y_GAMMAPARMDLG_H__3E26322E_D99B_46C0_A81A_CF31ABAB7195__INCLUDED_)
