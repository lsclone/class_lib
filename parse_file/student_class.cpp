#include "student_class.h"
#include <iostream>
#include <list>
#include <string>
using namespace std;


void Student::setItem(int nId, int nAge, string sName)
{
	m_nId = nId;
	m_nAge = nAge;
	m_sName = sName;
	toString();
}

void Student::display(int count) const
{
	switch(count)
	{
	case 3:
		printf("ID: %d\t Age: %d\t Name: %s\n", m_nId, m_nAge, m_sName.c_str());
		break;
	case 2:
		printf("ID: %d\t Name: %s\n", m_nId, m_sName.c_str());
		break;
	case 1:
		printf("Name: %s\n", m_sName.c_str());
		break;
	}
}

void Student::toString()
{
	list<char> listStr;

	int tmpAge = m_nAge;
	int age = tmpAge;
	while(1)
	{
		age = tmpAge %10;
		char cage = age + '0';
		listStr.push_front(cage);
		tmpAge /= 10;
		if(tmpAge == 0)
			break;
	}
	listStr.push_front('=');
	listStr.push_front('e');
	listStr.push_front('g');
	listStr.push_front('a');
	listStr.push_front('|');

	int tmpId = m_nId;
	int id = tmpId;
	while( 1 )
	{
		id = tmpId % 10;
		char cid = id + '0';
		listStr.push_front(cid);
		tmpId /= 10;
		if(tmpId == 0)
			break;
	}
	
	listStr.push_front('=');
	listStr.push_front('d');
	listStr.push_front('i');
	listStr.push_back('|');

	char* str = new char[listStr.size() + strlen("name=") + sizeof(m_sName)];
	memset(str, 0, listStr.size() + sizeof(m_sName));
	char* p = str;						//记录申请空间的地址
	
	list<char>::iterator itStr;
	for(itStr = listStr.begin(); itStr != listStr.end(); itStr ++, str ++)
	{
		*str = *itStr;
	}
	strcat(p, "name=");
	strcat(p, m_sName.c_str());

	m_pStrItem = p;
	delete p;
}
