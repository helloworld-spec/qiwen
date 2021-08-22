#if !defined(AFX_SYSDLG_H__B4A5FC4C_7BF5_45DF_B974_E022B182C8BF__INCLUDED_)
#define AFX_SYSDLG_H__B4A5FC4C_7BF5_45DF_B974_E022B182C8BF__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SysDlg.h : header file
//

#define CFG_FILE_VERSION_LEN	15
#define CFG_FILE_NOTES_LEN		383

/////////////////////////////////////////////////////////////////////////////
// CSysDlg dialog

class CSysDlg : public CDialog, public CBasePage
{
// Construction
public:
	CSysDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSysDlg)
	enum { IDD = IDD_DIALOG_SYS };
	CString	m_addrstr;
	CString	m_valuestr;
	CString	m_Version;
	CString	m_Notes;
	CString	m_sensorIdStr;
	//}}AFX_DATA

	unsigned short m_addr;
	unsigned short m_value;
	T_U8 m_style_id;
	T_U32 m_sensor_id;
	UINT m_timer;

	int CSysDlg::SetEnable(int nFlag, BOOL bEnable);

	int GetAddrInfo(void * pAddrInfo, int & nStLen);
	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	void GetDataValue(void);
	void SetDataValue(void);
	void SetSensorId(T_U32 sensorId);
	T_U32 GetSensorId(void);
	void SetVersion(char *str);
	void GetVersion(char *buf);
	void SetNotes(char *str);
	bool GetNotes(char *buf);
	T_U8 GetStyleId(void);
	void SetStyleId(T_U8 styleId);
	void Clean(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSysDlg)
	afx_msg void OnButtonGetSen();
	afx_msg void OnButtonSetSen();
	afx_msg void OnButtonClear();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSDLG_H__B4A5FC4C_7BF5_45DF_B974_E022B182C8BF__INCLUDED_)
