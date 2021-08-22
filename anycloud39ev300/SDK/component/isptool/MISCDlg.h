#if !defined(AFX_MISCDLG_H__39B35741_5741_4C28_BE27_2E6136D82255__INCLUDED_)
#define AFX_MISCDLG_H__39B35741_5741_4C28_BE27_2E6136D82255__INCLUDED_

#include "basepage.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"
#include "isp_struct_v3.h"
#include "miscotherdlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MISCDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMISCDlg dialog

class CMISCDlg : public CDialog, public CBasePage
{
// Construction
public:
	CMiscOtherDlg m_OtherDlg;
	CMISCDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMISCDlg)
	enum { IDD = IDD_DIALOG_MISC };
	CSpinButtonCtrl	m_pattern_cfg;
	CSliderCtrl	m_pattern_cfg_slider;
	CSpinButtonCtrl	m_cfa_mode;
	CSliderCtrl	m_cfa_mode_slider;
	CSpinButtonCtrl	m_fsd_num;
	CSliderCtrl	m_fsd_num_slider;
	CSpinButtonCtrl	m_oneline_cycle;
	CSpinButtonCtrl	m_rawhblank_cycle;
	CSpinButtonCtrl	m_yuvhblank_cycle;
	CSpinButtonCtrl	m_data_w;
	CSliderCtrl	m_data_w_slider;
	int		m_hsyn_pol;
	int		m_vsync_pol;
	int		m_pattern_en;
	int		m_fsd_en;
	int		m_pclk;
	int		m_flip_en;
	int		m_mirror_en;

	UINT	m_oneline_cycle_value;
	UINT	m_hblank_cycle_value;
	//}}AFX_DATA

	AK_ISP_INIT_MISC m_Misc;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	
	int Convert_v2_data(AK_ISP_INIT_MISC *struct_new, AK_ISP_INIT_MISC_V2* struct_v2);
	int Convert_v3_data(AK_ISP_INIT_MISC *struct_new, AK_ISP_INIT_MISC_V3* struct_v3);
	void Clean(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMISCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMISCDlg)
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
	afx_msg void OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit5();
	afx_msg void OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpin10(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit6();
	afx_msg void OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	afx_msg void OnRadio39();
	afx_msg void OnRadio87();
	afx_msg void OnRadio88();
	afx_msg void OnRadio89();
	afx_msg void OnRadio90();
	afx_msg void OnRadio91();
	afx_msg void OnButtonOther();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISCDLG_H__39B35741_5741_4C28_BE27_2E6136D82255__INCLUDED_)
