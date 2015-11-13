#pragma once

#include <list>
#include <string>
#include "thread_class.h"

using namespace std;


class SubThread : public Thread
{
public:
	SubThread();

	/*  return: 0 success, -1 error */
	int Quit();

	/*  return: 0 success, -1 error */
	int AddMessage(const string& message);

protected:
	virtual void run();

private:
	enum Status
	{
		STATUS_INIT,
		STATUS_RUNNING,
		STATUS_QUITREADY,
		STATUS_QUIT,
		STATUS_QUITFAILED,
	};
	Status m_status;
	list<string> m_list;
	Lock m_lock;
	WaitCondition m_cond;
};
