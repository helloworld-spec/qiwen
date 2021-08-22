// anyka_TestTool.h : main header file for the ANYKA_TESTTOOL application
//

#if !defined(AFX_ANYKA_TESTTOOL_H__330760C7_30C6_4E31_9E41_693082FF7371__INCLUDED_)
#define AFX_ANYKA_TESTTOOL_H__330760C7_30C6_4E31_9E41_693082FF7371__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAnyka_TestToolApp:
// See anyka_TestTool.cpp for the implementation of this class
//

class CAnyka_TestToolApp : public CWinApp
{
public:
	CAnyka_TestToolApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnyka_TestToolApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAnyka_TestToolApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANYKA_TESTTOOL_H__330760C7_30C6_4E31_9E41_693082FF7371__INCLUDED_)
