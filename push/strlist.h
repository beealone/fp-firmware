#ifndef STRLIST_H__
#define STRLIST_H__

#define PRIME_TBLSIZ 2044
#define CAP_SIZE	256
#define LINE_SIZE	64

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _strList_{
	char **allStrs;
	char **strs;											//current string (without deleted string)
	char *buffer;
	int allCount;											//current string count(with deleted string)
	int count;												//current string count(without deleted string)
	int capability;											//current string capability
	int bufferCapability;									//current buffer size
	char valueSeparator[16];
	int size;
	int strHash[PRIME_TBLSIZ], nameHash[PRIME_TBLSIZ];		//Hash Table
	int *nextStr, *nextName;								//Next Chained Hash Node
	char **sortIndex;
}TStrList, *PStrList;

PStrList slCreate(const char *valueSeparator);
void slFree(PStrList strLst);
void slClear(PStrList strLst);
int slAdd(PStrList strLst, const char *s);
int slDeleteAt(PStrList strLst, int index);
int slDelete(PStrList strLst, const char *s);
int slDeleteName(PStrList strLst, const char *name);
char *slGetText(PStrList strLst, const char *lineSeparator, int *size);
int slSetText(PStrList strLst, const char *s, const char *lineSeparator);
int slSetLines(PStrList strlst, const char *s);
char *slGetValue(PStrList strLst, const char *strName);
int slGetValueInt(PStrList strLst, const char *strName, int defValue);
int slSetValue(PStrList strLst, const char *strName, const char *value);
int slSetValueInt(PStrList strLst, const char *strName, int value);
int slIndexOf(PStrList strLst, const char *s);
int slIndexOfName(PStrList strLst, const char *strName);
int slIndexOfAllName(PStrList strLst, const char *strName);
int slIndexOfAllNameNext(PStrList strLst, const char *strName, int index);
char *slValueAt(PStrList strLst, int index);
int slNameAt(PStrList strLst, int index, char *strName, int nameSize);
char *slNameValueAt(PStrList strLst, int index, char *strName, int *nameSize);
int slSort(PStrList strLst, int order);

int slSaveToFile(PStrList strLst, const char *fileName);
int slLoadFromFile(PStrList strLst, const char *fileName);
int slLoadFromRawMem(PStrList strLst, void *memory, int size);
int slSaveToRawFile(PStrList strLst, const char *fileName);
int slLoadFromRawFile(PStrList strLst, const char *fileName);

PStrList slClone(PStrList strLst);
int slCopy(PStrList strLstDst, PStrList strLstSrc);

int slAddObject(PStrList strLst, const char *objName, void *object, int size);
int slAddStrings(PStrList strLst, char **strings);
int slObjectAt(PStrList strLst, int index, void *object);
int slPackNames(PStrList strLst);
int slAppendFrom(PStrList strLst, PStrList addLst);

#ifdef  __cplusplus
}
#endif

#endif

