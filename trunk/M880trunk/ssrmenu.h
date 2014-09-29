/*
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */


#ifndef __CWX_GUI_MENU
#define __CWX_GUI_MENU

#include "ssrpub.h"
#include "exfun.h"

typedef void (*MENUBACKCALL)(HWND hWnd);
extern int gAlarmDelay;
typedef int CHANDLE;

#define MaxMenuItem 	8
#define MainMenuWidth 	55
#define MainMenuHeight 	55
#define MainMenuRow  	2
#define MainMenuCol  	4
#define MENUNAMELEN	30//24

typedef struct _tag_MenuList
{
	char Name[MENUNAMELEN];
	BITMAP Image;
	unsigned short HotKey;
	unsigned short Width,Height;
	MENUBACKCALL Proc;
}
MenuList;

typedef struct _tag_MainMenu
{
	PLOGFONT logfont;
	short active;
	MenuList* menu;
	short menucnt;
	BITMAP bmp_bkgnd;
	HDC dc;
}
MainMenu;

CHANDLE  Menu_Init(MainMenu* mm,unsigned short menuitem,char* bmp)
{
	if(!mm) {
		return 0;
	}
	if(bmp)	{
		if (LoadBitmap(HDC_SCREEN,&mm->bmp_bkgnd,GetBmpPath(bmp))) {
			return 0;
		}
	}
	if (gfont==NULL) {
		mm->logfont=CreateLogFont(NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12,0);
	} else	{
		mm->logfont=gfont;
	}
	mm->menucnt=0;
	mm->menu=(MenuList *)MALLOC(menuitem*sizeof(MenuList));
	if(mm->menu) {
		int i;

		mm->menucnt=menuitem;
		for (i=0;i<menuitem;i++) {//add by jazzy 2009.06.09  for init default value
			mm->menu[i].Image.bmBits=NULL;
			memset(mm->menu[i].Name,0,sizeof(mm->menu[i].Name));
		}
	}

	return mm->menucnt;
}

void Menu_Free(MainMenu* mm)
{
	int i;
	if(mm) {
		if(mm->menu&&mm->menucnt) {
			for (i=0;i<mm->menucnt;i++) {
				if (mm->menu[i].Image.bmBits!=NULL) {	//add by jazzy 2009.09 for if not used
					UnloadBitmap(&mm->menu[i].Image);
				}
			}
			FREE(mm->menu);
		}
		mm->menucnt=0;
		DeleteMemDC(mm->dc);
		UnloadBitmap(&mm->bmp_bkgnd);

		if (gfont==NULL) {
			DestroyLogFont(mm->logfont);
		}
	}
}


CHANDLE Menu_SetMenuItem(MainMenu* mm,unsigned short Width,unsigned short Height,short Active)
{
	int cnt;
	if(!mm&&!mm->menucnt) {
		return 0;
	}

	for(cnt=0;cnt<mm->menucnt;cnt++)
	{
		mm->menu[cnt].Width=Width;
		mm->menu[cnt].Height=Height;
	}
	mm->active=Active;
	return 1;
}

CHANDLE Menu_Create(MainMenu* mm,unsigned short idx,char* Name,char *Img,unsigned short Key,MENUBACKCALL Proc)
{
	//printf("idx %d,len:%d\n\n",idx,strlen(Name));
	if(!mm&&!mm->menucnt) {
		return 0;
	}
	if(idx>=mm->menucnt) {
		return 0;
	}
	if(Name) {
		strncpy(mm->menu[idx].Name,Name,MENUNAMELEN);
	} else {
		strcpy(mm->menu[idx].Name,"Name");
	}

	if(Img)	{
		if(LoadBitmap(HDC_SCREEN,&mm->menu[idx].Image,GetBmpPath(Img))) {
			return 0;
		}
	}
	if(Key) {
		mm->menu[idx].HotKey=Key;
	} else {
		mm->menu[idx].HotKey=0;
	}

	if(Proc) {
		mm->menu[idx].Proc=Proc;
	} else {
		mm->menu[idx].Proc=NULL;
	}
	return 1;
}

static void MainMenu_Show(HDC hdc,MainMenu* mm,int MaxMenuCnt)
{
	int r,c,idx;
	int Width,Height, tmpvalue =0;

	FillBoxWithBitmap(mm->dc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, &mm->bmp_bkgnd);

	SelectFont(mm->dc,mm->logfont);
	tmpvalue = SetBkMode(mm->dc,BM_TRANSPARENT);
	SetPenColor(mm->dc,0x0000ff00);
	SetBrushColor(mm->dc,0x009999ff);
	SetTextColor(mm->dc,PIXEL_lightwhite);

	idx=0;
	for(r=0;r<MainMenuRow;r++)
	{
		for(c=0;c<MainMenuCol;c++)
		{
			if(idx>=MaxMenuCnt) {
				break;
			}
			mm->menu[idx].Image.bmType=BMP_TYPE_COLORKEY;
			mm->menu[idx].Image.bmColorKey=GetPixelInBitmap(&mm->menu[idx].Image,0,0);
			Width=MainMenuWidth;
			Height=MainMenuHeight;
			if(mm->active==idx) {
				Rectangle(mm->dc,10+c*(MainMenuWidth+20+gOptions.MenuOffset),30+r*(MainMenuHeight+50),10+c*(MainMenuWidth+20+gOptions.MenuOffset)+MainMenuWidth+20+gOptions.MenuOffset,30+r*(MainMenuHeight+50)+MainMenuHeight+40);
				FillBox(mm->dc,10+c*(MainMenuWidth+20+gOptions.MenuOffset)+1,30+r*(MainMenuHeight+50)+1,MainMenuWidth+20+gOptions.MenuOffset-1,MainMenuHeight+40-1);
			}
			if (mm->menu[idx].Name) {
				if (&mm->menu[idx].Image!=NULL)
					FillBoxWithBitmap(mm->dc,17+c*(MainMenuWidth+20+gOptions.MenuOffset),30+r*(MainMenuHeight+50),MainMenuWidth,MainMenuHeight,&mm->menu[idx].Image);

				TabbedTextOut(mm->dc,17+c*(MainMenuWidth+20+gOptions.MenuOffset),35+r*(MainMenuHeight+50)+MainMenuHeight,mm->menu[idx].Name);
			}
			++idx;
		}
	}
	BitBlt(mm->dc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,hdc,0,0,0);
}

#endif
