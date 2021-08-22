#if !defined(AFX_AFSTATDLG_H__BA0E7099_3272_42DC_8FD9_126D554AB8E2__INCLUDED_)
#define AFX_AFSTATDLG_H__BA0E7099_3272_42DC_8FD9_126D554AB8E2__INCLUDED_
#include "basepage.h"
#include "anyka_types.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AFStatDlg.h : header file
//

typedef struct ak_isp_af_stat_info
{
   T_U32  af_statics[5];			 //统计结果  
}AK_ISP_AF_STAT_INFO;

/////////////////////////////////////////////////////////////////////////////
// CAFStatDlg dialog

class CAFStatDlg : public CDialog, public CBasePage
{
// Construction
public:
	CAFStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAFStatDlg();
// Dialog Data
	//{{AFX_DATA(CAFStatDlg)
	enum { IDD = IDD_DIALOG_AF_STAT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	UINT m_timer;
	AK_ISP_AF_STAT_INFO m_AFStat;
	HANDLE m_semaphore;
	bool m_bInit;
	
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void SetDataValue(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAFStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAFStatDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AFSTATDLG_H__BA0E7099_3272_42DC_8FD9_126D554AB8E2__INCLUDED_)
