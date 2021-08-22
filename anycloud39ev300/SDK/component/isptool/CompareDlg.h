#if !defined(AFX_COMPAREDLG_H__74533858_C008_4AFE_96BC_6B7B00C04347__INCLUDED_)
#define AFX_COMPAREDLG_H__74533858_C008_4AFE_96BC_6B7B00C04347__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompareDlg.h : header file
//

#include "isp_struct.h"
#include "basepage.h"



typedef struct  ak_isp_compare_info
{
	int					subfilecnt;
	AK_ISP_INIT_PARAM   buf_isp[SUBFILE_NUM_MAX];
	CFGFILE_HEADINFO    headinfo[SUBFILE_NUM_MAX];
	UINT                sensor_num[SUBFILE_NUM_MAX];

}AK_ISP_COMPARE_INFO;

/////////////////////////////////////////////////////////////////////////////
// CCompareDlg dialog

class CCompareDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCompareDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompareDlg)
	enum { IDD = IDD_DIALOG_COMPARE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompareDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	TCHAR	path_module[MAX_PATH+1];
	TCHAR   m_path[MAX_PATH+1];
	UINT    diffrent_num;
	AK_ISP_INIT_SENSOR      *m_Isp_sensor;
	BOOL    only_day_or_night_flag;
	
	BOOL On_Read(CString path, AK_ISP_COMPARE_INFO *isp_compare_info, UINT *file_len);
	
	void On_Write_diffrent_info(AK_ISP_COMPARE_INFO *isp_compare_1, AK_ISP_COMPARE_INFO *isp_compare_2);
	TCHAR *ConvertAbsolutePath(LPCTSTR path);
	void Write_info(CStdioFile *pFile, AK_ISP_COMPARE_INFO *isp_compare_1, AK_ISP_COMPARE_INFO *isp_compare_2);
	void Write_BLC_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2);
	void Write_lsc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2); 
	void Write_raw_lut_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2); 
	void Write_nr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_3dnr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_gb_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_demo_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_gamma_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2); 
	void Write_ccm_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_fcs_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_wdr_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_edge_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_sharp_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_saturation_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_contrast_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_rgb2yuv_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_effect_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_dpc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_weight_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_af_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_wb_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_exp_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2) ;
	void Write_misc_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2);
	void Write_y_gamma_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2);
	void Write_hue_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2);
	void Write_sensor_info(CStdioFile *pFile, AK_ISP_INIT_PARAM *isp_compare_1, AK_ISP_INIT_PARAM *isp_compare_2, UINT sensor_1_num, UINT sensor_2_num);
	bool CheckSubFileHeadInfo(CFGFILE_HEADINFO* headinfo);
	int CheckTotalFileData(char* cfgBuf, T_U32 size);
protected:

	// Generated message map functions
	//{{AFX_MSG(CCompareDlg)
	virtual void OnOK();
	afx_msg void OnButtonCompareFile1();
	afx_msg void OnButtonCompareFile2();
	afx_msg void OnButtonCompare();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPAREDLG_H__74533858_C008_4AFE_96BC_6B7B00C04347__INCLUDED_)
