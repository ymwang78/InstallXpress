/*******************************************************
* 文件描叙:互斥锁;
* 日期:;
*******************************************************/
#ifndef _SYNCMUTEX_H
#define _SYNCMUTEX_H

#include <windows.h>

class  Mutex {
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();

private:
	CRITICAL_SECTION cs;
};

inline Mutex::Mutex()
{
	InitializeCriticalSection(&cs);
}

inline Mutex::~Mutex()
{
	DeleteCriticalSection(&cs);
}

inline void Mutex::lock()
{
	EnterCriticalSection(&cs);
}

inline void Mutex::unlock()
{
	LeaveCriticalSection(&cs);
}

class AutoMutex
{
public:
	AutoMutex();
	~AutoMutex();
private:
	CRITICAL_SECTION cs;
};

inline AutoMutex::AutoMutex()
{
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
}

inline AutoMutex::~AutoMutex()
{
	LeaveCriticalSection(&cs);
	DeleteCriticalSection(&cs);
}
#endif 