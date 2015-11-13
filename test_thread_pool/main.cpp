#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "list.h"
#include "thread_pool.h"


class MyProcessEvent : public ProcessEvent
{
public:
	enum
	{
		Status_Wait,
		Status_Exec,
	};

	MyProcessEvent(int index) : m_index(index), m_status(Status_Wait) {}
	inline int getStatus() const { return m_status; }
	inline void setStatus(int status) { m_status = status; }
	virtual void process()
	{
		for(int i = 0; i < 10; i++)
		{
			printf("[%03d] %02d\n", m_index, i);
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
		}
	}

private:
	int m_index;
	int m_status;
};


#if 0
void loop(int min_num, int max_num, int process_num)
{
	ThreadPool th_pool;
	th_pool.init(min_num, max_num);

#ifdef WIN32
	Sleep(1000);
#else
	sleep(1);
#endif

	list<MyProcessEvent*> m_list;
	for(int i = 0; i < process_num; i++)
	{
		MyProcessEvent* pMyProcessEvent = new MyProcessEvent(i);
		m_list.push_back(pMyProcessEvent);
	}

	list<MyProcessEvent*>::iterator it;
	while(m_list.size())
	{
		for(it = m_list.begin(); it != m_list.end();)
		{
			if(MyProcessEvent::Status_Wait == (*it)->getStatus())
			{
				if(-1 == th_pool.process_job(*it))
#ifdef WIN32
					Sleep(10);
#else
					usleep(10*1000);
#endif
				else // return 0
				{
					it = m_list.erase(it);
					continue;
				}
			}
			++it;
		}
#ifdef WIN32
		Sleep(10);
#else
		usleep(10*1000);
#endif
	}

#ifdef WIN32
	Sleep(20*1000);
#else
	sleep(20);
#endif
}


int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("pls input like this: a.out min_num max_num process_num\n");
		return 0;
	}

	char *stopstring = NULL;
	int min_num = strtoul(argv[1], &stopstring, 10);
	int max_num = strtoul(argv[2], &stopstring, 10);
	int process_num = strtoul(argv[3], &stopstring, 10);

	for(;;)
	{
		loop(min_num, max_num, process_num);

#ifdef WIN32
		Sleep(10*1000);
#else
		sleep(10);
#endif

		printf("\n\n-----------------\n\n\n");
	}

	return 0;
}
#else
void loop(int process_num)
{
	list<MyProcessEvent*> m_list;
	for(int i = 0; i < process_num; i++)
	{
		MyProcessEvent* pMyProcessEvent = new MyProcessEvent(i);
		m_list.push_back(pMyProcessEvent);
	}

	list<MyProcessEvent*>::iterator it;
	while(m_list.size())
	{
		for(it = m_list.begin(); it != m_list.end();)
		{
			if(MyProcessEvent::Status_Wait == (*it)->getStatus())
			{
				if(-1 == ThreadPool::getThreadPool()->process_job(*it))
#ifdef WIN32
					Sleep(10);
#else
					usleep(10*1000);
#endif
				else // return 0
				{
					it = m_list.erase(it);
					continue;
				}
			}
			++it;
		}
#ifdef WIN32
		Sleep(10);
#else
		usleep(10*1000);
#endif
	}
}


int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("pls input like this: a.out min_num max_num process_num\n");
		return 0;
	}

	char *stopstring = NULL;
	int min_num = strtoul(argv[1], &stopstring, 10);
	int max_num = strtoul(argv[2], &stopstring, 10);
	int process_num = strtoul(argv[3], &stopstring, 10);

	ThreadPool::getThreadPool()->init(min_num, max_num);

#ifdef WIN32
	Sleep(1000);
#else
	sleep(1);
#endif

	for(;;)
	{
		loop(process_num);

		srand((unsigned)time(NULL)); //srand: 随机数发生器的初始化函数
		int sleep_rand = (int)((rand()%100/99.0)*20); // 生成一个0~20之间的随机数

		printf("\n\n--------sleep %02ds---------\n\n\n", sleep_rand+1);

#ifdef WIN32
		Sleep((sleep_rand+1)*1000);
#else
		sleep(sleep_rand+1);
#endif
	}

	return 0;
}
#endif
