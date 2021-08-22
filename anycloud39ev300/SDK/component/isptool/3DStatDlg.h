#if !defined(AFX_3DSTATDLG_H__F9F219F0_5B61_4764_B8E5_2A8F9D0CA913__INCLUDED_)
#define AFX_3DSTATDLG_H__F9F219F0_5B61_4764_B8E5_2A8F9D0CA913__INCLUDED_
#include "basepage.h"
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DStatDlg.h : header file
//

typedef struct  ak_isp_3d_nr_stat_info
{
	T_U16	MD_stat_max;	
	T_U16	MD_stat[24][32];    //运动检测分块输出
	T_U32	MD_level;           //运动检测输出
}AK_ISP_3D_NR_STAT_INFO;

/////////////////////////////////////////////////////////////////////////////
// C3DStatDlg dialog

class C3DStatDlg : public CDialog, public CBasePage
{
// Construction
public:
	C3DStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~C3DStatDlg();
// Dialog Data
	//{{AFX_DATA(C3DStatDlg)
	enum { IDD = IDD_DIALOG_3D_STAT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	UINT m_timer;
	AK_ISP_3D_NR_STAT_INFO m_3DStat;
	HANDLE m_semaphore;
	bool m_bInit;
	
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void SetDataValue(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(C3DStatDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DSTATDLG_H__F9F219F0_5B61_4764_B8E5_2A8F9D0CA913__INCLUDED_)
