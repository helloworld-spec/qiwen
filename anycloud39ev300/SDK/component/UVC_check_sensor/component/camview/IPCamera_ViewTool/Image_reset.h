#pragma once

#include "resource.h"
#include "afxwin.h"


#define YUV_WIDTH_720P	1280
#define YUV_HEIGHT_720P	720
#define YUV_WIDTH_960P	1280
#define YUV_HEIGHT_960P	960


typedef enum 
{
	IMG_MODE_720P = 0,
	IMG_MODE_960P = 0,

	IMG_MODE_NUM
} T_IMG_MODE;


// CImage_reset 对话框

class CImage_reset : public CDialog
{
	DECLARE_DYNAMIC(CImage_reset)

public:
	CImage_reset(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CImage_reset();

// 对话框数据
	enum { IDD = IDD_DIALOG_IMAGE_RESET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()


public:
	CRect m_ImgRect;
	TCHAR m_imgpath[260];
	CPoint m_keyPoint[2];
	bool m_bRefreshFlag;
	bool m_bInit;
	BOOL m_drag;
	int m_moveflag;

	unsigned char m_img_mode;
	unsigned short m_img_width;
	unsigned short m_img_height;
	
	unsigned char *m_yuvbuf;
	unsigned char *m_bmpbuf;
	
	void SetImgPath(TCHAR * path);
	bool GetBMP(void);
	unsigned char* LoadYuvData(TCHAR* path);
	void BMShow(CDC *pDC, int x, int y, int width, int height);
	bool GetBMP_net(void);
	void BMShow_net(CDC *pDC, int x, int y, int width, int height);

	BOOL CImage_reset::OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonOpenYuv();
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonComeback();
	afx_msg void OnBnClickedButtonCorrectImage();
	afx_msg void OnBnClickedButtonShow();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonRight();
	//afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnKillfocusEditX();
	afx_msg void OnEnUpdateEditX();
	afx_msg void OnNMThemeChangedEditX(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnErrspaceEditX();
	CEdit m_img_x;
	CEdit m_img_y;
};
