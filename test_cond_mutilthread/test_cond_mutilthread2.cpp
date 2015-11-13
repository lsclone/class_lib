#include <list>
#include <string>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <thread_class.h>

using namespace std;

class WorkThread : public Thread
{
public:
	WorkThread(pthread_cond_t& cond, pthread_mutex_t& mutex, list<string>& msglist)
		: m_cond(cond), m_mutex(mutex), m_msglist(msglist) {}

protected:
	virtual void run();

private:
	pthread_cond_t& m_cond;
	pthread_mutex_t& m_mutex;
	list<string>& m_msglist;
};

void WorkThread::run()
{
	string msg;
	for (;;)
	{
		pthread_mutex_lock(&m_mutex);
		if (m_msglist.empty())
		{
			pthread_cond_wait(&m_cond, &m_mutex);
			if (m_msglist.empty())
			{
				pthread_mutex_unlock(&m_mutex);
				continue;
			}
		}
		msg = m_msglist.front();
		m_msglist.pop_front();
		pthread_mutex_unlock(&m_mutex);

		printf("[thread %ld] %s\n", (long)getThreadID(), msg.c_str());
	}
}

int main()
{
	char strMsg[1024];
	list<string> msglist;
	pthread_cond_t thread_cond;
	pthread_mutex_t thread_lock;

	pthread_cond_init(&thread_cond, NULL);
	pthread_mutex_init(&thread_lock, NULL);

	WorkThread t1(thread_cond, thread_lock, msglist);
	WorkThread t2(thread_cond, thread_lock, msglist);
	WorkThread t3(thread_cond, thread_lock, msglist);

	t1.start();
	t2.start();
	t3.start();

	for (;;)
	{
		usleep(500*1000);

		memset(strMsg, 0x00, sizeof(strMsg));
		sprintf(strMsg, "cur time: %d", (int)time(NULL));

		pthread_mutex_lock(&thread_lock);
		//pthread_cond_signal(&thread_cond);
		msglist.push_back(strMsg);
		pthread_cond_broadcast(&thread_cond);
		pthread_mutex_unlock(&thread_lock);
	}

	pthread_cond_destroy(&thread_cond);
	pthread_mutex_destroy(&thread_lock);

	return 0;
}