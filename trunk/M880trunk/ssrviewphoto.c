#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>  
#include <dirent.h>
#include "exfun.h"
#include "ssrcommon.h"
#include "ssrpub.h"
#include "flashdb.h"

BITMAP  vpup, vpdown;
int photoidx = 0;
int photocount = 0;
time_t startTime, endTime;
int viewmode;
int showall;
char PhotoUserPIN[30];

PPhotoList pPhotoList = NULL;		//首节点指针
static void ClearLinkList(void)
{
	PPhotoList pNode = NULL;

	if (pPhotoList != NULL)
	{
		while(pPhotoList->next != pPhotoList)
		{
			//从链表中摘除节点
			pNode = pPhotoList->next;
			pPhotoList->next = pNode->next;
			pNode->next->pre = pPhotoList;
			//释放节点	
			FREE(pNode);
		}

		FREE(pPhotoList);
		pPhotoList = NULL;
		pNode = NULL;
	}
}

char* DecodePhotoFileName(char* filename, int type)
{
	static char tpbuf[40]={0};
	int i = 0;
	int j = 0;

	memset(tpbuf, 0, sizeof(tpbuf));
	if (type==0)	//date
	{
		//while(filename[i] != '-' && filename[i]!='.')
		while((filename[i] != '-')&&(filename[i] != '\0'))
		{
			tpbuf[j] = filename[i];
			i++;
			j++;
			if (j==4 || j==7)
			{
				tpbuf[j++] = '.';
			}
			else if (j==10)
			{
				tpbuf[j++] = ' ';
			}
			else if (j==13 || j==16)
			{
				tpbuf[j++] = ':';
			}
		}
	}
	else if(type==1) 		//user pin2
	{
		while(filename[i] != '-')
		{
			i++;
		}

		i++;
		while(filename[i] != '\0')
		{
			tpbuf[j++] = filename[i++]; 
		}
	}
	else if (type==2) //get date yyyymmdd str;
	{
		memcpy(tpbuf, filename, 8);
		tpbuf[8] = '\0';
	}
	else
		;

	return (char*)tpbuf;
}

static void ShowUserPhoto(HWND hWnd)
{
	BITMAP photopic;
	char filePath[80]={0};
	char fileDate[30]={0};
	char userInfo[20]={0};
	//TTime fileTime;
	HDC hdc;
	RECT photo_current_rc = {10, 190, 240, 215};
	int tmpvalue =0;

	memset(filePath,0,80);
	memset(fileDate,0,30);
	memset(userInfo,0,20);

	if (viewmode==0)
		sprintf(filePath, "%s%s.jpg", GetCapturePath("capture/pass/"), pPhotoList->PhotoFile);
	else
		sprintf(filePath, "%s%s.jpg", GetCapturePath("capture/bad/"), pPhotoList->PhotoFile);

	//printf("ShouUserPhoto PhotoFileName:%s\n", filePath);

	hdc = GetClientDC(hWnd);
	if (!LoadBitmap(hdc, &photopic, filePath))
	{
#ifdef ZEM600
		if (gOptions.IsRotatePhoto)
		{
			if(gOptions.isRotateCamera)
			{
				FillBoxWithBitmap(hdc, 58, 12, gOptions.LCDHeight*0.6, gOptions.LCDWidth*0.6, &photopic);
			}
			else
			{
				RotateScaledBitmapVFlip(hdc, &photopic, 5, -65, 90*64, 180, gOptions.LCDHeight);
			}
		}
		else
		{
			if(gOptions.isRotateCamera)
			{
				if(photopic.bmWidth == gOptions.LCDWidth && photopic.bmHeight==gOptions.LCDHeight)
				{
					if(gOptions.EuropeDevice)
					{
						RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, -90*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
					}
					else if(gOptions.RotateDev)
					{
						RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, 270*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
					}
					else
					{
						RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, 90*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
					}
				}
				else
				{
					FillBoxWithBitmap(hdc, 5, 5, gOptions.LCDHeight, 180, &photopic);
				}
			}
			else
			{
				FillBoxWithBitmap(hdc, 5, 5, gOptions.LCDHeight, 180, &photopic);
			}
		}
#else
		if(gOptions.isRotateCamera)
		{
			if(photopic.bmWidth == gOptions.LCDWidth && photopic.bmHeight==gOptions.LCDHeight)
			{
				if(gOptions.EuropeDevice)
				{
					RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, -90*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
				}
				else if(gOptions.RotateDev)
				{
					RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, 270*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
				}
				else
				{
					RotateScaledBitmapVFlip(hdc, &photopic, -30, -12, 90*64, gOptions.LCDWidth*0.6, gOptions.LCDHeight*0.6);
				}
			}
			else
			{
				FillBoxWithBitmap(hdc, 5, 5, gOptions.LCDHeight, 180, &photopic);
			}
		}
		else
		{
			FillBoxWithBitmap(hdc, 5, 5, gOptions.LCDHeight, 180, &photopic);
		}
#endif

		UnloadBitmap(&photopic);


		tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,PIXEL_red);
		if (viewmode == 0)
		{
			memset(userInfo, 0, sizeof(userInfo));
			sprintf(userInfo, "%s: %s", LoadStrByID(MID_ACNO), DecodePhotoFileName(pPhotoList->PhotoFile, 1));
			TextOut(hdc, 20, 10, userInfo);
		} 
		sprintf(fileDate, "%s", DecodePhotoFileName(pPhotoList->PhotoFile, 0));
		TextOut(hdc, 20, 165, fileDate);
	}

	SetTextColor(hdc, PIXEL_lightwhite);
	sprintf(fileDate, "%d/%d", photoidx+1, photocount);
	InvalidateRect(hWnd, &photo_current_rc, TRUE);
	TextOut(hdc, 20, 193, fileDate); 
	ReleaseDC(hdc);

}

static int ViewPhotoWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int tmpvalue = 0;
	static char keyupFlag=0;

	switch (message) 
	{
		case MSG_CREATE:
			//if (LoadBitmap(HDC_SCREEN, &vpbkg, GetBmpPath("submenubg.jpg")))
			//        return 0;
			if (LoadBitmap(HDC_SCREEN, &vpup, GetBmpPath("up.gif")))
				return 0;
			if (LoadBitmap(HDC_SCREEN, &vpdown, GetBmpPath("down.gif")))
				return 0;

			UpdateWindow(hWnd, TRUE);
			ShowUserPhoto(hWnd);
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*)lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				if(hdc == 0)
				{
					hdc = GetClientDC(hWnd);
					fGetDC = TRUE;
				}

				if(clip)
				{
					rcTemp = *clip;
					ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hWnd,&rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, 0, 0, get_submenubg_jgp());
				if(fGetDC) ReleaseDC (hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
			SetTextColor(hdc, PIXEL_lightwhite);

			TextOut(hdc, 250, 130, LoadStrByID(HIT_UPIC4));
			FillBoxWithBitmap(hdc, 298, 130, 15, 15, &vpup);
			TextOut(hdc, 250, 160, LoadStrByID(HIT_UPIC5));
			FillBoxWithBitmap (hdc, 299, 160, 15, 15, &vpdown);
			TextOut(hdc, 240, 193, LoadStrByID(MID_EXIT));

			EndPaint(hWnd,hdc);
			return 0;

		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}

			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
			{
				pPhotoList=pPhotoList->next;
				if (++photoidx > photocount-1)
					photoidx = 0;
				ShowUserPhoto(hWnd);	
				return 0;
			}
			if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
			{
				pPhotoList=pPhotoList->pre;
				if (--photoidx < 0)
					photoidx = photocount-1;
				ShowUserPhoto(hWnd);
				return 0;
			}
			if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				SendMessage(hWnd, MSG_CLOSE, 0, 0);
				return 0;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&vpbkg);
			UnloadBitmap(&vpup);
			UnloadBitmap(&vpdown);
			ClearLinkList();
			//	printf("clear link list success!\n");
			//MainWindowCleanup(hWnd);
			DestroyMainWindow(hWnd);
			return 0;

	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

extern HWND createStatusWin1 (HWND hParentWnd, int width, int height,char* title, char* text, ...);
extern void destroyStatusWin1(HWND hwnd);
int ViewPhotoWindow(HWND hWnd, char* userPIN, time_t sttime, time_t edtime, int mode) 
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	char capstr[30];
	HWND statushWnd;

	viewmode = mode;
	statushWnd = createStatusWin1(hWnd, 250, 50, LoadStrByID(MID_APPNAME), LoadStrByID(MID_PHOTO_HINT2));
	busyflag = 1;
	pPhotoList = FDB_GetPhotoToList(sttime, edtime, userPIN, 1, &photocount, mode);
	SetMenuTimeOut(time(NULL));
	photoidx = 0;
	destroyStatusWin1(statushWnd);

	//no photo
	if (photocount==0) return 0;
	memset(capstr ,0,sizeof(capstr));
	if (mode==0)
		sprintf(capstr, "%s", LoadStrByID(MID_QUERY_SET));
	else
		sprintf(capstr, "%s", LoadStrByID(MID_QUERY_SET2));

	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = capstr;
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ViewPhotoWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hWnd;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg, hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	return 1;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
