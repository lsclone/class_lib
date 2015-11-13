#pragma once
#include <string>
#include <iostream>
using namespace std;

class Student
{
public:
	Student(int nId = 100, int nAge = 20, string sName = "")
		:m_nId(nId), m_nAge(nAge), m_sName(sName) { toString(); }
	void setItem(int nId, int nAge, string sName);
	void display(int count) const;
	void toString();
	inline const char* getString() const { return m_pStrItem.c_str(); }

private:
	int m_nId;
	int m_nAge;
	string m_sName;
	string m_pStrItem;
};
