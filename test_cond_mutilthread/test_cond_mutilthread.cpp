#include <stdio.h>
#include <thread_class.h>

class WorkThread : public Thread
{
public:
	WorkThread(int msec, pthread_cond_t& cond, pthread_mutex_t& mutex) 
		: m_msec(msec), m_cond(cond), m_mutex(mutex) {}

protected:
	virtual void run();

private:
	int m_msec;
	pthread_cond_t& m_cond;
	pthread_mutex_t& m_mutex;
};

void WorkThread::run()
{
	for (;;)
	{
		usleep(1000*m_msec);

		pthread_mutex_lock(&m_mutex);
		pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);

		printf("%d\n", (int)getThreadID());
	}
}

int main()
{
	pthread_cond_t thread_cond;
	pthread_mutex_t thread_lock;

	pthread_cond_init(&thread_cond, NULL);
	pthread_mutex_init(&thread_lock, NULL);

	WorkThread t1(400, thread_cond, thread_lock);
	WorkThread t2(800, thread_cond, thread_lock);
	WorkThread t3(2000, thread_cond, thread_lock);

	t1.start();
	t2.start();
	t3.start();

	for (;;)
	{
		usleep(600*1000);

		pthread_mutex_lock(&thread_lock);
		//pthread_cond_signal(&thread_cond);
		 pthread_cond_broadcast(&thread_cond);
		pthread_mutex_unlock(&thread_lock);
	}

	pthread_cond_destroy(&thread_cond);
	pthread_mutex_destroy(&thread_lock);

	return 0;
}