#include "timer.h"


TimerInterface::TimerInterface()
{
	m_timer = NULL;
}

TimerInterface::~TimerInterface()
{
	killTimer();
	for(;;)
	{
		if(!m_timer)
			break;
#ifdef WIN32
		Sleep(10);
#else
		usleep(10*1000);
#endif
	}
}

void TimerInterface::startTimer(int interval)
{
	if(!m_timer)
		m_timer = TimerManager::getTimerManager()->startTimer(interval, this);
	else
		m_timer->m_count = m_timer->m_interval = interval;

	TimerManager::getTimerManager()->activeTimer(m_timer);
}

void TimerInterface::stopTimer()
{
	if(m_timer)
		TimerManager::getTimerManager()->stopTimer(m_timer);
}

void TimerInterface::killTimer()
{
	if(m_timer && m_timer->m_status == Timer::TIMER_ACTIVE)
	{
		m_timer->m_status = Timer::TIMER_INVALID;
		TimerManager::getTimerManager()->killTimer(m_timer);
	}
}


Timer::~Timer()
{
	if(m_pObj && m_pObj->m_timer == this)
		m_pObj->m_timer = NULL;
}

void Timer::tick(int dur)
{
    // 修改定时器的计数器
	m_count -= dur;
    
    if(m_count <= 0)
    {
        // 计数器到期
        m_count = m_interval;

        // 发送消息给目标对象
		if(m_pObj)
			m_pObj->onTimer();
    }
}


TimerManager* TimerManager::pTimerManager = NULL;

TimerManager* TimerManager::getTimerManager()
{
	if(!pTimerManager)
		pTimerManager = new TimerManager;
	return pTimerManager;
}

void startTimerManager()
{
	TimerManager::getTimerManager();
}

TimerManager::TimerManager()
{
	m_timeDur = 0;

	this->start();
}

Timer* TimerManager::startTimer(int interval, TimerInterface* pObj)
{
	Timer* pTimer = new Timer(interval, pObj);

	return pTimer;
}

void TimerManager::activeTimer(Timer* pTimer)
{
	m_activeLock.lock();
	m_activeTimerList.push_back(pTimer);
	m_activeLock.unlock();
}

void TimerManager::stopTimer(Timer* pTimer)
{
	m_stopLock.lock();
	m_stopTimerList.push_back(pTimer);
	m_stopLock.unlock();
}

void TimerManager::killTimer(Timer* pTimer)
{
	m_delLock.lock();
	m_delTimerList.push_back(pTimer);
	m_delLock.unlock();
}

void TimerManager::run()
{
	m_timeBegin = m_timeEnd = getCurTime();

	for(;;)
	{
		m_timeEnd = getCurTime();
		calculateDuration();
		m_timeBegin = m_timeEnd;

		process();

#ifdef WIN32
		Sleep(10);
#else
		usleep(10*1000);
#endif
	}
}

void TimerManager::process()
{
	list<Timer*>::iterator it;

	m_delLock.lock();
	for(it = m_delTimerList.begin(); it != m_delTimerList.end(); ++it)
	{
		// iterator set::erase(iterator)
		//      int set::erase(key)
		if(0 == m_timerList.erase(*it))
			printf("[Warning] set::erase fail.\n");
		delete (*it);
	}
	m_delTimerList.clear();
	m_delLock.unlock();

	m_stopLock.lock();
	for(it = m_stopTimerList.begin(); it != m_stopTimerList.end(); ++it)
		m_timerList.erase(*it);
	m_stopTimerList.clear();
	m_stopLock.unlock();

	m_activeLock.lock();
	for(it = m_activeTimerList.begin(); it != m_activeTimerList.end(); ++it)
		m_timerList.insert(*it);
	m_activeTimerList.clear();
	m_activeLock.unlock();

	set<Timer*>::const_iterator setIt;
	for(setIt = m_timerList.begin(); setIt != m_timerList.end(); ++setIt)
		(*setIt)->tick(m_timeDur);
}

struct timeval TimerManager::getCurTime()
{
    struct timeb timebuffer;
    ftime(&timebuffer);

	struct timeval timedetail;
    timedetail.tv_sec = timebuffer.time;
    timedetail.tv_usec = timebuffer.millitm * 1000;

	return timedetail;
}

void TimerManager::calculateDuration()
{
	if(m_timeBegin.tv_usec > m_timeEnd.tv_usec)
		m_timeDur = (m_timeEnd.tv_sec - m_timeBegin.tv_sec - 1)*1000 +(m_timeEnd.tv_usec + 1000000 - m_timeBegin.tv_usec)/1000;
	else
		m_timeDur = (m_timeEnd.tv_sec - m_timeBegin.tv_sec)*1000 +(m_timeEnd.tv_usec - m_timeBegin.tv_usec)/1000;
}
