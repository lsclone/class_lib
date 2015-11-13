#pragma once


class Test
{
public:
	Test(char* data, int len);
	~Test();
	void releaseSelf();

private:
	int m_len;
	char* m_str;
};

