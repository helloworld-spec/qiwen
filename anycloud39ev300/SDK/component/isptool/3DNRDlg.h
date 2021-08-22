#if !defined(AFX_3DNRDLG_H__AF113F62_03FA_43D2_8D82_6A8A46B04C2E__INCLUDED_)
#define AFX_3DNRDLG_H__AF113F62_03FA_43D2_8D82_6A8A46B04C2E__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"
#include "isp_struct_v3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DNRDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// C3DNRDlg dialog

class C3DNRDlg : public CDialog, public CBasePage
{
// Construction
public:
	C3DNRDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(C3DNRDlg)
	enum { IDD = IDD_DIALOG_3DNR };

	CSpinButtonCtrl		m_t_y_k_cfg;
	CSpinButtonCtrl		m_uvnr_k;
	CSpinButtonCtrl		m_uvlp_k;
	CSpinButtonCtrl		m_t_uv_k;
	CSpinButtonCtrl		m_t_uv_mf_th1;
	CSpinButtonCtrl		m_t_uv_mf_th2;
	CSpinButtonCtrl		m_t_uv_diffth_k1;
	CSpinButtonCtrl		m_t_uv_diffth_k2;
	CSpinButtonCtrl		m_t_uv_mc_k;
	CSpinButtonCtrl		m_ynr_calc_k;
	CSpinButtonCtrl		m_ynr_k;		
	CSpinButtonCtrl		m_ylp_k;	
	CSpinButtonCtrl		m_t_y_th1;
	CSpinButtonCtrl		m_t_y_k1;
	CSpinButtonCtrl		m_t_y_k2;
	CSpinButtonCtrl		m_t_y_kslop;
	CSpinButtonCtrl		m_t_y_mf_th1;
	CSpinButtonCtrl		m_t_y_mf_th2;
	CSpinButtonCtrl		m_t_y_diffth_k1;
	CSpinButtonCtrl		m_t_y_diffth_k2;
	CSpinButtonCtrl		m_t_y_mc_k;
	
	int		m_mode;
	int		m_envi;

	int		m_tnr_y_Enable;
	int		m_tnr_uv_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_3DNR m_3Dnr;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
	void CheckData(AK_ISP_INIT_3DNR *p_3dnr);	
	int Convert_v2_data(AK_ISP_INIT_3DNR* struct_new, AK_ISP_INIT_3DNR_V2* struct_v2); 
	int Convert_v3_data(AK_ISP_INIT_3DNR* struct_new, AK_ISP_INIT_3DNR_V3* struct_v3); 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DNRDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(C3DNRDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
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

#endif // !defined(AFX_3DNRDLG_H__AF113F62_03FA_43D2_8D82_6A8A46B04C2E__INCLUDED_)
