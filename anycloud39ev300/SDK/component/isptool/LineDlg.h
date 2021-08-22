#if !defined(AFX_LINEDLG_H__969F4C22_9FD6_4439_856B_6827CB3AA1DF__INCLUDED_)
#define AFX_LINEDLG_H__969F4C22_9FD6_4439_856B_6827CB3AA1DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LineDlg.h : header file
//
#include <vector>
#include "isp_struct.h"

using namespace std;
#define LINE_LEVEL_NUM			256

#define LINE_WINDOW_HEIGHT		384
#define LINE_WINDOW_WIDTH		384

#define LINE_X_MAX				127
#define LINE_Y_MAX				255
#define LINE_X_MIN				(-128)
#define LINE_Y_MIN				(-256)


#define LINE_KEY_POINT_MAX		16

/////////////////////////////////////////////////////////////////////////////
// CLineDlg dialog

class CLineDlg : public CDialog
{
// Construction
public:
	CLineDlg(CWnd* pParent = NULL);   // standard constructor
	afx_msg void OnClose();
	vector<CPoint>* GetKeyPts(void);
	void GetLevel(char* pbuf, int* size);
	void SetLevel(char* pbuf, int size);
	void Refresh(void);
	void SetKeyPts(vector<CPoint>* keypts, T_U16 *key, T_S16* curve);
// Dialog Data
	//{{AFX_DATA(CLineDlg)
	enum { IDD = IDD_DIALOG_LINE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLineDlg)
	protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLineDlg)
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnButtonReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void GetImageLevel();

private:

	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;

	vector<CPoint> m_keyPts;
	short m_level[LINE_LEVEL_NUM];
	BOOL m_drag;
	int m_moveflag;

	ULONG_PTR m_pGdiToken;
	HCURSOR	m_handCursor;

	CRect m_Rect, m_CurveRect, m_CurveFrameRect;

	int	m_level_num;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LINEDLG_H__969F4C22_9FD6_4439_856B_6827CB3AA1DF__INCLUDED_)
