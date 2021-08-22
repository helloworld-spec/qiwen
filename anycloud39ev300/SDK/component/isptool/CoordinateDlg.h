#if !defined(AFX_COORDINATEDLG_H__FCF12A77_408D_47C8_9772_FDDCB3708EBA__INCLUDED_)
#define AFX_COORDINATEDLG_H__FCF12A77_408D_47C8_9772_FDDCB3708EBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CoordinateDlg.h : header file
//

#include <vector>
#include "isp_struct.h"
#include "basepage.h"

/////////////////////////////////////////////////////////////////////////////
// CCoordinateDlg dialog

using namespace std;

#define COORDINATE_LEVEL_NUM			512
#define COORDINATE_WINDOW_HEIGHT		512
#define COORDINATE_WINDOW_WIDTH		    512
#define GBR_MUTIL		(COORDINATE_WINDOW_WIDTH / GBR_VALUE)
#define GBR_VALUE	256   //与谢鹏鹤讨论，基于框好看先，同时GBR的值一般不超过256，所以定最大值是256


class CCoordinateDlg : public CDialog, public CBasePage
{
// Construction
public:
	CCoordinateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCoordinateDlg)
	enum { IDD = IDD_DIALOG_COORDINATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCoordinateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:

	CPoint m_keyPoint[2];
	CPoint m_keyPoint_BR[10][2];

	// Generated message map functions
	//{{AFX_MSG(CCoordinateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnButtonCoordinateReset();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point); 
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButtonGetWbInfo();
	afx_msg void OnButtonSetWbInfo();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnButtonCalcWb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:	
	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;

	vector<CPoint> m_keyPts;
	short m_level[COORDINATE_LEVEL_NUM];
	
	BOOL m_drag;
	int m_moveflag;
	int m_move_idex_flag;

	BOOL m_drag_br;
	int m_moveflag_br;
	int m_move_idex_flag_br;
	
	ULONG_PTR m_pGdiToken;
	HCURSOR	m_handCursor;
	
	CRect m_Rect, m_CoorRect, m_CoorFrameRect;
	
	int	m_level_num;


public:
	CPoint *InPoint_GBR;
	UINT InPoint_GBR_num;
	AK_ISP_INIT_WB	m_coor_Isp_wb;
	AK_ISP_AWB_ATTR m_calc_awb;
	int m_get_wbinfo_flag;
	UINT    ctrl_time;
    DWORD   used_time;
	CPoint m_keyPoint_GBR[10][2];

	BOOL A_enable;
	BOOL TL84_enable;
	BOOL D50_enable;
	BOOL D65_enable;
	BOOL D75_enable;
	BOOL Default1_enable;
	BOOL Default2_enable;
	BOOL Default3_enable;
	BOOL Default4_enable;
	BOOL Default5_enable;
	BOOL m_update_flag;
	BOOL m_calc_flag;

	void Close();
	void Set_keyPoint(AK_ISP_AWB_ATTR *p_awb);
	void Init_keyPoint_reset();
	void StartTimer();
	void StopTimer();
	void Get_keyPoint();
	void inset_key();
	void CoordinateReset();
	void ShowCalcWbInfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COORDINATEDLG_H__FCF12A77_408D_47C8_9772_FDDCB3708EBA__INCLUDED_)
