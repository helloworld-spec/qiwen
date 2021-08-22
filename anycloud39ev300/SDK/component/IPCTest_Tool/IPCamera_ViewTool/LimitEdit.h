#pragma once


// CLimitEdit

class CLimitEdit : public CEdit
{
	DECLARE_DYNAMIC(CLimitEdit)

public:
	CLimitEdit();
	virtual ~CLimitEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};


