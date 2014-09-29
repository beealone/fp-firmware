#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "arca.h"
#include "strutils.h"

int upperChar(int ch)
{
	if(ch<'a') return ch;
	if(ch>'z') return ch;
	return ch-'a'+'A';
}

int extractStrs(const char *s, char *sp[], int maxSp)
{
	int i,j=0,k=0;
	if(s)
	for(i=0;i<maxSp;i++)
	{
		k=0;
		while(1)
		{
			char ch=s[j++];
			if(ch==0)
			{
				sp[i][k]=0;
				return i;
			}
			else if(ch==';')
			{
				sp[i][k]=0;
				break;
			}
			else
				sp[i][k++]=ch;
		}
	}
	return k;
}


#ifdef _WIN32
/*
   int strncasecmp(char *s1, char *s2, size_t n)
{
  if (n == 0)
    return 0;

  while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
      if (n == 0 || *s1 == '\0' || *s2 == '\0')
	break;
      s1++;
      s2++;
    }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
*/
#endif

int replaceInList(char *content, int contentLen, int *contentSize, PStrList tags)
{
	int count=tags->count;
	int lastC=0;
	int i=count-1;
	while(tags->count)
	{
		char tagTag[22];
		int pos, lenDiff, newLen, oldLen=21;
		char *newStr=slNameValueAt(tags, i, tagTag, &oldLen);
		tagTag[10]=0;
		pos=atoi(tagTag); oldLen=atoi(tagTag+11);
		newLen=strlen(newStr);
		lenDiff=newLen-oldLen;
		if(lenDiff>0)
		{
			if(*contentSize>=contentLen+lenDiff)
			{
				char *tail=content+contentLen+1;
				char *src=content+pos+oldLen;
				char *dst=content+pos+newLen;
				memmove(dst, src, tail-src);
				memmove(dst-newLen, newStr, newLen);
				contentLen+=lenDiff;
				slDeleteAt(tags, tags->count-1);
				i--;
			}
			else
			{
				i--;
				if(i<0) 
				{
					if(lastC==tags->count) break;
					i=tags->count-1;
					lastC=tags->count;
				}
			}
		}
		else
		{
			memcpy(content+pos, newStr, newLen);
			if(lenDiff<0)
			{
				lenDiff=0-lenDiff;
				deleteMemo(content, contentLen+1, pos+newLen, lenDiff);
				contentLen-=lenDiff;
			}
			slDeleteAt(tags, tags->count-1);
			i--;
		}
	}
	return *contentSize=contentLen;
}

int addReplacedTag(PStrList tags, int index, int size, char *newStr)
{
	char tagTag[22];
	sprintf(tagTag, "%10d %10d", index, size);
	return slSetValue(tags, tagTag, newStr);
}

int replaceTagProp(char *content, int *contentSize, PStrList repTable, PStrList props)
{
	char *contP=content;
	char *propP=NULL,
		*tagP=NULL;
	int count, inQuote=0;
	int matchTags[1024];
	int matchCount=0;
	PStrList tags=slCreate("\011\7\011");
	while(props->count>1024) slDeleteAt(props, props->count-1);
	while(1)
	{
		char ch=*contP++;
		if(ch==0) break;
		if(tagP==NULL)
		{
			if(ch=='<')
			{
				tagP=contP;
				matchCount=-1;
				inQuote=0;
			}
		}
		else if(propP==NULL)
		{
			if(inQuote==0 && ch==' ')
			{
				//check the if the TAG in the list
				if(matchCount==-1) //Not checked
				{
					int i,len=contP-tagP-1;
					matchCount=0;
					for(i=0;i<props->count;i++)
						if(strncasecmp(tagP, props->strs[i], len)==0)
							if(memcmp(props->strs[i]+len, props->valueSeparator, strlen(props->valueSeparator))==0)
								matchTags[matchCount++]=i;
				}
				if(matchCount)
					propP=contP;
			}
		}
		else if(ch=='>' || ch==' ')
		{
			if((inQuote==0) &&
				(matchCount>0)) //has matched tag, check if the attribute is matched;
			{
				int i;
				while(' '==*propP||'\r'==*propP||'\n'==*propP||9==*propP) 
					propP++;
				for(i=0;i<matchCount;i++)
				{
					char *value=slValueAt(props, matchTags[i]);
					int vlen=strlen(value);
					if(strncasecmp(value, propP, vlen)==0 && ('='==propP[vlen]))
					{// matched attribute name, check the attribute value
						int j;
						propP+=vlen+1;
						for(j=0;j<repTable->count;j++)
						{
							char *rep=repTable->strs[j];
							int nameLen=0;
							while(rep[nameLen]) if(rep[nameLen]=='=') break; else nameLen++;
							if(nameLen==0)
							{
								if(*rep=='=') //Just insert a string
								{
									addReplacedTag(tags, propP-content, 0, rep+1);
								}
							}
							else
							{
								int pati=0;
								char *p=propP;
								while(contP>=p-nameLen)
								{
									int ch=*p++;
									if(ch==0) break;
									if(upperChar(rep[pati])==upperChar(ch))
									{
										pati++;
										if(pati==nameLen) //get a match
										{
											addReplacedTag(tags, p-content-nameLen, nameLen, rep+nameLen+1);
											break;
										}
									}
									else
										pati=0;
								}
							}
						}
						propP=NULL;
					}
				}
				propP=contP;
			}
			else if(ch=='\"')
			{
				if(inQuote==0)
					inQuote=1;
				else
					inQuote=0;
			}
		}
		if(ch=='>')
		{
			tagP=NULL;
			propP=NULL;
		}
	}
	count=tags->count;
	if(tags->count)
		*contentSize=replaceInList(content, contP-content, contentSize, tags);
	else
		*contentSize=contP-content;
	if(tags->count)
		count=tags->count-count;
	slFree(tags);
	return count;
}

//Tag processing <#IMAGE Ref=Value>
//typedef char * (*TagReplacer)(char *tagName, char *tagProperty, void *param);
int replaceTag(char *content, int *contentSize, TagReplacer replacer, void *param)
{
	char *contP=content;
	char *propP=NULL,
		*tagP=NULL;
	char lastCh=0;
	int count;
	PStrList tags=slCreate("\011\7\011");
	while(1)
	{
		char ch=*contP++;
		if(ch==0) break;
		if(tagP==NULL)
		{
			if((ch=='#') && (lastCh='<') && ' '!=*contP && '<'==contP[-2])
				tagP=contP;
		}
		else
		{
			if((ch==' '||ch==','||ch==9) && propP==NULL)
				propP=contP;
			else if(ch=='>')
			{
				if(contP-tagP+4<MAX_TAG_SIZE)
				{
					int tagLen=propP>tagP?propP-tagP-1:contP-tagP-1;
					char tagName[MAX_TAG_SIZE], *text;
					char tagTag[24];
					memcpy(tagName, tagP, tagLen);
					tagName[tagLen]=0;
					if(propP)
					{
						char *prop=tagName+tagLen+1;
						memcpy(prop, propP, contP-propP-1);
						prop[contP-propP-1]=0;
						text=replacer(tagName, prop, param);
					}
					else
						text=replacer(tagName, NULL, param);
					sprintf(tagTag, "%10d %10d", tagP-content-2, contP-tagP+2);
					if(text)
						slSetValue(tags, tagTag, text);
					else
						slSetValue(tags, tagTag, "");
					if(text) FREE(text);
				}
				propP=NULL;
				tagP=NULL;
			}
		}
		lastCh=ch;
	}
	count=tags->count;
	if(tags->count)
		replaceInList(content, contP-content, contentSize, tags);
	if(tags->count)
		count=tags->count-count;
	slFree(tags);
	return count;
}

char *replaceStr(const char *oldStr, const char *newStr, char *content, int *contentSize, int *count)
{
	int oldLen=strlen(oldStr), 
		newLen=strlen(newStr),
		contentLen, posCount=0;
	int lenDiff=newLen-oldLen;
	int posList[2048];
	char *p;
	if(*count>2048) *count=2048;
	p=content;
	while(1)
	{
		p=strstr(p, oldStr);
		if(p==NULL) break;
		posList[posCount++]=p-content;
		if(posCount>=*count) 
			break;
		p+=oldLen;
	}
	
	if(posCount)
	{
		contentLen=strlen(content);
		*count=posCount;
		if(lenDiff>0)
		{
			int i;
			if(*contentSize>=contentLen+posCount*lenDiff+1)
			{
				char *tail=content+contentLen+1;
				for(i=posCount-1;i>=0; i--)
				{
					char *src=content+posList[i]+oldLen;
					char *dst=content+posList[i]+i*lenDiff+newLen;
					memmove(dst, src, tail-src);
					memmove(dst-newLen, newStr, newLen);
					tail=src-oldLen;
				}
				*contentSize=contentLen+posCount*lenDiff+1;
				return content;
			}
		}
		else
		{
			int i;
			for(i=0;i<posCount;i++) memcpy(content+posList[i], newStr, newLen);
			if(lenDiff<0)
			{
				lenDiff=0-lenDiff;
				contentLen+=1;
				for(i=posCount-1;i>=0;i--)
				{
					deleteMemo(content, contentLen, posList[i]+newLen, lenDiff);
					contentLen-=lenDiff;
				}
			}
			return content;
		}
	}
	return NULL;
}

char *deleteStr(char *content, int index, int count)
{
	char *p, *s=content;
	while(index--)
		if(*s++==0) return content;
	p=s;
	if(count)
		*p=0;
	while(count--)
		if(*s++==0)	return content;
	while(1)
	{
		char ch=*s++;
		*p++=ch;
		if(ch==0) return content;
	}
	return content;
}

char *insertStr(char *content, int index, const char *newStr)
{
	int len=strlen(newStr);
	char *p=insertChar(content, index, len, ' ');
	if(p)
	{
		memcpy(content+index, newStr, len);
	}
	return p;
}

char *insertChar(char *content, int index, int count, char ch)
{
	char *np, *p=content, *sp;
	while(index--)
		if(*p++==0) return NULL;
	sp=p;
	while(1) if(*p++==0) break;
	np=p+count;
	while(p>sp)
		*--np=*--p;
	while(count--)
		*sp++=ch;
	return content;
}

char *deleteMemo(char *s, int size, int index, int count)
{
	int len=size-index-count;
	if(index>=size) return NULL;
	if(len>0)
		memmove(s+index, s+index+count, len);
	return s;
}

char *insertMemo(char *s, int size, int index, int count)
{
	if(index>size) return NULL;
	memmove(s+index+count, s+index, count);
	return s;
}

char *EnvTagReplacer(char *tagName, char *tagProperty, void *param)
{
	PStrList strs=(PStrList)param;
	char *p, *v;
	printf("TAG: \"%s\", \"%s\"\n", tagName, tagProperty);
	if(param==NULL) return NULL;
	v=slGetValue(strs, tagName);
	if(v==NULL) return v;
	p=(char*)MALLOC(strlen(v)+10);
	sprintf(p,"%s", v);
	return p;
}

int strToIntN(const char *s, int n)
{
	char *p=(char*)s;
	int ret=0, neg=0;
	if(*s=='-') neg=1;
	while('0'>*p || '9'<*p) if(!*p++) return 0;
	while('0'<=*p && '9'>=*p && n--) 
	{
		ret=ret*10+(*p-'0');
		p++;
	}
	if(neg) ret=0-ret;
	return ret;
}

int encodeLine(const char *code, const char *source)
{
	char *p=(char*)source, *t=(char*)code;
	if(p)
	{
		while(1)
		{
			char ch=*p++;
			if(0==ch)
				break;
			else if('\\'==ch)
			{
				*t++='\\';
				*t='\\';
			}
			else if('\r'==ch)
			{
				*t++='\\';
				*t='r';
			}
			else if('\n'==ch)
			{
				*t++='\\';
				*t='n';
			}
			else if((char)255==ch)
			{
				*t++='\\';
				*t='E';
			}
			else
				*t=ch;
			t++;
		}
		*t=0;
		return t-code;
	}
	return 0;
}


int decodeLine(const char *code, const char *target)
{
	int ret=0;
	char lastCh=0;
	char *p=(char*)code, *t=(char*)target;
	if(t==NULL) t=(char*)code;
	while(1)
	{
		char ch=*p++;
		if(ch==0)
			break;
		if(lastCh=='\\')
		{
			if(ch=='\\')
				*t='\\';
			else if(ch=='0')
				*t=0;
			else if(ch=='r')
				*t='\r';
			else if(ch=='n')
				*t='\n';
			else if(ch=='E')
				*t=(char)255;
			else
			{
				*t++='\\';
				*t=ch;
				ret++;
			}
			t++;
			ret++;
			lastCh=0;
		}
		else if(ch!='\\')
		{
			*t++=ch;
			lastCh=ch;
			ret++;
		}
		else
			lastCh=ch;
	}
	*t=0;
	return ret;
}

void testRepHTML(void)
{
	PStrList strs=slCreate("");
	PStrList repTbl=slCreate("");
	PStrList tags=slCreate("");
	char *content;
	int size;
	slSetValue(repTbl,"\"en", "\"utf-8"); slSetValue(tags, "html","lang");
	//slSetValue(repTbl,"appendix", "APPDX"); slSetValue(tags, "div","class");
	slLoadFromFile(strs, "D:\\diveintopython-html-5.4_zh-ch\\html\\index.html");
	slLoadFromFile(strs, "c:\\index.html");
	content=slGetText(strs, "\n", &size);
	size+=3;
	content=(char*)REALLOC(content, size);
	replaceTagProp(content, &size, repTbl, tags);
	slSetText(strs, content, "\n");
	slSaveToFile(strs, "c:\\s.html");
	slFree(tags);
	slFree(repTbl);
	slFree(strs);
	FREE(content);
}

void testStrUtils(void)
{
	char *p, *p2;
	int size, count;
	PStrList strs=slCreate("");
	testRepHTML();
	slLoadFromFile(strs, "c:\\t1.txt");
	slDeleteName(strs, "_ACP_LIB");
	slDeleteName(strs, "_ACP_PATH");
	slDeleteName(strs, "PATH");
//	slDeleteName(strs, "USERPROFILE");
//	slDeleteName(strs, "_ACP_ATLPROV");
//	slDeleteName(strs, "_ACP_INCLUDE");
//	slDeleteName(strs, "CLASSPATH");
//	while(strs->count>5) slDeleteAt(strs, 5);
	p=slGetText(strs, "\n", &size);
	size+=1024;
	p=(char*)REALLOC(p, size);
	count=3000;
//	sprintf(p,"ADS");
	replaceStr("ADS","AADDSS", p, &size, &count);
	replaceStr("SS","", p, &size, &count);
	printf(p);
	sprintf(p,"<html><head>\n<title>Richard's Home Page </title>\n</head>\n"
		"<body>\r\n"
		"Name    :<#Name><br/>\n\n"
		"Sex     :<#Sex charset=utf8 color=red><br/>\n"
		"LIB     :<#LIB ><br/>\n"
		"LUA_INIT:<#LUA_INIT><br/>\n"
		"</body></html>\n");
	count=replaceTag(p, &size, EnvTagReplacer, strs);
	printf("---------------HTML, Tag Count=%d\n",count);
	printf(p);
	printf("Count=%d\n",count);
	p2=(char*)MALLOC(size+1024);
	count=encodeLine(p2, p);
	printf("---------------Encoded, Size=%d\n",count);
	printf(p2);
	FREE(p);
	size=decodeLine(p2, NULL);
	printf("---------------Decoded, Size=%d\n",size);
	printf(p2);
	FREE(p2);
	slFree(strs);
}

char str2Hex(char c1, char c2)
{
	if(c1>='A' && c1<='F') c1=c1-'A'+10; else if(c1>='a' && c1<='f') c1=c1-'a'+10; else c1-='0';
	if(c2>='A' && c2<='F') c2=c2-'A'+10; else if(c2>='a' && c2<='f') c2=c2-'a'+10; else c2-='0';
	return (char)((((int)c1)*16)+(int)c2);
}

char *decodeQueryStr(char *str)
{
	char c, index=0;
	char *ret=str, *p=str;
	if(str==NULL) return str;
	while(1)
	{
		c=*str++;
		if(c==0) break;
		if(c=='+') 
			*p++=' ';
		else 
		{
			if(index==1)
				index=2;
			else if(index==2)
			{
				*p++=str2Hex(str[-2], str[-1]);
				index=0;
			}
			else if(c=='%')
			{
				index=1;
			}
			else
				*p++=c;
		}
	}
	*p=0;
	return ret;
}

char *strimStr(char *str)
{
	char *ret=str;
	char *p=NULL;
	if(str==NULL) return NULL;
	while(' '==*str) str++;
	ret=str;
	while(1)
	{
		char c=*str++;
		if(c==0)
		{
			if(p) *p=0;
			break;
		}
		else if(' '==c || 13==c || 10==c || 9==c)
		{
			if(p==NULL)
				p=str-1;
		}
		else
			p=NULL;
	}
	return ret;
}

int EncodeObject(char *code, void *object, int size)
{
	char *p=object, *t=code;
	if(p)
	{
		int i;
		for(i=0;i<size;i++)
		{
			char ch=*p++;
			if(0==ch)
			{
				*t++='\\';
				*t++='0';
			}
			else
			if('\r'==ch)
			{
				*t++='\\';
				*t++='r';
			}
			else
			if('\n'==ch)
			{
				*t++='\\';
				*t++='r';
			}
			else
			if('\''==ch)
			{
				*t++='\\';
				*t++='\'';
			}
			else
			if('\"'==ch)
			{
				*t++='\\';
				*t++='\"';
			}
			else
			if('\\'==ch)
			{
				*t++='\\';
				*t++='\\';
			}
			else
				*t++=ch;
		}
		*t=0;
		return t-code;
	}
	return 0;
}

int DecodeObject(const char *code, void *object)
{
	int ret=0;
	char lastCh=0;
	char *p=(char*)code,*t=(char*)object;
	while(1)
	{
		char ch=*p++;
		if(ch==0)
			break;
		if(lastCh=='\\')
		{
			if(ch=='\\')
				*t='\\';
			else if(ch=='r')
				*t='\r';
			else if(ch=='n')
				*t='\n';
			else if(ch=='t')
				*t='\t';
			else if(ch=='\'')
				*t='\'';
			else if(ch=='\"')
				*t='\"';
			else if(ch=='0')
				*t=0;
			else
			{
				*t++='\\';
				*t=ch;
			}
			t++;
			lastCh=0;
		}
		else if(ch!='\\')
		{
			*t++=ch;
			lastCh=ch;
		}
		else
			lastCh=ch;
		ret++;
	}
	return ret;
}

//return <0 No the key
//>=0 length of the value
int extractValue(char* s, char *key, char sp, char *value, int maxlen)
{
	char *cookieStart=NULL, *p;
	char cookieExp[200];
	int nl;
	*value=0;
	if(s==NULL) return -1;
	nl=sprintf(cookieExp,"%c%s=", sp, key);
	if(strncmp(s,cookieExp+1,nl-1)==0)
		cookieStart=s+nl-1;
	else
	{
		cookieStart=strstr(s, cookieExp);
		if(cookieStart) cookieStart+=nl;
	}
	if(cookieStart==NULL)
	{
		*value=0;
		return -1;
	}
	p=value;
	nl=0;
	while(1)
	{
		int ch=*cookieStart++;
		if(ch==sp || ch==0) 
		{
			*p=0;
			return p-value;
		}
		*p++=ch;
		if(++nl>=maxlen)
			break;
	}
	return 0;
}


int extractValueInt(char* s, char *key, char sp, int defValue)
{
	char value[40];
	int len=extractValue(s, key, sp, value, sizeof(value));
	if(len<=0)
		return defValue;
	else
		return atoi(value);

}

