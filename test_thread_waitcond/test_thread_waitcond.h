#pragma once

#include <list>
#include <string>
#include "thread_class.h"

using namespace std;


class DataRepertory
{
public:
	static DataRepertory* getDataRepertory();

	DataRepertory() {}

	void push(const string& data);

	void poP(string& data);

	/* return: 0 success, -1 failed. */
	int pop(string& data);

private:
	Lock m_lock;
	list<string> m_list;
	WaitCondition m_cond;
	static DataRepertory* pDataRepertory;
};


class SubThread : public Thread
{
public:
	SubThread() {}

protected:
	void run();
};