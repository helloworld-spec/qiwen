#pragma once

#include "printf_set.h"
#include "afxcmn.h"

// CBottomCtrlDlg 对话框

class CBottomCtrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CBottomCtrlDlg)

public:
	CBottomCtrlDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBottomCtrlDlg();

	Cprintf_set  printf_set;

	BOOL start_record_flag;

	// 对话框数据
	enum { IDD = IDD_BOTTOMVIEW };

	BOOL ReadFileWithAlloc( LPTSTR szFileName, LPDWORD pdwSize, LPBYTE *ppBytes ) ; 
	BOOL RawDataToPrinter( LPTSTR szPrinterName, LPBYTE lpData, DWORD dwCount ); 
	void print_main( void );
	void print_LAB_main( void );
	void print_image_main( TCHAR *path_filename, TCHAR *uid);

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
	afx_msg void OnBnClickedButtonAutoMove();
	afx_msg void OnBnClickedButtonDeleteUid();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnDropDownButtonUp(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnKillfocusButtonUp();
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEditDeleteUid();
	afx_msg void OnBnClickedButtonCloseMonitor();
	afx_msg void OnBnClickedButtonGetLisence();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonPrintfSet();
	afx_msg void OnBnClickedButtonPrintf();
	afx_msg void OnBnClickedButtonUpdateAll();

	afx_msg void OnBnClickedButtonImgaeReset();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonRecord();
	//CListCtrl m_test_wifi;
	CListCtrl m_test_wifi_list;
	afx_msg void OnBnClickedWriteMac();
	afx_msg void OnBnClickedButtonWriteMac();
	afx_msg void OnLvnItemchangedListWifi(NMHDR *pNMHDR, LRESULT *pResult);
};
