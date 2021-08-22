#if !defined(AFX_CURVEDLG_H__680614C0_1B4F_46C0_9794_85E53785F919__INCLUDED_)
#define AFX_CURVEDLG_H__680614C0_1B4F_46C0_9794_85E53785F919__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CurveDlg.h : header file
//

#include <vector>
#include "isp_struct.h"
#include "basepage.h"

using namespace std;
#define LEVEL_NUM				129
#define KEY_POINT_MAX			8


#define CURVE_WINDOW_HEIGHT		256
#define CURVE_WINDOW_WIDTH		256

#define X_MAX					(LEVEL_NUM - 1)
#define Y_MAX					1023



/////////////////////////////////////////////////////////////////////////////
// CCurveDlg dialog

class CCurveDlg : public CDialog, public CBasePage
{
// Construction
public:
	BOOL m_no_key_flag;
	BOOL m_no_key_show_flag;

	CCurveDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCurveDlg();
	afx_msg void OnClose();
	vector<CPoint>* GetKeyPts(void);
	void GetLevel(char* pbuf, int* size);
	void SetLevel(char* pbuf, int size);
	int SetEnable(BOOL bEnable);
	void SetLevelNum(int levelNum);
	void Refresh(void);
	void SetKeyPts(vector<CPoint>* keypts, T_U16 *key, T_U16* curve);
	void SetWdrTh(T_U16 th0, T_U16 th1);

	afx_msg void OnButtonReset();
	BOOL GetCheck(void);
// Dialog Data
	//{{AFX_DATA(CCurveDlg)
	enum { IDD = IDD_DIALOG_CURVE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCurveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	// Generated message map functions
	//{{AFX_MSG(CCurveDlg)
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnCheckNoKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	//afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void GetImageLevel(BOOL bEnable);
	
private:

	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;

	vector<CPoint> m_keyPts;
	//vector<short>	m_FileLoadPtsY;
	//vector<short>	m_FromServerPtsY;
	unsigned short m_level[LEVEL_NUM];
	BOOL m_drag;
	int m_moveflag;

	ULONG_PTR m_pGdiToken;
	HCURSOR	m_handCursor;

	CRect m_Rect, m_CurveRect, m_CurveFrameRect;
	
	//BOOL m_bUseFildLoadCurve;
	BOOL m_bEnable;
	int	m_level_num;
	T_U16 m_wdr_th[2];
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CURVEDLG_H__680614C0_1B4F_46C0_9794_85E53785F919__INCLUDED_)
