#pragma once
#include "afxwin.h"

class CStaticEx :public CStatic
{
	DECLARE_DYNAMIC(CStaticEx)

public:
	CStaticEx();
	virtual ~CStaticEx();
	void SetFontColor(COLORREF clr);
	void SetFontSize(UINT size);
	void UpdateCtrlArea();
	void ReconstructFont();


	LOGFONT		m_lf;
	CFont*		m_font;


protected:
	DECLARE_MESSAGE_MAP()
	COLORREF m_foreColor;
	int m_fontSize;

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT );

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};