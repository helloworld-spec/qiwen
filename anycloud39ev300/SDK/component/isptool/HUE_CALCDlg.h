#if !defined(AFX_HUE_CALCDLG_H__E5981AFD_AC97_4383_91A5_5B364CA610BC__INCLUDED_)
#define AFX_HUE_CALCDLG_H__E5981AFD_AC97_4383_91A5_5B364CA610BC__INCLUDED_

#include "basepage.h"
#include "HUE_ImgDlg.h"
#include "anyka_types.h"
#include "isp_struct.h"
#include "CCM_RGBvalDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HUE_CALCDlg.h : header file
//
//RGB Transform to Lab
#define RGB2L(R,G,B)   ((54*(R) +183*(G) +18*(B))>>8)
#define RGB2U(R,G,B)   (((-31*(R) -97*(G) +128*(B))>>8))
#define RGB2V(R,G,B)   (((128*(R) -116*(G) -12*(B))>>8))

#define   A_HUE      0
#define   TL84_HUE   1
#define   D50_HUE    2
#define   D65_HUE    3

#define CLIP(val, min, max)  ((val) > max? max : ((val) < min ? min: (val))) 

//24色块中前18色块RGB单元数据结构
typedef struct sRGB{
	BYTE R;
	BYTE G;
	BYTE B;
}RGB;

typedef struct yuv{
	int y;
	int u;
	int v;
}YUV;

int CalcHUEparam(RGB Original_RGB[],const RGB Target_RGB[],AK_ISP_HUE* para, int sat);

/////////////////////////////////////////////////////////////////////////////
// CHUE_CALCDlg dialog

class CHUE_CALCDlg : public CDialog, public CBasePage
{
// Construction
public:
	CHUE_CALCDlg(CWnd* pParent = NULL);   // standard constructor
	CCCM_RGBvalDlg m_RGBvalDlg;
	AK_ISP_HUE hue_para;

// Dialog Data
	//{{AFX_DATA(CHUE_CALCDlg)
	enum { IDD = IDD_DIALOG_HUE_CALC };
	CSliderCtrl	m_slider_saturation;
	CSpinButtonCtrl	m_spin_saturation;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	bool Show_flag;
	bool Calculate_flag;
	bool export_hue_flag;
	bool m_img_bConnect;
	bool m_bRefreshFlag;
	bool m_bInit;
	int Saturation;
	int last_Saturation;
	T_U8 *m_bmpbuf;
	HANDLE g_Calc_Thread;

	void SetImgPath(char* path);
	void Img_SetConnectState(bool bConnect);
	void Calc_Input_RGB();
	void Calc_Frames();
	BOOL Creat_Calc_thread(void);
	void Close_Calc_thread(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHUE_CALCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHUE_CALCDlg)
	afx_msg void OnButtonGetYuvImg();
	afx_msg void OnButtonOpenYuvImg();
	afx_msg void OnPaint();
	afx_msg void OnButtonRgbVal();
	afx_msg void OnButtonCalc();
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButtonExportHue();
	afx_msg void OnKillfocusEditSaturation();
	afx_msg void OnDeltaposSpinSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	char m_imgpath[260];
	T_U8 *m_yuvbuf;
	CRect m_ImgRect;
	CPoint m_keyPoint[4];
	BOOL m_drag;
	int m_moveflag;
	T_U8 m_img_mode;
	CPoint m_framePoint[4][6][2];
	T_U16 m_img_width;
	T_U16 m_img_height;
	T_U16 m_img_mutil;

	void BMShow(CDC *pDC, int x, int y, int width, int height);
	bool GetBMP(void);
	T_U8* LoadYuvData(char* path) ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUE_CALCDLG_H__E5981AFD_AC97_4383_91A5_5B364CA610BC__INCLUDED_)
