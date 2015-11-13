/*
**	@file
**	@circle buffer implementation (first in first out)
**	@author Li Shuai
**	@date 2014-09-18
**
**  The author disclaims copyright to this source code.  In place of
**  a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/

#pragma once

#include "thread_class.h"

#define CIRCLE_BUFF_SIZE 20


template <typename ITEMTYPE>
class CircleBuff
{
public:
	CircleBuff() {
		m_cirBuffRD = 0;
		m_cirBuffWR = 0;
		m_cirUsedSize = 0;
	}

	/* return: 0 success, -1 error */
	int push(const ITEMTYPE& item) {
		if (isFull())
			return -1;
		m_cirBuff[m_cirBuffWR++] = item;
		if (m_cirBuffWR >= CIRCLE_BUFF_SIZE)
			m_cirBuffWR = 0;
		m_cirUsedSize++;
		return 0;
	}

	/* return: 0 success, -1 error */
	int pop(ITEMTYPE& item) {
		if (isEmpty())
			return -1;
		item = m_cirBuff[m_cirBuffRD++];
		if (m_cirBuffRD >= CIRCLE_BUFF_SIZE)
			m_cirBuffRD = 0;
		m_cirUsedSize--;
		return 0;
	}

	bool isFull() {
		if (m_cirUsedSize >= CIRCLE_BUFF_SIZE)
			return true;
		else
			return false;
	}

	bool isEmpty() {
		if (m_cirUsedSize <= 0)
			return true;
		else
			return false;
	}

	bool isNotFull() {
		return !isFull();
	}

	bool isNotEmpty() {
		return !isEmpty();
	}

	void waitNotFull(unsigned long millisecond = 0) {
		if (millisecond == 0)
			m_conNotFull.Wait(m_cirBuffLock);
		else
			m_conNotFull.Wait(m_cirBuffLock, millisecond);
	}

	void wakeNotFull() {
		m_conNotFull.WakeUp();
	}

	void waitNotEmpty(unsigned long millisecond = 0) {
		if (millisecond == 0)
			m_conNotEmpty.Wait(m_cirBuffLock);
		else
			m_conNotEmpty.Wait(m_cirBuffLock, millisecond);
	}

	void wakeNotEmpty() {
		m_conNotEmpty.WakeUp();
	}

	void lock() {
		m_cirBuffLock.lock();
	}

	void unlock() {
		m_cirBuffLock.unlock();
	}

private:
	int m_cirBuffRD;
	int m_cirBuffWR;
	int m_cirUsedSize;

	Lock m_cirBuffLock;
	WaitCondition m_conNotFull;
	WaitCondition m_conNotEmpty;
	ITEMTYPE m_cirBuff[CIRCLE_BUFF_SIZE];
};
