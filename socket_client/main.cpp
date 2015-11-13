#include <stdio.h>
#include "thread_class.h"
#include "socket_class.h"
#if 1
#include "string_class.h"
#else
#include <iostream>
#include <string>
using namespace std;
#endif


int main()
{
	IniSock::GetIniSockObj();

	string strMsg = "33768287";
	FILE *pfile = fopen("1.xml", "rb");
	if(pfile == NULL)
	{
		return -1;
	}

	int len;
	fseek(pfile, 0, SEEK_END);
	len = ftell(pfile);
	char strLen[16] = {0};
	sprintf(strLen, "%08x", len);

	strMsg += strLen;

	fseek(pfile, 0, SEEK_SET);

	int rec;
	char *strBuf = new char[len+1];
	memset(strBuf, 0, len+1);
	rec = fread(strBuf, 1, len, pfile);
	if(rec != len)
	{
		delete []strBuf;
		fclose(pfile);
		return -1;
	}

	strMsg += strBuf;

	delete []strBuf;
	strBuf = NULL;

	fclose(pfile);
	pfile = NULL;

	Socket sockfd;

	rec = sockfd.Create();
	if(rec == -1)
	{
		return -1;
	}

#if 1
	rec = sockfd.Connect("127.0.0.1", 30000);
#else
	rec = sockfd.Connect("124.205.155.90", 30000);
#endif
	if(rec == -1)
	{
		sockfd.Close();
		return -1;
	}

	string strRecvMsg;
	for(;;)
	{
		rec = sockfd.Send(strMsg.c_str());
		if(rec == -1)
		{
			sockfd.Close();
			return -1;
		}

		strRecvMsg.clear();
		rec = sockfd.Recv(strRecvMsg, strlen("recved message"));
		if(rec == -1)
		{
			sockfd.Close();
			return -1;
		}

		if(strRecvMsg.c_str() != NULL)
		{
			printf("recv message: %s\n", strRecvMsg.c_str());
		}
	}

	sockfd.Close();

	return 0;
}
