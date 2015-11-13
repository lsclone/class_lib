#include "Test.h"
#include <string.h>


Test::Test(char* data, int len)
{
	m_len = 0;
	m_str = NULL;

	if (data != NULL)
	{
		m_str = new char[len+1];
		memcpy(m_str, data, len);
		m_str[len] = '\0';
		m_len = len;
	}
}


Test::~Test()
{
	if (m_str != NULL)
		delete[] m_str;
}

void Test::releaseSelf()
{
	delete this;
}

