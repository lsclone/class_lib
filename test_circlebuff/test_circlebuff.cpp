#include <stdio.h>
#include "circlebuffer.h"

class Message
{
public:
	Message() {
		m_nLen = 0;
		m_data = NULL;
	}

	const Message& operator=(const Message& msg) {
		if (!msg.m_data) // NULL
		{
			if (m_data)
			{
				free(m_data);
				m_data = NULL;
				m_nLen = 0;
			}
		}
		else
		{
			if (m_data)
				m_data = (char*)realloc(m_data, msg.m_nLen);
			else // NULL
				m_data = (char*)malloc(msg.m_nLen);

			memcpy(m_data, msg.m_data, msg.m_nLen);
			m_nLen = msg.m_nLen;
		}
		return (*this);
	}

	~Message() {
		if (m_data)
			free(m_data);
	}

	void setMessage(const char* data, int len) {
		if (data == NULL || len == 0)
			return;

		if (m_data)
			m_data = (char*)realloc(m_data, len);
		else // NULL
			m_data = (char*)malloc(len);

		memcpy(m_data, data, len);
		m_nLen = len;
	}

	void getMessage(char* data)
	{
		if (data == NULL)
			return;

		if (m_data == NULL || m_nLen == 0)
			return;

		memcpy(data, m_data, m_nLen);
	}

private:
	int m_nLen;
	char* m_data;
};

CircleBuff<Message> cirbuff;

class SubThread : public Thread
{
public:
	SubThread() {}

protected:
	void run() {
		Message msg;

		for (;;)
		{
			cirbuff.lock();
			if (cirbuff.isNotEmpty())
			{
				if (0 == cirbuff.pop(msg)) // success
				{
					cirbuff.wakeNotFull();
					cirbuff.unlock();
					// process data
					char strMsg[16] = { 0 };
					msg.getMessage(strMsg);
					printf("1 message is : %s\n", strMsg);
				}
				else
				{
					cirbuff.wakeNotFull();
					cirbuff.unlock();
				}
			}
			else // empty
			{
				printf("pre waitNotEmpty\n");
				cirbuff.waitNotEmpty();
				printf("aft waitNotEmpty\n");

				if (0 == cirbuff.pop(msg)) // success
				{
					cirbuff.wakeNotFull();
					cirbuff.unlock();
					// process data
					char strMsg[16] = { 0 };
					msg.getMessage(strMsg);
					printf("2 message is : %s\n", strMsg);

					if (atoi(strMsg) % 10 == 9)
#ifdef WIN32
						Sleep(1000 * 10);
#else
						usleep(1000 * 1000 * 10);
#endif
				}
				else
				{
					printf("pop failed.\n");
					cirbuff.wakeNotFull();
					cirbuff.unlock();
				}
			}
		}
	}
};

int main()
{
	SubThread subThread;
	subThread.start();

	int index = 0;
	Message msg;
	for (;;)
	{
		// set message content
		char strMsg[16] = { 0 };
		sprintf(strMsg, "%04d", index++);
		msg.setMessage(strMsg, 4);

		cirbuff.lock();
		if (cirbuff.isNotFull())
		{
			cirbuff.push(msg);
			cirbuff.wakeNotEmpty();
			cirbuff.unlock();
		}
		else // full
		{
			printf("pre waitNotFull --\n");
			cirbuff.waitNotFull(1000);
			printf("aft waitNotFull --\n");
			if(0 == cirbuff.push(msg))
				cirbuff.wakeNotEmpty();
			cirbuff.unlock();
		}

#ifdef WIN32
		Sleep(500);
#else
		usleep(500 * 1000);
#endif
	}

	return 0;
}
