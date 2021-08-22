#if !defined(AFX_IMGDLG_H__FAE8974C_1CF0_4FD7_9538_85A8515831CF__INCLUDED_)
#define AFX_IMGDLG_H__FAE8974C_1CF0_4FD7_9538_85A8515831CF__INCLUDED_
#include "anyka_types.h"
#include "CoordinateDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImgDlg.h : header file
//

#define NUM_COLOR_TMP 5

typedef enum {
    IMG_MODE_720P = 0,
	IMG_MODE_960P,
	IMG_MODE_1080P,

    IMG_MODE_NUM
} T_IMG_MODE;


/////////////////////////////////////////////////////////////////////////////
// CImgDlg dialog

class CImgDlg : public CDialog, public CBasePage
{
// Construction
public:
	CImgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImgDlg)
	enum { IDD = IDD_DIALOG_IMG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	bool m_bRefreshFlag;
	bool m_bInit;

	bool m_img_bConnect;

	CCoordinateDlg m_CoordinateDlg;

	void SetImgPath(char* path);
	void Img_SetConnectState(bool bConnect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImgDlg)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButtonReset();
	afx_msg void OnCheckAEnable();
	afx_msg void OnCheckTL84Enable();
	afx_msg void OnCheckD50Enable();
	afx_msg void OnCheckD65Enable();
	afx_msg void OnCheckD75Enable();
	afx_msg void OnCheckDfault1Enable();
	afx_msg void OnCheckDfault2Enable();
	afx_msg void OnCheckDfault3Enable();
	afx_msg void OnCheckDfault4Enable();
	afx_msg void OnCheckDfault5Enable();
	afx_msg void OnButtonGetYuvImg();
	afx_msg void OnButtonOpenYuvImg(); 
	afx_msg void OnTimer(UINT nIDEvent);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	char m_imgpath[260];
	T_U8 *m_yuvbuf;
	T_U8 *m_bmpbuf;
	CRect m_ImgRect;
	CPoint m_keyPoint[2];
	BOOL m_drag;
	int m_moveflag;

	float GrLow;
	float GrHigh;
	float GrAvg;
	float GbLow;
	float GbHigh;
	float GbAvg;
	float RbLow;
	float RbHigh;
	float RbAvg;

	T_U8 Ravg;
	T_U8 Gavg;
	T_U8 Bavg;
	T_U8 Yavg;

	T_U8 m_img_mode;
	T_U16 m_img_width;
	T_U16 m_img_height;
	T_U16 m_img_mutil;
	
	int m_color_tmp[NUM_COLOR_TMP];
	int m_Gr[NUM_COLOR_TMP];
	int m_Gb[NUM_COLOR_TMP];
	UINT m_timer;
	

	void BMShow(CDC *pDC, int x, int y, int width, int height);
	bool GetBMP(void);
	void Calc_Ratio();
	void SetDataValue(void);
	T_U8* LoadYuvData(char* path) ;
	void Calc_Edit_GrGb() ;
	void Calc_WB() ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMGDLG_H__FAE8974C_1CF0_4FD7_9538_85A8515831CF__INCLUDED_)
