#include "test_thread_waitcond.h"

int main()
{
	SubThread sub1, sub2, sub3;
	sub1.start();
	sub2.start();
	sub3.start();

	Sleep(1000);

	for (;;)
	{
		DataRepertory::getDataRepertory()->push(string("hello1"));
		Sleep(5000);
	}

	return 0;
}