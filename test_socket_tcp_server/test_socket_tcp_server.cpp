/*
** @ftp server
** @author Li Shuai
** @date 2014-07-26
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

#include <crypto.h>
#include <thread_class.h>
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


class ClientThread : public Thread
{
public:
	ClientThread(int sockclient) : m_sockclient(sockclient) {}

protected:
	virtual void run();

	void processGetMsg(int msgContentLength);
	void processPutMsg(int msgContentLength);

	/* return -1 if error, or name length if success. */
	int parsePathForName(const char* path, std::string& name);

private:
	Socket m_sockclient;
};

void ClientThread::run()
{
	AppProtocol prot;

	if (m_sockclient.RecvBytes((char*)&prot, sizeof(AppProtocol)) != 0)
	{
		m_sockclient.Close();
		return;
	}

	prot.ntoh();

	int msgType = prot.getMsgType();
	switch (msgType)
	{
	case AppProtocol::MsgHeader::MSG_GET:
		processGetMsg(prot.getMsgContentLength());
		break;
	case AppProtocol::MsgHeader::MSG_PUT:
		processPutMsg(prot.getMsgContentLength());
		break;
	default:
		printf("[Error] invalid message type.\n");
		break;
	}

	m_sockclient.Close();

	delete this;
}

void ClientThread::processGetMsg(int msgContentLength)
{
	if (msgContentLength <= 0)
		return;

	char* msgContent = (char*)malloc(msgContentLength+1);
	if (msgContent == NULL)
	{
		printf("[Error] out of memory.\n");
		return;
	}

	/*  receive file absolute path for sending file back to client*/
	if (m_sockclient.RecvBytes(msgContent, msgContentLength) != 0)
	{
		free(msgContent);
		return;
	}
	msgContent[msgContentLength] = '\0';
	if (strlen(msgContent) == 0)
	{
		free(msgContent);
		return;
	}

	std::string filename;
	FileProtocol fileProt;
	int ret = parsePathForName(msgContent, filename);
	if (ret <= 0)
	{
		printf("[Error] parse file name  %s.\n", msgContent);
		free(msgContent);
		return;
	}

	fileProt.setFileNameLength(ret);

	/* open file for sending to client */
	FILE* pfile = fopen(msgContent, "rb");
	if (pfile == NULL)
	{
		printf("[Error] open file: %s.\n", msgContent);
		free(msgContent);
		return;
	}

	fseek(pfile, 0, SEEK_END);
	int fileLength = ftell(pfile);
	fseek(pfile, 0, SEEK_SET);

	fileProt.setFileContentLength(fileLength);
	fileProt.hton();

	/* send file header protocol to client */
	if (m_sockclient.SendBytes((const char*)&fileProt, sizeof(FileProtocol)) == -1)
	{
		free(msgContent);
		fclose(pfile);
		return;
	}

	/* send back file name to client */
	if (m_sockclient.SendBytes(filename.c_str(), filename.size()) == -1)
	{
		free(msgContent);
		fclose(pfile);
		return;
	}

	/* prepare file content buffer */
	msgContent = (char*)realloc(msgContent, FILE_BUFF_SIZE);
	if (msgContent == NULL)
	{
		printf("[Error] out of memory.\n");
		fclose(pfile);
		return;
	}

	/* send file to client */
	int readlen;
	while (fileLength)
	{
		readlen = fileLength > FILE_BUFF_SIZE ? FILE_BUFF_SIZE : fileLength;
		ret = fread(msgContent, 1, readlen, pfile);
		if (readlen != ret)
		{
			printf("[Error] read file.\n");
			free(msgContent);
			fclose(pfile);
			return;
		}

		fileLength -= readlen;

		if (m_sockclient.SendBytes(msgContent, readlen) == -1)
		{
			free(msgContent);
			fclose(pfile);
			return;
		}
	}

	free(msgContent);
	fclose(pfile);
}

void ClientThread::processPutMsg(int msgContentLength)
{
	if (msgContentLength <= 0)
		return;

	char* msgContent = (char*)malloc(msgContentLength);
	if (msgContent == NULL)
	{
		printf("[Error] out of memory.\n");
		return;
	}

	/*  receive name of file which will upload from client. */
	if (m_sockclient.RecvBytes(msgContent, msgContentLength) != 0)
	{
		free(msgContent);
		return;
	}

	Crypto myCrypto(cryptKey);
	if (myCrypto.TDecrypt((unsigned char*)msgContent, 0, msgContentLength) != 0)
	{
		printf("[Error] decrypt file name.\n");
		free(msgContent);
		return;
	}

	free(msgContent);
	msgContent = NULL;

	std::string filename((char*)myCrypto.GetDecryptedBytes(), myCrypto.GetDecryptedSize());

	FileProtocol fileProt;
	if (m_sockclient.RecvBytes((char*)&fileProt, sizeof(FileProtocol)) == -1)
		return;
	fileProt.ntoh();

	FILE* pfile = fopen(filename.c_str(), "wb");
	if (pfile == NULL)
	{
		printf("Error, open file: %s\n", filename.c_str());
		return;
	}

	int recvLen;
	char* strFileCont = NULL;
	int fileSize = fileProt.getFileContentLength();
	while (fileSize)
	{
		if (m_sockclient.RecvBytes((char*)&recvLen, sizeof(int)) == -1)
			break;

		recvLen = (int)ntohl((unsigned long)recvLen);
		if (!strFileCont)
			strFileCont = (char*)malloc(recvLen);
		else
			strFileCont = (char*)realloc(strFileCont, recvLen);

		if (m_sockclient.RecvBytes(strFileCont, recvLen) == -1)
			break;

		if (myCrypto.TDecrypt((unsigned char*)strFileCont, 0, recvLen) != 0)
		{
			printf("[Error] decrypt file content.\n");
			break;
		}

		if (myCrypto.GetDecryptedSize() != fwrite(myCrypto.GetDecryptedBytes(), 1, myCrypto.GetDecryptedSize(), pfile))
		{
			printf("[Error] write file failed.\n");
			break;
		}

		fileSize -= myCrypto.GetDecryptedSize();
	}

	fclose(pfile);

	if (strFileCont)
		free(strFileCont);
}

int ClientThread::parsePathForName(const char* path, std::string& name)
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

	if (flag == -1 || i-flag <= 1)
		return 0;

	char* strTmp = (char*)malloc(i-flag);
	if (strTmp == NULL)
	{
		printf("[Error] out of memory.\n");
		return 0;
	}
	
	memcpy(strTmp, path+flag+1, i-flag-1);
	strTmp[i-flag-1] = '\0';
	name = strTmp;

	free(strTmp);

	return (i-flag-1);
}


int main()
{
	IniSock::GetIniSockObj();

	int rec;
	Socket socklisten;

	rec = socklisten.Create();
	if (rec == -1)
	{
		return -1;
	}

	socklisten.SetReuseAddrOption();

	/* port: 1024 ~ 65535 */
	rec = socklisten.Bind(1049);
	if (rec == -1)
	{
		socklisten.Close();
		return -1;
	}

	rec = socklisten.Listen(5);
	if (rec == -1)
	{
		socklisten.Close();
		return -1;
	}

	for (;;)
	{
		rec = socklisten.Accept();
		if (rec == -1)
		{
			socklisten.Close();
			return -1;
		}

		ClientThread* pClientThread = new ClientThread(rec);
		pClientThread->start();
	}

	return 0;
}
