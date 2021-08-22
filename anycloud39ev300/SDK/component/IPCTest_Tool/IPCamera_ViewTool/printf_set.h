#pragma once

#include "resource.h"
#include "afxwin.h"



#define		   CONFIG_PRINTF          _T("printercfg.ini")

#define     LINE_0     0
#define     LINE_1     1
#define     LINE_2     2
#define     LINE_3     3
#define     LINE_4     4
#define     LINE_5     5

// Cprintf_set 对话框

class Cprintf_set : public CDialog
{
	DECLARE_DYNAMIC(Cprintf_set)

public:
	Cprintf_set(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Cprintf_set();

	void Cprintf_set::Set_printer_info();

// 对话框数据
	enum { IDD = IDD_DIALOG_PRINTF_SET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_printf_set;
	afx_msg void OnBnClickedButtonSetPrintf();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnCbnSelchangeCombo4();
	afx_msg void OnCbnSelchangeCombo5();
	
	afx_msg void OnCbnSelchangeComboNum();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheckPrintfLab();
	afx_msg void OnBnClickedCheckPrintfImg();
	afx_msg void OnBnClickedRadioPrintfLab();
	afx_msg void OnBnClickedRadioPrintfImg();
	afx_msg void OnEnChangeEditWith();
	afx_msg void OnEnChangeEditHeith();
	afx_msg void OnEnChangeEditWithOffset();
	afx_msg void OnEnChangeEditHeithOffset();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEditPid();
	afx_msg void OnEnChangeEditCharHeight();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio();
	afx_msg void OnBnClickedRadioUid();
	afx_msg void OnBnClickedRadioMac();
};
