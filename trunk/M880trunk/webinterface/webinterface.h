#ifndef _Web_Interface
#define _Web_Interface

#include <dlfcn.h>

#include "../flashdb.h"
#include "../utils.h"
#include "../options.h"
#include "../exfun.h"

#include "webfirmware.h"

#ifdef _ZKWEBFUNC
#define EXTWI
#else
#define EXTWI extern
#endif

#define MSG_WEBINTERFACE ((int)1<<17)

typedef int HANDLER;
typedef HANDLER (*WSCB)(char*);
typedef HANDLER (*WICB)(void);
typedef void (*ConfigStart)(char *Name);
typedef void (*ConfigClose)(void);

typedef struct _tag_WebInterface
{
	WICB Start;
	WICB Run;
	WICB Close;
	WICB Event;
	WICB GetCB;
	WSCB GetFWCB;
	WSCB Load;
	WSCB Free;
	ConfigStart CfgInit;
	ConfigClose CfgFree;
	FWCBT* CB;
	void* dlhandler;
	struct _tag_WebInterface* this;
}
WebInterface;

EXTWI WebInterface* ZKWeb(WebInterface* wi);
EXTWI void ZKWebDebug(char *Note);

EXTWI WebInterface IWebServer; 

#define THIS &IWebServer

#define ZKWEBCALLBACK ZKWeb(THIS)->CB

#define _WEBDEBUG

#ifdef _ZKWEBFUNC

WebInterface* ZKWeb(WebInterface* wi)
{
	IWebServer.this=wi;
	return &IWebServer;
}

void ZKWebDebug(char *Note)
{
#ifdef _WEBDEBUG
        fprintf(stderr,"%s\n",Note);
#endif
}

HANDLER defStart(void)
{
	return 0;
}

HANDLER defRun(void)
{
	return 0;
}

HANDLER defClose(void)
{
	return 0;
}

HANDLER defEvent(void)
{
	return 0;
}


HANDLER defGetCB(void)
{
	return 0;
}

HANDLER defGetFWCB(char *Name)
{
	IWebServer.CB=(FWCBT*)IWebServer.GetCB();
	if(IWebServer.CB)
		return 1;
	return 0;
}

void defCfgInit(char *Name)
{
	        ;
}

void defCfgFree(void)
{
	        ;
}

HANDLER defLoad(char *Name)
{
	WICB func;
	ConfigStart cs;
	ConfigClose cc;

	if(Name)
	{
		if(IWebServer.dlhandler)
			dlclose(IWebServer.dlhandler);

		IWebServer.dlhandler=dlopen(Name,RTLD_LAZY);
		if(IWebServer.dlhandler)
		{
			func=(WICB)dlsym(IWebServer.dlhandler,"WebStart");
			if(func)
				IWebServer.Start=func;
			
			func=(WICB)dlsym(IWebServer.dlhandler,"WebClose");
			if(func)
				IWebServer.Close=func;

			func=(WICB)dlsym(IWebServer.dlhandler,"WebReady");
			if(func)
				IWebServer.Event=func;

			func=(WICB)dlsym(IWebServer.dlhandler,"WebProcess");
			if(func)
				IWebServer.Run=func;

			func=(WICB)dlsym(IWebServer.dlhandler,"WebGetCB");
			if(func)
			        IWebServer.GetCB=func;	

			cs=(ConfigStart)dlsym(IWebServer.dlhandler,"WebInitializingConfig");
			if(cs)
				IWebServer.CfgInit=cs;

			cc=(ConfigClose)dlsym(IWebServer.dlhandler,"WebFreeConfig");
			if(cc)
				IWebServer.CfgFree=cc;
		}
		IWebServer.CfgInit("webs.cfg");
	}
	return 1;
}

HANDLER defFree(char *Name)
{
	IWebServer.CfgFree();

	if(IWebServer.dlhandler)
		dlclose(IWebServer.dlhandler);

	return 1;
}

void WebInterfaceInit(void)
{
        IWebServer.Start=defStart;
	IWebServer.Run=defRun;
	IWebServer.Close=defClose;
	IWebServer.Event=defEvent;
	IWebServer.GetCB=defGetCB;
	IWebServer.GetFWCB=defGetFWCB;
	IWebServer.Load=defLoad;
	IWebServer.Free=defFree;
	IWebServer.CfgInit=defCfgInit;
	IWebServer.CfgFree=defCfgFree;
	IWebServer.CB=NULL;
	IWebServer.dlhandler=NULL;
	IWebServer.this=NULL;
}
#endif

#endif
