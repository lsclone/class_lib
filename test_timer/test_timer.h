#pragma once

#include <stdio.h>
#include "timer.h"


class Test : public TimerInterface
{
public:
	Test(int index = -1) : m_index(index) {}
	void onTimer();

private:
	int  m_index;
};
