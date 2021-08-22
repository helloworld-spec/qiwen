#pragma once
#include "afxcmn.h"
#include "StaticEx.h"
#include "afxwin.h"


// CRightCtrlDlg 对话框

class CRightCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CRightCtrlDlg)

public:
	CRightCtrlDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRightCtrlDlg();

// 对话框数据
	enum { IDD = IDD_FORMVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPass();
	afx_msg void OnBnClickedButtonFailed();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	CToolTipCtrl m_ToolTip;
	CListCtrl m_test_config;
	CStaticEx m_Status;
	CStaticEx m_test_Status;
	CStaticEx m_test_2_Status;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticTestContent();
	afx_msg void OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_test_pass_btn;
	afx_msg void OnBnClickedButtonFindIp();
	afx_msg void OnBnClickedButtonPreTest();
	//CListCtrl m_test_wifi_list;
	afx_msg void OnLvnItemchangedListWifi(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckSdTest();
	afx_msg void OnBnClickedCheckWifiTest();
	afx_msg void OnBnClickedRadioRtsp();
	afx_msg void OnBnClickedRadioOnvif();
	CButton m_test_fail_btn;
	afx_msg void OnBnClickedButtonSdRetest();
	afx_msg void OnBnClickedButtonWifiRetest();
	afx_msg void OnBnClickedCheckAll();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonUpdateAll();
	afx_msg void OnBnClickedCheckUpdateFlag();
	afx_msg void OnBnClickedRadioNet();
	afx_msg void OnBnClickedRadioWifi();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnStnClickedStaticTestTitle();
	afx_msg void OnStnClickedStatic1();
	afx_msg void OnStnClickedStatic2();
	afx_msg void OnStnClickedStatictextwifi();
	afx_msg void OnStnClickedStaticTestVideo();
};
