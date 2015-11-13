#include "test_timer.h"


void Test::onTimer()
{
	struct timeb timebuffer;
	ftime(&timebuffer);

	printf("%.3f   index: %d\n", timebuffer.time + timebuffer.millitm/1000.0, m_index);
}