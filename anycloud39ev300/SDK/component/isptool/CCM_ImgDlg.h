#if !defined(AFX_CCM_IMGDLG_H__84C77F3C_A6B9_4393_BF21_C9F6B57A3612__INCLUDED_)
#define AFX_CCM_IMGDLG_H__84C77F3C_A6B9_4393_BF21_C9F6B57A3612__INCLUDED_
#include "basepage.h"
#include "CCM_LinkageDlg.h"
#include "anyka_types.h"
#include "CCM_RGBvalDlg.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCM_ImgDlg.h : header file
//


#define WIDTHBYTES(bits) (((bits)+31)/32*4)
//RGB Transform to Lab
#define RGB2L(R,G,B)   ((54*(R) +183*(G) +18*(B))>>8)
#define RGB2A(R,G,B)   (((84*(R) -128*(G) +44*(B))>>8)+128 )
#define RGB2B(R,G,B)   (((31*(R) +97*(G) -128*(B))>>8)+128 )
#define LAB2R(L,A,B)   (L +((267*(A-128) +111*(B-128))>>7))
#define LAB2G(L,A,B)   (L -((80*(A-128) +9*(B-128))>>7))
#define LAB2B(L,A,B)   (L +((4*(A-128) -236*(B-128))>>7))

#define CLAMP8(val)    ((val) > 255 ? 255 : ((val) < 0 ? 0: (val)))


#define   A_CCM      0
#define   TL84_CCM   1
#define   D50_CCM    2
#define   D65_CCM    3



#define C11_BEGIN  256
#define C11_END    1000
#define C12_BEGIN  -512
#define C12_END    256
#define C13_BEGIN  -512
#define C13_END    256


#define C21_BEGIN  -512
#define C21_END    256
#define C22_BEGIN  256
#define C22_END    1000
#define C23_BEGIN  -512
#define C23_END    256

#define C31_BEGIN  -512
#define C31_END    256
#define C32_BEGIN  -512
#define C32_END    256
#define C33_BEGIN  256
#define C33_END    1000


//Calc Saturation Para
#define Min_Main_Diagonal   256
#define Max_Diagonal        1500


#define LARGEFIRSTSTEP  20
#define LARGESECONDSTEP 20

#define LOWFIRSTSTEP  5
#define LOWSECONDSTEP 5

#define THRESHOLD 51
//#define TOTAL_THRESHOLD 600

//24色块中前18色块RGB单元数据结构
typedef struct sRGB_temp{
	T_U8 R;
	T_U8 G;
	T_U8 B;
}RGB_temp;

typedef struct ab{
	int L;
	int a;
	int b;
}LAB;

typedef struct point_temp{
	int x;
	int y;
}Point_temp;

typedef struct tagRGB{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
}BMPRGB;

/////////////////////////////////////////////////////////////////////////////
// CCCM_ImgDlg dialog

class CCCM_ImgDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCCM_RGBvalDlg m_RGBvalDlg;
	CCCM_ImgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCCM_ImgDlg)
	enum { IDD = IDD_DIALOG_CCM_IMG };
	CSliderCtrl	m_slider_saturation;
	CSpinButtonCtrl	m_spin_saturation;
	CTabCtrl	m_tab;
	//}}AFX_DATA
	bool m_bRefreshFlag;
	bool m_bInit;
	bool m_bOutput;
	bool export_ccm_flag;
	bool m_img_bConnect;
	bool Gamma_Enable;
	bool Show_flag;
	bool Calculate_flag;
	double m_weight[4][6];
	int CCM_Mat[9];
	int cal_CCM_Mat[9];
	int Saturation;
	int last_Saturation;
	int threshold;
	RGB_temp Out_RGB[24];
	AK_ISP_RGB_GAMMA_ATTR gamma_ccm;
	HANDLE g_Calc_Thread;
	T_U8 *m_bmpbuf;
	T_U8 *m_bmpbuf_out;
	char ccm_type;
	BOOL cal_flag;


	void SetImgPath(char* path);
	void Img_SetConnectState(bool bConnect);
	void Calc_Frames();
	void Calc_Input_RGB();
	BOOL Creat_Calc_thread(void);
	void Close_Calc_thread(void); 
	void Set_CCMValue(void);
	void show_ccm(void);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCCM_ImgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCCM_ImgDlg)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButtonReset();
	afx_msg void OnButtonGetYuvImg();
	afx_msg void OnButtonOpenYuvImg();
	afx_msg void OnButtonRgbVal();
	afx_msg void OnButtonCalc();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioACcm();
	afx_msg void OnRadioTl84Ccm();
	afx_msg void OnRadioD50Ccm();
	afx_msg void OnRadioD65Ccm();
	afx_msg void OnButtonExportCcm();
	afx_msg void OnOutofmemorySpinSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditSaturation();
	afx_msg void OnKillfocusEditSaturation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	char m_imgpath[260];
	T_U8 *m_yuvbuf;
	CRect m_ImgRect;
	CPoint m_keyPoint[4];
	BOOL m_drag;
	int m_moveflag;
	T_U8 m_img_mode;
	T_U16 m_img_width;
	T_U16 m_img_height;
	T_U16 m_img_mutil;
	CPoint m_framePoint[4][6][2];

	void BMShow_output(CDC *pDC, int x, int y, int width, int height);	
	void BMShow(CDC *pDC, int x, int y, int width, int height);
	bool GetBMP(void);
	T_U8* LoadYuvData(char* path) ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCM_IMGDLG_H__84C77F3C_A6B9_4393_BF21_C9F6B57A3612__INCLUDED_)
