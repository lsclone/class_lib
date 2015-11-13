#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <time.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#if 0
#include "string_class.h"
#else
#include <iostream>
#include <string>
using namespace std;
#endif

#ifdef WIN32
#define close closesocket
#define ioctl ioctlsocket
#define socklen_t int
#endif

#ifdef CLASS_EXPORT
#define SOCKET_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define SOCKET_API __declspec(dllimport)
#else
#define SOCKET_API
#endif

class SOCKET_API IniSock
{
public:
	static IniSock* GetIniSockObj();
	IniSock();
	~IniSock();
private:
	static IniSock* pIniSock;
};

class SOCKET_API Socket
{
public:
	Socket(int sockfd = -1) : m_sockfd(sockfd) {}
	virtual ~Socket() {}

	/*
	**  return socket fd, or -1 if an error occurred.
	*/
	virtual int Create();

	/*
	**  调用Close()的时候会立刻返回，丢弃缓冲区的数据，强制关闭连接并通知对端(send RST packet)
	**  return: 0 success, -1 error.
	*/
	virtual int SetOption();

	/*
	**	listen socket on sever peer usually call SetReuseAddrOption() after Create()
	**  return: 0 success, -1 error
	*/
	virtual int SetReuseAddrOption();

	/*
	**	socket will wait for sending or timeout occur.
	**	parameter: msec(millisecond)
	**  return: 0 success, -1 error
	*/
	virtual int SetSndTimeoutOption(int msec);

	/*
	**	socket will wait for receiving or timeout occur.
	**	parameter: msec(millisecond)
	**  return: 0 success, -1 error
	*/
	virtual int SetRcvTimeoutOption(int msec);

	virtual int Bind(unsigned short port);
	int Listen(int num);
	int Accept();
	int Connect(char *ip, unsigned short port);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, Recv will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int Recv(string &strMsgBuf, int nMsgLen, int timeout = -1);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, Send will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int Send(const char *strMsg, int timeout = -1);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, RecvBytes will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int RecvBytes(char* bytesBuf, int nBytes, int timeout = -1);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, SendBytes will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int SendBytes(const char *bytesBuf, int nBytes, int timeout = -1);

	void Close();
	void ShutDown();

protected:
	int m_sockfd;
};

class SOCKET_API UdpSocket : public Socket
{
public:
	UdpSocket(int sockfd = -1) : Socket(sockfd) {}

	/*
	**  return socket fd, or -1 if an error occurred.
	*/
	virtual int Create();

	/*
	**  set remote host ip and port
	*/
	void SetRemoteAddr(const char* ipAddr, unsigned short port);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, RecvBytes will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int RecvBytes(char* bytesBuf, int nBytes, int timeout = -1);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, SendBytes will block indefinitely.
	**  return: 0 success, -1 error
	*/
	virtual int SendBytes(const char *bytesBuf, int nBytes, int timeout = -1);

	/*
	**	parameter: timeout(millisecond)
	**  if timeout is -1, BroadcastBytes will block indefinitely.
	**  return: 0 success, -1 error
	*/
	int BroadcastBytes(const char *bytesBuf, int nBytes, int timeout = -1);

private:
	struct sockaddr_in m_remoteAddr;
};
