/*
 * ** $Id: gridview.c,v 1.3.6.2 2006/06/28 07:21:40 xwyan Exp $
 * **
 * ** gridview.c: Sample program for MiniGUI Programming Guide
 * **      Usage of GRID control.
 * **
 * ** Copyright (C) 2004 ~ 2006 Feynman Software.
 * **
 * ** License: GPL
 * */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include "ssrcommon.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

#define ID_MENU 55
#define ID_PROGBAR 431

enum {
	IDC_GRIDVIEW,
	IDC_LISTVIEW,
};

static HWND hGVWnd;
static int g_colcount;
static int g_rowcount=0;
static char mybuff[24];

BITMAP mgridbmp1;
BITMAP mgridbmp2;
BITMAP mgridbmp3;
BITMAP mgridbmp4;
BITMAP mgridbmp0;
BITMAP mgridbmp5;
BITMAP mgridbmp6;
BITMAP mgridbmp7;
BITMAP mgridbmp8;
BITMAP mgridbmp9;
BITMAP mgridbmp10;
BITMAP mgridbmp11;
BITMAP mgridbmp12;
BITMAP mgridbmp13;

PLOGFONT gridfont;

extern int userdatacuroffset_s;
extern int userdatacuroffset_e;
extern PUserlb ppuserlb;
extern int userrowindex;

#ifdef FACE
static BITMAP viewface;
#endif

static void SetListViewData(HWND mainwnd,int itemindex,int uppass,int privilege ,char **userdata);
static int FindUserItem(HWND listwnd,char *fddata);

void FreePUserlb(PUserlb tmpuserlb)
{
	if (tmpuserlb != NULL)
        {
        	PUserlb tmptmpuserlb;
		tmptmpuserlb = tmpuserlb;
		while (tmptmpuserlb != NULL)
		{
			tmpuserlb = tmptmpuserlb;
			tmptmpuserlb = tmpuserlb->next;
			free(tmpuserlb);
		}
	}
}

void RefreshGridview(int directionFlag)
{

	int j=0;
	char *(userinfo[6]);
	char fpnumchar[4];
	unsigned int idnumber=0;
	char mynamename[100];
#ifdef FACE 
	int faceid=0;
#endif
	int usercnt;

	PUserlb tmpuserlb, puserlb, userlb1,pb1;
	tmpuserlb = NULL;
	puserlb = NULL;
	userlb1 = NULL;
	pb1 = NULL;
	//puserlb = GetUsermestolb(0);
	if(directionFlag == DIRECTINON_FLAG_FORWARD){
		puserlb = GetUsermestolb_PageForward(0, &usercnt, 0);//forward
	}
	else {
		puserlb = GetUsermestolb_PageBackward(0, &usercnt, 0);//backward
	}
	if(puserlb == NULL){
		return;
	}
	SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
	tmpuserlb = puserlb;
	j=0;

#ifdef FACE 
	if(gOptions.FaceFunOn)
		GetUserFaceInfoForListview(puserlb);
#endif

    SendMessage (hGVWnd, MSG_FREEZECTRL, TRUE, 0);

	if((directionFlag == DIRECTINON_FLAG_BACKWARD) && (gOptions.PIN2Width > 5)){
		while(puserlb != NULL)
		{
			//printf("pin2=%s\n",puserlb->userlb.PIN2);
			pb1 = (PUserlb)malloc(sizeof(TUserlb));
			memset(pb1, 0, sizeof(TUserlb));
			memcpy(&pb1->userlb, &puserlb->userlb, sizeof(TUser));
			pb1->next = userlb1;
			userlb1 = pb1;
			puserlb = puserlb->next;
		}
		FreePUserlb(tmpuserlb);
		puserlb = userlb1;
		tmpuserlb = puserlb;
	}

	while (puserlb != NULL)
    {
		userinfo[0]=NULL;
		userinfo[1]=NULL;
		userinfo[2]=NULL;
		userinfo[3]=NULL;
		userinfo[4]=NULL;
		userinfo[5]=NULL;

        if ((strlen(puserlb->userlb.PIN2)==0) ||(strcmp(puserlb->userlb.PIN2,"0")==0) ||
            (strlen(puserlb->userlb.PIN2)>gOptions.PIN2Width))
        {
            int k;
            k=FDB_DelUser(puserlb->userlb.PIN);
            FDB_AddOPLog(ADMINPIN, OP_DEL_USER, puserlb->userlb.PIN,k,0,0);
            sync();
            puserlb = puserlb->next;
            continue;
        }
		userinfo[0]=puserlb->userlb.PIN2;

		memset(mynamename,0,100);
       	Str2UTF8(tftlocaleid,(unsigned char *)puserlb->userlb.Name,(unsigned char *)mynamename);
//		printf("mynamename:%s\t%d\n",mynamename,mynamename[0]);
		if (mynamename[0])
			userinfo[1]=mynamename;

		if(!gOptions.IsOnlyRFMachine)
		{
			memset(fpnumchar,0,4);
			sprintf(fpnumchar,"%d",FDB_GetTmpCnt(puserlb->userlb.PIN));
			userinfo[2]=fpnumchar;
			if (puserlb->userlb.Password[0])
				userinfo[3]="1";
			else
				userinfo[3]=NULL;
			if(gOptions.RFCardFunOn)
			{
				memcpy(&idnumber,puserlb->userlb.Card,4);
				if(idnumber)
					userinfo[4]="1";
				else
					userinfo[4]=NULL;
#ifdef FACE
				faceid=5;
#endif
			}
#ifdef FACE
			else
				faceid=4;
			if(gOptions.FaceFunOn) //add by caona for face
			{
				if(puserlb->userlb.Password[2])
					userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
			}
#endif
		}
		else
		{
                        if (puserlb->userlb.Password[0])
                                userinfo[2]="1";
                        else
                                userinfo[2]=NULL;
                        if(gOptions.RFCardFunOn)
                        {
                                memcpy(&idnumber,puserlb->userlb.Card,4);
                                if(idnumber)
                                        userinfo[3]="1";
                                else
                                        userinfo[3]=NULL;
#ifdef FACE
				faceid=4;
#endif
                        }
#ifdef FACE
			else
				faceid=3;
                        if(gOptions.FaceFunOn) //add by caona for face
                        {
                                if(puserlb->userlb.Password[2])
                                        userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
                        }
#endif
		}
		
		
		SetListViewData(hGVWnd,j,0,puserlb->userlb.Privilege,userinfo);
               	puserlb = puserlb->next;
		j++;

	}
	SendMessage (hGVWnd, MSG_FREEZECTRL, FALSE, 0);
	g_rowcount = SendMessage(hGVWnd,LVM_GETITEMCOUNT,0,0);
	FreePUserlb(tmpuserlb);
	if(directionFlag == DIRECTINON_FLAG_FORWARD){
		SendMessage(hGVWnd,LVM_CHOOSEITEM,0,0);
	} else {
		SendMessage(hGVWnd,LVM_CHOOSEITEM,(GRIDVIEWROWCNT-1),0);
	}
}

static void CreateListView(HWND mainwnd)
{
	int i=0,j;
	LVCOLUMN lvcol;
	char* (colnames[6]);

	j=gOptions.RFCardFunOn;
	if(!gOptions.IsOnlyRFMachine)
		g_colcount = (j)?5:4;
	else
		g_colcount = (j)?4:3;

	colnames[i++]=LoadStrByID(MID_ACNO);
	colnames[i++]=LoadStrByID(MID_NAME);
	if(!gOptions.IsOnlyRFMachine)
		colnames[i++]=LoadStrByID(MID_FINGER);
	colnames[i++]=LoadStrByID(MID_PWD);
	if(j)
		colnames[i++]=LoadStrByID(MID_CARD);

#ifdef FACE
	if(gOptions.FaceFunOn)
	{
		g_colcount++;
		colnames[i++]=LoadStrByID(MID_FACE);
	}
#endif

	hGVWnd=CreateWindowEx(CTRL_LISTVIEW,"user View",WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER,WS_EX_NONE,IDC_LISTVIEW,0,0,gOptions.LCDWidth,gOptions.LCDHeight-25,mainwnd,0);

	SendMessage(hGVWnd,LVM_SETHEADHEIGHT,20,0);
	SendMessage(hGVWnd,SVM_SETSCROLLPAGEVAL,0,192);

	for (i=0;i<g_colcount;i++)
	{
		lvcol.nCols=i;
		lvcol.pszHeadText = colnames[i];
		if(!gOptions.IsOnlyRFMachine)
		{
#ifdef FACE
			if(gOptions.FaceFunOn) //add by caona for face
			{
				switch(i)
				{
					case 0:
						lvcol.width=(j)?(90+gOptions.GridWidth):(95+gOptions.GridWidth);
						break;
					case 1:
						lvcol.width=(j)?(80+gOptions.GridWidth/2):(90+gOptions.GridWidth/2);
						break;
					case 2:
						lvcol.width=(j)?(35+gOptions.GridWidth):(40+gOptions.GridWidth);
						break;
					case 3:
						lvcol.width=(j)?(35+gOptions.GridWidth):(40+gOptions.GridWidth);
						break;
					case 4:
						lvcol.width=(j)?36:48;
						break;
					case 5:
						lvcol.width=36;
						break;
				}
			}
			else
#endif
			{

				switch(i)
				{
					case 0:
						lvcol.width=(j)?100+gOptions.GridWidth:110+gOptions.GridWidth;
						break;
					case 1:
						lvcol.width=(j)?95+gOptions.GridWidth/2:105+gOptions.GridWidth/2;
						break;
					case 2:
						lvcol.width=(j)?38+gOptions.GridWidth:49+gOptions.GridWidth;
						break;
					case 3:
						lvcol.width=(j)?40+gOptions.GridWidth:49+gOptions.GridWidth;
						break;
					case 4:
						lvcol.width=40;
						break;
				}
			}
		}
		else
		{
#ifdef FACE
			if(gOptions.FaceFunOn) //add by caona for face
			{
				switch(i)
				{
					case 0:
						lvcol.width=(j)?(98+gOptions.GridWidth):(110+gOptions.GridWidth);
						break;
					case 1:
						lvcol.width=(j)?(92+gOptions.GridWidth):(110+gOptions.GridWidth);
						break;
					case 2:
						lvcol.width=(j)?(38+gOptions.GridWidth):(38+gOptions.GridWidth);
						break;
					case 3:
						lvcol.width=38+gOptions.GridWidth;
						break;
					case 4:
						lvcol.width=38+gOptions.GridWidth;
						break;
				}
			}
			else
#endif
			{
				switch(i)
				{
					case 0:
						lvcol.width=(j)?110+gOptions.GridWidth:130+gOptions.GridWidth;
						break;
					case 1:
						lvcol.width=(j)?105+gOptions.GridWidth:125+gOptions.GridWidth;
						break;
					case 2:
						lvcol.width=(j)?48+gOptions.GridWidth:58+gOptions.GridWidth;
						break;
					case 3:
						lvcol.width=50+gOptions.GridWidth;
						break;
				}
			}
		}
		lvcol.pfnCompare = NULL;
		lvcol.colFlags = 0;
		SendMessage (hGVWnd, LVM_ADDCOLUMN, 0, (LPARAM) &lvcol);
	}
}

static void SetListViewData(HWND mainwnd,int itemindex,int uppass,int privilege ,char **userdata)
{

	LVITEM item;
	LVSUBITEM subdata;
	HLVITEM item1;
	int i;

	item.nItemHeight = 24;
	if (!uppass)
	{
		item.nItem=itemindex;
		item1 = SendMessage (mainwnd, LVM_ADDITEM, 0, (LPARAM)&item);
	}
	else
	{
		item1=SendMessage(mainwnd,LVM_GETSELECTEDITEM,0,0);
		SendMessage(mainwnd,LVM_GETITEM,item1,(LPARAM)&item);
	}
	for (i=0;i<g_colcount;i++)
	{
		subdata.nItem = item.nItem;
		subdata.subItem = i;
		subdata.pszText = userdata[i];
		subdata.nTextColor = 0;
		if(i==0)
		{
			if(privilege>0)
			{
				subdata.flags|=LVFLAG_BITMAP;
				subdata.image=(DWORD)&mgridbmp0;
			}
			else
			{
				subdata.flags=0;
				subdata.image=0;
			}
		}
		else if(i==1)
		{
			subdata.flags=0;
			subdata.image=0;
		}
		else if(i==2)
		{
			if(!gOptions.IsOnlyRFMachine)
			{
				subdata.flags=0;
				subdata.image=0;
			}
			else
			{
				subdata.flags|=LVFLAG_BITMAP;
				subdata.pszText=NULL;
				if(userdata[i]==NULL)
					subdata.image=0;
				else
					subdata.image=(DWORD)&mgridbmp5;
			}
		}
		else if(i==3)
		{
			subdata.flags|=LVFLAG_BITMAP;
			subdata.pszText=NULL;
			if (userdata[i]==NULL)
				subdata.image=0;
			else
			{
				if(!gOptions.IsOnlyRFMachine)
					subdata.image=(DWORD)&mgridbmp5;
				else
				{
#ifdef FACE
					if(gOptions.FaceFunOn && !gOptions.RFCardFunOn)
					{
						subdata.image = (DWORD)&viewface;
					}
					else
#endif
					{
						subdata.image = (DWORD)&mgridbmp13;
					}
				}
			}
		}
		else if(i==4)
		{
#ifdef FACE
			if(gOptions.FaceFunOn && !gOptions.RFCardFunOn)
			{
				subdata.flags|=LVFLAG_BITMAP;
				subdata.pszText=NULL;
				if (userdata[i]==NULL)
					subdata.image=0;
				else
					subdata.image = (DWORD)&viewface;
			}
			else if(gOptions.RFCardFunOn && (!gOptions.IsOnlyRFMachine))
#endif
			{
				subdata.flags|=LVFLAG_BITMAP;
				subdata.pszText=NULL;
				if (userdata[i]==NULL)
					subdata.image=0;
				else
					subdata.image = (DWORD)&mgridbmp13;
			}
		}
#ifdef FACE
		else if(i==5 && gOptions.FaceFunOn) //add by caona for face
		{
			subdata.flags|=LVFLAG_BITMAP;
			subdata.pszText=NULL;
			if (userdata[i]==NULL)
				subdata.image=0;
			else
				subdata.image = (DWORD)&viewface; //idcard
		}
#endif

		SendMessage (mainwnd, LVM_SETSUBITEM, item1, (LPARAM) &subdata);
	}
}

int processscrollview_UserGridView(HWND listwnd, int down,int incseed)
{
	int additemindex;
	additemindex = SendMessage(listwnd,SVM_GETCURSEL,0,0);
	if (additemindex<0) {
		SendMessage(listwnd,LVM_CHOOSEITEM,0,0);
		return 0;
	}
	
	if(down == DIRECTINON_FLAG_FORWARD){
		if(additemindex >= (GRIDVIEWROWCNT-1)) {
			RefreshGridview(DIRECTINON_FLAG_FORWARD);
			return 1;
		}
	} else {
		if(additemindex <= 0) {
			RefreshGridview(DIRECTINON_FLAG_BACKWARD);
			return 1;
		}
	}
	
	if (down) {
		additemindex--;
		SendMessage(listwnd,LVM_CHOOSEITEM,additemindex,0);
	} else {
		additemindex++;
		SendMessage(listwnd,LVM_CHOOSEITEM,additemindex,0);
	}
	return 1;
}

int processscrollview(HWND listwnd, int down,int incseed)
{
	int additemindex;
	int totalcount=SendMessage(listwnd,LVM_GETITEMCOUNT,0,0);

	if (totalcount<=0)
		return 0;
	additemindex = SendMessage(listwnd,SVM_GETCURSEL,0,0);
	if (additemindex<0)
		return 0;
	if (down)
	{
		additemindex+=incseed;
		if (additemindex>=(totalcount-1))
			additemindex=(totalcount-1);
	}
	else
	{
		additemindex-=(incseed);
		if (additemindex<=0)
			additemindex=0;
	}
	SendMessage(listwnd,LVM_CHOOSEITEM,additemindex,0);
	return 1;
}

/*
int processscrollview(HWND listwnd, int down,int incseed)
{
	int additemindex;
	int itemscount = 0;
	additemindex = SendMessage(listwnd,SVM_GETCURSEL,0,0);
	itemscount = SendMessage(listwnd, SVM_GETITEMCOUNT, 0, 0);
	//printf("_____%s%d, additemindex=%d,itemscount=%d\n", __FILE__, __LINE__, additemindex, itemscount);
	if ((additemindex <= 0) && !down) {
		SendMessage(listwnd,LVM_CHOOSEITEM,0,0);
		return 0;
	}
	if((additemindex >= (itemscount-1)) && down) {
		return 0;
	}
	
	if (!down) {
		additemindex--;
		SendMessage(listwnd,LVM_CHOOSEITEM,additemindex,0);
	} else {
		additemindex++;
		SendMessage(listwnd,LVM_CHOOSEITEM,additemindex,0);
	}
	return 1;
}
*/

static int GetListSelData(HWND listwnd)
{
	U16 userpin=0;
	HLVITEM hItemSelected;
	LVSUBITEM subitem;
	TUser myuser;

	memset(&myuser,0,sizeof(TUser));
	memset(&subitem,0,sizeof(LVSUBITEM));
	memset(mybuff,0,24);

	hItemSelected=SendMessage(listwnd,LVM_GETSELECTEDITEM,0,0);
	if (hItemSelected==0)
		return userpin;

	subitem.pszText=mybuff;
	subitem.subItem=0;
	SendMessage(listwnd,LVM_GETSUBITEMTEXT,hItemSelected,(LPARAM)&subitem);
	if (subitem.pszText)
	{
		nmemcpy((BYTE*)myuser.PIN2,(BYTE*)mybuff,strlen(mybuff));
		if (FDB_GetUserByCharPIN2(myuser.PIN2,&myuser))
		{
			if (myuser.PIN)
				return myuser.PIN;
			else return 0;
		}
		else
			return 0;

	}
	else
		return 0;
}

static int UpdateListuserInfo(HWND gdnd,int pin,int opmode)
{
	TUser updateu;
	char *(uu[6]);
	char fpnumchar[4];
	int idnum=0;
	char mynamename[100];
#ifdef FACE
	int faceid=0;
#endif

	memset(&updateu,0,sizeof(updateu));
	updateu.PIN=pin;
	FDB_GetUser(pin,&updateu);

	uu[0]=updateu.PIN2;
	if (updateu.Name[0])
	{
		memset(mynamename,0,100);	//add by jazzy 8008.07.25
		Str2UTF8(tftlocaleid,(unsigned char *)updateu.Name,(unsigned char *)mynamename);
		if (mynamename[0])
			uu[1]=mynamename;
	}
	else uu[1]=NULL;	//add by jazzy 2008.12.01 for if no Name,show error name

	if(!gOptions.IsOnlyRFMachine)
	{
		memset(fpnumchar,0,sizeof(fpnumchar));
		sprintf(fpnumchar,"%d", FDB_GetTmpCnt(updateu.PIN));
		//printf("%s pin %d  fpnumchar %s \n",__FUNCTION__, updateu.PIN, fpnumchar);
		uu[2]=fpnumchar;
		if(updateu.Password[0])
			uu[3]="1";
		else
			uu[3]=NULL;
		if(gOptions.RFCardFunOn)
		{
			memcpy(&idnum, updateu.Card,4);
			if(idnum)
				uu[4]="1";
			else
				uu[4]=NULL;
#ifdef FACE
			faceid=5;
#endif
		}
#ifdef FACE
		else
			faceid=4;

		if(gOptions.FaceFunOn) //add by caona for face
		{
			if(FDB_GetFaceTmp(updateu.PIN, 0 , NULL)) //face
				uu[faceid]="1";
			else
				uu[faceid]=NULL;
		}
#endif
	}
	else
	{
		if(updateu.Password[0])
			uu[2]="1";
		else
			uu[2]=NULL;
		if(gOptions.RFCardFunOn)
		{
			memcpy(&idnum, updateu.Card,4);
			if(idnum)
				uu[3]="1";
			else
				uu[3]=NULL;
#ifdef FACE
			faceid=4;
#endif
		}
#ifdef FACE
		else
			faceid=3;
		if(gOptions.FaceFunOn) //add by caona for face
		{
			if(FDB_GetFaceTmp(updateu.PIN, 0 , NULL)) //face
				uu[faceid]="1";
			else
				uu[faceid]=NULL;
		}
#endif
	}

	if (opmode)
		SetListViewData(gdnd,0,opmode,updateu.Privilege,uu);
	else
		SetListViewData(gdnd,g_rowcount,opmode,updateu.Privilege,uu);
	FindUserItem(gdnd,updateu.PIN2);
	return 0;
}

static HMENU CreateQuickMenu(HWND hWnd)
{
	int i;
	int menucount=0;
	char *msg[6];

	HMENU hNewMenu;
	MENUITEMINFO mii;
	HMENU hMenuFloat;

	msg[menucount++] = LoadStrByID(MID_QUERYUSER);			//query user
	msg[menucount++] = LoadStrByID(MID_ATTQUERY);			//query attlog
	msg[menucount++] = LoadStrByID(MID_EDITUSER);			//edit user
	msg[menucount++] = LoadStrByID(MID_DELUSER);			//delete user info
	msg[menucount++] = LoadStrByID(MID_AD_ADDUSER);			//add new user
	if((gOptions.LockFunOn&LOCKFUN_ADV) ||(gOptions.AttUseTZ))					//
		msg[menucount++] = LoadStrByID(MID_USER_DOOR);		//user door function

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.type = MFT_BMPSTRING;
	mii.id = 0;
	mii.uncheckedbmp=&mgridbmp1;
	mii.typedata = (DWORD)"opt";

	hNewMenu = CreatePopupMenu(&mii);
	for (i=0;i< menucount; i++)
	{
		memset(&mii,0,sizeof(MENUITEMINFO));
		mii.type = MFT_BMPSTRING;
		mii.id = 100+i;
		mii.state = 0;
		switch (i)
		{
			case 0:
				mii.uncheckedbmp=&mgridbmp4;
				break;
			case 1:
				mii.uncheckedbmp=&mgridbmp7;
				break;
			case 2:
				mii.uncheckedbmp=&mgridbmp2;
				break;
			case 3:
				mii.uncheckedbmp=&mgridbmp3;
				break;
			case 4:
				mii.uncheckedbmp=&mgridbmp1;
				break;
			case 5:
				mii.uncheckedbmp=&mgridbmp12;
				break;

		}
		mii.typedata= (DWORD)msg[i];
		InsertMenuItem(hNewMenu,i,TRUE,&mii);
	}
	hMenuFloat = StripPopupHead(hNewMenu);
	TrackPopupMenu(hMenuFloat,TPM_LEFTALIGN|TPM_VCENTERALIGN,5,140,hWnd);

	return hMenuFloat;

}

void GetUserInfo()
{
	int j=0;
	char *(userinfo[6]);
	char fpnumchar[4];
	unsigned int idnumber=0;
	char mynamename[100];
#ifdef FACE
	int faceid=0;
#endif
	int usercnt;

	PUserlb tmpuserlb, puserlb;
	//puserlb = GetUsermestolb(0);
	puserlb = GetUsermestolb_PageForward(0, &usercnt, 0);
	tmpuserlb = puserlb;
	j=0;

#ifdef FACE 
	if(gOptions.FaceFunOn)
		GetUserFaceInfoForListview(puserlb);
#endif

    SendMessage (hGVWnd, MSG_FREEZECTRL, TRUE, 0);

	while (puserlb != NULL)
    {
		userinfo[0]=NULL;
		userinfo[1]=NULL;
		userinfo[2]=NULL;
		userinfo[3]=NULL;
		userinfo[4]=NULL;
		userinfo[5]=NULL;

        if ((strlen(puserlb->userlb.PIN2)==0) ||(strcmp(puserlb->userlb.PIN2,"0")==0) ||
            (strlen(puserlb->userlb.PIN2)>gOptions.PIN2Width))
        {
            int k;
            k=FDB_DelUser(puserlb->userlb.PIN);
            FDB_AddOPLog(ADMINPIN, OP_DEL_USER, puserlb->userlb.PIN,k,0,0);
            sync();
            puserlb = puserlb->next;
            continue;
        }
		userinfo[0]=puserlb->userlb.PIN2;

		memset(mynamename,0,100);
       	Str2UTF8(tftlocaleid,(unsigned char *)puserlb->userlb.Name,(unsigned char *)mynamename);
//		printf("mynamename:%s\t%d\n",mynamename,mynamename[0]);
		if (mynamename[0])
			userinfo[1]=mynamename;

		if(!gOptions.IsOnlyRFMachine)
		{
			memset(fpnumchar,0,4);
			sprintf(fpnumchar,"%d",FDB_GetTmpCnt(puserlb->userlb.PIN));
			userinfo[2]=fpnumchar;
			if (puserlb->userlb.Password[0])
				userinfo[3]="1";
			else
				userinfo[3]=NULL;
			if(gOptions.RFCardFunOn)
			{
				memcpy(&idnumber,puserlb->userlb.Card,4);
				if(idnumber)
					userinfo[4]="1";
				else
					userinfo[4]=NULL;
#ifdef FACE
				faceid=5;
#endif
			}
#ifdef FACE
			else
				faceid=4;
			if(gOptions.FaceFunOn) //add by caona for face
			{
				if(puserlb->userlb.Password[2])
					userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
			}
#endif
		}
		else
		{
                        if (puserlb->userlb.Password[0])
                                userinfo[2]="1";
                        else
                                userinfo[2]=NULL;
                        if(gOptions.RFCardFunOn)
                        {
                                memcpy(&idnumber,puserlb->userlb.Card,4);
                                if(idnumber)
                                        userinfo[3]="1";
                                else
                                        userinfo[3]=NULL;
#ifdef FACE
				faceid=4;
#endif
                        }
#ifdef FACE
			else
				faceid=3;
                        if(gOptions.FaceFunOn) //add by caona for face
                        {
                                if(puserlb->userlb.Password[2])
                                        userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
                        }
#endif
		}
		SetListViewData(hGVWnd,j,0,puserlb->userlb.Privilege,userinfo);

               	puserlb = puserlb->next;
		j++;

	}
	SendMessage (hGVWnd, MSG_FREEZECTRL, FALSE, 0);
	g_rowcount = SendMessage(hGVWnd,LVM_GETITEMCOUNT,0,0);
        if (tmpuserlb != NULL)
        {
        	PUserlb tmptmpuserlb;
		tmptmpuserlb = tmpuserlb;
		while (tmptmpuserlb != NULL)
		{
			tmpuserlb = tmptmpuserlb;
			tmptmpuserlb = tmpuserlb->next;
			free(tmpuserlb);
		}
		tmptmpuserlb = NULL;
	}
}

void FindUserInfoRefresh(void)
{
	int j=0;
	char *(userinfo[6]);
	char fpnumchar[4];
	unsigned int idnumber=0;
	char mynamename[100];
#ifdef FACE 
	int faceid=0;
#endif

	PUserlb tmpuserlb, puserlb;
	//puserlb = GetUsermestolb(0);
	puserlb = ppuserlb;
	tmpuserlb = puserlb;
	j=0;

#ifdef FACE 
	if(gOptions.FaceFunOn)
		GetUserFaceInfoForListview(puserlb);
#endif

    SendMessage (hGVWnd, MSG_FREEZECTRL, TRUE, 0);

	while (puserlb != NULL)
    {
		userinfo[0]=NULL;
		userinfo[1]=NULL;
		userinfo[2]=NULL;
		userinfo[3]=NULL;
		userinfo[4]=NULL;
		userinfo[5]=NULL;

        if ((strlen(puserlb->userlb.PIN2)==0) ||(strcmp(puserlb->userlb.PIN2,"0")==0) ||
            (strlen(puserlb->userlb.PIN2)>gOptions.PIN2Width))
        {
            int k;
            k=FDB_DelUser(puserlb->userlb.PIN);
            FDB_AddOPLog(ADMINPIN, OP_DEL_USER, puserlb->userlb.PIN,k,0,0);
            sync();
            puserlb = puserlb->next;
            continue;
        }
		userinfo[0]=puserlb->userlb.PIN2;
		//printf("_____%s%d  PIN2=%s\n", __FILE__, __LINE__, puserlb->userlb.PIN2);
		memset(mynamename,0,100);
       	Str2UTF8(tftlocaleid,(unsigned char *)puserlb->userlb.Name,(unsigned char *)mynamename);
//		printf("mynamename:%s\t%d\n",mynamename,mynamename[0]);
		if (mynamename[0])
			userinfo[1]=mynamename;

		if(!gOptions.IsOnlyRFMachine)
		{
			memset(fpnumchar,0,4);
			sprintf(fpnumchar,"%d",FDB_GetTmpCnt(puserlb->userlb.PIN));
			userinfo[2]=fpnumchar;
			if (puserlb->userlb.Password[0])
				userinfo[3]="1";
			else
				userinfo[3]=NULL;
			if(gOptions.RFCardFunOn)
			{
				memcpy(&idnumber,puserlb->userlb.Card,4);
				if(idnumber)
					userinfo[4]="1";
				else
					userinfo[4]=NULL;
#ifdef FACE
				faceid=5;
#endif
			}
#ifdef FACE
			else
				faceid=4;
			if(gOptions.FaceFunOn) //add by caona for face
			{
				if(puserlb->userlb.Password[2])
					userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
			}
#endif
		}
		else
		{
                        if (puserlb->userlb.Password[0])
                                userinfo[2]="1";
                        else
                                userinfo[2]=NULL;
                        if(gOptions.RFCardFunOn)
                        {
                                memcpy(&idnumber,puserlb->userlb.Card,4);
                                if(idnumber)
                                        userinfo[3]="1";
                                else
                                        userinfo[3]=NULL;
#ifdef FACE
				faceid=4;
#endif
                        }
#ifdef FACE
			else
				faceid=3;
                        if(gOptions.FaceFunOn) //add by caona for face
                        {
                                if(puserlb->userlb.Password[2])
                                        userinfo[faceid]="1";
                                else
                                        userinfo[faceid]=NULL;
                        }
#endif
		}
		SetListViewData(hGVWnd,j,0,puserlb->userlb.Privilege,userinfo);

               	puserlb = puserlb->next;
		j++;

	}
	SendMessage(hGVWnd,LVM_CHOOSEITEM,userrowindex,0);
	SendMessage (hGVWnd, MSG_FREEZECTRL, FALSE, 0);
	g_rowcount = SendMessage(hGVWnd,LVM_GETITEMCOUNT,0,0);
        if (tmpuserlb != NULL)
        {
        	PUserlb tmptmpuserlb;
		tmptmpuserlb = tmpuserlb;
		while (tmptmpuserlb != NULL)
		{
			tmpuserlb = tmptmpuserlb;
			tmptmpuserlb = tmpuserlb->next;
			free(tmpuserlb);
		}
	}
	ppuserlb = NULL;
}

static int FindUserItem(HWND listwnd,char *fddata)
{
	//LVFINDINFO find_info;
	LVSUBITEM subitem_info;
	LVITEM get_info;
	HLVITEM nlvi;
	char tempStr[MAX_WORKCODE_LEN+1];
	int icount=0;
	int count=0;

	memset(&subitem_info,0,sizeof(LVSUBITEM));	
	icount = SendMessage (listwnd, LVM_GETITEMCOUNT, 0, 0);
	for(count=0; count < icount; count++)
	{
		get_info.nItem=count;
		nlvi=SendMessage (listwnd, LVM_GETITEM, 0, (LPARAM)&get_info);
		memset(tempStr,0,MAX_WORKCODE_LEN+1);
		subitem_info.nItem = get_info.nItem;
		subitem_info.subItem = 0;
		subitem_info.flags = 0;
		subitem_info.pszText = tempStr;
		subitem_info.nTextColor = 0;
		SendMessage (listwnd, LVM_GETSUBITEMTEXT, nlvi, (LPARAM)&subitem_info);	
		if(strcmp(tempStr,fddata)==0)
		{
			SendMessage (listwnd, LVM_CHOOSEITEM, count, 0);
			return 1;
		}
	}
	return 0;
}

static HWND stahwnd;
static int ControlTestWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int tmppin;
	HDC hdc;
	char finduseracno[24];
	struct tm  ttlog;
	time_t logst;
	time_t loged;
	char hintchar[20];
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			CreateListView(hWnd);
			stahwnd = createStatusWin1(hWnd , 250 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
			LoadBitmap(HDC_SCREEN,&mgridbmp1,GetBmpPath("add.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp2,GetBmpPath("edit.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp3,GetBmpPath("delete.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp4,GetBmpPath("find.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp0,GetBmpPath("fphint.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp5,GetBmpPath("key.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp6,GetBmpPath("no.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp7,GetBmpPath("findatt.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp8,GetBmpPath("pageup.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp9,GetBmpPath("pagedown.gif"));
			if(gOptions.TFTKeyLayout!=3)
				LoadBitmap(HDC_SCREEN,&mgridbmp10,GetBmpPath("function.gif"));
			else
				LoadBitmap(HDC_SCREEN,&mgridbmp10,GetBmpPath("fun2.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp11,GetBmpPath("okkey.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp12,GetBmpPath("edit.gif"));
			LoadBitmap(HDC_SCREEN,&mgridbmp13,GetBmpPath("cards.gif"));

#ifdef FACE
			if(gOptions.FaceFunOn)
				LoadBitmap(HDC_SCREEN,&viewface,GetBmpPath("faceok.gif"));
#endif
			if (gfont1==NULL)
				gridfont = CreateLogFont (NULL,"fixed","GB2312",FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,12, 0);
			userdatacuroffset_e = 0;
			userdatacuroffset_s = 0;
			GetUserInfo();
			destroyStatusWin1 (stahwnd);

			SetFocusChild(hGVWnd);
			SendMessage(hGVWnd,LVM_CHOOSEITEM,0,0);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			SetBkColor(hdc,0x00FFA2BE);
			if (gfont1==NULL)
				SelectFont(hdc,gridfont);
			else	SelectFont(hdc,gfont1);
			SetTextColor(hdc,PIXEL_lightwhite);
			if(gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4)
			{
				memset(hintchar,0,20);
				if (fromRight==1)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEUP));
				else
					sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEUP));
				TextOut(hdc,6,220,hintchar);
				FillBoxWithBitmap (hdc, 60, 218,16, 16, &mgridbmp8);
				memset(hintchar,0,20);
				if (fromRight==1)
					sprintf(hintchar,"%s",LoadStrByID(MID_PAGEDOWN));
				else
					sprintf(hintchar,"%s:",LoadStrByID(MID_PAGEDOWN));
				TextOut(hdc,86,220,hintchar);
				FillBoxWithBitmap (hdc, 140, 218,16, 16, &mgridbmp9);
			}
			if(gOptions.TFTKeyLayout!=3)
			{
				memset(hintchar,0,20);
				if (fromRight==1)
					sprintf(hintchar,"%s",LoadStrByID(MID_EDIT));
				else
					sprintf(hintchar,"%s:",LoadStrByID(MID_EDIT));
				TextOut(hdc,171,220,hintchar);
				FillBoxWithBitmap (hdc, 210, 218,16, 16, &mgridbmp11);
			}
			memset(hintchar,0,20);
			if (fromRight==1)
				sprintf(hintchar,"%s",LoadStrByID(MID_DEL));
			else
				sprintf(hintchar,"%s:",LoadStrByID(MID_DEL));
			TextOut(hdc,246,220,hintchar);
			FillBoxWithBitmap (hdc, 287, 219,24, 16, &mgridbmp10);

			EndPaint(hWnd,hdc);
			return 0;

		case MSG_COMMAND:
			switch (wParam)
			{
				case 105:
					tmppin = GetListSelData(hGVWnd);
					if(tmppin)
					{
						TUser tmpuser;
						memset(&tmpuser, 0, sizeof(TUser));
						tmpuser.PIN = tmppin;
						FDB_GetUser(tmppin, &tmpuser);
						if(((tmpuser.Privilege&PRI_SUPERVISOR) != 0 || (tmpuser.Privilege&PRI_OPTIONS) != 0)
								&& (TESTPRIVILLEGE(PRI_SUPERVISOR) == 0) && (TESTPRIVILLEGE(PRI_OPTIONS) == 0)) {
							MessageBox1(hWnd, LoadStrByID(HID_ACCESSDENY), LoadStrByID(HIT_ERR),
								MB_OK | MB_BASEDONPARENT);
						}
						else {
							CreateLockWindow(hWnd,&tmppin);
						}
						UpdateListuserInfo(hGVWnd,tmppin,1);
					}
					else
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_CHOOSEUSER));
					}
					break;

				case 104:
					tmppin = 0;
					CreateAddUserWindow(hWnd,0,&tmppin);
					if (tmppin)
					{
						int cntuser;
						TUser refreshuser;
						memset(&refreshuser,0,sizeof(TUser));
						refreshuser.PIN=tmppin;
						FDB_GetUser(tmppin,&refreshuser);
						SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
						//GetUserInfo();
						//g_rowcount = SendMessage(hGVWnd,LVM_GETITEMCOUNT,0,0);
						//FindUserItem(hGVWnd,refreshuser.PIN2);

						//printf(" refreshuser.PIN2 =%s, %s,%d\n", refreshuser.PIN2, __FILE__, __LINE__);
						GetUsermestolb_NameOrAcno("", refreshuser.PIN2, &cntuser);
						FindUserInfoRefresh();
					}
					break;

				case 103:
				case 102:
				case 101:
					tmppin = GetListSelData(hGVWnd);
					if (tmppin)
					{
						//edit
						if (wParam==102)
						{
							int cntuser_1;
							TUser tmpuser;
							memset(&tmpuser, 0, sizeof(TUser));
							tmpuser.PIN = tmppin;
							FDB_GetUser(tmppin, &tmpuser);
							//printf("chg user window ***************\n");
							//add by zxz 2012-10-12
							if(((tmpuser.Privilege&PRI_SUPERVISOR) != 0 || (tmpuser.Privilege&PRI_OPTIONS) != 0)
										&& (TESTPRIVILLEGE(PRI_SUPERVISOR) == 0) && (TESTPRIVILLEGE(PRI_OPTIONS) == 0)){
								MessageBox1(hWnd, LoadStrByID(HID_ACCESSDENY), LoadStrByID(HIT_ERR),
									MB_OK | MB_BASEDONPARENT);
							}
							else {
								CreateAddUserWindow(hWnd,1,&tmppin);
							}
							if(ismenutimeout) return 0;
							SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
							GetUsermestolb_NameOrAcno("", tmpuser.PIN2, &cntuser_1);
							FindUserInfoRefresh();
							//UpdateRowuserInfo(hGVWnd,temprow,tmppin);
							//UpdateListuserInfo(hGVWnd,tmppin,1);
							//userdatacuroffset_e = 0;
							//userdatacuroffset_s = 0;
							//SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
							//GetUserInfo();
						}
						//logquery
						if (wParam==101)
						{
							GetTime(&ttlog);
							ttlog.tm_mday = 1;
							ttlog.tm_hour = 0;
							ttlog.tm_min = 0;
							ttlog.tm_sec = 0;
							ttlog.tm_isdst = -1;
							logst = OldEncodeTime(&ttlog);

							ttlog.tm_mday =GetLastDayofmonth(ttlog.tm_year+1900,ttlog.tm_mon+1);
							ttlog.tm_hour = 23;
							ttlog.tm_min = 59;
							ttlog.tm_sec = 0;

							//loged =logst+OneDay-OneSecond;
							loged =OldEncodeTime(&ttlog);
							//if (!CreateOneLogWindow(hWnd,tmppin,logst,loged))
							if (!CreateAttLogWindow(hWnd,mybuff,logst,loged, 0) && !ismenutimeout)
								myMessageBox1(hWnd,MB_OK|MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOATTLOG));
						}
						if (wParam==103)
						{
							int delreturn=0;
							TUser tmpuser;
							memset(&tmpuser, 0, sizeof(TUser));
							tmpuser.PIN = tmppin;
							FDB_GetUser(tmppin, &tmpuser);
							if(((tmpuser.Privilege&PRI_SUPERVISOR) != 0 || (tmpuser.Privilege&PRI_OPTIONS) != 0)
										&& (TESTPRIVILLEGE(PRI_SUPERVISOR) == 0) && (TESTPRIVILLEGE(PRI_OPTIONS) == 0)){
								MessageBox1(hWnd, LoadStrByID(HID_ACCESSDENY), LoadStrByID(HIT_ERR),
									MB_OK | MB_BASEDONPARENT);
							}
							else {
								delreturn =CreateDelUserWindow(hWnd,mybuff, tmppin);
							}
							if(ismenutimeout) return 0;
							if(delreturn)
							{
								if(delreturn==1)
								{
									//userdatacuroffset_e = 0;
									//userdatacuroffset_s = 0;
									//SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
									//GetUserInfo();
									
									HLVITEM hItemSel;
									int iindex=SendMessage(hGVWnd,SVM_GETCURSEL,0,0);

									hItemSel=SendMessage(hGVWnd,LVM_GETSELECTEDITEM,0,0);
									if (hItemSel)
									{
										int cntuser;
										SendMessage(hGVWnd,LVM_DELITEM,0,(LPARAM)hItemSel);
										g_rowcount = SendMessage(hGVWnd,LVM_GETITEMCOUNT,0,0);
										iindex--;
										if (iindex<0) iindex=0;
										SendMessage(hGVWnd,LVM_CHOOSEITEM,iindex,0);
										tmppin = 0;
										tmppin = GetListSelData(hGVWnd);
										if(tmppin <= 0) {
											RefreshGridview(DIRECTINON_FLAG_BACKWARD);//backward
											return 0;
										}
										memset(&tmpuser, 0, sizeof(TUser));
										tmpuser.PIN = tmppin;
										FDB_GetUser(tmppin, &tmpuser);
										SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
										GetUsermestolb_NameOrAcno("", tmpuser.PIN2, &cntuser);
										FindUserInfoRefresh();
									}
									
								}
								else
									UpdateListuserInfo(hGVWnd,tmppin,1);
							}
						}
					}
					else
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_CHOOSEUSER));
					}
					break;

				case 100:
					memset(finduseracno,0,24);
					if(CreateFindUserWindow(hWnd,finduseracno))
					{
						SendMessage(hGVWnd,LVM_DELALLITEM,0,0);
						FindUserInfoRefresh();
						//if(!FindUserItem(hGVWnd,finduseracno))
						//{
						//	myMessageBox1(hWnd,MB_OK|MB_ICONINFORMATION,LoadStrByID(MID_APPNAME),LoadStrByID(HID_NOTENROLLED));
						//}
					}
					if(ismenutimeout) return 0;
					break;

				case IDCANCEL:
					PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;

			}
			break;

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
			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			 {
				RefreshGridview(DIRECTINON_FLAG_BACKWARD);//backward
				return 0;
			 }

			 if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT ){
				RefreshGridview(DIRECTINON_FLAG_FORWARD);//forward
				return 0;
			 }

			if (((LOWORD(wParam)>=SCANCODE_1&&(LOWORD(wParam)<=SCANCODE_9))|| LOWORD(wParam)==SCANCODE_0))
			{
				SendMessage(hWnd,MSG_COMMAND,100,0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
				return 0;
			}
			else if((LOWORD(wParam)==SCANCODE_MENU) || (gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_ENTER))
			{
				CreateQuickMenu(hWnd);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_ENTER || LOWORD(wParam)==SCANCODE_F10)
			{
				SendMessage(hWnd,MSG_COMMAND,102,0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_F12)
			{
				//SendMessage(hGVWnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0);
				//processscrollview_UserGridView(hGVWnd,DIRECTINON_FLAG_FORWARD,GRIDVIEWROWCNT);
				RefreshGridview(DIRECTINON_FLAG_FORWARD);//forward
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_F11)
			{
				//SendMessage(hGVWnd,MSG_KEYDOWN,SCANCODE_PAGEUP,0);
				//processscrollview_UserGridView(hGVWnd,DIRECTINON_FLAG_BACKWARD,GRIDVIEWROWCNT);
				RefreshGridview(DIRECTINON_FLAG_BACKWARD);//backward
				return 0;
			}
			else if (LOWORD(wParam) == SCANCODE_CURSORBLOCKDOWN)
			{
				processscrollview_UserGridView(hGVWnd,DIRECTINON_FLAG_FORWARD,GRIDVIEWROWCNT);
				return 0;
			}
			else if (LOWORD(wParam) == SCANCODE_CURSORBLOCKUP)
			{
				processscrollview_UserGridView(hGVWnd,DIRECTINON_FLAG_BACKWARD,GRIDVIEWROWCNT);
				return 0;
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&mgridbmp1);
			UnloadBitmap(&mgridbmp2);
			UnloadBitmap(&mgridbmp3);
			UnloadBitmap(&mgridbmp4);
			UnloadBitmap(&mgridbmp0);
			UnloadBitmap(&mgridbmp5);
			UnloadBitmap(&mgridbmp6);
			UnloadBitmap(&mgridbmp7);
			UnloadBitmap(&mgridbmp8);
			UnloadBitmap(&mgridbmp9);
			UnloadBitmap(&mgridbmp10);
			UnloadBitmap(&mgridbmp11);
			UnloadBitmap(&mgridbmp12);
			UnloadBitmap(&mgridbmp13);
#ifdef FACE
			if(gOptions.FaceFunOn)
				UnloadBitmap(&viewface);
#endif
			if (gfont1==NULL) {
				DestroyLogFont(gridfont);
			}
			DestroyMainWindow(hWnd);
			//MainWindowCleanup (hWnd);
			return 0;

	}
	return DefaultMainWinProc (hWnd, message, wParam, lParam);

}

int CreateUserBrowseWindow(HWND hOwner)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	hOwner = GetMainWindowHandle (hOwner);
	InitMiniGUIExt();

	//CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_USERMNG);
	CreateInfo.hMenu = 0;
	//CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ControlTestWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hMainWnd = CreateMainWindow (&CreateInfo);

	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup(hMainWnd);
	MiniGUIExtCleanUp ();

	return 0;
}
