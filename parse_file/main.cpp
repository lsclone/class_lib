#include "analysis.h"
#include "student_class.h"

void fileWrite();
typedef list<Student*> SList;
typedef list<Student*>::iterator SIt;


void addSep(const char* strSep, CharpList& sepList)
{
	int len = strlen(strSep);
	char* pTmpSep = new char[len+1];
	memcpy(pTmpSep, strSep, len);
	pTmpSep[len] = '\0';
	sepList.push_back(pTmpSep);
}

int main(int argc, char** argv)
{
	//fileWrite();
	string fileName = "./Student Information.txt";
	Analysis* analysis = new Analysis(fileName);
	DataList dataList;
	CharpList sepList;
	CharpIt tmpIt;
	addSep("id", sepList);
	addSep("name", sepList);
	addSep("age", sepList);

	analysis->getData('|', sepList, dataList);

	// process data
	DataIt tmpDataIt;
	for(tmpDataIt = dataList.begin(); tmpDataIt != dataList.end(); ++tmpDataIt)
	{
		if(*tmpDataIt)
		{
			int i;
			int nId;
			int nAge;
			string sName;
			
			for(i = 0, tmpIt = (*tmpDataIt)->begin(); i < (*tmpDataIt)->size() && tmpIt != (*tmpDataIt)->end(); ++tmpIt, ++i)
			{
				switch(i)
				{
				case 0:
					nId = atoi(*tmpIt);
					break;
				case 1:
					sName = *tmpIt;
					break;
				case 2:
					nAge = atoi(*tmpIt);
					break;
				default:
					break;
				}
			}

			Student stu(nId, nAge, sName);
			stu.display(3);
		}
	}

	for(tmpIt = sepList.begin(); tmpIt != sepList.end(); ++tmpIt)
		if(*tmpIt)
			delete []*tmpIt;

	for(tmpDataIt = dataList.begin(); tmpDataIt != dataList.end(); ++tmpDataIt)
	{
		if(*tmpDataIt)
		{
			for(tmpIt = (*tmpDataIt)->begin(); tmpIt != (*tmpDataIt)->end(); ++tmpIt)
				if(*tmpIt)
					delete []*tmpIt;
			delete *tmpDataIt;
		}
	}
	dataList.clear();


	delete analysis;
	return 0;
}

void fileWrite()
{
	FILE* fp = fopen("Student Information.txt", "a+b");
	if(NULL == fp)
	{
		printf("Failed to open file Student Information.\n");
		return;
	}
	printf("Succeed to open file Student Information.\n");

	SList listStu;

	Student *stu1 = new Student(101, 21, "s1");
	Student *stu2 = new Student(102, 22, "st2");
	Student *stu3 = new Student(103, 23, "stu3");

	listStu.push_back(stu1);
	listStu.push_back(stu2);
	listStu.push_back(stu3);

	/*
		将链表中的学生信息写入到文件。
	*/
	SIt itStu;
	size_t wSize = 0;

	fseek(fp, 0, SEEK_END);
	long fSize = ftell(fp);

	for(itStu = listStu.begin(); itStu != listStu.end(); itStu ++)
	{
		if(SEEK_SET != ftell(fp))
		{
			fwrite("\r\n", 1, 2, fp);
		}
		const char* sWrite = (*itStu)->getString();
		wSize = fwrite(sWrite, 1, strlen(sWrite), fp);
		printf("wSize = %d\n", wSize);
		if(wSize != strlen(sWrite))
		{
			printf("Failed to write *itStu to file!\n");
			break;
		}
	}

	fclose(fp);
#if 0
	delete stu1;
	delete stu2;
	delete stu3;
#else
	for(itStu = listStu.begin(); itStu != listStu.end(); ++itStu)
	{
		if(*itStu)
			delete *itStu;
	}
#endif
}