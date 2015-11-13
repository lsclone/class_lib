#include "analysis.h"
#include <assert.h>


Analysis::Analysis(string fileName)
	: m_fileName(fileName)
{
	analysis();
}

Analysis::~Analysis()
{
	CharpIt tmpIt;

	for(tmpIt = m_recordList.begin(); tmpIt != m_recordList.end(); ++tmpIt)
		if(*tmpIt)
			delete [](*tmpIt);

	for(tmpIt = m_memberList.begin(); tmpIt != m_memberList.end(); ++tmpIt)
		if(*tmpIt)
			delete [](*tmpIt);
}

void Analysis::analysis()
{
	/*获取文件中保存的所有数据，以字符串形式存储在pRecordStr中*/
	FILE* fp = fopen(m_fileName.c_str(), "rb");
	if(NULL == fp)
	{
		printf("Failed to open file %s\n", m_fileName.c_str());
		return;
	}
	//printf("Succeed to open file %s\n", m_fileName.c_str());

	fseek(fp, 0, SEEK_END);
	int nSize = ftell(fp);

	if(nSize)
		printf("file size is %ld\n", nSize);
	else
	{
		printf("%s is empty.\n", m_fileName.c_str());
		fclose(fp);
		return;
	}

	fseek(fp, 0, SEEK_SET);

	char* pRecordStr = new char[nSize + 1];
	assert(pRecordStr != NULL);
	memset(pRecordStr, 0, nSize + 1);

	size_t sRead = fread(pRecordStr, 1, nSize, fp);
	if(sRead != nSize)
	{
		printf("Read %s failed!\n", m_fileName.c_str());
		delete []pRecordStr;
		fclose(fp);
		return;
	}
	printf("sRead = %d\n", sRead);

	//printf("%s",pRecordStr);

	fclose(fp);
	
	/*将获取的字符串analysis，分别获取以'\r\n'为结束符的每一条记录，保存在m_recordList中*/
	int head = 0;
	int tail = 0;
	while(1)
	{
		if(tail >= nSize)
		{
			if(tail > head)
			{
				char* pStr = new char[tail - head + 1];
				assert(pStr != NULL);
				memcpy(pStr, &pRecordStr[head], tail - head);
				pStr[tail - head] = '\0';
				m_recordList.push_back(pStr);
			}
			break;
		}
		char cStr = pRecordStr[tail++];
		if('\r' == cStr)
		{
			if(tail >= nSize)
			{
				if(tail > head)
				{
					char* pStr = new char[tail - head + 1];
					assert(pStr != NULL);
					memcpy(pStr, &pRecordStr[head], tail - head);
					pStr[tail - head] = '\0';
					m_recordList.push_back(pStr);
				}
				break;
			}
			if('\n' == pRecordStr[tail++])
			{
				if(tail - 2 > head)
				{
					char* pStr = new char[tail - 2 - head + 1];
					assert(pStr != NULL);
					memcpy(pStr, &pRecordStr[head], tail - 2 - head);
					pStr[tail - 2 - head] = '\0';
					m_recordList.push_back(pStr);
				}
				head = tail;
			}
		}
		else if('\n' == cStr)
		{
			if(tail - 1 > head)
			{
				char* pStr = new char[tail - 1 - head + 1];
				assert(pStr != NULL);
				memcpy(pStr, &pRecordStr[head], tail - 1 - head);
				pStr[tail - 1 - head] = '\0';
				m_recordList.push_back(pStr);
			}
			head = tail;
		}
	}

	delete []pRecordStr;
}

void Analysis::analysis(string fileName)
{
	m_fileName = fileName;

	CharpIt tmpIt;
	for(tmpIt = m_recordList.begin(); tmpIt != m_recordList.end(); ++tmpIt)
		if(*tmpIt)
			delete[] *tmpIt;
	m_recordList.clear();

	analysis();
}

void Analysis::getData(char chrSep, const CharpList& sepList, DataList& dataList)
{
	CharpIt tmpIt;
	for(tmpIt = m_recordList.begin(); tmpIt != m_recordList.end(); ++tmpIt)
	{
		toMemberList(*tmpIt, chrSep);
		getDataFromMemberList(sepList, dataList);
	}
}

void Analysis::toMemberList(const char* strRecordItem, char chrSep)
{
	int len = strlen(strRecordItem);
	int head = 0;
	int tail = 0;
	int i = 0;
	while(1)
	{
		if(tail >= len)
		{
			if(tail > head)
			{
				char* pStr = new char[tail - head + 1];
				assert(pStr != NULL);
				memcpy(pStr, &strRecordItem[head], tail - head);
				pStr[tail - head] = '\0';
				m_memberList.push_back(pStr);
			}
			break;
		}
		if(chrSep == strRecordItem[tail++])
		{
			if(tail - 1 > head)
			{
				char* pStr = new char[tail - 1 - head + 1];
				assert(pStr != NULL);
				memcpy(pStr, &strRecordItem[head], tail - 1 - head);
				pStr[tail - 1 - head] = '\0';
				m_memberList.push_back(pStr);
			}
			head = tail;
		}
	}
}

void Analysis::getDataFromMemberList(const CharpList& sepList, DataList& dataList)
{
	CharpIt tmpIt;
	if(sepList.size() <= m_memberList.size())
	{
		ConstCharpIt tmpConstIt;
		CharpList* pCharpList = new CharpList;
		for(tmpConstIt = sepList.begin(); tmpConstIt != sepList.end(); ++tmpConstIt)
		{
			for(tmpIt = m_memberList.begin(); tmpIt != m_memberList.end(); ++tmpIt)
			{
				char* pSep = strchr(*tmpIt, '=');
				if(pSep)
				{
					if(strlen(*tmpConstIt) == pSep - *tmpIt && 
					   0 == memcmp(*tmpConstIt, *tmpIt, pSep - *tmpIt))
					{
						int tmpLen = strlen(*tmpIt) - (pSep - *tmpIt) - 1;
						char* tmpData = new char[tmpLen+1];
						assert(tmpData != NULL);
						memcpy(tmpData, pSep+1, tmpLen);
						tmpData[tmpLen] = '\0';
						pCharpList->push_back(tmpData);
						break;
					}
				}
			}
		}

		if(sepList.size() == pCharpList->size())
			dataList.push_back(pCharpList);
		else
		{
			for(tmpIt = pCharpList->begin(); tmpIt != pCharpList->end(); ++tmpIt)
				if(*tmpIt)
					delete [](*tmpIt);
			delete pCharpList;
		}
	}

	for(tmpIt = m_memberList.begin(); tmpIt != m_memberList.end(); ++tmpIt)
		if(*tmpIt)
			delete[] *tmpIt;
	m_memberList.clear();
}

