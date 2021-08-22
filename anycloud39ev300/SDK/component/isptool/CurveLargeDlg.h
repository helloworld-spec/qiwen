#if !defined(AFX_CURVELARGEDLG_H__3ECD8ABB_6F90_46A9_8828_5164112F7BC0__INCLUDED_)
#define AFX_CURVELARGEDLG_H__3ECD8ABB_6F90_46A9_8828_5164112F7BC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CurveLargeDlg.h : header file
//

#include <vector>
#include "isp_struct.h"
#include "basepage.h"

using namespace std;
#define LEVEL_NUM				129
#define KEY_POINT_MAX			8


#define LCURVE_WINDOW_HEIGHT		512
#define LCURVE_WINDOW_WIDTH			512

#define X_MAX					(LEVEL_NUM - 1)
#define Y_MAX					1023



/////////////////////////////////////////////////////////////////////////////
// CCurveLargeDlg dialog

class CCurveLargeDlg : public CDialog, public CBasePage
{
// Construction
public:
	BOOL m_no_key_flag;
	BOOL m_no_key_show_flag;

	CCurveLargeDlg(CWnd* pParent = NULL);   // standard constructor
	afx_msg void OnClose();
	vector<CPoint>* GetKeyPts(void);
	void GetLevel(char* pbuf, int* size);
	void SetLevel(char* pbuf, int size);
	int SetEnable(BOOL bEnable);
	void SetLevelNum(int levelNum);
	void Refresh(void);
	void SetKeyPts(vector<CPoint>* keypts, T_U16 *key, T_U16* curve);

	afx_msg void OnButtonReset();
	BOOL GetCheck(void);
// Dialog Data
	//{{AFX_DATA(CCurveLargeDlg)
	enum { IDD = IDD_DIALOG_CURVELARGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCurveLargeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	// Generated message map functions
	//{{AFX_MSG(CCurveLargeDlg)
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
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CURVELARGEDLG_H__3ECD8ABB_6F90_46A9_8828_5164112F7BC0__INCLUDED_)
