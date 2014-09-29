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
#include <dirent.h>
#include "exfun.h"
#include "ssrcommon.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>

#include "libcam.h"
#include "flashdb.h"
#include "flashdb2.h"
#include "zkface.h"
#include "facedb.h"
#include "bmp.h"
#include "main.h"

#define IDC_INPUTNO	9113
#define IDC_NUMNO	9114
#define IDC_OK		9115
#define IDC_CANCEL	9116
#define IDC_FACE_TIMER	9120
#define IDC_WND_TITLE	9999


extern void InitParam(int mod);
extern int SaveAttLog(U16 pin, int VerifiedMethod, HDC hdc, HWND hWnd);
int ShowCaptureFace(HWND hWnd , HDC hdc , int flag);
void ShowFaceQuality(HWND hWnd ,HDC hdc ,int Quality, PBITMAP bmpbg,int flag);
void ShowFaceHint(HWND hWnd , int Fhint ,HDC hdc , PBITMAP bmpbg,int flag);
void UpdateFaceWindow(HWND hWnd , int flag);
int CheckFaceStkey(HWND hWnd, int index ,int flag);
int CreatFacePinWindow(HWND hWnd , int mode);
void ProcessDisplayAfterVerify(HWND hWnd, int userID);
extern long SSR_MENU_MAINMENU(HWND);
extern void SwitchMsgType(int Enabled);
extern void ShowSelectStateHint(HWND hWnd);

extern PLOGFONT gfont;	//display font
extern char KeyBuffer[24];	//µ±Ç°ÊäÈëµÄ¿¼ÇÚºÅÂë¡ª¡ª£¨ÃÜÂëÑéÖ¤£©
extern int admflag;			//administrator verify flag
extern char ownwkcd[MAX_WORKCODE_LEN+1];
extern int FaceRegState;
extern BITMAP facetestdbkg;
extern int g_usverify;
extern int WaitAdminRemainCnt;
extern int WaitAdmVerifyRetryCnt;
extern int ledbool;
extern int menuflag;
extern int WaitAdmins[MaxAdminVerify];
extern int VerifyLogState;

//HDC g_hdcFace;
BITMAP fvdbkg,fvbar,  fvphoto;

RECT Facerect={5,40,70,70};	
RECT title_rc={0,0,320,30};
RECT hint_rc={245,40,315,218};
RECT quality_rc={0,40,80,240};

static int FaceVFHint=0;
static int CurHint=0;

BITMAP FaceImage;
int InitShowFaceImage(void)
{
	FaceImage.bmType=0;
        FaceImage.bmBitsPerPixel=16;
        FaceImage.bmBytesPerPixel=2;
        FaceImage.bmAlpha=0;
        FaceImage.bmColorKey=0;
        FaceImage.bmWidth=480;
        FaceImage.bmHeight=640;
        FaceImage.bmPitch=FaceImage.bmBytesPerPixel*FaceImage.bmWidth;
        FaceImage.bmBits=NULL;
        FaceImage.bmAlphaPixelFormat=0;
	CurHint=0;
	return 1;
}

// flag=0: Verify, flag=1: Enroll ; flag=2 :test
int ShowCaptureFace(HWND hWnd , HDC hdc , int flag)
{	
	int x,y,x1,y1 ;
	int Fy,Fw,Fh;
	int ret=-1;
	

	ret=FaceDetection((char*)face_bmp);
	if(0==ret)  // get image failed!
	{
		usleep(100);
		return 0;
	}	
	else 
	{
		hdc =GetClientDC(hWnd);
		Bmp8ToRGB565((unsigned char*)(face_bmp), FACE_IMG_W, FACE_IMG_H);
		FaceImage.bmBits=(unsigned char*)(face_bmp);
		if(flag==1) //enroll
			FillBoxWithBitmap(hdc,89,29,480*0.3,640*0.3,&FaceImage);
		else
			FillBoxWithBitmap(hdc,82,29,480*0.33,640*0.33,&FaceImage);
		if(1==ret)
		{
			if(flag==1)
			{
				ExBeep(2);
				ExBeep(2);
			}
			ReleaseDC(hdc);
			return 0;
		}


		SetPenWidth(hdc, 5);
                SetPenCapStyle(hdc, PT_CAP_BUTT);
                SetPenColor(hdc, PIXEL_green);

		Fy=Position[POSITIOV_FACE_Y];
		Fw=Position[POSITIOV_FACE_WIDTH];
                Fh=Position[POSITIOV_FACE_HEIGHT];
		
		if(flag==1) //enroll
		{
			x=87+Position[POSITIOV_FACE_X]*0.3;
               	 	y=24+Fy*0.3;
                	x1=x+Fw*0.3;
                	y1=y+Position[POSITIOV_FACE_HEIGHT]*0.3;
                        if(x< 90)
                                x=90;
                        if(y< 29)
                                y=29;
                        if(x1> 230)
                                x1=230;
			if(y1>217)
				y1=217;
		}
		else
		{
			x=77+Position[POSITIOV_FACE_X]*0.34;
               	 	y=24+Fy*0.34;
                	x1=x+Fw*0.34;
                	y1=y+Position[POSITIOV_FACE_HEIGHT]*0.34;
			
			if(x< 87)
				x=87;
			if(y< 24)
				y=24;
			if(x1> 234)
				x1=234;
		}

		//Rectangle
		LineEx(hdc,x,y,x1,y);
                LineEx(hdc,x,y,x,y1);
                LineEx(hdc,x1,y,x1,y1);
                LineEx(hdc,x,y1,x1,y1);
		//BitBlt(hdc,0,0,320,240,HDC_SCREEN,0,0,0);

		x=FaceDistance;
		if(!FaceDistance)
                        x=HINT_CHG ;
		if(flag==1) //enroll
		{
			ShowFaceQuality(hWnd ,hdc ,FaceQuality, get_submenubg_jgp(), 1);
			if(FaceDistance)
				ShowFaceHint(hWnd , x ,hdc , get_submenubg_jgp() , 1);
			else
				ShowFaceHint(hWnd ,FaceRegState ,hdc , get_submenubg_jgp() , 1);
		}
		else if(flag==2) //test
		{
			ShowFaceQuality(hWnd ,hdc ,FaceQuality, &facetestdbkg,2);
			//ShowFaceHint(hWnd , x ,hdc , &facetestdbkg , 2);
		}
		else //verify 
		{
			ShowFaceQuality(hWnd ,hdc ,FaceQuality, &fvdbkg,0);
			ShowFaceHint(hWnd , x ,hdc , &fvdbkg , 0);
		}
		ReleaseDC(hdc);

		if(3==ret) //get face template
		{
			if(flag == 2)
				return 1;
			else
				return FaceExtractProc();
		}
	}
	return 0;
}

// flag=1: enroll :  flag=0: verify : flag=2: test
void ShowFaceQuality(HWND hWnd ,HDC hdc ,int Quality, PBITMAP bmpbg,int flag)
{
	RECT rect={5,40,70,80};
	char buffer[300];
	int i;
	int Cnt;
	int y=235;
	int num=22, fail=11;

	if(flag==1) //enroll
	{
		y=212;
		num=18;
		fail=9;
	}
	
	if(flag==1) //enroll
		FillBoxWithBitmapPart (hdc, quality_rc.left,quality_rc.top,quality_rc.right-quality_rc.left,\
                                        quality_rc.bottom-quality_rc.top-20, bmpbg->bmWidth, bmpbg->bmHeight, \
                                        bmpbg, quality_rc.left, quality_rc.top-MENUBAR_HEIGHT);
	else
		FillBoxWithBitmapPart (hdc, quality_rc.left,quality_rc.top,quality_rc.right-quality_rc.left,\
                                        quality_rc.bottom-quality_rc.top, bmpbg->bmWidth, bmpbg->bmHeight, \
                                        bmpbg, quality_rc.left, quality_rc.top-MENUBAR_HEIGHT);

	SelectFont(hdc,gfont);
        SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%s:%d",LoadStrByID(MID_VIDEO_QULITY),Quality);
	DrawText(hdc,buffer,-1,&rect,DT_LEFT | DT_WORDBREAK);
        //TextOut(hdc,quality_rc.left+2,quality_rc.top+5,buffer);

	SetPenWidth(hdc, 5);
        SetPenCapStyle(hdc, PT_CAP_BUTT);
        SetPenColor(hdc, PIXEL_darkyellow);

	Cnt=num*Quality/100;

	for(i=0;i<Cnt;i++)
	{
		if(i==fail)
			SetPenColor(hdc, PIXEL_blue);
		LineEx(hdc,20,y,60,y);
		y-=7;
	}

}

// flag=1: enroll ; flag=2: test; flag=0: verify
void ShowFaceHint(HWND hWnd , int Fhint ,HDC hdc , PBITMAP bmpbg ,int flag)
{
	RECT rect;
	char* txt=NULL;

#if 0
	if(CurHint == Fhint && flag != 2)
	{
		return ;
	}
#endif
	CurHint=Fhint;
	if(flag==1) //reg
		FillBoxWithBitmapPart (hdc, hint_rc.left-4,hint_rc.top,320-hint_rc.left+4,\
                                                hint_rc.bottom-hint_rc.top+2, bmpbg->bmWidth, bmpbg->bmHeight, \
                                                bmpbg, hint_rc.left-4, hint_rc.top-MENUBAR_HEIGHT);
	else
		FillBoxWithBitmapPart (hdc, hint_rc.left-4,hint_rc.top,320-hint_rc.left+4,\
                                                hint_rc.bottom-hint_rc.top+2, bmpbg->bmWidth, bmpbg->bmHeight, \
                                                bmpbg, hint_rc.left-4, hint_rc.top-MENUBAR_HEIGHT);

        SelectFont(hdc,gfont);
        SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);

	if(flag==2) //test
	{
		char buf[300];	
		sprintf(buf,"Exprose:%d \nBlank:%d\nGray:%d\nImage:%d",\
					 Quality[QUALITY_DEFINITION],\
					 Quality[QUALITY_BANLANCE],\
					 Quality[QUALITY_GRAYLEVEL],\
					 FaceGray);
		DrawText(hdc,buf,-1,&hint_rc,DT_LEFT | DT_WORDBREAK);
		return ;
	}
	

	switch(Fhint)
	{
                case HINT_FFAR:
                        txt=LoadStrByID(FACE_FAR);
                        break;
                case HINT_FNEAR:
                        txt=LoadStrByID(FACE_NEAR);
                        break;

		case HINT_FCHG:
                case HINT_CHG:
                        txt=LoadStrByID(FACE_CHG_POS); 
                        break;
		case FACE_REG_PREPARE:
			txt=LoadStrByID(FACE_REG_PREPARE);
			break;
		case FACE_REG_FRONT:
			txt=LoadStrByID(FACE_REG_FRONT);
			break;
		case FACE_REG_SCREEN:
			txt=LoadStrByID(FACE_REG_SCREEN);
			break;
		case FACE_REG_LEFT:
			txt=LoadStrByID(FACE_REG_LEFT);
			break;
		case FACE_REG_RIGHT:
			txt=LoadStrByID(FACE_REG_RIGHT);
			break;
		case FACE_REG_CAMERA:
			txt=LoadStrByID(FACE_REG_CAMERA);
			break;

		default:
			break;
	}
	if(txt)
	{
		DrawText(hdc,txt,-1,&hint_rc,DT_LEFT | DT_WORDBREAK);
	}

	if(!flag)
	{
		char buffer[100];
		memcpy(&rect,&hint_rc,sizeof(RECT));
		rect.top =rect.bottom -50;
		if(FaceVFHint == FACE_CUR_GROUP)
			sprintf(buffer,"%s%d",LoadStrByID(FaceVFHint),CurFaceGroup);
		else
			sprintf(buffer,"%s",LoadStrByID(FaceVFHint));
		DrawText(hdc,buffer,-1,&rect,DT_LEFT | DT_WORDBREAK);
	}
}


static int ShowUserPhoto(HWND hWnd, HDC dc ,char* pin2)
{
	HDC hdc=dc;
	BITMAP usrbmp;
	char photoname[100];
	int mount = -1, ret = 0;

	memset(photoname,0,sizeof(photoname));
        if (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0)
        {
                DoUmountUdisk();
                mount=DoMountUdisk();
                if(mount!=0) return 0;
        }
	if(!dc)
		hdc=GetClientDC(hWnd);

        sprintf(photoname, "%s%s.jpg", GetPhotoPath(""), pin2);
	printf("user photo path:%s\n", photoname);
	if(!LoadBitmap(hdc, &usrbmp, photoname))
	{
		int width,height;
		width=usrbmp.bmWidth;
		height=usrbmp.bmHeight;
		if(width<120)
		{
			height=height*120/width;
			width=120;
		}
		if(width>120)
		{
			width=120;
			height=usrbmp.bmHeight*120/usrbmp.bmWidth;
			if(height>160)
			{
				width=width*160/height;
				height=160;
			}
		}
		if(height>160)
		{
			width=width*160/height;
			height=160;
		}
		FillBoxWithBitmap(hdc,180+(120-width)/2, 60+(160-height)/2,width,height,&usrbmp);
		UnloadBitmap(&usrbmp);
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	if(!dc)
		ReleaseDC(hdc);
	return ret;
}

extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char* title, char* text, ...);
extern void OutputPrinterFmt1(int pin);
extern void OutputPrinterFmt2(int pin);
extern void OutputPrinterFmt3(int pin);
extern void OutputPrinterFmt4(int pin);
extern void OutputPrinterFmt5(int pin);
extern void OutputPrinterFmt6(int pin);
extern void OutputPrinterFmt7(int pin);
static int ShowFaceVerifyInfo(HWND hWnd, int flag)
{
	HDC hdc;
	char buffer[300];
	char temp[300];
	TUser user;
	int wkflag=0;

	if (admflag==0 && gOptions.MustChoiceInOut && gOptions.AttState<0)
	{
		ShowSelectStateHint(hWnd);
		UpdateFaceWindow(hWnd ,0);
		PostMessage(hWnd, MSG_CLOSE, 0, 0);
		return 0;
	}
	if(FDB_GetUser(FacePin,&user) == NULL)
		return 0;

	if(curWorkCode && ifUseWorkCode && gOptions.MustChoiceWorkCode)
		wkflag=1;

	if (admflag==0 &&ifUseWorkCode && !curWorkCode && !ownwkcd[0] && gOptions.MustChoiceWorkCode)
	{
		SwitchMsgType(0);
		wkcdwinparam = 1;
		memset(ownwkcd, 0, sizeof(ownwkcd));
		curWorkCode = CreateWorkCodeManageWindow(hWnd);
		wkcdwinparam = 0;
		SwitchMsgType(1);
		UpdateFaceWindow(hWnd ,0);
		if (!curWorkCode && !ownwkcd[0])
		{
			PostMessage(hWnd, MSG_CLOSE, 0, 0);
			return 0;
		}
	}
	hdc=GetClientDC(hWnd);
	UpdateFaceWindow(hWnd , 1);
        SelectFont(hdc,gfont);
	SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);

	if(gOptions.IsSupportPhoto && gOptions.ShowPhoto)
	{
		if(!ShowUserPhoto(hWnd, hdc,user.PIN2))
		{
			FillBoxWithBitmap (hdc, 180,60,480*0.25,640*0.25,&FaceImage);
		}
	}
	else
		FillBoxWithBitmap (hdc, 180,60,480*0.25,640*0.25,&FaceImage);


	if(WaitAdminRemainCnt && admflag)
	{
		if(user.PIN && ISADMIN(user.Privilege))
		{
			if(ISINVALIDUSER(user))          //·Ç·¨¹ÜÀíÔ±
                        {
				TextOut(hdc, 10, 80, LoadStrByID(HID_PRI_INVALID));
				DelayMS(500);
                                if(--WaitAdmVerifyRetryCnt)     //ÖØÐÂÑéÖ¤¹ÜÀíÔ±
                                {
                                 	InitParam(1);
					return 0;
                                 }
                                 else
                                    PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			else if(WaitAdminRemainCnt>=1)	//
			{
				WaitAdminRemainCnt=0;
				ledbool=TRUE;

				menuflag=1;
				KillTimer(hWnd, IDC_FACE_TIMER);
			#ifndef ZEM600
				KillTimer(hWnd, IDC_VF_TIMER);
				KillTimer(hMainWindowWnd,IDC_TIMER);
			#else
				EnableMsgType(MSG_TYPE_TIMER, 0);
			#endif
				EnableMsgType(MSG_TYPE_FINGER, 0);//changed by luoxw 20120307
				SSR_MENU_MAINMENU(hWnd);
				EnableMsgType(MSG_TYPE_FINGER, 1);//changed by luoxw 20120307
				menuflag=0;
			#ifndef ZEM600
				SetTimer(hMainWindowWnd,IDC_TIMER,100);
			#else
				EnableMsgType(MSG_TYPE_TIMER, 1);
			#endif

				PostMessage(hWnd, MSG_CLOSE, 0, 0);
			}
			else if(WaitAdminRemainCnt>0)
			{
				int i;
				for(i=0;i<MaxAdminVerify;i++)
					if(WaitAdmins[i]==user.PIN) break;
				if(i==MaxAdminVerify)
				{
					WaitAdminRemainCnt--;
				}
			}	
		}
		else
		{
			TextOut(hdc,10,80,LoadStrByID(HID_ACCESSDENY));
			DelayMS(500);
			if(--WaitAdmVerifyRetryCnt)	//ÖØÐÂÑéÖ€¹ÜÀíÔ±
			{
				InitParam(1);
			}
			else
				PostMessage(hWnd, MSG_CLOSE, 0, 0);
		}
		return 0;
	}

	exceedflag=0;
	if (gOptions.AlarmAttLog || !LoadInteger("ISLR", 0))
	{
		if(CurAttLogCount>=gOptions.MaxAttLogCount*10000)
			exceedflag=1;
		else if(CurAttLogCount+gOptions.AlarmAttLog>=gOptions.MaxAttLogCount*10000)
			exceedflag=2;
		if(gOptions.IclockSvrFun && (exceedflag==1) && gOptions.DelRecord>0)
                {
                	FDB_DelOldAttLog(gOptions.DelRecord);
                        if(exceedflag==1) exceedflag=0;
		}
	}

	g_usverify=15;
	SaveAttLog(FacePin, g_usverify, HDC_SCREEN, hWnd);

	memset(buffer,0, sizeof(buffer));
	sprintf(buffer,"%s: %d",LoadStrByID(MID_FACE_SCORE),FaceVScore);
	TextOut(hdc,70,40,buffer);

	VerifiedPIN=user.PIN;	
	//pin2
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%s: %s", LoadStrByID(MID_ACNO), user.PIN2);
        TextOut(hdc,10,80,buffer);
	//name
	memset(buffer,0,sizeof(buffer));
	memset(temp,0,sizeof(temp));
	memcpy(temp, user.Name,24);
	sprintf(buffer,"%s: %s",LoadStrByID(MID_NAME),temp);
        TextOut(hdc,10,110,buffer);

	//verify method
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"%s: %s",LoadStrByID(HID_SVERIFY),LoadStrByID(MID_FACE));
        TextOut(hdc,10,140,buffer);

	if(VerifyLogState&LOG_REPEAT)
	{
		TextOut(hdc, 10, 170, LoadStrByID(HID_ALREADY));
		BitBlt (hdc, 0, 0,320,240, HDC_SCREEN, 0, 0, 0);
	}
	else
	{
		TextOut(hdc, 10, 170, LoadStrByID(HID_VSUCCESS));
		BitBlt (hdc, 0, 0,320,240, HDC_SCREEN, 0, 0, 0);
	}

	if(gOptions.VoiceOn)
	{
		if(exceedflag!=1)
		{
			if(VerifyLogState<=LOG_REPEAT)
			{
				if(VerifyLogState&LOG_REPEAT)
				{
					ExPlayVoice(VOICE_ALREADY_LOG);
				}
				else
				{
					ExPlayVoice(VOICE_THANK);
				}
			}
		}
	}
	else
	{
		ExBeep(1);
	}
#if 1
                         //串口打印(liming)
        if (gOptions.PrinterFunOn && !(VerifyLogState&LOG_REPEAT))
        {
		if(gOptions.PrinterOn==1)
			OutputPrinterFmt1(FacePin);
		else if(gOptions.PrinterOn==2)
			OutputPrinterFmt2(FacePin);
		else if(gOptions.PrinterOn==3)
			OutputPrinterFmt3(FacePin);
		else if(gOptions.PrinterOn==4)
			OutputPrinterFmt4(FacePin);
		else if(gOptions.PrinterOn==5)
		{
    			if(myMessageBox1(hWnd,MB_OKCANCEL | MB_ICONQUESTION,LoadStrByID(MID_APPNAME),LoadStrByID(MID_PT_HINT))==IDOK)
			{
				OutputPrinterFmt5(FacePin);
			}
		}
		else if(gOptions.PrinterOn==6)
			OutputPrinterFmt6(FacePin);
		else if(gOptions.PrinterOn==7)
			OutputPrinterFmt7(FacePin);
	}
#endif
	InitParam(0);
	FaceVFHint = FACE_CUR_GROUP;

	if(exceedflag==1)
	{
		TextOut(hdc,10,200,LoadStrByID(HID_EXCEED));
		if(gOptions.VoiceOn)
			ExPlayVoice(VOICE_NO_LOG_RECSPACE);
		else
			ExBeep(1);
	}
	else if(exceedflag==2)
	{
		TextOut(hdc,10,200,LoadStrByID(HID_LEFTSPACE));
                sprintf(buffer, "%d", gOptions.MaxAttLogCount*10000-FDB_CntAttLog());
                TextOut(hdc,20,225,buffer);
	}

	curWorkCode = 0;
	memset(ownwkcd, 0, sizeof(ownwkcd));
	ReleaseDC(hdc);

	if(wkflag)
	{
		DelayMS(800);
		PostMessage(hWnd, MSG_CLOSE, 0, 0);
	}

	return 1;
}

/***************************************
* flag=0; verify wnd
* flag=1; info wnd
* flag=2; input pin2
****************************************/
void UpdateFaceWindow(HWND hWnd , int flag)
{
	HDC hdc=GetClientDC(hWnd);

	InvalidateRect(hWnd,&Facerect,TRUE);
	SelectFont(hdc,gfont);
        SetTextColor(hdc,PIXEL_lightwhite);
        SetBkMode(hdc,BM_TRANSPARENT);

	if(flag==1) // verify success ! show user info 
	{
		FillBoxWithBitmap (hdc, 0, MENUBAR_HEIGHT,320, 240, get_submenubg_jgp());
		FillBoxWithBitmap (hdc, 177, 55, 126, 170, &fvphoto);
	}
	else // verify face 
	{
		FillBoxWithBitmap (hdc, 0, MENUBAR_HEIGHT,320, 240, &fvdbkg);
	}
	FillBoxWithBitmap (hdc, 0, -1, 0, MENUBAR_HEIGHT, &fvbar);
	DrawText(hdc,(char*)LoadStrByID(MID_FACE_VERIFY),-1,&title_rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	ReleaseDC(hdc);
}

static int FaceWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	switch (message) 
	{
		case MSG_CREATE: 

			InitShowFaceImage();				
			UpdateFaceWindow(hWnd,0);
			VerifiedPIN=0;
			SetTimer(hWnd, IDC_FACE_TIMER, IDC_TIMER_NUM);
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
                        if(fGetDC) ReleaseDC (hdc);
                        break;
                }
                case MSG_PAINT:
			hdc = BeginPaint(hWnd);
			EndPaint(hWnd,hdc);
                        return 0;

		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
                        if (gOptions.KeyPadBeep)
                                ExKeyBeep();
			if(LOWORD(wParam) == SCANCODE_ESCAPE)
			{
				PostMessage(hWnd, MSG_COMMAND, IDC_CANCEL, 0);
			}
			else
			{
				int stkyidx = isShortKey(LOWORD(wParam));
                                if (stkyidx)
				{
					KillTimer(hWnd, IDC_FACE_TIMER);
					CheckFaceStkey(hWnd, stkyidx,0);
					SetTimer(hWnd, IDC_FACE_TIMER, IDC_TIMER_NUM);
				}
			}
			break;

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_OK:
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;

				case IDC_CANCEL:
						PostMessage(hWnd, MSG_CLOSE, 0, 0);
					break;
			}
			break;

		case MSG_TIMER:
			if (wParam == IDC_FACE_TIMER)
			{
				KillTimer(hWnd, IDC_FACE_TIMER);
				if(ismenutimeout)
				{
					PostMessage(hWnd, MSG_CLOSE, 0, 0);
					return 0;
				}
				if(ShowCaptureFace( hWnd, HDC_SCREEN ,0))
				{
					ExBeep(1);
					SetMenuTimeOut(time(NULL));
					if(VerifyFaceWithTmp( KeyBuffer ,Extracttmp , FaceLen)) // verify ok
					{
                                                if(ShowFaceVerifyInfo(hWnd,1))
						{
                                        		DelayMS(500);
							if(gOptions.IsSupportSMS)
							{
								char  smsBuffer[512];
								if(CheckUserSMS(VerifiedPIN, (BYTE *)smsBuffer))
								{
									PostMessage(hWnd, MSG_CLOSE, 0, 0);
									return 0;
								}
							}
							UpdateFaceWindow(hWnd,0);
						}
					}
                                }
				SetTimer(hWnd, IDC_FACE_TIMER, IDC_TIMER_NUM);
			}
			return 1;

		case MSG_IDLE:
			{
				timer_t ctimer=time(NULL);
                                if(((int)ctimer >= (gMenuTimeOut+60)) && !busyflag)
                                {
                                        ismenutimeout=1;
                                        PostMessage(hWnd,MSG_CLOSE,0,0);
                                }
			}
			break;
		case MSG_CLOSE:
			KillTimer(hWnd, IDC_FACE_TIMER);
                        DestroyMainWindow(hWnd);
		        return 0;
	}
    
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

// flag=0; init keybuffer ;  flag=1: 1:1 not init keybuffer
int CreateFaceVerifyWindow(HWND hWnd , int flag)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	char keytmp[24];
	
	vfwndflag=1;
	hWnd = GetMainWindowHandle(hWnd);
	CreateInfo.dwStyle = WS_VISIBLE ;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "Face Verify";
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = FaceWinProc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = 0x00FFA2BE;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hWnd;

	ismenutimeout=0;
	SetMenuTimeOut(time(NULL));
	LoadBitmap(HDC_SCREEN, &fvdbkg, GetBmpPath("mainmenu.jpg"));
	LoadBitmap(HDC_SCREEN, &fvbar, GetBmpPath("bar.bmp"));
	//LoadBitmap(HDC_SCREEN, &fvbg, GetBmpPath("submenubg.jpg"));
	LoadBitmap(HDC_SCREEN, &fvphoto, GetBmpPath("photo.jpg"));

	
        hMainWnd = CreateMainWindow(&CreateInfo);
        if (hMainWnd == HWND_INVALID)
	{
		vfwndflag=0;
		UnloadBitmap(&fvdbkg);
		UnloadBitmap(&fvbar);
		//UnloadBitmap(&fvbg);
		UnloadBitmap(&fvphoto);
                return 0;
	}

	if(flag)
	{
		memcpy(keytmp, KeyBuffer, 24);
		InitParam(1);
		memcpy(KeyBuffer, keytmp,24);
		FaceVFHint=MID_ONE_FACE;
	}
	else
	{
		InitParam(1);
		FaceVFHint=FACE_CUR_GROUP;
	}	

	EnableMsgType(MSG_TYPE_FINGER,0);
        EnableMsgType(MSG_TYPE_HID,0);
	EnableMsgType(MSG_TYPE_MF,0);
	EnableMsgType(MSG_TYPE_ICLASS,0);

        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&msg, hMainWnd))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
        MainWindowThreadCleanup(hMainWnd);

	InitParam(1);
	vfwndflag=0;
	UnloadBitmap(&fvdbkg);
	UnloadBitmap(&fvbar);
	//UnloadBitmap(&fvbg);
	UnloadBitmap(&fvphoto);

        return 1;
}

static int  StkeyFaceGroupVerifyProc(HWND hWnd, int Group ,int flag)
{
        if( Group > 0 && Group <= gFaceGroupNum)
        {
                CurFaceGroup=Group;
                FDB_LoadFaceGroupTmp(CurFaceGroup);
                if(flag)
                        CreateFaceVerifyWindow(hWnd , 0);
                return 1;
        }
        return 0;
}
//flag=0 , int verify wnd,  flag=1, not in verify wnd
int ProcFaceShortKey(HWND hWnd, int keyfun,int flag)
{
	int ret=0;
	if(keyfun)
        {
		switch(keyfun)
		{
			case STK_FUN:
				if(flag)
				{
					CreateFaceVerifyWindow(hWnd , 0);
				}
				break;
			case STK_VERIFY: //1:1
				InitParam(1);
				ret=CreatFacePinWindow(hWnd,0);
				if(ret==2) //1:1
				{
					FaceVFHint=MID_ONE_FACE;
					if(flag)
						CreateFaceVerifyWindow(hWnd , 1);
				}
				if(!flag)
					UpdateFaceWindow(hWnd,0);
				break;
			case STK_GMATCH:
				InitParam(1);
				ret=CreatFacePinWindow(hWnd,1);
				if(ret==1) //1:G
				{
					CurFaceGroup=atoi(KeyBuffer);
                                        FDB_LoadFaceGroupTmp(CurFaceGroup);
                                        memset(KeyBuffer,0,sizeof(KeyBuffer));
                                        FaceVFHint=FACE_CUR_GROUP;
					StkeyFaceGroupVerifyProc(hWnd, CurFaceGroup,flag);
				}
				if(!flag)
					UpdateFaceWindow(hWnd,0);
				break;
			case STK_GROUP1:
				StkeyFaceGroupVerifyProc(hWnd,1,flag);
				break;
			case STK_GROUP2:
				StkeyFaceGroupVerifyProc(hWnd,2,flag);
				break;
			case STK_GROUP3:
				StkeyFaceGroupVerifyProc(hWnd,3,flag);
				break;
			case STK_GROUP4:
				StkeyFaceGroupVerifyProc(hWnd,4,flag);
				break;
			case STK_GROUP5:
				StkeyFaceGroupVerifyProc(hWnd,5,flag);
				break;
			default:
				break;
		}
		return 1;
	}
	return 0;
}
int CheckFaceStkey(HWND hWnd, int index,int flag)
{
        TSHORTKEY mystkey;
        if(!index) return 0;

        memset(&mystkey,0,sizeof(TSHORTKEY));
        if(FDB_GetShortKey(index, &mystkey)!=NULL)
        {
                if( mystkey.keyFun>= STK_FUN && mystkey.keyFun <= STK_GROUP5)
		{
			if(flag==0 && mystkey.keyFun == STK_FUN)
				return 0;
                        return ProcFaceShortKey(hWnd, mystkey.keyFun,flag);
		}
        }
	return 0;
}


#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

