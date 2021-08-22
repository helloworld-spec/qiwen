#if !defined(AFX_LSCBRIGHTNESSCALCDLG_H__3BF23653_783C_4DC8_B556_0902CB816551__INCLUDED_)
#define AFX_LSCBRIGHTNESSCALCDLG_H__3BF23653_783C_4DC8_B556_0902CB816551__INCLUDED_
#include "basepage.h"
#include "isp_struct.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LscBrightnessCalcDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLscBrightnessCalcDlg dialog

#define BRIGHTNESS_POINT_NUM  (128)

class CLscBrightnessCalcDlg : public CDialog, public CBasePage
{
// Construction
public:
	CLscBrightnessCalcDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLscBrightnessCalcDlg)
	enum { IDD = IDD_DIALOG_LSC_BRIGHTNESS_CALC };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	AK_ISP_INIT_LSC m_Lsc;
	int m_Strength;

	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;
	CBitmap m_GrayBarBitmap;

	CRect m_Rect[2][2];
	CRect m_ImgRect;

	bool m_bRefreshFlag;
	bool m_bInit;
	bool m_img_bConnect;
	
	char m_imgpath[260];
	T_U8 *m_yuvbuf;
	T_U8 *m_bmpbuf;
	T_U16 m_img_width;
	T_U16 m_img_height;
	T_U16 m_img_mutil;

	T_U8 m_R[2][2][BRIGHTNESS_POINT_NUM + 1];
	Point m_Points_R[2][2][BRIGHTNESS_POINT_NUM + 1];

	T_U8 m_G[2][2][BRIGHTNESS_POINT_NUM + 1];
	Point m_Points_G[2][2][BRIGHTNESS_POINT_NUM + 1];

	T_U8 m_B[2][2][BRIGHTNESS_POINT_NUM + 1];
	Point m_Points_B[2][2][BRIGHTNESS_POINT_NUM + 1];

	void DrawBrightnessLine(CDC *pDC);
	void SetImgPath(char* path);
	bool GetBMP(void);
	void BMShow(CDC *pDC, int x, int y, int width, int height);
	void Calc_Brightness(void);
	T_U8* LoadYuvData(char* path) ;
	void Img_SetConnectState(bool bConnect);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLscBrightnessCalcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLscBrightnessCalcDlg)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnButtonGetYuvImg();
	afx_msg void OnButtonOpenYuvImg();
	afx_msg void OnButtonCalc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LSCBRIGHTNESSCALCDLG_H__3BF23653_783C_4DC8_B556_0902CB816551__INCLUDED_)
