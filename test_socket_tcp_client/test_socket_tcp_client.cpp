/*
** @ftp client
** @author Li Shuai
** @date 2014-07-27
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
********************************************************************
** update:
** add crypt(encryp&decrypt) into the process of upload(PUT) file.
********************************************************************
** follow-up work:
** add speed limit (download and upload).
*/

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <crypto.h>
#include <socket_class.h>


unsigned char cryptKey[] = { 0xfb, 0x12, 0x22, 0x23,
						     0xa1, 0x12, 0x21, 0x26,
							 0x00, 0x00, 0x00, 0x00,
							 0x00, 0xfd, 0xfd, 0xfd };


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
		enum MsgType
		{
			MSG_PUT,
			MSG_GET,
		};
		int msgType;
		int msgContentLength;
	}MsgHeader, *PMsgHeader;

	AppProtocol() {}
	inline void setMsgType(int msgType) { m_header.msgType = msgType; }
	inline void setMsgContentLength(int msgContentLength) { m_header.msgContentLength = msgContentLength; }
	inline int getMsgType() const { return m_header.msgType; }
	inline int getMsgContentLength() const { return m_header.msgContentLength; }
	void ntoh() {
		m_header.msgType = (int)ntohl((unsigned long)m_header.msgType);
		m_header.msgContentLength = (int)ntohl((unsigned long)m_header.msgContentLength);
	}
	void hton() {
		m_header.msgType = (int)htonl((unsigned long)m_header.msgType);
		m_header.msgContentLength = (int)htonl((unsigned long)m_header.msgContentLength);
	}

private:
	MsgHeader m_header;
};


class FileProtocol
{
#define FILE_BUFF_SIZE (1024*16)

public:
	/* file protocol */
	typedef struct _FileHeader
	{
		int fileNameLength;
		int fileContentLength;
	}FileHeader, *PFileHeader;

	FileProtocol() {}
	inline void setFileNameLength(int fileNameLength) { m_header.fileNameLength = fileNameLength; }
	inline void setFileContentLength(int fileContentLength) { m_header.fileContentLength = fileContentLength; }
	inline int getFileNameLength() const { return m_header.fileNameLength; }
	inline int getFileContentLength() const { return m_header.fileContentLength; }
	void ntoh() {
		m_header.fileNameLength = (int)ntohl((unsigned long)m_header.fileNameLength);
		m_header.fileContentLength = (int)ntohl((unsigned long)m_header.fileContentLength);
	}
	void hton() {
		m_header.fileNameLength = (int)htonl((unsigned long)m_header.fileNameLength);
		m_header.fileContentLength = (int)htonl((unsigned long)m_header.fileContentLength);
	}

private:
	FileHeader m_header;
};


class CommandMotivator
{
public:
	CommandMotivator(Socket& sockfd) : m_sockfd(sockfd) {}

	/*  return : 0 success, else error */
	int processCmd(const char* cmd);

protected:
	/*  return : 0 success, -1 error */
	int motivateMsgPut(const char* url);

	/*  return : 0 success, -1 error */
	int motivateMsgGet(const char* url);

private:
	AppProtocol m_prot;
	Socket& m_sockfd;
};

int CommandMotivator::processCmd(const char* cmd)
{
	int i, cmdLen, rec = 0;

	if (0 == memcmp(cmd, "put", 3))
		m_prot.setMsgType(AppProtocol::MsgHeader::MSG_PUT);
	else if (0 == memcmp(cmd, "get", 3))
		m_prot.setMsgType(AppProtocol::MsgHeader::MSG_GET);
	else
	{
		printf("Warning, invalid input.\n");
		return 1;
	}

	for (i = 3; cmd[i] != '\0'; i++)
	{
		if (cmd[i] == ' ')
			continue;
		else {
			if (cmd[i-1] == ' ')
				break;
			else {
				printf("Warning, invalid input.\n");
				return 1;
			}
		}
	}
	if (cmd[i] == '\0')
	{
		printf("Warning, invalid input.\n");
		return 1;
	}

	cmdLen = strlen(cmd);
	char* url = (char*)malloc(cmdLen-i+1);
	if (url == NULL)
	{
		printf("Error, out of memory.\n");
		return 2;
	}

	memcpy(url, &cmd[i], cmdLen - i);
	url[cmdLen - i] = '\0';

	switch (m_prot.getMsgType())
	{
	case AppProtocol::MsgHeader::MSG_PUT:
		rec = motivateMsgPut(url);
		break;
	case AppProtocol::MsgHeader::MSG_GET:
		rec = motivateMsgGet(url);
		break;
	default:
		printf("Error, invalid message type.\n");
		rec = 1;
		break;
	}

	free(url);

	return rec;
}

int parsePathForName(const char* path, std::string& name)
{
	int i = 0;
	int flag = -1;
	for (;;)
	{
		if (path[i] == '\0')
			break;
		if (path[i] == '/' || path[i] == '\\')
			flag = i;
		i++;
	}

	if (flag == -1 || i - flag <= 1)
		return 0;

	char* strTmp = (char*)malloc(i - flag);
	if (strTmp == NULL)
	{
		printf("[Error] out of memory.\n");
		return 0;
	}

	memcpy(strTmp, path + flag + 1, i - flag - 1);
	strTmp[i - flag - 1] = '\0';
	name = strTmp;

	free(strTmp);

	return (i - flag - 1);
}

int CommandMotivator::motivateMsgPut(const char* url)
{
	FileProtocol fileProt;
	char strFileCont[FILE_BUFF_SIZE] = { 0 };
	FILE* pfile = NULL;
	int fileSize = 0;

	std::string filename;
	int ret = parsePathForName(url, filename);
	if (ret <= 0)
	{
		printf("[Error] parse file name  %s.\n", url);
		return -1;
	}

	Crypto myCrypto(cryptKey);
	myCrypto.TEncrypt((unsigned char*)filename.c_str(), 0, filename.size());

	m_prot.setMsgContentLength(myCrypto.GetEncryptedSize());
	m_prot.hton();

	if (m_sockfd.SendBytes((const char*)&m_prot, sizeof(AppProtocol)) == -1)
		return -1;

	if (m_sockfd.SendBytes((const char*)myCrypto.GetEncryptedBytes(), myCrypto.GetEncryptedSize()) == -1)
		return -1;

	/* open file for upload to server */
	pfile = fopen(url, "rb");
	if (pfile == NULL)
	{
		printf("[Error] open file: %s.\n", url);
		return -1;
	}

	fseek(pfile, 0, SEEK_END);
	fileSize = ftell(pfile);
	fseek(pfile, 0, SEEK_SET);

	fileProt.setFileContentLength(fileSize);
	fileProt.hton();

	/* send file header protocol to server */
	if (m_sockfd.SendBytes((const char*)&fileProt, sizeof(FileProtocol)) == -1)
	{
		fclose(pfile);
		return -1;
	}

	/* upload file to server */
	int readlen;
	while (fileSize)
	{
		readlen = fileSize > FILE_BUFF_SIZE ? FILE_BUFF_SIZE : fileSize;
		ret = fread(strFileCont, 1, readlen, pfile);
		if (readlen != ret)
		{
			printf("[Error] read file: %s.\n", url);
			fclose(pfile);
			return -1;
		}

		myCrypto.TEncrypt((unsigned char*)strFileCont, 0, readlen);

		ret = (int)htonl((unsigned long)myCrypto.GetEncryptedSize());
		if (m_sockfd.SendBytes((const char*)&ret, sizeof(int)) == -1)
		{
			fclose(pfile);
			return -1;
		}

		if (m_sockfd.SendBytes((const char*)myCrypto.GetEncryptedBytes(), myCrypto.GetEncryptedSize()) == -1)
		{
			fclose(pfile);
			return -1;
		}

		fileSize -= readlen;
	}

	fclose(pfile);

	return 0;
}

int CommandMotivator::motivateMsgGet(const char* url)
{
	int urlLen = strlen(url);
	FileProtocol fileProt;
	char* strFileName = NULL;
	char strFileCont[FILE_BUFF_SIZE] = { 0 };
	FILE* pfile = NULL;
	int fileSize = 0;
	int recvLen = 0;

	m_prot.setMsgContentLength(urlLen);
	m_prot.hton();
	if (m_sockfd.SendBytes((const char*)&m_prot, sizeof(AppProtocol)) == -1)
		return -1;

	if (m_sockfd.SendBytes(url, urlLen) == -1)
		return -1;

	if (m_sockfd.RecvBytes((char*)&fileProt, sizeof(FileProtocol)) == -1)
		return -1;
	fileProt.ntoh();

	strFileName = (char*)malloc(fileProt.getFileNameLength()+1);
	if (strFileName == NULL)
		return -1;

	if (m_sockfd.RecvBytes(strFileName, fileProt.getFileNameLength()) == -1)
	{
		free(strFileName);
		return -1;
	}
	strFileName[fileProt.getFileNameLength()] = '\0';

	pfile = fopen(strFileName, "wb");
	if (pfile == NULL)
	{
		printf("Error, open file: %s\n", strFileName);
		free(strFileName);
		return -1;
	}

	fileSize = fileProt.getFileContentLength();
	while (fileSize)
	{
		recvLen = fileSize > FILE_BUFF_SIZE ? FILE_BUFF_SIZE : fileSize;
		if (m_sockfd.RecvBytes(strFileCont, recvLen) == -1)
		{
			free(strFileName);
			fclose(pfile);
			return -1;
		}

		if (recvLen != fwrite(strFileCont, 1, recvLen, pfile))
		{
			free(strFileName);
			fclose(pfile);
			return -1;
		}

		fileSize -= recvLen;
	}

	if (pfile)
		fclose(pfile);

	if (strFileName)
		free(strFileName);

	return 0;
}


int main(int argc, char** argv)
{
	if (argc == 1 || argc > 3)
	{
		printf("pls press \"ftpclient.exe -help\" to obtain specific information.\n");
		return -1;
	}

	if (argc == 2)
	{
		if (strlen(argv[1]) == 5 && memcmp(argv[1], "-help", 5) == 0)
		{
			std::cout << "Common examples are:" << std::endl
					  << "	ftpclient.exe  ip  port" << std::endl
					  << "usage: " << std::endl
					  << "	put filename" << std::endl
					  << "	get filepath" << std::endl;
			return 0;
		}
		else
		{
			printf("pls press \"ftpclient.exe -help\" to obtain specific information.\n");
			return -1;
		}
	}

	IniSock::GetIniSockObj();

	int rec;
	Socket sockfd;
	char cmd[2048];

	for (;;)
	{
		rec = sockfd.Create();
		if (rec == -1)
		{
			return -1;
		}

		rec = sockfd.Connect(argv[1], atoi(argv[2]));
		if (rec == -1)
		{
			sockfd.Close();
			return -1;
		}

		for (;;)
		{
			if (NULL == gets(cmd))
			{
				printf("Error, pls check input.\n");
				continue;
			}
			CommandMotivator motivator(sockfd);
			if (motivator.processCmd(cmd) == -1)
				break;
		}

		sockfd.Close();
	}

	return 0;
}