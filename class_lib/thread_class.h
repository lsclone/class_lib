#pragma once

#include <stdio.h>
#include <assert.h>
#ifdef WIN32
#include <process.h>
#include <windows.h>
#else
/* /user/include/pthread.h */
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#endif

/*
**	/usr/include/bits/pthreadtypes.h
**	typedef unsigned long pthread_t;
*/

#ifdef WIN32
typedef HANDLE			THREAD_HANDLE;
typedef unsigned long	TID;

/******* update on 2014/06/19 *******/
#ifdef SECOND_VERTION
#define DWORD			unsigned
#define LPVOID			void*
#define WINAPI			__stdcall
#endif
/******* update on 2014/06/19 *******/

#else
typedef pthread_t THREAD_HANDLE;
typedef pthread_t TID;
typedef void*     DWORD;
typedef void*     LPVOID;
#define WINAPI
#endif

#ifdef CLASS_EXPORT
#define THREAD_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define THREAD_API __declspec(dllimport)
#else
#define THREAD_API
#endif

class THREAD_API Thread
{
public:
	Thread();
	virtual ~Thread() {}
	bool start();
	TID getThreadID() { return m_threadID; }

protected:
	static DWORD WINAPI ThreadCallBack(LPVOID parameter);
	virtual void run() = 0;

private:
	THREAD_HANDLE m_hThreadHandle;
	TID m_threadID;
};

class WaitCondition;

class THREAD_API Lock
{
	friend class WaitCondition;

public:
	Lock();
	~Lock();
	void lock();
	void unlock();

private:
#ifdef WIN32
	CRITICAL_SECTION section;
#else
	pthread_mutex_t mutex;
#endif
};

class THREAD_API AutoLock
{
public:
	AutoLock(Lock &lock);
	~AutoLock();

private:
	Lock& m_lock;
};

class THREAD_API WaitCondition
{
public:
	WaitCondition();
	~WaitCondition();

	/* wait until wakeup */
	void Wait(Lock& lock);

	/* wait until wakeup or timeout */
	void Wait(Lock& lock, unsigned long millisecond);

	void WakeUp();

private:
#ifdef WIN32
	HANDLE m_cond;
#else
	pthread_cond_t m_cond;
#endif
};

