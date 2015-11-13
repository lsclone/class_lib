/*
 *
 * Windows, Unix and ANSI C API Comparison: 
 * http://blogs.msdn.com/b/csliu/archive/2009/03/20/windows-unix-and-ansi-c-api-comparison.aspx
 *
*/

#pragma once


#ifdef CLASS_EXPORT
#define THREAD_POOL_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define THREAD_POOL_API __declspec(dllimport)
#else
#define THREAD_POOL_API
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#ifdef WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif


#define BUSY_THRESHOLD		0.5		// (busy thread)/(all thread threshold)
#define MANAGE_INTERVAL		60		// tp manage thread sleep interval
#define THREAD_EXIT_TIMEOUT 10000	// waiting time for work thread to exit


class THREAD_POOL_API ProcessEvent
{
public:
	ProcessEvent() {}
	virtual ~ProcessEvent() {}
	virtual void process() {}
};


//thread info
typedef struct tp_thread_info_s
{
#ifdef WIN32
	DWORD thread_id;		//thread id num
	HANDLE thread_hdl;
	HANDLE thread_cond;
	CRITICAL_SECTION thread_lock;
#else
	pthread_t thread_id;	//thread id num
	pthread_cond_t thread_cond;
	pthread_mutex_t	thread_lock;
#endif
	bool is_busy;			//thread status:true-busy;flase-idle
	bool quit_flag;			//quit thread flag:true-quit;flase-keep alive
	ProcessEvent* pProcessEvent;
}tp_thread_info;


class THREAD_POOL_API ThreadPool
{
public:
	static ThreadPool* getThreadPool();

	ThreadPool();
	~ThreadPool();

	bool init(int min_num, int max_num);
	int process_job(ProcessEvent* pProcessEvent);
#ifdef WIN32
	int get_thread_by_id(DWORD id);
#else
	int get_thread_by_id(unsigned long id);
#endif
	bool add_thread();
	bool delete_thread();
	int get_tp_status();

	tp_thread_info* thread_info;	//work thread relative thread info

private:
	static ThreadPool* pThreadPool;

	int min_th_num;					//min thread number in the pool
	volatile int cur_th_num;		//current thread number in the pool, "volatile" for mutilthread to access.
	int max_th_num;					//max thread number in the pool
#ifdef WIN32
	CRITICAL_SECTION tp_lock;
	HANDLE manage_thread_id;		//manage thread id num
#else
	pthread_mutex_t tp_lock;
	pthread_t manage_thread_id;		//manage thread id num
#endif
};

