#pragma once

// CBottomCtrlDlg 对话框

class CBottomCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CBottomCtrlDlg)

public:
	CBottomCtrlDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBottomCtrlDlg();

	// 对话框数据
	enum { IDD = IDD_BOTTOMVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonConfigure();
	afx_msg void OnBnClickedButtonWriteUid();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnEnChangeEditPresent();
};
