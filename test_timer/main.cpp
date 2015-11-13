#include <stdio.h>
#include "test_timer.h"


#if 0
int main()
{
	startTimerManager();

	Test t1(1);
	t1.startTimer(20);

	/*
	Test t2(2);
	t2.startTimer(80);
	*/

	for(;;)
	{
#ifdef WIN32
		Sleep(51);
#else
		usleep(51*1000);
#endif

#if 1
		static bool ifrun = true;
		static int num = 1;
		if(num)
			num--;
		else
		{
			if(ifrun)
			{
				t1.stopTimer();
				t1.startTimer(23);
				t1.stopTimer();
				t1.startTimer(23);
				t1.stopTimer();
				t1.stopTimer();
				ifrun = false;
				printf("\n------------\n\n");
			}
			else
			{
				t1.startTimer(27);
				t1.stopTimer();
				t1.stopTimer();
				t1.startTimer(33);
				t1.startTimer(54);
				ifrun = true;
			}
			num = 1;
		}
#endif
	}

	return 0;
}
#else
int main()
{
	startTimerManager();

	Test t2(2);
	t2.startTimer(80);

	Test t22(200);
	t2.startTimer(60);

	for(;;)
	{
#ifdef WIN32
		Sleep(100);
#else
		usleep(100*1000);
#endif

		if(1)
		{
			Test t1(20);
			t1.startTimer(20);

			Test t11(123);
			t11.startTimer(123);

			Test t12(66);
			t12.startTimer(66);

			Test t13(97);
			t13.startTimer(97);

#ifdef WIN32
			Sleep(100);
#else
			usleep(100*1000);
#endif
			/*
			t1.stopTimer();
			t11.stopTimer();
			t12.stopTimer();
			t13.stopTimer();
			*/
		}
	}

	return 0;
}
#endif
