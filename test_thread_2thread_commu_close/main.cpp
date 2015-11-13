#include "test_thread_2thread_commu_close.h"
#include <time.h>
#include <string.h>
#include <iostream>

using namespace std;

#ifndef WIN32
void Sleep(int msec)
{
	usleep(msec*1000);
}
#endif

int main()
{
	while (1)
	{
		SubThread* pSubThread = new SubThread;
		pSubThread->start();

		char strTime[1024];
		string tmp;
		for (int i = 0; i < 10; i++)
		{
			const time_t t = time(NULL);
			struct tm* current_time = localtime(&t);
			sprintf(strTime, "%d-%d-%d %d:%d:%d\n",
					current_time->tm_year + 1900,
					current_time->tm_mon + 1,
					current_time->tm_mday,
					current_time->tm_hour,
					current_time->tm_min,
					current_time->tm_sec);
			tmp = strTime;
			if (pSubThread->AddMessage(tmp) == -1)
				break;
			Sleep(1000);
		}
		
		pSubThread->Quit();
		pSubThread = NULL;

		printf("\n***** END ******\n\n");
	}

	return 0;
}
