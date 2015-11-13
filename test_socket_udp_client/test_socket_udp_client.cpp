/*
#include <stdio.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#define RECV_BUFF_SIZE 1024*4


int main()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0)
	{
		printf("WSAStartup Error %d !", WSAGetLastError());
		return 1;
	}

	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	int ret;
	char buffer[RECV_BUFF_SIZE];
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(3030);
	remoteAddr.sin_addr.s_addr = inet_addr("10.16.14.173");
	int remoteAddrLength = sizeof(remoteAddr);

	for (;;)
	{
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "hello");
		if ((ret = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&remoteAddr, remoteAddrLength)) > 0)
			printf("send %d bytes data: %s\n", ret, buffer);
		else
			break;

		memset(buffer, 0, sizeof(buffer));
		if ((ret = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remoteAddr, &remoteAddrLength)) > 0)
			printf("recv %d bytes data: %s\n", ret, buffer);
		else
			break;

		Sleep(1000);
	}

end:
	closesocket(sockfd);
	WSACleanup();

	return 0;
}
*/

#include <stdio.h>
#include <assert.h>
#include <socket_class.h>

#define SEND_SIZE 1024*16

#ifndef WIN32
void Sleep(int msec)
{
	usleep(1000 * msec);
}
#endif

class AppProtocol
{
public:
	/* application protocol */
	typedef struct _MsgHeader
	{
		enum DataFormat
		{
			DataFormat_HTML,
			DataFormat_XML,
		};
		int dataFormat;
		int dataLength;
	}MsgHeader, *PMsgHeader;

	AppProtocol() {}
	inline void setDataFormat(int dataFormat) { m_header.dataFormat = dataFormat; }
	inline void setDataLength(int dataLength) { m_header.dataLength = dataLength; }
	inline int getDataFormat() const { return m_header.dataFormat; }
	inline int getDataLength() const { return m_header.dataLength; }
	void ntoh() {
		m_header.dataFormat = (int)ntohl((unsigned long)m_header.dataFormat);
		m_header.dataLength = (int)ntohl((unsigned long)m_header.dataLength);
	}
	void hton() {
		m_header.dataFormat = (int)htonl((unsigned long)m_header.dataFormat);
		m_header.dataLength = (int)htonl((unsigned long)m_header.dataLength);
	}

private:
	MsgHeader m_header;
};

int main()
{
	IniSock::GetIniSockObj();

	int ret, fileLen, sendLen;
	char* buff = NULL;
	char* tmpbuff = NULL;
	UdpSocket sock;

	ret = sock.Create();
	if (ret == -1)
		return 1;

	//sock.Bind(30123);

	AppProtocol appPro;
	appPro.setDataFormat(AppProtocol::MsgHeader::DataFormat_XML);

	FILE* pFile = fopen("1.xml", "rb");
	if (!pFile)
		goto end;

	fseek(pFile, 0, SEEK_END);
	fileLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	appPro.setDataLength(fileLen);
	appPro.hton();

	buff = (char*)malloc(fileLen);
	assert(buff != NULL);
	ret = fread(buff, 1, fileLen, pFile);
	if (ret != fileLen)
		goto end;

#if 1
	sock.SetRemoteAddr("10.16.14.105", 3030);

	ret = sock.SendBytes((const char*)&appPro, sizeof(AppProtocol));
	if (ret == -1)
		goto end;
	Sleep(50);

	tmpbuff = buff;
	while (fileLen)
	{
		sendLen = fileLen > SEND_SIZE ? SEND_SIZE : fileLen;
		ret = sock.SendBytes(tmpbuff, sendLen);
		if (ret == -1)
			goto end;
		tmpbuff += sendLen;
		fileLen -= sendLen;
		Sleep(50);
	}
#else
	sock.SetRemoteAddr("10.16.14.255", 3030); // 255.255.255.255

	ret = sock.BroadcastBytes((const char*)&appPro, sizeof(AppProtocol));
	if (ret == -1)
		goto end;
	Sleep(50);

	tmpbuff = buff;
	while (fileLen)
	{
		sendLen = fileLen > SEND_SIZE ? SEND_SIZE : fileLen;
		ret = sock.BroadcastBytes(tmpbuff, sendLen);
		if (ret == -1)
			goto end;
		tmpbuff += sendLen;
		fileLen -= sendLen;
		Sleep(50);
	}
#endif

end:
	if (buff)
		free(buff);

	if (pFile)
		fclose(pFile);

	sock.Close();

	return 0;
}
