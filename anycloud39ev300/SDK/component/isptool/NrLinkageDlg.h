#if !defined(AFX_NRLINKAGEDLG_H__693A6E2F_714B_4009_9018_00BB3D44E59F__INCLUDED_)
#define AFX_NRLINKAGEDLG_H__693A6E2F_714B_4009_9018_00BB3D44E59F__INCLUDED_
#include "anyka_types.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NrLinkageDlg.h : header file
//
typedef struct nr_linkage
{
	T_U16	nr1_enable;
	T_U16	nr1_k;              //[0,15]
	T_U16	nr1_calc_g_k;
	T_U16	nr1_calc_r_k;
	T_U16	nr1_calc_b_k;

	T_U16	nr2_enable;
	T_U16	nr2_k;               //[0,15]
	T_U16	nr2_calc_y_k;
	T_U16   y_dpc_enable;
	T_U16	y_dpc_th;
	T_U16	y_black_dpc_enable;
	T_U16	y_white_dpc_enable;
}NR_LINKAGE;

/////////////////////////////////////////////////////////////////////////////
// CNrLinkageDlg dialog

class CNrLinkageDlg : public CDialog
{
// Construction
public:
	CNrLinkageDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNrLinkageDlg)
	enum { IDD = IDD_DIALOG_NR_LINKAGE };
		// NOTE: the ClassWizard will add data members here
		
	//}}AFX_DATA
	int m_Nr1_Enable[9];
	int m_Nr2_Enable[9];
	int m_Y_Dpc_Enable[9];
	int m_Y_Black_Dpc_Enable[9];
	int m_Y_White_Dpc_Enable[9];
	
	int	m_Nr2strength[9];
	int	m_Nr2_k[9];
	int	m_Nr1_k[9];
	int	m_Bstrength[9];
	int	m_Gstrength[9];
	int	m_Rstrength[9];
	int	m_Y_Dpc_th[9];

	NR_LINKAGE		m_linkage[9];

	void GetDataValue(char* pbuf, int* size);
	void SetDataValue(char* pbuf, int size);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNrLinkageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNrLinkageDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnButtonCancel();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnRadio40();
	afx_msg void OnRadio41();
	afx_msg void OnRadio44();
	afx_msg void OnRadio45();
	afx_msg void OnRadio33();
	afx_msg void OnRadio34();
	afx_msg void OnRadio42();
	afx_msg void OnRadio43();
	afx_msg void OnRadio46();
	afx_msg void OnRadio47();
	afx_msg void OnRadio48();
	afx_msg void OnRadio49();
	afx_msg void OnRadio50();
	afx_msg void OnRadio51();
	afx_msg void OnRadio52();
	afx_msg void OnRadio53();
	afx_msg void OnRadio54();
	afx_msg void OnRadio55();
	afx_msg void OnRadio56();
	afx_msg void OnRadio57();
	afx_msg void OnRadio58();
	afx_msg void OnRadio59();
	afx_msg void OnRadio60();
	afx_msg void OnRadio61();
	afx_msg void OnRadio62();
	afx_msg void OnRadio63();
	afx_msg void OnRadio64();
	afx_msg void OnRadio65();
	afx_msg void OnRadio66();
	afx_msg void OnRadio67();
	afx_msg void OnRadio68();
	afx_msg void OnRadio69();
	afx_msg void OnRadio70();
	afx_msg void OnRadio71();
	afx_msg void OnRadio35();
	afx_msg void OnRadio36();
	afx_msg void OnRadio37();
	afx_msg void OnRadio38();
	afx_msg void OnRadio39();
	afx_msg void OnRadio73();
	afx_msg void OnRadio74();
	afx_msg void OnRadio85();
	afx_msg void OnRadio86();
	afx_msg void OnRadio93();
	afx_msg void OnRadio94();
	afx_msg void OnRadio95();
	afx_msg void OnRadio96();
	afx_msg void OnRadio97();
	afx_msg void OnRadio98();
	afx_msg void OnRadio99();
	afx_msg void OnRadio100();
	afx_msg void OnRadio101();
	afx_msg void OnRadio102();
	afx_msg void OnRadio103();
	afx_msg void OnRadio104();
	afx_msg void OnRadio105();
	afx_msg void OnRadio106();
	afx_msg void OnRadio107();
	afx_msg void OnRadio108();
	afx_msg void OnRadio109();
	afx_msg void OnRadio110();
	afx_msg void OnRadio111();
	afx_msg void OnRadio112();
	afx_msg void OnRadio113();
	afx_msg void OnRadio114();
	afx_msg void OnRadio115();
	afx_msg void OnRadio116();
	afx_msg void OnRadio117();
	afx_msg void OnRadio118();
	afx_msg void OnRadio119();
	afx_msg void OnRadio120();
	afx_msg void OnRadio121();
	afx_msg void OnRadio122();
	afx_msg void OnRadio123();
	afx_msg void OnRadio124();
	afx_msg void OnRadio125();
	afx_msg void OnRadio126();
	afx_msg void OnRadio127();
	afx_msg void OnRadio128();
	afx_msg void OnRadio129();
	afx_msg void OnRadio130();
	afx_msg void OnRadio131();
	afx_msg void OnRadio132();
	afx_msg void OnRadio133();
	afx_msg void OnRadio134();
	afx_msg void OnRadio135();
	afx_msg void OnRadio136();
	afx_msg void OnRadio137();
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NRLINKAGEDLG_H__693A6E2F_714B_4009_9018_00BB3D44E59F__INCLUDED_)
