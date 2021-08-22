#pragma once

#include "resource.h"
#include "Test_ConfigDlg.h"
#include "afxwin.h"


// CLogin_testconfig 对话框

class CLogin_testconfig : public CDialog
{
	DECLARE_DYNAMIC(CLogin_testconfig)

public:
	CLogin_testconfig(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLogin_testconfig();
	CString m_login_password;
	CString m_login_user;

// 对话框数据
	enum { IDD = IDD_DIALOG_LOGIN_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	CTest_ConfigDlg m_test_config;

	void Set_confim();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedLogin();
	afx_msg void OnEnChangeEditUser();

	
	afx_msg void OnBnClickedCheckAutoLogin();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	CComboBox m_combo_username;
	afx_msg void OnCbnSelchangeComboUser();
};
