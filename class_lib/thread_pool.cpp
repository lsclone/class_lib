/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*********************************************************************
** Windows, Unix and ANSI C API Comparison: 
** http://blogs.msdn.com/b/csliu/archive/2009/03/20/windows-unix-and-ansi-c-api-comparison.aspx
*********************************************************************
** Second Version has optimized two places on windows platform.
** 1. CreateThread() -> _beginthreadex()
** 2. PulseEvent() -> SetEvent()
** 3. optimize delete_thread(), do not terminate thread directly, 
**    but use quit flag to make thread break from circle by itself,
**    meanwhile, manage thread call function to wait for work thread to exit.
** See details below.
*********************************************************************
** Third Version need to optimize several places, and these work to be delayed.
** 1. use class "PooledThread" to replace struct "tp_thread_info"
** 2. use list<PooledThread*> to replace struct array.
** 3. when delete_thread(), prepare a release thread list, 
**    move the thread point which wanna delete from main thread list to release thread list.
**    remember use mutex when doing the above operation.
*********************************************************************
** what's more:
** 1. you can use map<int, struct> instead of struct array for func get_thread_by_id().
** 2. delete_thread() need to be optimized. It's not good to close thread from the last one
**    when the manage thread finds that status of thread pool is idle.
**    this insufficient place may be changed when third version.
** 3. It's not safe when using pthread_cond_signal and pthread_cond_wait.
**    It's unuseful to emit signal when pthread_cond_wait is not blocked at the same time.
**    So may to set a flag to decide whether to emit pthread_cond_signal or not.
**    if decide not to emit signal, should circle to another thread and decide again.
** 4. cause the above problem about pthread_cond_signal and pthread_cond_wait,
**    delay to optimize delete_thread() about Second Version 3rd item where using POSIX thread, eg. LINUX.
*/

#include "thread_pool.h"


//internal interface. real work thread.
#ifdef WIN32

	/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
DWORD WINAPI tp_work_thread(LPVOID parameter)
#else
unsigned __stdcall tp_work_thread(void* parameter)
#endif
	/******* update on 2014/06/19 *******/

{
	DWORD curid;//current thread id
	int nseq;//current thread seq in the this->thread_info array
	ThreadPool* pThreadPool = (ThreadPool*)parameter;//main thread pool struct instance

	//get current thread id
	curid = GetCurrentThreadId();
	
	//get current thread's seq in the thread info struct array.
	nseq = pThreadPool->get_thread_by_id(curid);
	if(nseq < 0)
		return 0;

	//waiting  cond  for processing real job.
	while(1)
	{
		EnterCriticalSection(&pThreadPool->thread_info[nseq].thread_lock);
		pThreadPool->thread_info[nseq].is_busy = false;
		LeaveCriticalSection(&pThreadPool->thread_info[nseq].thread_lock);

		WaitForSingleObject(pThreadPool->thread_info[nseq].thread_cond, INFINITE);

		//process_job : user interface
		if(pThreadPool->thread_info[nseq].pProcessEvent)
		{
			pThreadPool->thread_info[nseq].pProcessEvent->process();
			delete pThreadPool->thread_info[nseq].pProcessEvent;
			pThreadPool->thread_info[nseq].pProcessEvent = NULL;
		}

		/******* update on 2014/06/23 *******/
#ifdef SECOND_VERTION
		EnterCriticalSection(&pThreadPool->thread_info[nseq].thread_lock);
		if (pThreadPool->thread_info[nseq].quit_flag == true)
		{
			LeaveCriticalSection(&pThreadPool->thread_info[nseq].thread_lock);
			break;
		}
		LeaveCriticalSection(&pThreadPool->thread_info[nseq].thread_lock);
#endif
		/******* update on 2014/06/23 *******/
	}

	return 0;
}
#else
static void* tp_work_thread(void* pthread)
{
	pthread_t curid;//current thread id
	int nseq;//current thread seq in the this->thread_info array
	ThreadPool* pThreadPool = (ThreadPool*)pthread;//main thread pool struct instance

	//get current thread id
	curid = pthread_self();
	
	//get current thread's seq in the thread info struct array.
	nseq = pThreadPool->get_thread_by_id(curid);
	if(nseq < 0)
		return NULL;

	//waiting  cond  for processing real job.
	while(1)
	{
		pthread_mutex_lock(&pThreadPool->thread_info[nseq].thread_lock);
		pThreadPool->thread_info[nseq].is_busy = false;
		pthread_cond_wait(&pThreadPool->thread_info[nseq].thread_cond, &pThreadPool->thread_info[nseq].thread_lock);
		pthread_mutex_unlock(&pThreadPool->thread_info[nseq].thread_lock);		

		//process_job : user interface
		if(pThreadPool->thread_info[nseq].pProcessEvent)
		{
			pThreadPool->thread_info[nseq].pProcessEvent->process();
			delete pThreadPool->thread_info[nseq].pProcessEvent;
			pThreadPool->thread_info[nseq].pProcessEvent = NULL;
		}
	}

	return NULL;
}
#endif

//internal interface. manage thread pool to delete idle thread.
#ifdef WIN32
DWORD WINAPI tp_manage_thread(LPVOID pthread)
#else
static void* tp_manage_thread(void* pthread)
#endif
{
	//main thread pool struct instance
	ThreadPool* pThreadPool = (ThreadPool*)pthread;

#ifdef WIN32
	Sleep(MANAGE_INTERVAL*1000); // 5s
#else
	sleep(MANAGE_INTERVAL); //5s
#endif

	do{
		if(pThreadPool->get_tp_status() == 0) // thread pool idle
		{
			do{
				if(!pThreadPool->delete_thread())
					break;
			}while(1);
		}

#ifdef WIN32
		Sleep(MANAGE_INTERVAL*1000); // 5s
#else
		sleep(MANAGE_INTERVAL); //5s
#endif
	}while(1);
}


ThreadPool* ThreadPool::pThreadPool = NULL;
ThreadPool* ThreadPool::getThreadPool()
{
	if(!pThreadPool)
		pThreadPool = new ThreadPool;
	return pThreadPool;
}

ThreadPool::ThreadPool()
{
#ifdef WIN32
	InitializeCriticalSection(&tp_lock);
#else
	pthread_mutex_init(&tp_lock, NULL);
#endif
}

ThreadPool::~ThreadPool()
{
#ifdef WIN32
	//close manage thread
	TerminateThread(manage_thread_id, 0);
	CloseHandle(manage_thread_id);

	//close work thread
	for(int i = 0; i < cur_th_num; i++)
	{
		TerminateThread(thread_info[i].thread_hdl, 0);
		CloseHandle(thread_info[i].thread_hdl);

		DeleteCriticalSection(&thread_info[i].thread_lock);
		CloseHandle(thread_info[i].thread_cond);
	}

	DeleteCriticalSection(&tp_lock);
#else
	//close manage thread
	pthread_cancel(manage_thread_id);

	//close work thread
	for(int i = 0; i < cur_th_num; i++)
	{
		pthread_cancel(thread_info[i].thread_id);
		pthread_mutex_destroy(&thread_info[i].thread_lock);
		pthread_cond_destroy(&thread_info[i].thread_cond);
	}

	pthread_mutex_destroy(&tp_lock);
#endif
	
	//free thread struct
	free(thread_info);
}

bool ThreadPool::init(int min_num, int max_num)
{
	//init member var
	min_th_num = min_num;
	cur_th_num = min_num;
	max_th_num = max_num;

	//malloc mem for num thread info struct
	thread_info = (tp_thread_info*)malloc(sizeof(tp_thread_info) * max_th_num);

	for (int i = 0; i < max_th_num; i++)
	{
		thread_info[i].quit_flag = false;
		thread_info[i].pProcessEvent = NULL;
	}

#ifdef WIN32
	//creat work thread and init work thread info
	for(int i = 0; i < min_th_num; i++)
	{
		// CreateEvent arg: bManualReset = false, means auto reset.
		thread_info[i].thread_cond = CreateEvent(NULL, FALSE, FALSE, NULL);
		InitializeCriticalSection(&thread_info[i].thread_lock);

		/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
		thread_info[i].thread_hdl = CreateThread(0, 0, tp_work_thread, (LPVOID)this, 0, &thread_info[i].thread_id);
#else	/* it's safe to use _beginthreadex() when doing IO operation in run() fun. */
		unsigned tmpThreadId;
		thread_info[i].thread_hdl = (HANDLE)_beginthreadex(0, 0, tp_work_thread, (void*)this, 0, &tmpThreadId);
		thread_info[i].thread_id = static_cast<DWORD>(tmpThreadId);
#endif
		/******* update on 2014/06/19 *******/

		if(thread_info[i].thread_hdl == NULL)
		{
			printf("tp_init: creat work thread failed\n");
			return false;
		}
	}

	//creat manage thread
	DWORD nThreadID;
	manage_thread_id = CreateThread(0, 0, tp_manage_thread, (LPVOID)this, 0, &nThreadID);
	if(manage_thread_id == NULL)
	{
		printf("tp_init: creat manage thread failed\n");
		return false;
	}
#else
	int err;

	//creat work thread and init work thread info
	for(int i = 0; i < min_th_num; i++)
	{
		pthread_cond_init(&thread_info[i].thread_cond, NULL);
		pthread_mutex_init(&thread_info[i].thread_lock, NULL);

		err = pthread_create(&thread_info[i].thread_id, NULL, tp_work_thread, this);
		if(0 != err)
		{
			printf("tp_init: creat work thread failed\n");
			return false;
		}
	}

	//creat manage thread
	err = pthread_create(&manage_thread_id, NULL, tp_manage_thread, this);
	if(0 != err)
	{
		printf("tp_init: creat manage thread failed\n");
		return false;
	}
#endif

	return true;
}

int ThreadPool::process_job(ProcessEvent* pProcessEvent)
{
#ifdef WIN32
	int i;

	//fill this->thread_info's relative work key
	for(i = 0; i < cur_th_num; i++)
	{
		EnterCriticalSection(&thread_info[i].thread_lock);
		if(!thread_info[i].is_busy)
		{
			//thread state be set busy before work
		  	thread_info[i].is_busy = true;
			LeaveCriticalSection(&thread_info[i].thread_lock);

			if(thread_info[i].pProcessEvent)
				delete thread_info[i].pProcessEvent;
			thread_info[i].pProcessEvent = pProcessEvent;

			printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, thread_info[i].thread_id);

			/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
			PulseEvent(thread_info[i].thread_cond);
#else		/* PulseEvent is not safe. SetEvent() and PulseEvent() are the same, when auto reset. */
			SetEvent(thread_info[i].thread_cond);
#endif
			/******* update on 2014/06/19 *******/

			return 0;
		}
		else 
			LeaveCriticalSection(&thread_info[i].thread_lock);
	}//end of for

	//if all current thread are busy, new thread is created here
	EnterCriticalSection(&tp_lock);
	if(add_thread())
	{
		i = cur_th_num - 1;
		LeaveCriticalSection(&tp_lock);

		if(thread_info[i].pProcessEvent)
			delete thread_info[i].pProcessEvent;
		thread_info[i].pProcessEvent = pProcessEvent;

		EnterCriticalSection(&thread_info[i].thread_lock);
		thread_info[i].is_busy = true;
		LeaveCriticalSection(&thread_info[i].thread_lock);

		//send cond to work thread
		printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, thread_info[i].thread_id);

		/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
		PulseEvent(thread_info[i].thread_cond);
#else		/* PulseEvent is not safe. SetEvent() and PulseEvent() are the same, when auto reset. */
		SetEvent(thread_info[i].thread_cond);
#endif
		/******* update on 2014/06/19 *******/

		return 0;
	}
	LeaveCriticalSection(&tp_lock);

	return -1;
#else
	int i;

	//fill this->thread_info's relative work key
	for(i = 0; i < cur_th_num; i++)
	{
		pthread_mutex_lock(&thread_info[i].thread_lock);
		if(!thread_info[i].is_busy)
		{
			//thread state be set busy before work
		  	thread_info[i].is_busy = true;
			pthread_mutex_unlock(&thread_info[i].thread_lock);

			if(thread_info[i].pProcessEvent)
				delete thread_info[i].pProcessEvent;
			thread_info[i].pProcessEvent = pProcessEvent;

			//send cond to work thread
			pthread_mutex_lock(&thread_info[i].thread_lock);
			pthread_cond_signal(&thread_info[i].thread_cond);
			pthread_mutex_unlock(&thread_info[i].thread_lock);
			//printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, thread_info[i].thread_id);

			return 0;
		}
		else 
			pthread_mutex_unlock(&thread_info[i].thread_lock);
	}//end of for

	//if all current thread are busy, new thread is created here
	pthread_mutex_lock(&tp_lock);
	if(add_thread())
	{
		i = cur_th_num - 1;
		pthread_mutex_unlock(&tp_lock);

		if(thread_info[i].pProcessEvent)
			delete thread_info[i].pProcessEvent;
		thread_info[i].pProcessEvent = pProcessEvent;

		pthread_mutex_lock(&thread_info[i].thread_lock);
		thread_info[i].is_busy = true;
		pthread_cond_signal(&thread_info[i].thread_cond);
		pthread_mutex_unlock(&thread_info[i].thread_lock);
		//printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, thread_info[i].thread_id);

		return 0;
	}
	pthread_mutex_unlock(&tp_lock);

	return -1;
#endif
}

#ifdef WIN32
int ThreadPool::get_thread_by_id(DWORD id)
{
	for(int i = 0; i < cur_th_num; i++)
		if(id == thread_info[i].thread_id)
			return i;
	return -1;
}
#else
int ThreadPool::get_thread_by_id(unsigned long id)
{
	for(int i = 0; i < cur_th_num; i++)
		if(id == thread_info[i].thread_id)
			return i;
	return -1;
}
#endif

bool ThreadPool::add_thread()
{
	tp_thread_info* new_thread = NULL;
	
	if(max_th_num <= cur_th_num)
		return false;
		
	//malloc new thread info struct
	new_thread = &thread_info[cur_th_num];
	
#ifdef WIN32
	//init new thread's cond & mutex. CreateEvent arg: bManualReset = false, means auto reset.
	new_thread->thread_cond = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeCriticalSection(&new_thread->thread_lock);

	//add current thread number in the pool.
	cur_th_num++;
	
	/******* update on 2014/06/19 *******/
#ifndef SECOND_VERTION
	new_thread->thread_hdl = CreateThread(0, 0, tp_work_thread, (LPVOID)this, 0, &new_thread->thread_id);
#else	/* it's safe to use _beginthreadex() when doing IO operation in run() fun. */
	unsigned tmpThreadId;
	new_thread->thread_hdl = (HANDLE)_beginthreadex(0, 0, tp_work_thread, (void*)this, 0, &tmpThreadId);
	new_thread->thread_id = static_cast<DWORD>(tmpThreadId);
#endif
	/******* update on 2014/06/19 *******/

	if(new_thread->thread_hdl == NULL)
	{
		printf("tp_add_thread: creat work thread failed\n");
		return false;
	}
	printf("tp_add_thread: creat work thread %d\n", thread_info[cur_th_num-1].thread_id);
	Sleep(100);
#else
	int err;

	//init new thread's cond & mutex
	pthread_cond_init(&new_thread->thread_cond, NULL);
	pthread_mutex_init(&new_thread->thread_lock, NULL);

	//add current thread number in the pool.
	cur_th_num++;
	
	err = pthread_create(&new_thread->thread_id, NULL, tp_work_thread, this);
	if(0 != err)
		return false;
	//printf("tp_add_thread: creat work thread %d\n", thread_info[cur_th_num-1].thread_id);
	usleep(100*1000);
#endif
	
	return true;
}

bool ThreadPool::delete_thread()
{
	/* current thread num can't < min thread num */
	if(cur_th_num <= min_th_num)
		return false;

	int index = cur_th_num - 1;

	/* if last thread is busy, do nothing */
	if (thread_info[index].is_busy)
		return false;

#ifdef WIN32
	/* kill the idle thread and free info struct */

	/******* update on 2014/06/23 *******/
#ifndef SECOND_VERTION
	TerminateThread(thread_info[index].thread_hdl, 0);
	CloseHandle(thread_info[index].thread_hdl);
#else
	EnterCriticalSection(&thread_info[index].thread_lock);
	thread_info[index].quit_flag = true;
	LeaveCriticalSection(&thread_info[index].thread_lock);

	SetEvent(thread_info[index].thread_cond);

	switch(WaitForSingleObject(thread_info[index].thread_hdl, THREAD_EXIT_TIMEOUT))
	{
	case WAIT_OBJECT_0:
		CloseHandle(thread_info[index].thread_hdl);
		thread_info[index].quit_flag = false;
		break;
	case WAIT_TIMEOUT:
		printf("[Warning] timeout, cannot stop thread\n");
		EnterCriticalSection(&thread_info[index].thread_lock);
		thread_info[index].quit_flag = false;
		LeaveCriticalSection(&thread_info[index].thread_lock);
		return false;
	default:
		printf("[Error] cannot stop thread\n");
		EnterCriticalSection(&thread_info[index].thread_lock);
		thread_info[index].quit_flag = false;
		LeaveCriticalSection(&thread_info[index].thread_lock);
		return false;
	}
#endif
	/******* update on 2014/06/23 *******/

	DeleteCriticalSection(&thread_info[index].thread_lock);
	CloseHandle(thread_info[index].thread_cond);
#else
	/* kill the idle thread and free info struct */
	pthread_cancel(thread_info[index].thread_id);

	pthread_mutex_destroy(&thread_info[index].thread_lock);
	pthread_cond_destroy(&thread_info[index].thread_cond);
#endif

	/******* update on 2014/06/23 *******/
#ifndef SECOND_VERTION
	if (thread_info[index].pProcessEvent)
	{
		delete thread_info[index].pProcessEvent;
		thread_info[index].pProcessEvent = NULL;
	}
#endif
	/******* update on 2014/06/23 *******/

	/* after delete idle thread, current thread num decrease */
	cur_th_num--;

	return true;
}

int ThreadPool::get_tp_status()
{
	float busy_num = 0.0;

	//get busy thread number
	for(int i = 0; i < cur_th_num; i++)
		if(thread_info[i].is_busy)
			busy_num++;

	// BUSY_THRESHOLD  0.5
	if(busy_num/cur_th_num < BUSY_THRESHOLD)
		return 0;	//idle status
	else
		return 1;	//busy status
}

