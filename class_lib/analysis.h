#pragma once

#include <stdio.h>
#include <iostream>
#include <list>
#include <string>
using namespace std;

#ifdef CLASS_EXPORT
#define STRING_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define STRING_API __declspec(dllimport)
#else
#define STRING_API
#endif

typedef list<char*> CharpList;
typedef list<char*>::iterator CharpIt;
typedef list<char*>::const_iterator ConstCharpIt;
typedef list<CharpList*> DataList;
typedef list<CharpList*>::iterator DataIt;


class STRING_API Analysis
{
public:
	Analysis(string fileName);
	~Analysis();
	void analysis();
	void analysis(string fileName);
	void getData(char chrSep, const CharpList& sepList, DataList& dataList);

private:
	void toMemberList(const char* strRecordItem, char chrSep);
	void getDataFromMemberList(const CharpList& sepList, DataList& dataList);

private:
	string m_fileName;

	CharpList m_recordList;
	CharpList m_memberList;
};
