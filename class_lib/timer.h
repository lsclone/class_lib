#pragma once

#include "list.h"
#include "thread_class.h"

#include <set>
#include <sys/timeb.h>
using namespace std;

#ifdef CLASS_EXPORT
#define TIMER_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define TIMER_API __declspec(dllimport)
#else
#define TIMER_API
#endif


class Timer;

class TIMER_API TimerInterface
{
	friend class Timer;
public:
	TimerInterface();
	virtual ~TimerInterface();
	virtual void onTimer() {}
	void startTimer(int interval); /* interval: ms */
	void stopTimer();

protected:
	void killTimer();
	Timer* m_timer;
};


class Timer
{
	friend class TimerInterface;

public:
	Timer(int interval = 0, TimerInterface* pObj = NULL) : m_status(TIMER_ACTIVE), m_count(interval), m_interval(interval), m_pObj(pObj) {}
	~Timer();
	void tick(int dur);

private:
	enum
	{
		TIMER_ACTIVE,
		TIMER_INVALID,
	};
	int m_status;

	int m_count;
	int m_interval;
	TimerInterface* m_pObj;
};


class TimerManager : public Thread
{
public:
	static TimerManager* getTimerManager();
	TimerManager();
	Timer* startTimer(int interval, TimerInterface* pObj);
	void activeTimer(Timer* pTimer);
	void stopTimer(Timer* pTimer);
	void killTimer(Timer* pTimer);

protected:
	void run();
	void process();
	struct timeval getCurTime();
	void calculateDuration();

private:
	static TimerManager* pTimerManager;

	Lock m_activeLock;
	Lock m_stopLock;
	Lock m_delLock;
	set<Timer*> m_timerList;
	list<Timer*> m_activeTimerList;
	list<Timer*> m_stopTimerList;
	list<Timer*> m_delTimerList;

	struct timeval m_timeBegin;
	struct timeval m_timeEnd;
	int m_timeDur;
};


TIMER_API void startTimerManager();
