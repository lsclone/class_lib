#include "Test.h"
#include <stdio.h>
#include <thread_class.h>

#define DATA "hello world~"
Test* pTest = NULL;

#ifndef WIN32
void Sleep(int msec)
{
	usleep(1000 * msec);
}
#endif

class TestThread : public Thread
{
public:
	TestThread() {}

protected:
	virtual void run();
};

void TestThread::run()
{
	pTest = new Test(DATA, strlen(DATA));
	assert(pTest != NULL);

	for (;;)
	{
		Sleep(100);
	}

	return;
}

int main()
{
	TestThread subthread;
	bool rec = subthread.start();
	if (!rec)
		printf("[error] create sub thread.\n");

	int nums = 5;
	while (nums--)
		Sleep(1000);

#if 0
	if (pTest != NULL)
	{
		delete pTest;
		pTest = NULL;
	}
#else
	if (pTest != NULL)
	{
		pTest->releaseSelf();
		pTest = NULL;
	}
#endif

	for (;;)
		Sleep(1000);

	return 0;
}
