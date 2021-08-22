// AutoLock.cpp: implementation of the CAutoLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AutoLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CriticalSection::CriticalSection(void)
{
	InitializeCriticalSection( &m_cCriSec );
}

CriticalSection::~CriticalSection(void)
{
	DeleteCriticalSection( &m_cCriSec );
}

void CriticalSection::lock()
{
	EnterCriticalSection( &m_cCriSec );
}

void CriticalSection::unlock()
{
	LeaveCriticalSection( &m_cCriSec );
}


CAutoLock::CAutoLock( CriticalSection * pCS )
: m_lock( pCS )
{
	m_lock->lock();
}

CAutoLock::~CAutoLock(void)
{
	m_lock->unlock();
}
