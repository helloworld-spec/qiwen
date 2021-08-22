// ISPCTRL_TOOL.h : main header file for the ISPCTRL_TOOL application
//

#if !defined(AFX_ISPCTRL_TOOL_H__42902BC1_E38F_4115_8CEE_E622DA97A3D5__INCLUDED_)
#define AFX_ISPCTRL_TOOL_H__42902BC1_E38F_4115_8CEE_E622DA97A3D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CISPCTRL_TOOLApp:
// See ISPCTRL_TOOL.cpp for the implementation of this class
//

class CISPCTRL_TOOLApp : public CWinApp
{
public:
	CISPCTRL_TOOLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CISPCTRL_TOOLApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CISPCTRL_TOOLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISPCTRL_TOOL_H__42902BC1_E38F_4115_8CEE_E622DA97A3D5__INCLUDED_)
