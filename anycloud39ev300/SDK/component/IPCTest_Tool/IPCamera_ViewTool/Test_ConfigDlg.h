#pragma once
#include "resource.h"
#include "LimitEdit.h"

// CTest_ConfigDlg 对话框

class CTest_ConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CTest_ConfigDlg)

public:
	CTest_ConfigDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTest_ConfigDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_CONFIGURE };

public:
	void Set_confim();
	CLimitEdit m_limit_edit,m_limit_edit1;
	BOOL Mac_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf);
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCheckVideo();
	afx_msg void OnBnClickedCheckMonitoring();
	afx_msg void OnBnClickedCheckIntercom();
	afx_msg void OnBnClickedCheckCradleHead();
	afx_msg void OnBnClickedCheckSdcard();
	afx_msg void OnBnClickedCheckWifi();
	//afx_msg void OnBnClickedCheckInfrared();
	afx_msg void OnBnClickedCheckReset();
	afx_msg void OnBnClickedCheckMac();
	afx_msg void OnBnClickedCheckBurnUid();
	afx_msg void OnBnClickedCheckIrcut();
	afx_msg void OnEnChangeEditStartMac();
	
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeTestTimeMonitor();
	afx_msg void OnEnChangeTestTimeReset();
//	afx_msg void OnEnChangeEditNewestVersion();
	afx_msg void OnEnChangeEditEndMac();
	afx_msg void OnEnChangeTestTimeIrcut();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheckNeedPrintf();
	afx_msg void OnBnClickedCheckNeedCloseMonitor();
	afx_msg void OnBnClickedRadioFindFile();
	afx_msg void OnBnClickedRadioChar();
};
