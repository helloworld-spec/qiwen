// AutoLock.h: interface for the CAutoLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOLOCK_H__9AA7E003_C4F9_4287_BAB9_40C5B48C585C__INCLUDED_)
#define AFX_AUTOLOCK_H__9AA7E003_C4F9_4287_BAB9_40C5B48C585C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CriticalSection
{
public:
	CriticalSection(void);
	virtual ~CriticalSection(void);
	
	void lock();
	
	void unlock();
	
private:
	CRITICAL_SECTION m_cCriSec;
};

class CAutoLock  
{
	public:
		CAutoLock( CriticalSection * pCS );
		~CAutoLock(void);
		
	private:
		CriticalSection * m_lock;
};

#endif // !defined(AFX_AUTOLOCK_H__9AA7E003_C4F9_4287_BAB9_40C5B48C585C__INCLUDED_)
