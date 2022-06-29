#include "socket_class.h"


/*
**	IP MTU(Maximum Transmission Unit): 1500 bytes
**	TCP MSS(Maximum Segment Size): 1460 bytes.
**	tcp: 1500-20(ip header length)-20(tcp header length) = 1460
**
**	tcp发包有机制, 如果data length > 1460, 会自动拆分;
**	如果data length < 1460, 会等待一定时间, 如果有新增data, 会组成1460字节发送, 超时则直接发送当前字节数据。
**	由于MTU值可以修改，故MSSS不固定为1460字节。
*/
#define BUFF_SIZE 1024
//#define BUFF_SIZE (1024*64)

IniSock* IniSock::pIniSock = NULL;
IniSock* IniSock::GetIniSockObj()
{
	if(pIniSock == NULL)
	{
		pIniSock = new IniSock;
	}
	return pIniSock;
}

IniSock::IniSock()
{
#ifdef WIN32
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0)
    {
        printf("WSAStartup Error %d !", WSAGetLastError());
    }
#endif
}

IniSock::~IniSock()
{
#ifdef WIN32
	WSACleanup();
#endif
}

/*
const Socket& Socket::operator=(const Socket& sock)
{
	this->m_sockfd = sock.m_sockfd;
	return *this;
}
*/

/*
**		PF_INET: internet protocol(ip) family
**	SOCK_STREAM: transmission control protocol(tcp)
**	 SOCK_DGRAM: user datagram protocol(udp)
*/
int Socket::Create()
{
	Close();

    m_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1)
        printf("create socket ERROR\n");

	return m_sockfd;
}

int Socket::SetOption()
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	struct linger mylinger;
    mylinger.l_onoff = 1;
    mylinger.l_linger = 0;
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_LINGER, (const char*)&mylinger, sizeof(struct linger)) == -1)
	{
		printf("setsockopt for socket force close ERROR\n");
		return -1;
	}

	return 0;
}

int Socket::SetReuseAddrOption()
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	int nReuseaddr = 1;
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nReuseaddr, sizeof(int)) == -1)
	{
		printf("setsockopt for socket reuse address ERROR\n");
		return -1;
	}

	return 0;
}

int Socket::SetSndTimeoutOption(int msec)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&msec, sizeof(int)) == -1)
	{
		printf("setsockopt for socket send timeout ERROR\n");
		return -1;
	}

	return 0;
}

int Socket::SetRcvTimeoutOption(int msec)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&msec, sizeof(int)) == -1)
	{
		printf("setsockopt for socket recv timeout ERROR\n");
		return -1;
	}

	return 0;
}

int Socket::Bind(unsigned short port)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_sockfd, (sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        printf("bind ERROR\n");
        return -1;
    }
	return 0;
}

int Socket::Listen(int num)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

    if (listen(m_sockfd, num) == -1)
    {
        printf("listen ERROR\n");
        return -1;
    }
	return 0;
}

int Socket::Accept()
{
	if (m_sockfd == -1)
	{
		return -1;
	}

    struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(struct sockaddr_in));
    int nLen = sizeof(struct sockaddr_in);

	int sockfd = accept(m_sockfd, (struct sockaddr *)&clientAddr, (socklen_t *)&nLen);
	if (-1 == sockfd)
	{
		printf("accept ERROR\n");
	}

	return sockfd;
}

int Socket::Connect(char *ip, unsigned short port)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	if(connect(m_sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
	{
		printf("connect ERROR\n");
		return -1;
	}
	return 0;
}

int Socket::Recv(string &strMsgBuf, int nMsgLen, int timeout)
{
	if (m_sockfd == -1 || nMsgLen < 0)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[Recv] timeout Invalid\n");
		return -1;
	}

	int retval;
	fd_set rfds;

	int n;
	int strlen;
	char buf[BUFF_SIZE];

	while (nMsgLen)
	{
		FD_ZERO(&rfds);
		FD_SET(m_sockfd, &rfds);

		retval = select(m_sockfd+1, &rfds, NULL, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &rfds))
			{
				memset(buf, 0, sizeof(buf));
				strlen = nMsgLen > sizeof(buf)-1 ? sizeof(buf)-1 : nMsgLen;

				if ((n = recv(m_sockfd, buf, strlen, 0)) != strlen)
				{
					if (n == 0)
					{
						printf("[Recv] connection closed\n");
						return -1;
					}
					else if(n == -1)
					{
						printf("[Recv] ERROR\n");
						return -1;
					}
					else // n > 0
					{
						strMsgBuf += buf;
						nMsgLen -= n;
					}
				}
				else
				{
					strMsgBuf += buf;
					nMsgLen -= strlen;
				}
			}
			else
			{
				printf("[Recv] socket recv buffer is Unreadable\n");
				return -1;
			}
		}
		else
		{
			if(retval == 0)
			{
				printf("[Recv] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[Recv] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

// char* const strMsg; 指向char的const指针
int Socket::Send(const char *strMsg, int timeout)
{
	if (m_sockfd == -1)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[Send] timeout Invalid\n");
		return -1;
	}

	int retval;
	fd_set wfds;

	int n;
	int strMsgLen = strlen(strMsg);

	while (strMsgLen)
	{
		FD_ZERO(&wfds);
		FD_SET(m_sockfd, &wfds);

		retval = select(m_sockfd+1, NULL, &wfds, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &wfds))
			{
				n = send(m_sockfd, strMsg, strMsgLen, 0);
				if(n == -1)
				{
					printf("[Send] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					strMsg += n;
					strMsgLen -= n;
				}
			}
			else
			{
				printf("[Send] socket send buffer is Unwritable\n");
				return -1;
			}
		}
		else
		{
			if(retval == 0)
			{
				printf("[Send] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[Send] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

int Socket::RecvBytes(char* bytesBuf, int nBytes, int timeout)
{
	if (m_sockfd == -1 || nBytes < 0)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[RecvBytes] timeout Invalid\n");
		return -1;
	}

	int n;
	int retval;
	fd_set rfds;

	while (nBytes)
	{
		FD_ZERO(&rfds);
		FD_SET(m_sockfd, &rfds);

		retval = select(m_sockfd + 1, &rfds, NULL, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &rfds))
			{
				n = recv(m_sockfd, bytesBuf, nBytes >= BUFF_SIZE ? BUFF_SIZE : nBytes, 0);
				if (n == 0)
				{
					printf("[RecvBytes] connection closed\n");
					return -1;
				}
				else if (n == -1)
				{
					printf("[RecvBytes] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					bytesBuf += n;
					nBytes -= n;
				}
			}
			else
			{
				printf("[RecvBytes] socket recv buffer is Unreadable\n");
				return -1;
			}
		}
		else
		{
			if (retval == 0)
			{
				printf("[RecvBytes] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[RecvBytes] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

int Socket::SendBytes(const char *bytesBuf, int nBytes, int timeout)
{
	if (m_sockfd == -1 || nBytes < 0)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[SendBytes] timeout Invalid\n");
		return -1;
	}

	int n;
	int retval;
	fd_set wfds;

	while (nBytes)
	{
		FD_ZERO(&wfds);
		FD_SET(m_sockfd, &wfds);

		retval = select(m_sockfd + 1, NULL, &wfds, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &wfds))
			{
				n = send(m_sockfd, bytesBuf, nBytes >= BUFF_SIZE ? BUFF_SIZE : nBytes, 0);
				if (n == -1)
				{
					printf("[SendBytes] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					bytesBuf += n;
					nBytes -= n;
				}
			}
			else
			{
				printf("[SendBytes] socket send buffer is Unwritable\n");
				return -1;
			}
		}
		else
		{
			if (retval == 0)
			{
				printf("[SendBytes] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[SendBytes] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

void Socket::Close()
{
	if(m_sockfd != -1)
	{
		close(m_sockfd);
		m_sockfd = -1;
	}
}

void Socket::ShutDown()
{
	if(m_sockfd != -1)
	{
#ifdef WIN32
		//shutdown(m_sockfd, SD_BOTH);
		shutdown(m_sockfd, 0x02);
#else
		shutdown(m_sockfd, SHUT_RDWR);
#endif
		m_sockfd = -1;
	}
}

/*
**		PF_INET: internet protocol(ip) family
**	SOCK_STREAM: transmission control protocol(tcp)
**	 SOCK_DGRAM: user datagram protocol(udp)
*/
int UdpSocket::Create()
{
	Close();

	m_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_sockfd == -1)
		printf("create socket ERROR\n");

	return m_sockfd;
}

void UdpSocket::SetRemoteAddr(const char* ipAddr, unsigned short port)
{
	m_remoteAddr.sin_family = AF_INET;
	m_remoteAddr.sin_port = htons(port);
	m_remoteAddr.sin_addr.s_addr = inet_addr(ipAddr);
}

int UdpSocket::RecvBytes(char* bytesBuf, int nBytes, int timeout)
{
	if (m_sockfd == -1 || nBytes < 0)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = 0;
		tv.tv_usec = timeout * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[RecvBytes] timeout Invalid\n");
		return -1;
	}

	int n;
	int retval;
	int length;
	fd_set rfds;

	while (nBytes)
	{
		FD_ZERO(&rfds);
		FD_SET(m_sockfd, &rfds);

		retval = select(m_sockfd + 1, &rfds, NULL, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &rfds))
			{
				length = sizeof(struct sockaddr_in);
				n = recvfrom(m_sockfd, bytesBuf, nBytes >= BUFF_SIZE ? BUFF_SIZE : nBytes,
					0, (struct sockaddr *)&m_remoteAddr, (socklen_t*)&length);
				if (n == 0)
				{
					printf("[RecvBytes] connection closed\n");
					return -1;
				}
				else if (n == -1)
				{
					printf("[RecvBytes] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					bytesBuf += n;
					nBytes -= n;
				}
			}
			else
			{
				printf("[RecvBytes] socket recv buffer is Unreadable\n");
				return -1;
			}
		}
		else
		{
			if (retval == 0)
			{
				printf("[RecvBytes] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[RecvBytes] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

int UdpSocket::SendBytes(const char *bytesBuf, int nBytes, int timeout)
{
	if (m_sockfd == -1 || nBytes < 0)
	{
		return -1;
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = 0;
		tv.tv_usec = timeout * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[SendBytes] timeout Invalid\n");
		return -1;
	}

	int n;
	int retval;
	fd_set wfds;

	while (nBytes)
	{
		FD_ZERO(&wfds);
		FD_SET(m_sockfd, &wfds);

		retval = select(m_sockfd + 1, NULL, &wfds, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &wfds))
			{
				n = sendto(m_sockfd, bytesBuf, nBytes >= BUFF_SIZE ? BUFF_SIZE : nBytes,
						   0, (struct sockaddr *)&m_remoteAddr, sizeof(struct sockaddr_in));
				if (n == -1)
				{
					printf("[SendBytes] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					bytesBuf += n;
					nBytes -= n;
				}
			}
			else
			{
				printf("[SendBytes] socket send buffer is Unwritable\n");
				return -1;
			}
		}
		else
		{
			if (retval == 0)
			{
				printf("[SendBytes] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[SendBytes] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

int UdpSocket::BroadcastBytes(const char *bytesBuf, int nBytes, int timeout)
{
	if (m_sockfd == -1 || nBytes < 0)
	{
		return -1;
	}

	bool bOpt;
	int optLen = sizeof(bool);
	if (getsockopt(m_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, (socklen_t*)&optLen) < 0)
	{
		printf("[BroadcastBytes] getsockopt ERROR\n");
		return -1;
	}
	if (bOpt == false)
	{
		bOpt = true;
		if (setsockopt(m_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bool)) < 0)
		{
			printf("[BroadcastBytes] setsockopt ERROR\n");
			return -1;
		}
	}

	struct timeval tv;
	struct timeval *ptv = NULL;
	if (timeout == -1)
	{
	}
	else if (timeout >= 0)
	{
		tv.tv_sec = 0;
		tv.tv_usec = timeout * 1000;
		ptv = &tv;
	}
	else
	{
		printf("[BroadcastBytes] timeout Invalid\n");
		return -1;
	}

	int n;
	int retval;
	fd_set wfds;

	while (nBytes)
	{
		FD_ZERO(&wfds);
		FD_SET(m_sockfd, &wfds);

		retval = select(m_sockfd + 1, NULL, &wfds, NULL, ptv);
		if (retval == 1)
		{
			if (FD_ISSET(m_sockfd, &wfds))
			{
				n = sendto(m_sockfd, bytesBuf, nBytes >= BUFF_SIZE ? BUFF_SIZE : nBytes,
					0, (struct sockaddr *)&m_remoteAddr, sizeof(struct sockaddr_in));
				if (n == -1)
				{
					printf("[BroadcastBytes] ERROR\n");
					return -1;
				}
				else // n > 0
				{
					bytesBuf += n;
					nBytes -= n;
				}
			}
			else
			{
				printf("[BroadcastBytes] socket send buffer is Unwritable\n");
				return -1;
			}
		}
		else
		{
			if (retval == 0)
			{
				printf("[BroadcastBytes] unblocked select TIMEOUT\n");
			}
			else // retval == -1
			{
				printf("[BroadcastBytes] unblocked select ERROR\n");
			}
			return -1;
		}
	}

	return 0;
}

