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


	BOOL m_video_connet_flag;

	BOOL m_save_video_flag;
	BOOL m_start_video_flag;
	BOOL m_start_stop_flag;
	BOOL m_play_video_flag;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_test_config;
	CStaticEx m_Status;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnStnClickedStaticTestContent();
	afx_msg void OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_test_pass_btn;
	CListCtrl m_test_wifi_list;
	CButton m_test_fail_btn;
	afx_msg void OnBnClickedButtonConnet();
	afx_msg void OnBnClickedButtonSet();
	afx_msg void OnBnClickedRadioVga();
	afx_msg void OnBnClickedRadio720p();
	afx_msg void OnBnClickedRadio960p();
	afx_msg void OnBnClickedRadioClinte();
	afx_msg void OnBnClickedRadioServer();
	afx_msg void OnBnClickedCheckSaveVideo();
	afx_msg void OnBnClickedButtonVideoPath();
	afx_msg void OnEnChangeEditSaveVideo();
	afx_msg void OnBnClickedButtonPlayVideo();
	afx_msg void OnEnChangeEditFrame();
	afx_msg void OnEnChangeEditVideoFrame();
	afx_msg void OnBnClickedButtonUpFrame();
	afx_msg void OnBnClickedButtonDownFrame();
	afx_msg void OnBnClickedCheckSetFramePlay();
	afx_msg void OnEnChangeEditPlayFrame();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonPlayFirstFrame();
	afx_msg void OnCbnSelchangeComboSamples();
	afx_msg void OnCbnEditchangeComboSamples();
	afx_msg void OnCbnEditupdateComboSamples();
	afx_msg void OnCbnSelendcancelComboSamples();
	afx_msg void OnBnClickedButtonRecode();
};
