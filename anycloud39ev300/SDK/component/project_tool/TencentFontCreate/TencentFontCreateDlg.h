// TencentFontCreateDlg.h : header file
//

#if !defined(AFX_TENCENTFONTCREATEDLG_H__BBEB787B_A727_44C0_8378_7CDD151C5B19__INCLUDED_)
#define AFX_TENCENTFONTCREATEDLG_H__BBEB787B_A727_44C0_8378_7CDD151C5B19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTencentFontCreateDlg dialog

class CTencentFontCreateDlg : public CDialog
{
// Construction
public:
	CTencentFontCreateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTencentFontCreateDlg)
	enum { IDD = IDD_TENCENTFONTCREATE_DIALOG };
	CEdit	m_osd_name_ctrl;
	CString	m_osd_name;
	CString	m_file_name;
	int		m_FontSize;
	CString	m_edit_font_file;
	CString	m_font_out_file_name;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTencentFontCreateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	void font_lib_get_font_from_bin(WORD *uni_code,  int len, int fontsize);
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTencentFontCreateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TENCENTFONTCREATEDLG_H__BBEB787B_A727_44C0_8378_7CDD151C5B19__INCLUDED_)
