#if !defined(AFX_HUE_LINEDLG_H__A9EE73CD_62C0_4345_B113_24608DF345EC__INCLUDED_)
#define AFX_HUE_LINEDLG_H__A9EE73CD_62C0_4345_B113_24608DF345EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HUE_lineDlg.h : header file
//

#include <vector>
#include "isp_struct.h"

using namespace std;
#define KEY_POINT_NUM			17
#define LINE_LEVEL_NUM			256

#define HUE_LINE_WINDOW_HEIGHT		512
#define HUE_LINE_WINDOW_WIDTH		512

#define HUE_LINE_X_MAX				127
#define HUE_LINE_Y_MAX				127
#define HUE_LINE_X_MIN				(-128)
#define HUE_LINE_Y_MIN				(-128)


#define HUE_LINE_KEY_POINT		16

/////////////////////////////////////////////////////////////////////////////
// CHUE_lineDlg dialog

class CHUE_lineDlg : public CDialog
{
// Construction
public:
	CHUE_lineDlg(CWnd* pParent = NULL);   // standard constructor
	vector<CPoint>* GetKeyPts(void);
	void Refresh(void);
	void SetKeyPts(vector<CPoint>* keypts, T_S16 *key_x, T_S16 *key_y);
	void Get_Key_point(void);
	void Close();
	void Set_background_color(void);
	void SetDataValue();
	void GetDataValue();
	void Set_init_DataValue(); 
	T_U8 account_s_value( T_S16 ket_x, T_S16 ket_y);
	BOOL check_all_data_is_zero();

// Dialog Data
	//{{AFX_DATA(CHUE_lineDlg)
	enum { IDD = IDD_DIALOG_HUE_LINE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHUE_lineDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:

	AK_ISP_HUE m_hue_info;
	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;
	BOOL m_show_backcolor;

	ULONG_PTR m_pGdiToken;
	HCURSOR	m_handCursor;
	T_S16 ket_point_x[KEY_POINT_NUM];
	T_S16 ket_point_y[KEY_POINT_NUM];
	double ket_point_degree[KEY_POINT_NUM];

	CRect m_Rect, m_CurveRect, m_CurveFrameRect;

	vector<CPoint> m_keyPts;
	short m_level[LINE_LEVEL_NUM];
	BOOL m_drag;
	int m_moveflag;
	int	m_level_num;

	void GetImageLevel();

protected:

	// Generated message map functions
	//{{AFX_MSG(CHUE_lineDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnButtonReset();
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUE_LINEDLG_H__A9EE73CD_62C0_4345_B113_24608DF345EC__INCLUDED_)
