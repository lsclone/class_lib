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

	struct sockaddr_in localAddr, remoteAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(3030);
	localAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr)) == -1)
	{
		printf("bind ERROR\n");
		goto end;
	}

	struct in_addr remote_addr;
	int remoteAddrLength;
	char buffer[RECV_BUFF_SIZE];
	for (;;)
	{
		memset(buffer, 0, sizeof(buffer));
		remoteAddrLength = sizeof(remoteAddr);
		if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remoteAddr, &remoteAddrLength) > 0)
		{
			memcpy(&remote_addr, &(remoteAddr.sin_addr.s_addr), 4);
			printf("recv data: %s, from: %s:%d\n", buffer, inet_ntoa(remote_addr), ntohs(remoteAddr.sin_port));
		}
		else
			break;

		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "nice to meet you.");
		if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&remoteAddr, remoteAddrLength) > 0)
			printf("send data: %s\n", buffer);
		else
			break;
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

	int ret;
	char* buff = NULL;
	AppProtocol appPro;
	UdpSocket sock;
	ret = sock.Create();
	if (ret == -1)
		return 1;
	ret = sock.Bind(3030);
	if (ret == -1)
		goto end;

	ret = sock.RecvBytes((char*)&appPro, sizeof(AppProtocol));
	if (ret == -1)
		goto end;
	appPro.ntoh();

	buff = (char*)malloc(appPro.getDataLength()+ 1);
	assert(buff != NULL);

	ret = sock.RecvBytes(buff, appPro.getDataLength());
	if (ret == -1)
		goto end;
	buff[appPro.getDataLength()] = '\0';
	printf("%s\n", buff);

end:
	if (buff)
		free(buff);

	sock.Close();

	return 0;
}
