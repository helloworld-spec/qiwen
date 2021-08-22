// TencentFontCreate.h : main header file for the TENCENTFONTCREATE application
//

#if !defined(AFX_TENCENTFONTCREATE_H__27774AF9_FB8A_43DE_A7F1_81E4309D2DAC__INCLUDED_)
#define AFX_TENCENTFONTCREATE_H__27774AF9_FB8A_43DE_A7F1_81E4309D2DAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTencentFontCreateApp:
// See TencentFontCreate.cpp for the implementation of this class
//

class CTencentFontCreateApp : public CWinApp
{
public:
	CTencentFontCreateApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTencentFontCreateApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTencentFontCreateApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TENCENTFONTCREATE_H__27774AF9_FB8A_43DE_A7F1_81E4309D2DAC__INCLUDED_)
