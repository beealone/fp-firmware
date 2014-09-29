#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "main.h"
#include <minigui/tftmullan.h>
#include "finger.h"

#define LB_TITLE 50
#define ID_CKFP 51
#define ID_CKPWD 52
#define ID_CBAUTH 53
#define ID_SAVEDEL 631
#define ID_EXIT 64
#define ID_CKBASE 65

#define ID_CKHID 54		//liming
#define ID_CKPHOTO 55

//For face
#ifdef FACE
#define ID_CKFACE 56
#endif
int facedelhint=0;
int noface=1;
static HWND btndelface;
static int h_face;
//end face

int delpin=0;
int delret=0;
int pwddelhint=0;
int fpdelhint=0;
int hiddelhint=0;		//liming
int photodelhint=0;
int nopwd=1;
int nofp=1;
int nohid=1;			//liming
int nophoto=1;
//BITMAP delbmp;
BITMAP delhintbmp;
static HWND Edtmp,btnexit2,focuswnd;
HWND btndelall,btndelfp,btndelpwd,btndelhid,btndelphoto;
TUser deluser;
PPhotoList dellist=NULL;
int h_fp, h_pwd, h_hid, h_pic;

int  MessageBox1 (HWND hParentWnd, const char* pszText,
		const char* pszCaption, DWORD dwStyle);
HWND createStatusWin1 (HWND hParentWnd, int width, int height,
		char* title, char* text, ...);
void destroyStatusWin1(HWND hwnd);

extern int gCurCaptureCount;

static void ProcDeleteUserPhoto(void)
{
	PPhotoList ptmp=NULL;
	char pPath[100];

	ptmp = dellist->next;
	while(ptmp != dellist)
	{
		memset(pPath, 0, 100);
		//sprintf(pPath, "rm -rf %s/%s.jpg && sync", GetCapturePath("capture/pass"), ptmp->PhotoFile);
		sprintf(pPath, "%s/%s.jpg", GetCapturePath("capture/pass"), ptmp->PhotoFile);
		//if (system(pPath) == EXIT_SUCCESS)
		if (remove(pPath) == 0)
			FDB_DelPhotoIndex(ptmp->index);

		dellist->next = ptmp->next;
		ptmp->next->pre = dellist;
		FREE(ptmp);
		ptmp = dellist->next;
	}

	memset(pPath, 0, 100);
	//sprintf(pPath, "rm -rf %s/%s.jpg && sync", GetCapturePath("capture/pass"), ptmp->PhotoFile);
	sprintf(pPath, "%s/%s.jpg", GetCapturePath("capture/pass"), ptmp->PhotoFile);
	if (remove(pPath) == 0)
		FDB_DelPhotoIndex(ptmp->index);

	FREE(dellist);

	gCurCaptureCount = FDB_CntPhotoCount();     //reset this flag for capture count

	dellist = NULL;
	ptmp = NULL;
}

static int  ProcessDeluser(U16 pin, HWND hWnd, int delupass)
{
	int fpcheck=0, pwdcheck=0, basecheck=0;
	int hidcheck=0;		//liming
	int photocheck=0;

	char delhintbuff[100];
	int delcount=0;
	TUser duser;

	delret=0;
	memset(delhintbuff,0,100);

	//delall
	if (delupass==0)
	{
		sprintf(delhintbuff,"%s?",LoadStrByID(MID_USEBASE));
		basecheck=1;
	}

	//delfp
	if (delupass==1)
	{
		sprintf(delhintbuff,"%s?",LoadStrByID(HID_DEL_FP));
		fpcheck=1;
	}

	//delpwd
	if (delupass==2)
	{
		sprintf(delhintbuff,"%s?",LoadStrByID(HID_DEL_PWD));
		pwdcheck=1;
	}

	//delhid
	if (delupass==3)
	{
		sprintf(delhintbuff,"%s?",LoadStrByID(MID_DEL_CARD));
		hidcheck=1;
	}

	//delphoto
	if (delupass==4)
	{
		sprintf(delhintbuff,"%s?", LoadStrByID(MID_QUERY_SET1));
		photocheck=1;
	}

	//For face
#ifdef FACE
	int facecheck=0;
	if (delupass==5)
	{
		sprintf(delhintbuff,"%s%s?", LoadStrByID(MID_REMOVEHINT), LoadStrByID(MID_DEL_FACE));
		facecheck=1;
	}
#endif
	//end face

	if (MessageBox1 (hWnd,delhintbuff, LoadStrByID(MID_APPNAME), MB_OKCANCEL | MB_ICONQUESTION | MB_BASEDONPARENT) != IDOK)
	{
		delret=0;
		return 0;
	}

	//提示清除管理员权限
	memset(&duser, 0, sizeof(TUser));
	FDB_GetUser(pin, &duser);
	if(delupass>0 && ((nopwd && nofp)||(nopwd && nohid)||(nofp && nohid)) && duser.Privilege==(BYTE)PRIVILLEGE3)
	{
		if(MessageBox1(hWnd,LoadStrByID(MID_ADUSER_HINT),LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONINFORMATION|MB_BASEDONPARENT)==IDOK)
		{
			int j=0;
			duser.Privilege=0;
			j=FDB_ChgUser(&duser);
			FDB_AddOPLog(ADMINPIN, OP_CLEAR_ADMIN,duser.PIN,j,0,0);
		}
		else
			return 0;
	}
	//
	//delete photo
	if (photocheck)
	{
		HWND statushWnd;

		statushWnd=createStatusWin1(hWnd, 250, 50, LoadStrByID(MID_APPNAME), LoadStrByID(MID_PHOTO_HINT1));
		busyflag = 1;
		ProcDeleteUserPhoto();
		FDB_AddOPLog(ADMINPIN, OP_DEL_ATTPHOTO,ADMINPIN,0,0,0);
		destroyStatusWin1(statushWnd);
		nophoto=1;
		delret=5;
		InvalidateRect(hWnd,NULL,TRUE);
		SendMessage(btndelphoto, MSG_ENABLE, 0, 0);
		photodelhint=1;
		SetMenuTimeOut(time(NULL));
		SetFocusChild(btnexit2);
	}

	//delete hid
	if (hidcheck)
	{
		TUser uh;
		memset(&uh,0,sizeof(TUser));
		if(FDB_GetUser(pin,&uh)!=NULL)
		{
			int j;
			memset(uh.Card,0,4);
			j=FDB_ChgUser(&uh);
			FDB_AddOPLog(ADMINPIN, OP_DEL_RFCARD,uh.PIN,j,0,0);
			delret=4;		//
			hiddelhint=1;
			InvalidateRect(hWnd,NULL,TRUE);
			SendMessage(btndelhid,MSG_ENABLE,0,0);
			nohid=1;
			SetFocusChild(btnexit2);
		}
	}

	//delete fp
	if (fpcheck)
	{	//dellfp
		int j=FDB_DeleteTmps(pin);
		FDB_AddOPLog(ADMINPIN, OP_DEL_FP,pin,j,0,0);
		delcount++;
		SendMessage(btndelfp,MSG_ENABLE,0,0);
		delret=2;
		fpdelhint=1;
		InvalidateRect(hWnd,NULL,TRUE);
		SendMessage(btndelfp,MSG_ENABLE,0,0);
		nofp=1;
		SetFocusChild(btnexit2);
	}

	if (pwdcheck)
	{
		TUser u1;
		int j;

		memset(&u1,0,sizeof(u1));
		FDB_GetUser(pin,&u1);
		nmemset((BYTE*)u1.Password, 0, sizeof(u1.Password));
		j=FDB_ChgUser(&u1);
		FDB_AddOPLog(ADMINPIN, OP_DEL_PWD,u1.PIN,j,0,0);
		delret=3;
		pwddelhint=1;
		InvalidateRect(hWnd,NULL,TRUE);
		SendMessage(btndelpwd,MSG_ENABLE,0,0);
		nopwd=1;
		SetFocusChild(btnexit2);

	}

	if (basecheck)
	{
		int j = FDB_DelUser(pin);
		FDB_AddOPLog(ADMINPIN, OP_DEL_USER,pin,j,0,0);
		if(FDB_OK==j)
		{
			if(gOptions.IsSupportSMS)
			{
				TUData tudt;
				U16 smsID;
				TSms tsms;

				if(FDB_GetUData(pin,&tudt)!=NULL)
				{
					smsID=tudt.SmsID;
					FDB_DelUData(pin,0);
					memset(&tudt,0,sizeof(TUData));
					if(FDB_GetUDataBySmsID(smsID,(PUData)&tudt)==0)
					{
						memset(&tsms,0,sizeof(TSms));
						if(FDB_GetSms(smsID,&tsms)!=NULL)
						{
							tsms.Tag=UDATA_TAG_TEMP;
							FDB_ChgSms(&tsms);
						}
					}
				}
			}

			if(gOptions.UserExtendFormat)
				FDB_DelExtUser(pin);

			if (gOptions.CameraOpen && !nophoto)
			{
				busyflag = 1;
				ProcDeleteUserPhoto();
				SetMenuTimeOut(time(NULL));
			}
			delcount++;
		}
		delret=1;
	}

#ifdef FACE
	if (facecheck)
	{
		FDB_DeleteFaceTmps(pin);
		delcount++;
		delret=6;
		facedelhint=1;
		InvalidateRect(hWnd,NULL,TRUE);
		SendMessage(btndelface,MSG_ENABLE,0,0);
		noface=1;
		SetFocusChild(btnexit2);
	}
#endif
	if (delcount>0)
	{
		sync();
		FPDBInit();
	}

	if (basecheck)
	{
		SendMessage(hWnd,MSG_CLOSE,0,0);
	}
	return 1;
}

static void SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp  = GetFocusChild(hWnd);
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp ==btndelall)
		{
			if(nofp)
			{
				if(nopwd)
				{
					if(noface)
					{
						if (nohid)
						{
							if (nophoto)
								focuswnd = btnexit2;
							else
								focuswnd = btndelphoto;
						}
						else
							focuswnd = btndelhid;
					}
					else
					{
						focuswnd = btndelface;
					}
				}
				else
					focuswnd = btndelpwd;
			}
			else
				focuswnd = btndelfp;
		}
		else if (Edtmp == btndelfp)
		{
			if (nopwd)
			{
				if(noface)
				{
					if(nohid)
					{
						if (nophoto)
							focuswnd = btnexit2;
						else
							focuswnd = btndelphoto;
					}
					else
						focuswnd = btndelhid;
				}
				else
				{
					focuswnd = btndelface;
				}
			}
			else
				focuswnd = btndelpwd;
		}
		else if (Edtmp == btndelpwd)
		{
			if(noface)
			{
				if(nohid)
				{
					if (nophoto)
						focuswnd = btnexit2;
					else
						focuswnd = btndelphoto;
				}
				else
					focuswnd = btndelhid;
			}
			else
			{
				focuswnd = btndelface;
			}
		}
		else if (Edtmp == btndelface)
		{
			if(nohid)
			{
				if(nophoto)
				{
					focuswnd = btnexit2;
				}
				else
				{
					focuswnd = btndelphoto;
				}
			}
			else
			{
				focuswnd = btndelhid;
			}
		}
		else if (Edtmp == btndelhid)
		{
			if (nophoto)
				focuswnd = btnexit2;
			else
				focuswnd = btndelphoto;
		}
		else if (Edtmp == btndelphoto)
			focuswnd = btnexit2;
		else if (Edtmp == btnexit2)
			focuswnd = btndelall;
		SetFocusChild(focuswnd);
	}
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == btndelall)
			focuswnd = btnexit2;
		else if (Edtmp == btndelfp)
			focuswnd = btndelall;
		else if (Edtmp == btndelpwd)
		{
			if (nofp)
				focuswnd = btndelall;
			else
				focuswnd=btndelfp;
		}
		else if(Edtmp == btndelface)
		{
			if(nopwd)
			{
				if (nofp)
				{
					focuswnd = btndelall;
				}
				else
				{
					focuswnd = btndelfp;
				}
			}
			else
			{
				focuswnd = btndelpwd;
			}
		}
		else if (Edtmp == btnexit2)
		{
			if (nophoto)
			{
				if (nohid)
				{
					if(noface)
					{
						if (nopwd)
						{
							if (nofp)
								focuswnd = btndelall;
							else
								focuswnd=btndelfp;
						}
						else
							focuswnd = btndelpwd;
					}
					else
					{
						focuswnd = btndelface;
					}
				}
				else
					focuswnd = btndelhid;
			}
			else
				focuswnd = btndelphoto;
		}
		else if (Edtmp == btndelphoto)
		{
			if (nohid)
			{
				if(noface)
				{
					if (nopwd)
					{
						if (nofp)
							focuswnd = btndelall;
						else
							focuswnd=btndelfp;
					}
					else
						focuswnd = btndelpwd;
				}
				else
				{
					focuswnd = btndelface;
				}
			}
			else
				focuswnd = btndelhid;
		}
		else if(Edtmp == btndelhid)
		{
			if(noface)
			{
				if(nopwd)
				{
					if(nofp)
						focuswnd = btndelall;
					else
						focuswnd = btndelfp;
				}
				else
					focuswnd = btndelpwd;
			}
			else
			{
				focuswnd = btndelface;
			}
		}
		SetFocusChild(focuswnd);
	}
}

static void InitWindow (HWND hWnd)
{
	int row=0;
	int posX1;

	if (fromRight==1)
		posX1=120+gOptions.ControlOffset;
	else
		posX1=60+gOptions.ControlOffset;

	btndelall = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_USEBASE), WS_VISIBLE | BS_DEFPUSHBUTTON, ID_CKBASE , posX1, 30+row*26, 150, 23, hWnd, 0);
	row++;

	if(!gOptions.IsOnlyRFMachine)
	{
		btndelfp = CreateWindow(CTRL_BUTTON, LoadStrByID(HID_DEL_FP), WS_VISIBLE, ID_CKFP, posX1, 30+row*26, 150, 23, hWnd, 0);
		h_fp = 34+row*26;
		row++;
	}

	btndelpwd = CreateWindow(CTRL_BUTTON, LoadStrByID(HID_DEL_PWD),	WS_VISIBLE, ID_CKPWD, posX1, 30+row*26, 150, 23, hWnd, 0);
	h_pwd = 34+row*26;
	row++;

#ifdef FACE
	if(gOptions.FaceFunOn)
	{
		btndelface = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DEL_FACE), WS_VISIBLE, ID_CKFACE, posX1, 30+row*26, 150, 23, hWnd, 0);
		h_face = 34+row*26;
		row++;
	}
#endif
	if(gOptions.RFCardFunOn)
	{
		btndelhid = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_DEL_RF),WS_VISIBLE, ID_CKHID, posX1, 30+row*26, 150, 23, hWnd, 0);
		h_hid = 34+row*26;
		row++;
	}

	if (gOptions.CameraOpen)
	{
		btndelphoto = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_QUERY_SET1), WS_VISIBLE, ID_CKPHOTO, posX1, 30+row*26,150,23, hWnd, 0);
		h_pic = 34+row*26;
		row++;
	}

	btnexit2 = CreateWindow(CTRL_BUTTON, LoadStrByID(MID_EXIT), WS_VISIBLE | BS_DEFPUSHBUTTON, ID_EXIT,posX1, 30+row*26, 150, 23, hWnd,0);
}

static int Delwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int id, nc;
	int mcard = 0;
	int posX1;
	static char keyupFlag=0;
	int focus = 0;
	int tmpvalue=0;

	switch (message)
	{
		case MSG_CREATE:
			//LoadBitmap(HDC_SCREEN,&delbmp,GetBmpPath("submenubg.jpg"));
			LoadBitmap(HDC_SCREEN,&delhintbmp,GetBmpPath("delhint.gif"));
			InitWindow(hWnd);
			UpdateWindow(hWnd,TRUE);
			//SetFocusChild(btndelall);

			/*dsl 2012.4.17*/
			//if (TEST_ONLY_ENROLL_PRIVILLEGE) {
			//	SendMessage(btndelall, MSG_ENABLE, 0, 0);
			//} else {
			//	focus = 1;
			//	SetFocusChild(btndelall);
			//}
			//change by zxz 2013-04-19
			focus = 1;
			SetFocusChild(btndelall);

			if (deluser.Password[0]){
				nopwd=0;
			} else {
				nopwd=1;
			}
			if (nopwd) {
				SendMessage(btndelpwd,MSG_ENABLE,0,0);
			} else if (focus==0){
				focus = 1;
				SetFocusChild(btndelpwd);
			}

			if(!gOptions.IsOnlyRFMachine) {
				if(FDB_GetTmpCnt(deluser.PIN)>0){
					nofp=0;
				} else {
					nofp=1;
				}
				if (nofp) {
					SendMessage(btndelfp,MSG_ENABLE,0,0);
				} else if (focus==0){
					focus = 1;
					SetFocusChild(btndelfp);
				}

			}
#ifdef FACE
			if(gOptions.FaceFunOn) {
				if(FDB_GetFaceTmp(deluser.PIN,0,NULL)) {
					noface=0;
				} else {
					noface=1;
				}

				if(noface) {
					SendMessage(btndelface,MSG_ENABLE,0,0);
				} else if (focus==0){
					focus = 1;
					SetFocusChild(btndelface);
				}
			}
#endif

			if(gOptions.RFCardFunOn) {
				memcpy(&mcard,deluser.Card,4);
				if(mcard) {
					nohid = 0;
				} else {
					nohid = 1;
				}
				if (nohid) {
					SendMessage(btndelhid,MSG_ENABLE,0,0);
				}else if (focus==0){
					focus = 1;
					SetFocusChild(btndelhid);
				}

			}

			if (gOptions.CameraOpen)
			{
				int ret;
				dellist = FDB_GetPhotoToList(0, 0, deluser.PIN2, 1, &ret, 0);
				if (ret) {
					nophoto = 0;
				} else {
					nophoto = 1;
				}
				if (nophoto) {
					SendMessage(btndelphoto, MSG_ENABLE, 0, 0);
				} else if (focus==0){
					focus = 1;
					SetFocusChild(btndelphoto);
				}
			}
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
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
				if(1 ==keyupFlag)
				{
					keyupFlag=0;
				}
				else
					break;
			}
			if (gOptions.KeyPadBeep)
				ExKeyBeep();

			if((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
			{
				SelectNext(hWnd,wParam);
				return 0;
			}
			else if (LOWORD(wParam)==SCANCODE_ESCAPE)
			{
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
				return 0;
			}
			break;

		case MSG_CLOSE:
			//UnloadBitmap(&delbmp);
			UnloadBitmap(&delhintbmp);
			DestroyMainWindow(hWnd);
			//MainWindowCleanup(hWnd);
			return 0;

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			switch (id) {
				case ID_EXIT:
					SendMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;
				case ID_CKBASE:
					if (!ProcessDeluser(delpin,hWnd,0))	UpdateWindow(hWnd,TRUE);
					break;
				case ID_CKFP:
					if (!ProcessDeluser(delpin,hWnd,1))	UpdateWindow(hWnd,TRUE);
					break;
				case ID_CKPWD:
					if (!ProcessDeluser(delpin,hWnd,2))	UpdateWindow(hWnd,TRUE);
					break;
				case ID_CKHID:
					if (!ProcessDeluser(delpin,hWnd,3))	UpdateWindow(hWnd,TRUE);
					break;
				case ID_CKPHOTO:
					if (!ProcessDeluser(delpin, hWnd, 4))	UpdateWindow(hWnd,TRUE);
					break;
#ifdef FACE
				case ID_CKFACE:
					if (!ProcessDeluser(delpin,hWnd,5))     
					{
						UpdateWindow(hWnd,TRUE);
					}
					break;
#endif
			}
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;
				char textstr[100];

				if (hdc == 0) {
					hdc = GetClientDC (hWnd);
					fGetDC = TRUE;
				}

				if (clip) {
					rcTemp = *clip;
					ScreenToClient (hWnd, &rcTemp.left, &rcTemp.top);
					ScreenToClient (hWnd, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect (hdc, &rcTemp);
				}

				FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
				FillBoxWithBitmap (hdc, 250+gOptions.ControlOffset, 0, 0, 0, &delhintbmp);

				if (deluser.Name[0])		//modify by jazzy 2008.07.26
				{
					unsigned char mynamename[100];

					memset(mynamename,0,100);
					Str2UTF8(tftlocaleid,(unsigned char *)deluser.Name, mynamename);

					sprintf(textstr,"%s:%s   %s:%s",LoadStrByID(MID_ACNO),deluser.PIN2,
							LoadStrByID(MID_NAME),mynamename);
				}
				else
					sprintf(textstr,"%s:%s", LoadStrByID(MID_ACNO), deluser.PIN2);

				tmpvalue=SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,PIXEL_lightwhite);

				if (fromRight==1)
				{
					TextOut(hdc, 50+gOptions.ControlOffset, 10, textstr);
					posX1=10+gOptions.ControlOffset;
				}
				else
				{
					TextOut(hdc, 10+gOptions.ControlOffset, 10, textstr);
					posX1=220+gOptions.ControlOffset;
				}
				if (delret==2)
				{
					TextOut(hdc, posX1, h_fp, LoadStrByID(MID_FPHADDEL));
					if (pwddelhint) TextOut(hdc, posX1, h_pwd, LoadStrByID(MID_PWDHADDEL));
					//For face
					if (facedelhint)TextOut(hdc, posX1, h_face, LoadStrByID(MID_FACEHADDEL));
					//end face
					if (hiddelhint)	TextOut(hdc, posX1, h_hid, LoadStrByID(MID_HIDHADDEL));
					if (photodelhint) TextOut(hdc, posX1, h_pic, LoadStrByID(MID_DELPHOTO_HINT));
					pwddelhint=0;	hiddelhint=0;	photodelhint=0;	fpdelhint=0;	//add by jazzy 2008.12.03
					facedelhint=0;
				}
				if (delret==3)
				{
					if (fpdelhint) TextOut(hdc, posX1, h_fp, LoadStrByID(MID_FPHADDEL));
					TextOut(hdc, posX1, h_pwd, LoadStrByID(MID_PWDHADDEL));
					if (facedelhint)TextOut(hdc, posX1, h_face, LoadStrByID(MID_FACEHADDEL));
					if (hiddelhint)	TextOut(hdc, posX1, h_hid, LoadStrByID(MID_HIDHADDEL));
					if (photodelhint) TextOut(hdc, posX1, h_pic, LoadStrByID(MID_DELPHOTO_HINT));
					pwddelhint=0;   hiddelhint=0;   photodelhint=0; fpdelhint=0;	//add by jazzy 2008.12.03
					facedelhint=0;
				}
				if (delret==4)
				{
					if (fpdelhint) TextOut(hdc, posX1, h_fp, LoadStrByID(MID_FPHADDEL));
					if (pwddelhint)	TextOut(hdc, posX1, h_pwd, LoadStrByID(MID_PWDHADDEL));
					if (facedelhint)TextOut(hdc, posX1, h_face, LoadStrByID(MID_FACEHADDEL));
					TextOut(hdc, posX1, h_hid, LoadStrByID(MID_HIDHADDEL));
					if (photodelhint) TextOut(hdc, posX1, h_pic, LoadStrByID(MID_DELPHOTO_HINT));
					pwddelhint=0;   hiddelhint=0;   photodelhint=0; fpdelhint=0;	//add by jazzy 2008.12.03
					facedelhint=0;
				}
				if (delret==5)
				{
					if (fpdelhint) TextOut(hdc,posX1, h_fp, LoadStrByID(MID_FPHADDEL));
					if (pwddelhint) TextOut(hdc,posX1, h_pwd, LoadStrByID(MID_PWDHADDEL));
					if (facedelhint)TextOut(hdc, posX1, h_face, LoadStrByID(MID_FACEHADDEL));
					if (hiddelhint) TextOut(hdc, posX1, h_hid, LoadStrByID(MID_HIDHADDEL));
					TextOut(hdc, posX1, h_pic, LoadStrByID(MID_DELPHOTO_HINT));
					pwddelhint=0;   hiddelhint=0;   photodelhint=0; fpdelhint=0;	//add by jazzy 2008.12.03
					facedelhint=0;
				}
				if (delret==6)
				{
					if (fpdelhint)
					{
						TextOut(hdc,posX1, h_fp, LoadStrByID(MID_FPHADDEL));
					}
					if(pwddelhint)
					{
						TextOut(hdc,posX1, h_pwd, LoadStrByID(MID_PWDHADDEL));
					}
					TextOut(hdc, posX1, h_face, LoadStrByID(MID_FACEHADDEL));
					if (hiddelhint)
					{
						TextOut(hdc, posX1, h_hid, LoadStrByID(MID_HIDHADDEL));
					}
					if (photodelhint)
					{
						TextOut(hdc, posX1, h_pic, LoadStrByID(MID_DELPHOTO_HINT));
					}
					pwddelhint=0;
					hiddelhint=0;
					photodelhint=0;
					fpdelhint=0;
					facedelhint=0;
				}
				ReleaseDC (hdc);
				return 0;
			}
	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

int CreateDelUserWindow(HWND hOwner, char *pin2, int pin)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	memset(&deluser, 0, sizeof(TUser));
	if (FDB_GetUserByCharPIN2(pin2, &deluser)==NULL)
		return 0;
	//	printf("deluser pin %d  pin2 %s\n",deluser.PIN,deluser.PIN2);
	delpin = deluser.PIN;

	printf("%s, %d, hOwner=0x%X\n", __FUNCTION__, __LINE__, hOwner);

	delret=0;
	pwddelhint=0;
	fpdelhint=0;
	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = LoadStrByID(MID_DELUSER);
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = Delwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = COLOR_black;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID)
		return -1;
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	printf("start:%s, %d\n", __FUNCTION__, __LINE__);
	MainWindowThreadCleanup(hMainWnd);
	printf("end:%s, %d\n", __FUNCTION__, __LINE__);

	return delret;

}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

