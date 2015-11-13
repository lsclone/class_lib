/*
come-up work: 
2. sub thread error, and main thread get info and how to quit and delete?
*/

#include "test_thread_2thread_commu_close.h"

#define WAITING_QUIT_LOOP_TIMES 50

#ifndef WIN32
void Sleep(int msec)
{
	usleep(msec * 1000);
}
#endif


SubThread::SubThread()
{
	m_status = STATUS_INIT;
}

int SubThread::Quit()
{
	m_lock.lock();
	if (m_status != STATUS_QUIT)
		m_status = STATUS_QUITREADY;
	m_lock.unlock();

	for (int i = 0; i < WAITING_QUIT_LOOP_TIMES; i++)
	{
		Sleep(50);
		m_lock.lock();
		if (m_status == STATUS_QUIT)
		{
			m_lock.unlock();
			delete this;
			return 0;
		}
		if (i == WAITING_QUIT_LOOP_TIMES - 1)
			m_status = STATUS_QUITFAILED;
		m_cond.WakeUp();
		m_lock.unlock();
	}

	return -1;
}

int SubThread::AddMessage(const string& message)
{
	AutoLock autolock(m_lock);
	if (m_status == STATUS_QUIT)
		return -1;
	m_list.push_back(message);
	m_cond.WakeUp();
	return 0;
}

void SubThread::run()
{
	bool ifMessage = false;
	string message;
	while (1)
	{
		m_lock.lock();

		switch (m_status)
		{
		case STATUS_INIT:
			m_status = STATUS_RUNNING;
			break;
		case STATUS_RUNNING:
			break;
		case STATUS_QUITFAILED:
			m_lock.unlock();
			delete this;
			return;
		case STATUS_QUITREADY:
		default:
			m_status = STATUS_QUIT;
			m_lock.unlock();
			return;
		}

		if (m_list.empty())
		{
			m_cond.Wait(m_lock);
			if (m_status != STATUS_RUNNING)
			{
				m_status = STATUS_QUIT;
				m_lock.unlock();
				return;
			}
			if (!m_list.empty())
			{
				message = m_list.front();
				m_list.pop_front();
				ifMessage = true;
			}
		}
		else
		{
			message = m_list.front();
			m_list.pop_front();
			ifMessage = true;
		}

		m_lock.unlock();

		if (ifMessage)
		{
			ifMessage = false;
			printf("message: %s\n", message.c_str());
			/* process message */
			/*
			if error
			{
				m_lock.lock();
				m_status = STATUS_QUIT;
				m_lock.unlock();
				return;
			}
			*/

		}
	}
}
