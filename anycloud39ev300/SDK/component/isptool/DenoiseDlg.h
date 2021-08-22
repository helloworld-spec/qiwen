#if !defined(AFX_DENOISEDLG_H__287297BA_9FEB_4B5E_BB5E_C424E65650EB__INCLUDED_)
#define AFX_DENOISEDLG_H__287297BA_9FEB_4B5E_BB5E_C424E65650EB__INCLUDED_
#include "basepage.h"
#include "NrLutDlg.h"	// Added by ClassView
#include "NrLinkageDlg.h"	// Added by ClassView
#include "isp_struct_v2.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DenoiseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDenoiseDlg dialog

class CDenoiseDlg : public CDialog, public CBasePage
{
// Construction
public:
	CNrLinkageDlg m_NrLinkageDlg;
	CNrLutDlg m_NrLutDlg;
	CDenoiseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDenoiseDlg)
	enum { IDD = IDD_DIALOG_DENOISE };

	CSpinButtonCtrl	m_Y_Dpc_th;
	CSpinButtonCtrl	m_Nr2strength;
	CSpinButtonCtrl	m_Nr2_k;
	CSpinButtonCtrl	m_Nr1_k;
	CSpinButtonCtrl	m_Bstrength;
	CSpinButtonCtrl	m_Gstrength;
	CSpinButtonCtrl	m_Rstrength;
	int		m_Nr1_Enable;
	int		m_Nr2_Enable;
	int		m_Nr1mode;
	int		m_Nr2mode;

	int		m_Y_Dpc_Enable;
	int		m_Y_Black_Dpc_Enable;
	int		m_Y_White_Dpc_Enable;
	//}}AFX_DATA

	AK_ISP_INIT_NR m_Nr;
	vector<CPoint> m_lut_keyPts[10];

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void Clean(void);
	
	int Convert_v2_data(AK_ISP_INIT_NR* struct_new, AK_ISP_INIT_NR_V2* struct_v2); 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDenoiseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDenoiseDlg)
	afx_msg void OnButtonLut();
	afx_msg void OnButton6();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnRadio5();
	afx_msg void OnRadio6();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	afx_msg void OnRadio39();
	afx_msg void OnRadio92();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DENOISEDLG_H__287297BA_9FEB_4B5E_BB5E_C424E65650EB__INCLUDED_)
