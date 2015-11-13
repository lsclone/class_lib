#include "thread_class.h"

#ifndef WIN32
void Sleep(int msec)
{
	usleep(1000*msec);
}
#endif

Thread::Thread()
{
	m_threadID = -1;
}

DWORD WINAPI Thread::ThreadCallBack(LPVOID parameter)
{
	assert(parameter != NULL);

	Thread* pMyThread = (Thread*)parameter;

#ifdef WIN32
	pMyThread->m_threadID = (unsigned long)::GetCurrentThreadId();
#else
	pMyThread->m_threadID = pthread_self();
#endif

	pMyThread->run();

	return 0;
}

bool Thread::start()
{
#ifdef WIN32
	DWORD nThreadID;

	/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
	m_hThreadHandle = CreateThread(0, 0, ThreadCallBack,
		(LPVOID)this, 0, &nThreadID);
#else	/* it's safe to use _beginthreadex() when doing IO operation in run() fun. */
	m_hThreadHandle = (HANDLE)_beginthreadex(0, 0, ThreadCallBack,
		(void*)this, 0, &nThreadID);
#endif
	/******* update on 2014/06/19 *******/

	if(m_hThreadHandle == NULL)
	{
		printf("create thread error.\n");
		return false;
	}
	else
	{
		CloseHandle(m_hThreadHandle);
		return true;
	}
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int nThreadID;
	nThreadID = pthread_create(&m_hThreadHandle, &attr, ThreadCallBack, this);
	if(nThreadID != 0)
	{
		printf("create thread error.\n");
		return false;
	}
	else
	{
		return true;
	}
#endif
}


Lock::Lock()
{
#ifdef WIN32
	::InitializeCriticalSection(&section);
#else
    pthread_mutexattr_t recursiveAttr;
    pthread_mutexattr_init(&recursiveAttr);
    pthread_mutexattr_settype(&recursiveAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &recursiveAttr);
    pthread_mutexattr_destroy(&recursiveAttr);
#endif
}

Lock::~Lock()
{
#ifdef WIN32
	::DeleteCriticalSection(&section);
#else
	pthread_mutex_destroy(&mutex);
#endif
}

void Lock::lock()
{
#ifdef WIN32
	::EnterCriticalSection(&section);
#else
	pthread_mutex_lock(&mutex);
#endif
}

void Lock::unlock()
{
#ifdef WIN32
	::LeaveCriticalSection(&section);
#else
	pthread_mutex_unlock(&mutex);
#endif
}


AutoLock::AutoLock(Lock &lock)
	: m_lock (lock)
{
	m_lock.lock();
}

AutoLock::~AutoLock()
{
	m_lock.unlock();
}


WaitCondition::WaitCondition()
{
#ifdef WIN32
	// CreateEvent arg: bManualReset = FALSE, means auto reset.
	m_cond = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	pthread_cond_init(&m_cond, NULL);
#endif
}

WaitCondition::~WaitCondition()
{
#ifdef WIN32
	CloseHandle(m_cond);
#else
	pthread_cond_destroy(&m_cond);
#endif
}

void WaitCondition::Wait(Lock& lock)
{
#ifdef WIN32
	lock.unlock();
	WaitForSingleObject(m_cond, INFINITE);
	lock.lock();
#else
	pthread_cond_wait(&m_cond, &lock.mutex);
#endif
}

void WaitCondition::Wait(Lock& lock, unsigned long millisecond)
{
#ifdef WIN32
	lock.unlock();
	WaitForSingleObject(m_cond, millisecond);
	lock.lock();
#else
	struct timeval now;
	struct timespec outtime;

	gettimeofday(&now, NULL);

	outtime.tv_sec = now.tv_sec + millisecond / 1000;
	if (now.tv_usec + millisecond % 1000 * 1000 < 1000 * 1000)
	{
		outtime.tv_nsec = (now.tv_usec + millisecond % 1000 * 1000) * 1000;
	}
	else
	{
		outtime.tv_sec += ((now.tv_usec + millisecond % 1000 * 1000) / (1000 * 1000));
		outtime.tv_nsec = (now.tv_usec + millisecond % 1000 * 1000) % (1000 * 1000) * 1000;
	}

	pthread_cond_timedwait(&m_cond, &lock.mutex, &outtime);
#endif
}

void WaitCondition::WakeUp()
{
#ifdef WIN32
	SetEvent(m_cond);
#else
	pthread_cond_signal(&m_cond);
#endif
}
