#include <stdio.h>
#include <stdlib.h>
#include "thread_class.h"
#include "socket_class.h"
#if 0
#include "string_class.h"
#else
#include <iostream>
#include <string>
using namespace std;
#endif

#ifndef WIN32
void Sleep(int msec)
{
	usleep(1000*msec);
}
#endif

class ThreadStatus
{
public:
	static ThreadStatus* GetThreadStatusObj();
	ThreadStatus() { m_status = 0; }
	void SetStatus(int status);
	int GetStatus();

private:
	static ThreadStatus* pThreadStatus;
	int m_status;
	Lock m_lock;
};

ThreadStatus* ThreadStatus::pThreadStatus = NULL;
ThreadStatus* ThreadStatus::GetThreadStatusObj()
{
	if(pThreadStatus == NULL)
	{
		pThreadStatus = new ThreadStatus;
	}
	return pThreadStatus;
}

void ThreadStatus::SetStatus(int status)
{
	m_lock.lock();
	m_status = status;
	m_lock.unlock();
}

int ThreadStatus::GetStatus()
{
	AutoLock autolock(this->m_lock);
	return m_status;
}


class SendThread : public Thread
{
public:
	SendThread(Socket sock) : m_sock(sock) {}

protected:
	void run();

private:
	Socket m_sock;
};

void SendThread::run()
{
	int rec;

	for(;;)
	{
		rec = ThreadStatus::GetThreadStatusObj()->GetStatus();
		if(1 == rec)
		{
			ThreadStatus::GetThreadStatusObj()->SetStatus(0);

			rec = m_sock.Send("recved message");
			if(rec == -1)
			{
				m_sock.Close();
				break;
			}
		}
		else // 0
		{
			Sleep(100);
		}
	}

	delete this;
}

int main()
{
	IniSock::GetIniSockObj();

	int rec;
	Socket sockfd;

	rec = sockfd.Create();
	if(rec == -1)
	{
		return -1;
	}

	rec = sockfd.Bind(30000);
	if(rec == -1)
	{
		sockfd.Close();
		return -1;
	}

	rec = sockfd.Listen(8);
	if(rec == -1)
	{
		sockfd.Close();
		return -1;
	}

	for(;;)
	{
		rec = sockfd.Accept();
		if(rec == -1)
		{
			sockfd.Close();
			return -1;
		}

		ThreadStatus::GetThreadStatusObj()->SetStatus(0);

		Socket sockfd2(rec);

		SendThread *sendthread = new SendThread(sockfd2);
		sendthread->start();

		int msglen;
		string strMsg;
		char strTmp[16];
		for(;;)
		{
			strMsg.clear();
			rec = sockfd2.Recv(strMsg, 16);
			if(rec == -1)
			{
				break;
			}

			if(0 != memcmp(strMsg.c_str(), "33768287", 8))
			{
				printf("communication protocol ERROR\n");
				break;
			}

			memset(strTmp, 0, sizeof(strTmp));
			memcpy(strTmp, strMsg.c_str()+8, 8);
			msglen = (int)strtoul(strTmp, NULL, 16);

			strMsg.clear();
#if 0
			// LAN use unblock select
			rec = sockfd2.Recv(strMsg, msglen, 50);
#else
			// WAN use block select
			rec = sockfd2.Recv(strMsg, msglen);
#endif
			if(rec == -1)
			{
				break;
			}

			printf("recv :\n%s\n", strMsg.c_str());

			ThreadStatus::GetThreadStatusObj()->SetStatus(1);
		}

		sockfd2.Close();
		ThreadStatus::GetThreadStatusObj()->SetStatus(1);

		Sleep(50);
	}

	return 0;
}
