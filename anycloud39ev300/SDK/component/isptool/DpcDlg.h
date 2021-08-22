#if !defined(AFX_DPCDLG_H__050AA90E_E30F_4FB3_9379_F90F3CB54EE7__INCLUDED_)
#define AFX_DPCDLG_H__050AA90E_E30F_4FB3_9379_F90F3CB54EE7__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"
#include "isp_struct_v2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DpcDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDpcDlg dialog

class CDpcDlg : public CDialog, public CBasePage
{
// Construction
public:
	CDpcDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDpcDlg)
	enum { IDD = IDD_DIALOG_DPC };
	CSpinButtonCtrl	m_ddpc_th;
	CSliderCtrl	m_ddpc_th_slider;

	int		m_Enable;
	int		m_mode;
	int		m_envi;

	int		m_WhiteEnable;
	int		m_BlackEnable;
	//}}AFX_DATA

	AK_ISP_INIT_DPC m_Dpc;

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(bool bToStruct);
	void SetDataValue(bool bFromStruct);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_DPC *struct_new, AK_ISP_INIT_DPC_V2* struct_v2);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDpcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	void EnableLinkageRadio(bool bEnable);
	
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDpcDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio10();
	afx_msg void OnRadio11();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio7();
	afx_msg void OnRadio8();
	afx_msg void OnRadio9();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DPCDLG_H__050AA90E_E30F_4FB3_9379_F90F3CB54EE7__INCLUDED_)
