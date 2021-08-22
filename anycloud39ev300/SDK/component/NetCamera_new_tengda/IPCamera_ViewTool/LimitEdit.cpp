// EditEx.cpp : 实现文件
//

#include "stdafx.h"
#include "LimitEdit.h"


// CLimitEdit

IMPLEMENT_DYNAMIC(CLimitEdit, CEdit)

CLimitEdit::CLimitEdit()
{

}

CLimitEdit::~CLimitEdit()
{
}


BEGIN_MESSAGE_MAP(CLimitEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CLimitEdit 消息处理程序



void CLimitEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TCHAR ch=nChar;
	if(!(ch>=_T('0')&&ch<=_T('9')|| ch==VK_BACK))
	{
		return;
	}
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
