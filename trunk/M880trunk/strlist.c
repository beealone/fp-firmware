#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strlist.h"
#include "strutils.h"
#include "arca.h"
static unsigned int hashpjw(const void *key) 
{
	unsigned char     *ptr=(unsigned char *)key;
	unsigned int       val=0;
	while (*ptr) {
	   int tmp;
	   val = (val << 4) + (*ptr);
	   if ((tmp = (val & 0xf0000000))!=0) {
		  val = val ^ (tmp >> 24);
		  val = val ^ tmp;
	   }
	   ptr++;
	}
	return val % (PRIME_TBLSIZ-1);
}

unsigned int simplhash(const void *key)
{
	unsigned char     *ptr=(unsigned char *)key;
	unsigned int       val=0;
	while(*ptr)
		val = 31 * val + *ptr++;
	return val % (PRIME_TBLSIZ-1);
}

PStrList slCreate(const char *valueSeparator)
{
	PStrList res=(PStrList)MALLOC(sizeof(TStrList));
	if(res)
	{
		res->capability=CAP_SIZE;
		res->allStrs=(char**)MALLOC(sizeof(char*)*res->capability);
		res->strs=(char**)MALLOC(sizeof(char*)*res->capability);
		res->nextStr=(int*)MALLOC(sizeof(int)*res->capability);
		res->nextName=(int*)MALLOC(sizeof(int)*res->capability);
		res->allCount=0;
		res->count=0;
		res->bufferCapability=CAP_SIZE*LINE_SIZE;
		res->buffer=(char*)MALLOC(res->bufferCapability);
		if(valueSeparator && *valueSeparator)
		{
			strncpy(res->valueSeparator, valueSeparator, sizeof(res->valueSeparator));
		}
		else
		{
			res->valueSeparator[0]='=';
			res->valueSeparator[1]=0;
		}
		res->size=0;
		res->sortIndex=NULL;
		slClear(res);
	}
	return res;
}

static int slIncCapability(PStrList strLst)
{
	int newCap=strLst->capability+CAP_SIZE;
	void *p;
	p=REALLOC(strLst->allStrs, sizeof(char*)*newCap);
	if(p==NULL)	return 0;
	strLst->allStrs=(char **)p;
	p=REALLOC(strLst->strs, sizeof(char*)*newCap);
	if(p==NULL)	return 0;
	strLst->strs=(char **)p;
	p=REALLOC(strLst->nextStr, sizeof(int)*newCap);
	if(p==NULL)	return 0;
	memset(((char*)p)+sizeof(int)*strLst->capability, 255, sizeof(int)*CAP_SIZE);
	strLst->nextStr=(int*)p;
	p=REALLOC(strLst->nextName, sizeof(int)*newCap);
	if(p==NULL)	return 0;
	memset(((char*)p)+sizeof(int)*strLst->capability, 255, sizeof(int)*CAP_SIZE);
	strLst->nextName=(int*)p;

	strLst->capability=newCap;
	return 1;
}

void slFree(PStrList strLst)
{
	if(strLst)
	{
		if(strLst->buffer) FREE(strLst->buffer);
		if(strLst->allStrs) FREE(strLst->allStrs);
		if(strLst->nextName) FREE(strLst->nextName);
		if(strLst->strs) FREE(strLst->strs);
		if(strLst->nextStr) FREE(strLst->nextStr);
		if(strLst->sortIndex) FREE(strLst->sortIndex);
		FREE(strLst);
	}
}

void slClear(PStrList strLst)
{
	if(strLst)
	{
		strLst->count=0;
		strLst->allCount=0;
		strLst->size=0;
		memset(strLst->nextName, 255, sizeof(int)*strLst->capability);
		memset(strLst->nextStr, 255, sizeof(int)*strLst->capability);
		memset(strLst->strHash, 255, sizeof(int)*PRIME_TBLSIZ);
		memset(strLst->nameHash, 255, sizeof(int)*PRIME_TBLSIZ);
		if(strLst->sortIndex)
		{
			FREE(strLst->sortIndex);
			strLst->sortIndex=NULL;
		}
	}
}

static int AddHashValue(int *hashTable, int *hashNext, unsigned int hashValue, int tableIndex)
{
	int index=hashTable[hashValue];
	int ret=1,oldIndex;
	if(tableIndex==255)
		tableIndex=255;
	if(index==-1)
		hashTable[hashValue]=tableIndex;
	else
	{
		while(1)
		{
			ret++;
			oldIndex=index;
			index=hashNext[index];
			if(index==-1)
			{
				hashNext[oldIndex]=tableIndex;
				break;
			}
		}
	}
//	printf("Hash Value=%d, Deep=%d\n", hashValue, ret);
	return ret;
}

int slAdd(PStrList strLst, const char *s)
{
	if(s)
	{
		unsigned char   *ptr=(unsigned char *)s;
		unsigned int     val=0, nameVal=0;
		unsigned char   *sep=(unsigned char *)strLst->valueSeparator;
		char			*buf=strLst->buffer+strLst->size;
		int cc=0, sepi=0;
		if(strLst->capability<=strLst->allCount)
			if(slIncCapability(strLst)==0)
				return -1;
		strLst->allStrs[strLst->allCount]=buf;
		strLst->strs[strLst->count]=buf;
		do
		{//Copy char, calc hash value
			int tmp;
			unsigned char ch=*ptr;
			if(strLst->size>=strLst->bufferCapability)
			{
				void *p;
				int i;
				tmp=strLst->bufferCapability+CAP_SIZE*LINE_SIZE;
				p=REALLOC(strLst->buffer, tmp);
				if(p==NULL) return -1;
				strLst->bufferCapability=tmp;
				tmp=(char*)p-strLst->buffer;
				for(i=0;i<=strLst->allCount;i++) strLst->allStrs[i]+=tmp;
				for(i=0;i<=strLst->count;i++) strLst->strs[i]+=tmp;
				buf+=tmp;
				strLst->buffer=p;
			}
			*buf++=ch;
			strLst->size++;
			if(ch==0) break;
			cc++;
			if(sepi>=0)
			{
				if(sep[sepi]==ch) 
				{
					if(sepi==0) nameVal=val;
					sepi++;
					if(sep[sepi]==0) //Name hash OK
					{
						AddHashValue(strLst->nameHash, strLst->nextName, nameVal % (PRIME_TBLSIZ-1), strLst->allCount);
						sepi=-1;
//						printf("Add Name[%d] At:%d\n",nameVal%(PRIME_TBLSIZ-1),cc);
					}
				}
				else
					sepi=0;
			}
			val = (val << 4) + ch;
			if (0!=(tmp = (val & 0xf0000000)))
			{
				val = val ^ (tmp >> 24);
				val = val ^ tmp;
			}
			ptr++;
		}while(1);
		AddHashValue(strLst->strHash, strLst->nextStr, val % (PRIME_TBLSIZ-1), strLst->allCount);
		strLst->allCount++;
		strLst->count++;
		if(strLst->sortIndex)
		{
			FREE(strLst->sortIndex);
			strLst->sortIndex=NULL;
		}
	}
	return strLst->count;
}

int slDelete(PStrList strLst, const char *s)
{
	int i=slIndexOf(strLst, s);
	if(i>=0)
	{
		if(strLst->sortIndex)
		{
			FREE(strLst->sortIndex);
			strLst->sortIndex=NULL;
		}
		return slDeleteAt(strLst, i);
	}
	return -1;
}

int slDeleteAt(PStrList strLst, int index)
{
	if(index>=0 && index<strLst->count)
	{
		int i;
		if(strLst->sortIndex)
		{
			FREE(strLst->sortIndex);
			strLst->sortIndex=NULL;
		}
		for(i=index;i<strLst->count-1;i++)
			strLst->strs[i]=strLst->strs[i+1];
		return --strLst->count;
	}
	return -1;
}

int slDeleteName(PStrList strLst, const char *strName)
{
	int index=slIndexOfName(strLst, strName);
	if(index>=0)
	{
		return slDeleteAt(strLst, index);
	}
	return -1;
}

char *slGetText(PStrList strLst, const char *lineSeparator, int *size)
{
	int i, bufSize;
	int sepLen=lineSeparator?strlen(lineSeparator):0;
	char *ret, *retp;
	char **strs=strLst->sortIndex?strLst->sortIndex:strLst->strs;
	if(strLst->count==0) return NULL;
	bufSize=strLst->size+(sepLen-1)*strLst->count+1;
	ret=(char*)MALLOC(bufSize); retp=ret;
	if(ret==NULL) return ret;
	for(i=0;i<strLst->count;i++)
	{
		char *p=strs[i];
		do
		{
			char ch=*p++;
			if(ch==0)
				break;
			*retp++=ch;
		}while(1);
//		if(*strs[i] || (i<strLst->count-1))
		{
			p=(char*)lineSeparator;
			do
			{
				char ch=*p++;
				if(ch==0)
					break;
				*retp++=ch;
			}while(1);
		}
	}
	*retp=0;
	if(bufSize!=retp-ret+1)
	{
		ret=REALLOC(ret, retp-ret+1);
		bufSize=retp-ret+1;
	}
	*size=bufSize;
	return ret;
}

#define MAXLINEC  1024

int slSetText(PStrList strLst, const char *s, const char *lineSeparator)
{
	char *p=(char*)s, *line, *lp;
	int linec=MAXLINEC,sepi=0;
	slClear(strLst);
	line=(char*)MALLOC(linec);
	lp=line;
	while(1)
	{
		char ch=*p++;
		if(lp-line>=linec)
		{
			line=(char*)REALLOC(line, linec+MAXLINEC);
			lp=line+linec;
			linec+=MAXLINEC;
		}
		*lp++=ch;
		if(ch==0) break;
		if(ch==lineSeparator[sepi])
		{
			sepi++;
			if(lineSeparator[sepi]==0)
			{
				lp-=sepi;
				*lp=0;
				slAdd(strLst, line);
//				printf("Add Line: %s\n", line);
				lp=line;
				sepi=0;
			}
		}
		else
			sepi=0;
	}
	if(lp>line)
		slAdd(strLst, line);
	FREE(line);
	return strLst->count;
}

int slSetLines(PStrList strLst, const char *s) //'\r', '\n', '\r\n' 
{
	char *p=(char*)s, *line, *lp;
	int linec=MAXLINEC;
	slClear(strLst);
	line=(char*)MALLOC(linec);
	lp=line;
	while(1)
	{
		char ch=*p++;
		if(lp-line>=linec)
		{
			line=(char*)REALLOC(line, linec+MAXLINEC);
			lp=line+linec;
			linec+=MAXLINEC;
		}
		*lp++=ch;
		if(ch==0) 
			break;
		else if(ch=='\r')
		{
			lp[-1]=0;
			slAdd(strLst, line);
			lp=line;
			if('\n'==*p) p++;
		}
		else if(ch=='\n')
		{
			lp[-1]=0;
			slAdd(strLst, line);
			lp=line;
			if('\r'==*p) p++;
		}
	}
	if(lp>line)
		slAdd(strLst, line);
	FREE(line);
	return strLst->count;
}

char *slGetValue(PStrList strLst, const char *strName)
{
	int index=slIndexOfName(strLst, strName);
	if(index<0) return NULL;
	return strLst->strs[index]+strlen(strName)+strlen(strLst->valueSeparator);
}

int slGetValueInt(PStrList strLst, const char *strName, int defValue)
{
	char *p=slGetValue(strLst, strName);
	if(p==NULL || 0==*p) return defValue;
	return atoi(p);
}

int slSetValue(PStrList strLst, const char *strName, const char *value)
{
	char *p=slGetValue(strLst, strName);
	if(p) 
	{
		if(strcmp(value, p)==0) 
			return strLst->count;
		slDeleteAt(strLst, slIndexOfName(strLst, strName));
	}
	if(value)
	{
		int ret;
		char *s;
		int size;
		size=strlen(value);
		s=(char*)MALLOC(size+strlen(strName)+strlen(strLst->valueSeparator)+10);
		ret=sprintf(s, "%s%s", strName, strLst->valueSeparator);
		memcpy(s+ret, value, size);
		s[ret+size]=0;
		ret=slAdd(strLst, s);
		FREE(s);
		return ret;
	}
	else
	{
		int ret;
		char *s;
		s=(char*)MALLOC(strlen(strName)+strlen(strLst->valueSeparator)+10);
		sprintf(s, "%s%s", strName, strLst->valueSeparator);
		ret=slAdd(strLst, s);
		FREE(s);
		return ret;
	}
}

int slSetValueInt(PStrList strLst, const char *strName, int value)
{
	char s[2048];
	int i=slGetValueInt(strLst, strName, value+1);
	if(i==value) return strLst->count;
	slDeleteAt(strLst, slIndexOfName(strLst, strName));
	sprintf(s, "%s%s%d", strName, strLst->valueSeparator, value);
	return slAdd(strLst, s);
}

int slIndexOfAllIndex(PStrList strLst, int index)
{
	int i;
	char *p=strLst->allStrs[index];
	if(index>=strLst->count) 
		i=strLst->count-1;
	else
		i=index;
	while(strLst->strs[i]>p) 
		if(i==0) 
			break; 
		else 
			i--;
	if(strLst->strs[i]==p) 
		return i;
	return -1;
}

int slIndexOf(PStrList strLst, const char *s)
{
	unsigned hashValue=hashpjw(s);
	int i=strLst->strHash[hashValue];
	while(i>=0)
	{
		if(strcmp(s,strLst->allStrs[i])==0) 
		{
			int ret=slIndexOfAllIndex(strLst, i);
			if(ret>=0)
				return i;
		}
		i=strLst->nextStr[i];
	}
	return -1;
}

int slIndexOfName(PStrList strLst, const char *strName)
{
	unsigned hashValue=hashpjw(strName);
	int i=strLst->nameHash[hashValue], nameLen=strlen(strName);
	while(i>=0)
	{
		if(strncmp(strName,strLst->allStrs[i],nameLen)==0)
		{
			int ret=slIndexOfAllIndex(strLst, i);
			if(ret>=0)
				return ret;
		}
		i=strLst->nextName[i];
	}
	return -1;
}

int slIndexOfAllName(PStrList strLst, const char *strName)
{
	unsigned hashValue=hashpjw(strName);
	int i=strLst->nameHash[hashValue], nameLen=strlen(strName);
	while(i>=0)
	{
		if(strncmp(strName,strLst->allStrs[i],nameLen)==0)
		{
			return i;
		}
		i=strLst->nextName[i];
	}
	return -1;
}

int slIndexOfAllNameNext(PStrList strLst, const char *strName, int index)
{
	int i=index, nameLen=strlen(strName);
	while(i>=0)
	{
		i=strLst->nextName[i];
		if(i<0 || strncmp(strName,strLst->allStrs[i], nameLen)==0)
			return i;
	}
	return -1;
}

char *slValueAt(PStrList strLst, int index)
{
	if(index>=0 && index<strLst->count)
	{
		char *p=strLst->strs[index];
		if(p)
		{
			int sepi=0;
			char *sep=strLst->valueSeparator;
			while(*p)
			{
				if(sep[sepi]==*p)
				{
					sepi++;
					if(sep[sepi]==0)
						return p+1;
				}
				else
					sepi=0;
				p++;
			}
		}
	}
	return NULL;
}

char *slNameValueAt(PStrList strLst, int index, char *strName, int *nameSize)
{
	if(index>=0 && index<strLst->count)
	{
		char *p=strLst->strs[index];
		if(p)
		{
			char *s=strstr(p, strLst->valueSeparator);
			if(s)
			{
				int len=*nameSize;
				if(len>s-p) len=s-p;
				strncpy(strName, p, len);
				strName[len]=0;
				*nameSize=s-p;
				return s+strlen(strLst->valueSeparator);
			}
		}
	}
	return NULL;
}


int slNameAt(PStrList strLst, int index, char *strName, int nameSize)
{
	if(index>=0 && index<strLst->count)
	{
		char *p=strLst->strs[index];
		if(p)
		{
			int sepi=0;
			char *sep=strLst->valueSeparator;
			char *ret=strName;
			while(*p)
			{
				*ret=*p;
				if(sep[sepi]==*p)
				{
					sepi++;
					if(sep[sepi]==0)
					{
						ret-=sepi-1;
						*ret=0;
						return ret-strName;
					}
				}
				else
					sepi=0;
				p++;
				ret++;
				nameSize--;
				if(nameSize==0)
					return -2;
			}
			*ret=0;
		}
		return 0;
	}
	return -1;
}

static int getStrIndex(char **strIndex, const char *s, int count)
{
	int i, p1, p2;
	p2 = count;
	p1 = 0;
	while (p1 < p2)
	{
		i = (p1 + p2) / 2;
		if(strcmp(strIndex[i],s)>0)
			p1 = i + 1;
		else
			p2 = i;
	}
	return p1;
}

static int getStrIndex2(char **strIndex, const char *s, int count)
{
	int i, p1, p2;
	p2 = count;
	p1 = 0;
	while (p1 < p2)
	{
		i = (p1 + p2) / 2;
		if(strcmp(s,strIndex[i])>0)
			p1 = i + 1;
		else
			p2 = i;
	}
	return p1;
}

int slSort(PStrList strLst, int order)
{
	int i;
	if(strLst->count==0) return 0;
	if(strLst->sortIndex) FREE(strLst->sortIndex);
	strLst->sortIndex=(char**)MALLOC(sizeof(char*)*strLst->count);
	for(i=0;i<strLst->count;i++)
	{
		int index,j;
		if(order==-1)
			index=getStrIndex(strLst->sortIndex, strLst->strs[i], i);
		else
			index=getStrIndex2(strLst->sortIndex, strLst->strs[i], i);
		for(j=i;j>index;j--)
			strLst->sortIndex[j]=strLst->sortIndex[j-1];
		strLst->sortIndex[index]=strLst->strs[i];
	}
	return strLst->count;
}

int slSaveToFile(PStrList strLst, const char *fileName)
{
	int i;
	FILE *f=fopen(fileName, "w");
	if(f)
	{
		char **strs=strLst->sortIndex?strLst->sortIndex:strLst->strs;
		for(i=0;i<strLst->count;i++)
		{
			char *p=strs[i];
			if(p)
			{
				fputs(p, f);
				if(i<strLst->count-1)
					fputs("\n", f);
			}
		}
		fclose(f);
		return i;
	}
	return -1;
}

#define MAXFILELINE 2048

int slLoadFromFile(PStrList strLst, const char *fileName)
{
	FILE *f=fopen(fileName, "r");
	slClear(strLst);
	if(f)
	{
		char line[MAXFILELINE];
		while(1)
		{
			int strLen;
			if(fgets(line, MAXFILELINE, f)==NULL)      // line 可能包含 '\n' 或 '\r\n'
			{
				if(feof(f)) break;
				line[MAXFILELINE-1]=0;
			}
			strLen=strlen(line)-1;
			if(line[strLen]=='\r') line[strLen--]=0;
			if(line[strLen]=='\n') line[strLen--]=0;
			if(line[strLen]=='\r') line[strLen--]=0;
			if(line[strLen]=='\n') line[strLen--]=0;
			slAdd(strLst, line);
		}
		fclose(f);
	}
	return strLst->count;
}

int slLoadFromRawMem(PStrList strLst, void *memory, int size)
{
	char *mem=memory, *p, *start;
	int i;
	slClear(strLst);
	p=mem;
	start=mem;
	for(i=0;i<size;i++)
	{
		if(*p==0)
		{
			slAdd(strLst, start);
			p++;
			start=p;
		}
		else
			p++;
	}
	if(p>start)
	{
		mem=(char*)MALLOC(p-start+1);
		memcpy(mem, start, p-start);
		mem[p-start]=0;
		slAdd(strLst, mem);
	}
	return strLst->count;
}

int slSaveToRawFile(PStrList strLst, const char *fileName)
{
	FILE *f=fopen(fileName, "wb");
	if(f)
	{
		fwrite(strLst->buffer, strLst->size, 1, f);
		fclose(f);
		return strLst->size;
	}
	return -1;
}

int slLoadFromRawFile(PStrList strLst, const char *fileName)
{
	FILE *f=fopen(fileName, "rb");
	if(f)
	{
		void *mem;
		size_t size;
		fseek(f,0,SEEK_END);
		size=ftell(f);
		if(size)
		{
			mem=MALLOC(size);
			fseek(f, 0, SEEK_SET);
			fread(mem, size, 1, f);
			slLoadFromRawMem(strLst, mem, size);
			FREE(mem);
			fclose(f);
			return strLst->count;
		}
		fclose(f);
		return 0;
	}
	return -1;		
}

PStrList slClone(PStrList strLst)
{
	PStrList ret=slCreate(strLst->valueSeparator);
	slCopy(ret, strLst);
	return ret;
}

int slCopy(PStrList strLstDst, PStrList strLstSrc)
{
	int i;
	char **strs=strLstSrc->sortIndex?strLstSrc->sortIndex:strLstSrc->strs;
	slClear(strLstDst);
	for(i=0;i<strLstSrc->count;i++)
		slAdd(strLstDst, strs[i]);
	return strLstDst->count;
}

int slAddStrings(PStrList strLst, char **strings)
{
	while(*strings)
	{		
		slAdd(strLst, *strings);
		strings++;
	}
	return strLst->count;
}


int slAddObject(PStrList strLst, const char *objName, void *object, int size)
{
	char *code=(char*)MALLOC(size*2+1);
	size=EncodeObject(code, object, size);
	if(size)
	{
		int ret=slSetValue(strLst, objName, code);
		FREE(code);
		return ret;
	}
	return 0;
}


int slGetObject(PStrList strLst, const char *objName, void *object)
{
	char *p=slGetValue(strLst, objName);
	if(p)
		return DecodeObject(p, object);
	return -1;
}

int slObjectAt(PStrList strLst, int index, void *object)
{
	char *p=slValueAt(strLst, index);
	if(p)
		return DecodeObject(p, object);
	return -1;	
}

int slPackNames(PStrList strLst)
{
	int i;
	PStrList opts=slCreate(strLst->valueSeparator);
	for(i=0;i<strLst->count; i++)
	{
		char name[2*1024];
		int len=2*1024;
		char *value=slNameValueAt(strLst, i, name, &len);
		if(value) slSetValue(opts, name, value);
	}
	slSort(opts, 1);
	slCopy(strLst, opts);
	slFree(opts);
	return strLst->count;
}

int slAppendFrom(PStrList strLst, PStrList addLst)
{
	int i;
	for(i=0;i<addLst->count;i++)
	{
		slAdd(strLst,addLst->strs[i]);
	}
	return strLst->count;
}

#ifdef TESTSTRLIST
int testStrList(char *envp[])
{
	int size,i, obj;
	char *p;
	PStrList old, strLst=slCreate("");
//	slAddStrings(strLst, envp);
	slAdd(strLst, "Name=Richard");
	slAdd(strLst, "Sex=Male");
	i=slAdd(strLst, "Name=ooooichard")-1;
	slAdd(strLst, "Address=192.168.1.201");
	slAdd(strLst, "System=Windows");
	slAdd(strLst, "Hardware=8051");
	slDeleteAt(strLst, i);
	slSetValue(strLst, "Name","Richard Chen");
	slAdd(strLst, "Name=Richard");
	slAdd(strLst, "=Hello!");
	slAdd(strLst, "Hi, I am Cathy.");
	slAdd(strLst, "==");
	slAdd(strLst, "Please=");
	i=slAddObject(strLst, "Obj", &p, sizeof(p))-1;

	printf("Name=%s\r\n", slGetValue(strLst, "Name"));
	printf("Sex=%s\r\n", slGetValue(strLst, "Sex"));
	printf("Address=%s\r\n", slGetValue(strLst, "Address"));
	printf("System=%s\r\n", slGetValue(strLst, "System"));
	printf("Hardware=%d\r\n", slGetValueInt(strLst, "Hardware", 800));
	printf("SName=%s\r\n", slGetValue(strLst, "SName"));
	i=slIndexOfName(strLst, "Obj");
	slObjectAt(strLst, i, &obj);
	printf("Obj=%d, p=%d\r\n", obj, (int)p);
	printf("All Names:\n");
	i=slIndexOfAllName(strLst, "Name");
	while(i>=0)
	{
		printf("\t%s\n", strLst->allStrs[i]);
		i=slIndexOfAllNameNext(strLst, "Name", i);
	}
	for(i=0;i<strLst->count;i++)
	{
		char name[1024]={0};
		char *value;
		size=slNameAt(strLst, i, name, 1024);
		if(size>0)
		{
			value=slValueAt(strLst, i);
			printf("I%d: %s(%d)=%s\n", i, name, size, value);
		}
		size=1024;
		value=slNameValueAt(strLst, i, name, &size);
		if(value)
			printf(" %d: %s(%d)=%s\n", i, name, size, value);
	}
	old=slClone(strLst);
	slSaveToRawFile(old, "c:\\t1.bin");
	slFree(old);
	slLoadFromRawFile(strLst, "c:\\t1.bin");
	p=slGetText(strLst, "\r\n\t", &size);
	if(p)
	{
		printf("Text=\r\n\t%s\r\n", p);
		slSetText(strLst, p, "\r\n\t");
		FREE(p);
		slDelete(strLst, "==");
		slSaveToFile(strLst, "c:\\t1.txt");
	}
	slLoadFromFile(strLst, "c:\\t1.txt");
	slSort(strLst, 1);
	slSaveToFile(strLst, "c:\\t2.txt");
	slSort(strLst, -1);
	slSaveToFile(strLst, "c:\\t3.txt");
	
	p=slGetText(strLst, "\n\t", &size);
	printf("Text=\r\n\t%s\r\n", p);
	FREE(p);
	slFree(strLst);
	return 0;
}
#endif
