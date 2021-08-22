// Clock.h: interface for the CClock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLOCK_H__33C4AB73_811C_4CA6_8CEA_9ED320ADD69D__INCLUDED_)
#define AFX_CLOCK_H__33C4AB73_811C_4CA6_8CEA_9ED320ADD69D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Autolock.h"

class CClock  
{
public:
	CClock();
	virtual ~CClock();
	int start(struct timeval sTime);
	int ReStart(struct timeval sTime);
	
	BOOL IsStart();
	
	int GetMsSinceStart(UINT64 & nMs);
	
	int convertTime2MsSinceStart(struct timeval sTime, UINT64 & nMs);
	
	int ReInit();
	
private:
	BOOL				m_bIsStart;
	double				m_dfFreq;
	LONGLONG			m_nStartTime;
	struct timeval		m_sFirstTime;
	CriticalSection		m_cs;

};

#endif // !defined(AFX_CLOCK_H__33C4AB73_811C_4CA6_8CEA_9ED320ADD69D__INCLUDED_)
