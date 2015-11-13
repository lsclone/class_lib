#pragma once

#include <stdio.h>
#include <string.h>

#ifdef CLASS_EXPORT
#define STRING_API __declspec(dllexport)
#elif defined(CLASS_IMPORT)
#define STRING_API __declspec(dllimport)
#else
#define STRING_API
#endif

class STRING_API string
{
public:
	string();
	string(char *str);
	string(const char *str);
	string(const string &src);
	~string();

	inline const char* c_str() { return m_pStr; }
	inline int size() { return m_len; }

	const string& operator=(char *str);
	const string& operator=(const char *str);
	const string& operator=(const string &src);
	const string& operator+=(char *str);
	const string& operator+=(const char *str);
	const string& operator+=(const string &src);

	bool operator==(char *str);
	bool operator==(const char *str);
	bool operator==(const string &src);
	bool operator!=(char *str);
	bool operator!=(const char *str);
	bool operator!=(const string &src);
	bool operator>(char *str);
	bool operator>(const char *str);
	bool operator>(const string &src);
	bool operator<(char *str);
	bool operator<(const char *str);
	bool operator<(const string &src);
	bool operator>=(char *str);
	bool operator>=(const char *str);
	bool operator>=(const string &src);
	bool operator<=(char *str);
	bool operator<=(const char *str);
	bool operator<=(const string &src);

	void clear();

private:
	char *m_pStr;
	int m_len;
};
