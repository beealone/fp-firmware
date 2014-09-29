/*
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */

#include <minigui/common.h>

#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "ssrpub.h"
#include <minigui/tftmullan.h>
#include "utils.h"
#include "ssrinfo.h"

#include "exfun.h"
#include "modem/modem.h"
#include "tftmsgbox.h"
#include "libcam.h"
#include "truncdate.h"
#include "pushapi.h"

#define PAGE_BASE   1
#define PAGE_RECORD 2
#define PAGE_DEVICE 3
#define PAGE_BAT	4

#ifdef FACE
#define PAGE_FACE       5
#endif
#define PAGE_OTHER	6

#define IDC_PROPSHEET   800
#define IDC_SYSINFO     801

static BITMAP line0,line1;
static PLOGFONT infolf;
static BITMAP bat0,bat1,bat2,bat3,bat4,bat5,bat6, bat7;
static int InfoItem;
static int Infoloadflag;//add by zxz 2012-10-22
static int InfoUserCnt;
static int InfoPwdCnt;
static int InfoManCnt;
static int InfoTmpCnt;
static int InfoAttCnt;
int gPageCnt = 0;

static DLGTEMPLATE PageSysInfo = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 0, 0, 0, 0, "", 0, 0, 1, NULL, 0};
static CTRLDATA CtrlSysInfo [] =
{
	{CTRL_STATIC, WS_VISIBLE | SS_LEFT, 1, 1, 310, 205, IDC_SYSINFO, "", 0}
};

unsigned char tflag=0,tbatmod;
static RECT rect0={218, 18, 300, 45};

extern int getmac(char *macaddr1);

static void DrawLine(HDC hdc,int x,int y,int num,int flage)
{
	int cnt=0;
	if(flage==0)
	{
		for(cnt=0;cnt<num;++cnt)
			FillBoxWithBitmap(hdc,x,y-cnt,55,1,&line0);
	}
	else
	{
		for(cnt=0;cnt<num;++cnt)
			FillBoxWithBitmap(hdc,x,y-cnt,55,1,&line1);
	}
}

static void DrawLine2(HDC hdc,int x,int y,int num,int flage)
{
	int cnt=0;
	if(flage==0)
	{
		for(cnt=0;cnt<num;++cnt)
			FillBoxWithBitmap(hdc,x,y-cnt,30,1,&line0);
	}
	else
	{
		for(cnt=0;cnt<num;++cnt)
			FillBoxWithBitmap(hdc,x,y-cnt,30,1,&line1);
	}
}

static int GetInfoText(HDC hdc,int type)
{
	int cUser=0,cMan=0,cPwd=0,cTmp=0,cAtt=0,cManage=0, tmpvalue =0;
	char buf[30]={0};
	char pdate[32]={0},sn[16]={0},vender[32]={0},device[16]={0},fpver[32]={0},fwver[32]={0};
	char nv[64]={0};
	int mTmp=0,mAtt=0,mManage=0,mUser=0;
	float mf=0.0;
	unsigned char macstr[256]={0};

	if (!getmac((char*)macstr)) {
		memset(macstr,0,sizeof(macstr));
	}

	SetTextColor(hdc,PIXEL_black);
	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	if (gfont1==NULL) {
		SelectFont(hdc,infolf);
	} else {
		SelectFont(hdc,gfont1);
	}
	if(!Infoloadflag){
		InfoUserCnt = FDB_CntUserByMap();
	}
	cUser = InfoUserCnt;

	if(!gOptions.IsOnlyRFMachine) {
		if(!Infoloadflag){
			InfoTmpCnt = FDB_CntTmp();
		}
		cTmp = InfoTmpCnt;
		
		mTmp=LoadInteger("~MaxFingerCount",0)*100;
	}
	if(!Infoloadflag){
    		InfoAttCnt = FDB_CntAttLog();
	}
	cAtt = InfoAttCnt;

	mAtt=LoadInteger("~MaxAttLogCount",0)*10000;
	mUser=LoadInteger("~MaxUserCount",0)*100;

	if(!Infoloadflag){
		InfoPwdCnt = FDB_CntPwdUser();
	}
	cPwd = InfoPwdCnt;

	if(!Infoloadflag){
		InfoManCnt = FDB_CntAdminUser();
	}
	cMan = InfoManCnt;

	cManage=FDB_CntOPLog();
	mManage=MAX_OPLOG_COUNT;

	if(type==PAGE_BASE)
	{
		;
	}
	else if(type==PAGE_RECORD)
	{
		SetPenColor(hdc,0x00ff88dd);
		Rectangle(hdc,5,5,gOptions.LCDWidth-10, 30);

		if (fromRight==1)
		{
			int sd=35;
			InfoShow(hdc,LoadStrByID(HIT_INFO19),cUser,10+gOptions.ControlOffset,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO13),cMan,110+gOptions.ControlOffset,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO14),cPwd,230+gOptions.ControlOffset,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),-1,275+gOptions.ControlOffset,35,0);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),-1,275+gOptions.ControlOffset,55,0);
		}
		else
		{
			InfoShow(hdc,LoadStrByID(HIT_INFO19),cUser,10+gOptions.ControlOffset,10,55);
			InfoShow(hdc,LoadStrByID(HIT_INFO13),cMan,110+gOptions.ControlOffset,10,75);
			InfoShow(hdc,LoadStrByID(HIT_INFO14),cPwd,230+gOptions.ControlOffset,10,55);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),-1,280+gOptions.ControlOffset,35,0);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),-1,280+gOptions.ControlOffset,55,0);
		}
		DrawLine2(hdc,240+gOptions.ControlOffset,45,10,0);
		DrawLine2(hdc,240+gOptions.ControlOffset,65,10,1);

		DrawLine(hdc,5,175,100,0);
		DrawLine(hdc,130,175,100,0);
		if(!gOptions.IsOnlyRFMachine)
		{
			InfoShow(hdc,LoadStrByID(HIT_INFO16),mTmp,5,55,50);
			if(cTmp==0||mTmp==0)
				mf=0.01;
			else
				mf=(float)cTmp/(float)mTmp;

			if((int)(mf*100)==0)
				mf=0.01;
			DrawLine(hdc,5,175,100*mf,1);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),cTmp,65,160,30);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),mTmp-cTmp,65,75,30);
		}
		else
		{
			InfoShow(hdc,LoadStrByID(HIT_INFO20),mUser,5,55,50);
			if(cUser==0||mUser==0)
				mf=0.01;
			else
				mf=(float)cUser/(float)mUser;

			if((int)(mf*100)==0)
				mf=0.01;
			DrawLine(hdc,5,175,100*mf,1);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),cUser,65,160,30);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),mUser-cUser,65,75,30);
		}

		InfoShow(hdc,LoadStrByID(HIT_INFO17),mAtt,130,55,50);
		if(cAtt==0||mAtt==0)
			mf=0.01;
		else
			mf=(float)cAtt/(float)mAtt;
		if((int)(mf*100)==0)
			mf=0.01;
		DrawLine(hdc,130,175,100*mf,1);
		InfoShow(hdc,LoadStrByID(HIT_INFO12),cAtt,190,160,50);
		InfoShow(hdc,LoadStrByID(HIT_INFO15),mAtt-cAtt,190,75,50);
	}
	else if(type==PAGE_DEVICE)
	{
		BITMAP bmp;
		FillBox(hdc,10,10,290,60);
		if (LoadBitmap(HDC_SCREEN,&bmp,GetBmpPath("info.gif")))
			return 0;
		FillBoxWithBitmap(hdc,10,10,0,0,&bmp);
		UnloadBitmap(&bmp);

		LoadStr("~ProductTime",pdate);

		//zsliu add
		if(gOptions.isUseHejiraCalendar)
		{
			struct tm gDate;
			GetTime(&gDate);
			char* pChar = pdate;
			char tempChar[5] = {0};

			//get year
			memcpy(tempChar, pChar, 4);
			gDate.tm_year = atoi(tempChar) - 1900;

			//get month
			pChar = pdate + 5;
			memset(tempChar, 0, 4);
			memcpy(tempChar, pChar, 2);
			gDate.tm_mon = atoi(tempChar) - 1;

			//get day
			pChar = pdate + 8;
			memset(tempChar, 0, 4);
			memcpy(tempChar, pChar, 2);
			gDate.tm_mday = atoi(tempChar);

			ChrisToHejiraTTime(&gDate);     //公历转伊朗日历
			//gDate.tm_mon = gDate.tm_mon - 1;

			memset(tempChar, 0, 4);
			sprintf(tempChar,"%4d",gDate.tm_year + 1900);

			int nCounter;
			for(nCounter = 0; nCounter < 4; nCounter ++)
				pdate[nCounter] = tempChar[nCounter];

			memset(tempChar, 0, 4);
			sprintf(tempChar,"%2d",gDate.tm_mon + 1);
			for(nCounter = 0; nCounter < 2; nCounter ++)
				pdate[nCounter + 5] = tempChar[nCounter];

			memset(tempChar, 0, 4);
			sprintf(tempChar,"%2d",gDate.tm_mday);
			for(nCounter = 0; nCounter < 2; nCounter ++)
				pdate[nCounter + 8] = tempChar[nCounter];
		}
		//zsliu add end ... ...

		LoadStr("~SerialNumber",sn);
		LoadStr("~OEMVendor",vender);
		LoadStr("~DeviceName",device);
		LoadStr("~AlgVer",buf);
		sprintf(fpver, "%s%d%s", buf, gOptions.ZKFPVersion, ".0");
		strcpy(fwver,MAINVERSION);
		memset(nv,0,64);
		if ((!(LoadStr("~SerialVer",nv))) ||(strlen(nv)==0))
			strcpy(nv,MAINVERSIONTFT);
		if (fromRight==1)
		{
			int posX1=60;
			InfoShowStr(hdc,LoadStrByID(HIT_INFO5) ,device,posX1,10);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO6) ,sn,posX1,30);
			if (gOptions.NetworkFunOn)
				InfoShowStr(hdc,LoadStrByID(HIT_MACINFO) ,(char*)macstr,posX1,50);

			if(!gOptions.IsOnlyRFMachine)
				InfoShowStr(hdc,LoadStrByID(HIT_INFO7) ,fpver,10,70);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO8) ,nv,10,95);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO9) ,vender,10,120);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO10) ,pdate,10,145);
		}
		else
		{
			InfoShowStr(hdc,LoadStrByID(HIT_INFO5) ,device,105,10);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO6) ,sn,105,30);
			if (gOptions.NetworkFunOn)
				InfoShowStr(hdc,LoadStrByID(HIT_MACINFO) ,(char*)macstr,105,50);

			if(!gOptions.IsOnlyRFMachine)
				InfoShowStr(hdc,LoadStrByID(HIT_INFO7) ,fpver,10,70);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO8) ,nv,10,95);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO9) ,vender,10,120);
			InfoShowStr(hdc,LoadStrByID(HIT_INFO10) ,pdate,10,145);
		}
	}
#ifdef FACE
	else if(type==PAGE_FACE)
	{
		int mFace=0,cFace=0;
		mFace=gOptions.MaxFaceCount*100;
		cFace=FDB_CntFaceUser();

		SetPenColor(hdc,0x00ff88dd);
		Rectangle(hdc,5,5,310,30);

		if (fromRight==1)
		{
			int sd=35;
			InfoShow(hdc,LoadStrByID(HIT_INFO19),cUser,10,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO13),cMan,110,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO14),cPwd,230,10,sd);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),-1,275,35,0);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),-1,275,55,0);
		}
		else
		{
			InfoShow(hdc,LoadStrByID(HIT_INFO19),cUser,10,10,55);
			InfoShow(hdc,LoadStrByID(HIT_INFO13),cMan,110,10,75);
			InfoShow(hdc,LoadStrByID(HIT_INFO14),cPwd,230,10,55);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),-1,280,35,0);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),-1,280,55,0);
		}
		DrawLine2(hdc,240,45,10,0);
		DrawLine2(hdc,240,65,10,1);

		DrawLine(hdc,90,175,100,0);
		//if(!gOptions.IsOnlyRFMachine)
		{
			sprintf(buf,"%s:",LoadStrByID(MID_FACE));
			InfoShow(hdc,buf,mFace,5,55,50);
			if(cFace==0||mFace==0)
				mf=0.01;
			else
				mf=(float)cFace/(float)mFace;

			if((int)(mf*100)==0)
				mf=0.01;
			DrawLine(hdc,90,175,100*mf,1);
			InfoShow(hdc,LoadStrByID(HIT_INFO12),cFace,150,160,30);
			InfoShow(hdc,LoadStrByID(HIT_INFO15),mFace-cFace,150,75,30);
		}
	}
#endif

	else if (type==PAGE_BAT)	//power infomation
	{
		int vbat;
		char buf[10];
		int posX1,posX2,posX3,posX4;
		int voltage=0;

		if (fromRight==1)
		{
			posX1=220;
			posX2=130;
			posX3=20;
			posX4=260;
		}
		else
		{
			posX1=20;
			posX2=130;
			posX3=200;
			posX4=260;
		}

		// supply power
		InfoShow(hdc,LoadStrByID(HIT_BAT_STR3),-1,posX1,30,0);

		if(BatteryMod==BATTERY_Internal)
		{
			vbat=check_battery_voltage(&voltage);

			// battery info
			InfoShow(hdc,LoadStrByID(HIT_BAT_STR2),-1,posX1,110,0);
			if(vbat>0)
				voltage= 25*vbat;
			else
			{
				voltage=0;
			}
			sprintf(buf,"%d%%",voltage);
			InfoShow(hdc,buf,-1,posX2,110,0);

			//battery
			TextOut(hdc,posX2,30,LoadStrByID(HIT_BAT_STR6));
			FillBoxWithBitmap(hdc,posX3,20,70,30,&bat7);

			switch (vbat)
			{
				case 0: FillBoxWithBitmap(hdc,posX3,90,60,70,&bat0);       break;
				case 1: FillBoxWithBitmap(hdc,posX3,90,60,70,&bat1);       break;
				case 2: FillBoxWithBitmap(hdc,posX3,90,60,70,&bat2);       break;
				case 3: FillBoxWithBitmap(hdc,posX3,90,60,70,&bat3);       break;
				case 4: FillBoxWithBitmap(hdc,posX3,90,60,70,&bat4);       break;
				default:
					FillBoxWithBitmap(hdc,posX3,90,60,70,&bat0);       break;
			}
		}
		else
		{
			//AC
			TextOut(hdc,posX2,30,LoadStrByID(HIT_BAT_STR5));
			FillBoxWithBitmap(hdc,posX3,20,70,30,&bat6);
		}
	}
	else if (type==PAGE_OTHER) /*Other information, push, gprs, camera version,*/
	{
		int y=5;
		//int posX1=60;
		int posX2=10;
#ifdef ZEM600
		int i=0;
#endif
		//posX1 = (fromRight==1?60:105);
		memset((void*)buf, 0, sizeof(buf));
		sprintf(buf, "%d", GetMCUVersion());
		InfoShowStr(hdc,"MCU Ver" ,buf,posX2,y);


		if (gOptions.IsSupportModem) {
			y+=20;
			memset((void*)buf, 0, sizeof(buf));
			sprintf(buf, "%s", modem_get_version());
			InfoShowStr(hdc,"Modem Ver" ,buf,posX2,y);

			y+=20;
			memset((void*)buf, 0, sizeof(buf));
			int type=LoadInteger("ModemModule", MODEM_MODULE_Q24PL);
			if (type == MODEM_MODULE_Q24PL) {
				sprintf(buf, "%s", "Q24PL");
			} else if (type == MODEM_MODULE_EM560) {
				sprintf(buf, "%s", "EM560");
			} else if (type == MODEM_MODULE_EM660) {
				sprintf(buf, "%s", "EM660");
			} else if (type == MODEM_MODULE_EM770) {
				sprintf(buf, "%s", "EM770");
			} else {
				sprintf(buf, "%s", "?");
			}
			InfoShowStr(hdc,"Modem Type" ,buf,posX2,y);
		}

		if (gOptions.IclockSvrFun) {
			y+=20;
			memset((void*)buf, 0, sizeof(buf));
			GetPushSDKVersion(buf, sizeof(buf));
			InfoShowStr(hdc, "PushVersion" ,buf,posX2, y);
		}

		if (gOptions.CameraOpen) {
			y+=20;
			memset((void*)buf, 0, sizeof(buf));
#ifdef	ZEM600		
			 if (strstr((const char*)get_hardware_info(), "301PLH") > 0) {
	                                i = 1;
	                } else if (strstr((const char*)get_hardware_info(), "VC353") > 0) {
	                        i = 2;
	                }
			sprintf(buf, "%s,C%d", (char*)get_version_info(), i);
			InfoShowStr(hdc, "Camera Ver" ,buf,posX2, y);
#endif	
/*
			if (strstr((const char*)get_hardware_info(), "301PLH") > 0) {
				i = 1;
			} else if (strstr((const char*)get_hardware_info(), "VC353") > 0) {
				i = 2;
			}
*/
			
		}


	}
	Infoloadflag = 1;
	return 1;
}

static int SysInfoPageProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	int type;
	HWND hwnd;
	HDC hdc;

	switch (message)
	{
		case MSG_INITPAGE:
			type = (int)GetWindowAdditionalData (hDlg);
			hwnd = GetDlgItem (hDlg, IDC_SYSINFO);
			break;

		case MSG_PAINT:
			hdc=BeginPaint(hDlg);
			type = (int)GetWindowAdditionalData (hDlg);

			switch (type)
			{
				case PAGE_BASE:
					GetInfoText(hdc,PAGE_BASE);
					break;
				case PAGE_RECORD:
					if (tflag==1)
						KillTimer(hDlg,0x80180);
					tflag=0;
					GetInfoText(hdc,PAGE_RECORD);
					break;
				case PAGE_DEVICE:
					if (tflag==1)
						KillTimer(hDlg,0x80180);
					tflag=0;
					GetInfoText(hdc,PAGE_DEVICE);
					break;
#ifdef FACE
				case PAGE_FACE:
					if (tflag==1)
					{
						KillTimer(hDlg,0x80180);
					}
					tflag=0;
					GetInfoText(hdc,PAGE_FACE);
					break;
#endif
				case PAGE_BAT:
					if (tflag==0)
					{
						SetTimer(hDlg,0x80180,100);
						tflag=1;
					}
					tbatmod=BatteryMod;
					GetInfoText(hdc,PAGE_BAT);
					break;
				case PAGE_OTHER:
					if (tflag==1) {
						KillTimer(hDlg,0x80180);
					}
					tflag=0;
					GetInfoText(hdc,PAGE_OTHER);
					break;
			}
			EndPaint(hDlg,hdc);
			break;
		case MSG_TIMER:
			if((wParam==0x80180) && (tbatmod!=BatteryMod))
			{
				rect0.right = gOptions.LCDWidth;
				rect0.bottom = gOptions.LCDHeight;		    
				InvalidateRect(hDlg, &rect0, TRUE);
				UpdateWindow(hDlg,TRUE);
			}
			break;

		case MSG_SHOWPAGE:
			return 1;
	}
	return DefaultPageProc (hDlg, message, wParam, lParam);
}

static DLGTEMPLATE InfoDlgBox = {WS_BORDER | WS_CAPTION, WS_EX_NONE, 1, 1, 319, 239, "", 0, 0, 1, NULL, 0};

static CTRLDATA InfoCtrl[] =
{
	{CTRL_PROPSHEET, WS_VISIBLE | PSS_COMPACTTAB, 1, 1, 315, 210, IDC_PROPSHEET, "", 0},
};


static int InfoDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HWND statushWnd;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_INITDIALOG:
			{
				HWND pshwnd = GetDlgItem (hDlg, IDC_PROPSHEET);

				if (LoadBitmap(HDC_SCREEN,&line0,GetBmpPath("line0.bmp")))
					return 0;
				if (LoadBitmap(HDC_SCREEN,&line1,GetBmpPath("line1.bmp")))
					return 0;


				PageSysInfo.controls = CtrlSysInfo;

				if (!gOptions.NotShowRecordInfo)
				{
					PageSysInfo.caption = LoadStrByID(HIT_INFO2);
					PageSysInfo.dwAddData = PAGE_RECORD;
					SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&PageSysInfo, (LPARAM)SysInfoPageProc);
					gPageCnt++;
				}

#ifdef FACE
				if(gOptions.FaceFunOn)
				{
					PageSysInfo.caption = LoadStrByID(MID_FACE_INFO);
					PageSysInfo.dwAddData = PAGE_FACE;
					SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&PageSysInfo, (LPARAM)SysInfoPageProc);
					gPageCnt++;
				}
#endif

				PageSysInfo.caption = LoadStrByID(HIT_INFO3);
				PageSysInfo.dwAddData = PAGE_DEVICE;
				SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&PageSysInfo, (LPARAM)SysInfoPageProc);
				gPageCnt++;

				if (gOptions.BatteryInfo)
				{
					PageSysInfo.caption = LoadStrByID(HIT_BAT_STR1);
					PageSysInfo.dwAddData = PAGE_BAT;
					SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&PageSysInfo, (LPARAM)SysInfoPageProc);
					gPageCnt++;
				}

				//dsl 2012.5.5
				PageSysInfo.caption = LoadStrByID(MID_LOGHINTOTHER);
				PageSysInfo.dwAddData = PAGE_OTHER;
				SendMessage (pshwnd, PSM_ADDPAGE, (WPARAM)&PageSysInfo, (LPARAM)SysInfoPageProc);

				InfoItem=0;
				SendDlgItemMessage (hDlg, IDC_PROPSHEET,PSM_SETACTIVEINDEX,InfoItem,0);

				if (gfont1==NULL) {
					infolf = CreateLogFont(NULL, "fixed", "GB2312", FONT_WEIGHT_REGULAR, 
							FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
							FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 12, 0);
				}
				statushWnd=createStatusWin1(hDlg , 250, 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(MID_WAIT));
				destroyStatusWin1(statushWnd);
				break;
			}

		case MSG_KEYUP:
			if(gOptions.TFTKeyLayout==3)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(gOptions.TFTKeyLayout==3)
			{
				if(keyupFlag==1) {
					keyupFlag=0;
				} else {
					break;
				}
			}
			if(gOptions.KeyPadBeep) {
				ExKeyBeep();
			}

			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{
				if (++InfoItem > gPageCnt) {
					InfoItem=0;
				}
				SendDlgItemMessage (hDlg, IDC_PROPSHEET,PSM_SETACTIVEINDEX,InfoItem,0);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) ||
					(gOptions.TFTKeyLayout==3 && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if (--InfoItem < 0) {
					InfoItem = gPageCnt;
				}
				SendDlgItemMessage (hDlg, IDC_PROPSHEET,PSM_SETACTIVEINDEX,InfoItem,0);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_ESCAPE) || (LOWORD(wParam)==SCANCODE_MENU) ||
					(LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
			{
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
				return 0;
			}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&line0);
			UnloadBitmap(&line1);

			if (gOptions.BatteryInfo)       //add by jazzy 2008.12.17 for battery infomation
			{
				UnloadBitmap(&bat0);
				UnloadBitmap(&bat1);
				UnloadBitmap(&bat2);
				UnloadBitmap(&bat3);
				UnloadBitmap(&bat4);
				UnloadBitmap(&bat5);
				UnloadBitmap(&bat6);
				UnloadBitmap(&bat7);
			}
			if(infolf!=NULL)
				DestroyLogFont(infolf);
			EndDialog (hDlg, IDCANCEL);
			return 0;
	}
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_INFO(HWND hWnd)
{
	gPageCnt = 0;
	if (gOptions.BatteryInfo)
	{
		LoadBitmap(HDC_SCREEN,&bat0,GetBmpPath("bat7.gif"));
		LoadBitmap(HDC_SCREEN,&bat1,GetBmpPath("bat8.gif"));
		LoadBitmap(HDC_SCREEN,&bat2,GetBmpPath("bat9.gif"));
		LoadBitmap(HDC_SCREEN,&bat3,GetBmpPath("bat10.gif"));
		LoadBitmap(HDC_SCREEN,&bat4,GetBmpPath("bat11.gif"));
		LoadBitmap(HDC_SCREEN,&bat5,GetBmpPath("bat5.gif"));
		LoadBitmap(HDC_SCREEN,&bat6,GetBmpPath("bat6.gif"));
		LoadBitmap(HDC_SCREEN,&bat7,GetBmpPath("bat4.gif"));
	}

	PageSysInfo.w = gOptions.LCDWidth-1;
	PageSysInfo.h = gOptions.LCDHeight-1;

	InfoDlgBox.w = gOptions.LCDWidth-1;
	InfoDlgBox.h = gOptions.LCDHeight-1;

	InfoCtrl[0].w = gOptions.LCDWidth-1;
	InfoCtrl[0].h = gOptions.LCDHeight-1;

	InfoDlgBox.caption = LoadStrByID(HIT_INFO4);
	InfoDlgBox.controls = InfoCtrl;
	Infoloadflag = 0;
	DialogBoxIndirectParam (&InfoDlgBox, hWnd, InfoDialogBoxProc, 0L);
}
