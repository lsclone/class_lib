/*
**	@file
**	@test one thread wake and multi-threads waiting
**	@author Li Shuai
**	@date 2014-11-21
**
**  The author disclaims copyright to this source code.  In place of
**  a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/

#include "test_thread_waitcond.h"


DataRepertory* DataRepertory::pDataRepertory = NULL;
DataRepertory* DataRepertory::getDataRepertory()
{
	if (!pDataRepertory)
		pDataRepertory = new DataRepertory;
	return pDataRepertory;
}

void DataRepertory::poP(string& data)
{
	m_lock.lock();

	while (m_list.empty())
	{
		printf("thread id: %lu, pre waiting...\n", (unsigned long)::GetCurrentThreadId());
		m_cond.Wait(m_lock);
		printf("thread id: %lu, aft waiting...\n", (unsigned long)::GetCurrentThreadId());
	}

	data = m_list.front();
	m_list.pop_front();

	m_lock.unlock();
}

int DataRepertory::pop(string& data)
{
	int ret = 0;
	m_lock.lock();
	if (m_list.empty())
	{
		m_cond.Wait(m_lock);
		if (!m_list.empty())
		{
			data = m_list.front();
			m_list.pop_front();
			m_lock.unlock();
		}
		else
		{
			m_lock.unlock();
			ret = -1;
		}
	}
	else
	{
		data = m_list.front();
		m_list.pop_front();
		m_lock.unlock();
	}
	return ret;
}

void DataRepertory::push(const string& data)
{
	AutoLock autLock(m_lock);
	printf("list size: %d\n", m_list.size());
	m_list.push_back(data);
	m_list.push_back(data);
	m_list.push_back(data);
	m_list.push_back(data);
	m_cond.WakeUp();
}


void SubThread::run()
{
	for (;;)
	{
		string data;
#if 0
		if (0 == DataRepertory::getDataRepertory()->pop(data))
		{
			printf("thread: %lu, data: %s\n", (unsigned long)(this->getThreadID()), data.c_str());
			Sleep(1500);
		}
		else
			printf("thread: %lu, empty data.\n", (unsigned long)(this->getThreadID()));
#else
		DataRepertory::getDataRepertory()->poP(data);
		printf("thread: %lu, data: %s\n", (unsigned long)(this->getThreadID()), data.c_str());
		Sleep(1800);
#endif
	}
}