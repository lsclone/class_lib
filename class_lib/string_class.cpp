#include "string_class.h"
#include <stdlib.h>
#include <assert.h>

string::string()
{
	m_pStr = (char *)malloc(1);
	assert(m_pStr != NULL);
	m_pStr[0] = '\0';
	m_len = 0;
}

string::string(char *str)
{
	assert(str != NULL);

	m_len = strlen(str);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, str, m_len);
	m_pStr[m_len] = '\0';
}

string::string(const char *str)
{
	assert(str != NULL);

	m_len = strlen(str);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, str, m_len);
	m_pStr[m_len] = '\0';
}

string::string(const string &src)
{
	assert(src.m_pStr != NULL);

	m_len = strlen(src.m_pStr);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, src.m_pStr, m_len);
	m_pStr[m_len] = '\0';
}

string::~string()
{
	assert(m_pStr != NULL);
	free(m_pStr);
}

const string& string::operator=(char *str)
{
	assert(str != NULL && m_pStr != NULL);
	free(m_pStr);
	m_pStr = NULL;

	m_len = strlen(str);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, str, m_len);
	m_pStr[m_len] = '\0';

	return *this;
}

const string& string::operator=(const char *str)
{
	assert(str != NULL && m_pStr != NULL);
	free(m_pStr);
	m_pStr = NULL;

	m_len = strlen(str);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, str, m_len);
	m_pStr[m_len] = '\0';

	return *this;
}

const string& string::operator=(const string &src)
{
	assert(src.m_pStr != NULL && m_pStr != NULL);
	free(m_pStr);
	m_pStr = NULL;

	m_len = strlen(src.m_pStr);
	m_pStr = (char *)malloc(m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr, src.m_pStr, m_len);
	m_pStr[m_len] = '\0';

	return *this;
}

const string& string::operator+=(char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int tmplen = strlen(str);
	m_pStr = (char *)realloc(m_pStr, m_len+tmplen+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr+m_len, str, tmplen);
	m_pStr[m_len+tmplen] = '\0';
	m_len += tmplen;

	return *this;
}

const string& string::operator+=(const char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int tmplen = strlen(str);
	m_pStr = (char *)realloc(m_pStr, m_len+tmplen+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr+m_len, str, tmplen);
	m_pStr[m_len+tmplen] = '\0';
	m_len += tmplen;

	return *this;
}

const string& string::operator+=(const string &src)
{
	assert(src.m_pStr != NULL && m_pStr != NULL);

	m_pStr = (char *)realloc(m_pStr, m_len+src.m_len+1);
	assert(m_pStr != NULL);
	memcpy(m_pStr+m_len, src.m_pStr, src.m_len);
	m_pStr[m_len+src.m_len] = '\0';
	m_len += src.m_len;

	return *this;
}

bool string::operator==(char *str)
{
	assert(str != NULL && m_pStr != NULL);

	if(m_len == strlen(str))
		if(0 == memcmp(str, m_pStr, m_len))
			return true;

	return false;
}

bool string::operator==(const char *str)
{
	assert(str != NULL && m_pStr != NULL);

	if(m_len == strlen(str))
		if(0 == memcmp(str, m_pStr, m_len))
			return true;

	return false;
}

bool string::operator==(const string &src)
{
	assert(src.m_pStr != NULL && m_pStr != NULL);

	if(m_len == src.m_len)
		if(0 == memcmp(src.m_pStr, m_pStr, m_len))
			return true;

	return false;
}

bool string::operator!=(char *str)
{
	return (!(*this == str));
}

bool string::operator!=(const char *str)
{
	return (!(*this == str));
}

bool string::operator!=(const string &src)
{
	return (!(*this == src));
}

bool string::operator>(char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int right_len = strlen(str);
	int rec = memcmp(m_pStr, str, m_len < right_len ? m_len : right_len);
	if(rec == 1)
		return true;
	else if(rec == -1)
		return false;
	else // rec == 0
		return m_len > right_len ? true : false;
}

bool string::operator>(const char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int right_len = strlen(str);
	int rec = memcmp(m_pStr, str, m_len < right_len ? m_len : right_len);
	if(rec == 1)
		return true;
	else if(rec == -1)
		return false;
	else // rec == 0
		return m_len > right_len ? true : false;
}

bool string::operator>(const string &src)
{
	assert(src.m_pStr != NULL && m_pStr != NULL);

	int rec = memcmp(m_pStr, src.m_pStr, m_len < src.m_len ? m_len : src.m_len);
	if(rec == 1)
		return true;
	else if(rec == -1)
		return false;
	else // rec == 0
		return m_len > src.m_len ? true : false;
}

bool string::operator<(char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int right_len = strlen(str);
	int rec = memcmp(m_pStr, str, m_len < right_len ? m_len : right_len);
	if(rec == 1)
		return false;
	else if(rec == -1)
		return true;
	else // rec == 0
		return m_len < right_len ? true : false;
}

bool string::operator<(const char *str)
{
	assert(str != NULL && m_pStr != NULL);

	int right_len = strlen(str);
	int rec = memcmp(m_pStr, str, m_len < right_len ? m_len : right_len);
	if(rec == 1)
		return false;
	else if(rec == -1)
		return true;
	else // rec == 0
		return m_len < right_len ? true : false;
}

bool string::operator<(const string &src)
{
	assert(src.m_pStr != NULL && m_pStr != NULL);

	int rec = memcmp(m_pStr, src.m_pStr, m_len < src.m_len ? m_len : src.m_len);
	if(rec == 1)
		return false;
	else if(rec == -1)
		return true;
	else // rec == 0
		return m_len < src.m_len ? true : false;
}

bool string::operator>=(char *str)
{
	return (!(*this < str));
}

bool string::operator>=(const char *str)
{
	return (!(*this < str));
}

bool string::operator>=(const string &src)
{
	return (!(*this < src));
}

bool string::operator<=(char *str)
{
	return (!(*this > str));
}

bool string::operator<=(const char *str)
{
	return (!(*this > str));
}

bool string::operator<=(const string &src)
{
	return (!(*this > src));
}

void string::clear()
{
	assert(m_pStr != NULL);

	m_pStr = (char *)realloc(m_pStr, 1);
	assert(m_pStr != NULL);
	m_pStr[0] = '\0';
	m_len = 0;
}
