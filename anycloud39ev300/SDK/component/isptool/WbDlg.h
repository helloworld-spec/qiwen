#if !defined(AFX_WBDLG_H__AB324A1E_5FC6_4F72_8369_038B79C66AB7__INCLUDED_)
#define AFX_WBDLG_H__AB324A1E_5FC6_4F72_8369_038B79C66AB7__INCLUDED_
#include "basepage.h"
#include "Awb_wb_statDlg.h"
#include "Awb_G_weightDlg.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"
#include "WB_EX_Attr.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WbDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWbDlg dialog

class CWbDlg : public CDialog, public CBasePage
{
// Construction
public:
	CWbDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWbDlg)
	enum { IDD = IDD_DIALOG_WB };
	CSpinButtonCtrl	m_spin_awb_mwb_r_offset;
	CSpinButtonCtrl	m_spin_awb_mwb_r_gain;
	CSpinButtonCtrl	m_spin_awb_mwb_g_offset;
	CSpinButtonCtrl	m_spin_awb_mwb_g_gain;
	CSpinButtonCtrl	m_spin_awb_mwb_b_offset;
	CSpinButtonCtrl	m_spin_awb_mwb_b_gain;
	CSliderCtrl	m_slider_awb_mwb_r_offset;
	CSliderCtrl	m_slider_awb_mwb_r_gain;
	CSliderCtrl	m_slider_awb_mwb_g_offset;
	CSliderCtrl	m_slider_awb_mwb_g_gain;
	CSliderCtrl	m_slider_awb_mwb_b_offset;
	CSliderCtrl	m_slider_awb_mwb_b_gain;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWbDlg)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CAWB_G_weightDlg m_AWB_G_weightDlg;
	CAwb_wb_statDlg m_AWB_wb_statDlg;
	WB_EX_Attr m_exDlg;
	AK_ISP_INIT_WB m_Isp_wb;


	
	void GetDataValue(void);
	void SetDataValue(void);
	int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_WB *struct_new, AK_ISP_INIT_WB_V2* struct_v2); 

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWbDlg)
	afx_msg void OnButtonWbRead();
	afx_msg void OnButtonWbSave();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonGWeight();
	afx_msg void OnButtonAwbStat();
	afx_msg void OnRadioMwbEnable();
	afx_msg void OnRadioMwbDisable();
	afx_msg void OnKillfocusEditAwbMwbRGain();
	afx_msg void OnKillfocusEditAwbMwbROffset();
	afx_msg void OnKillfocusEditAwbMwbBGain();
	afx_msg void OnKillfocusEditAwbMwbBOffset();
	afx_msg void OnKillfocusEditAwbMwbGGain();
	afx_msg void OnKillfocusEditAwbMwbGOffset();
	afx_msg void OnCustomdrawSliderAwbMwbBGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderAwbMwbBOffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderAwbMwbGGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderAwbMwbGOffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderAwbMwbRGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderAwbMwbROffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbBGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbBOffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbGGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbGOffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbRGain(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinAwbMwbROffset(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioMwb();
	afx_msg void OnRadioAwb();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonWbExAttr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AWBDLG_H__AB324A1E_5FC6_4F72_8369_038B79C66AB7__INCLUDED_)
