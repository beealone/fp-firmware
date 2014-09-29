/************************************************

  iClock-2000

  user managment

  Copyright (C) 2006-2010, ZKSoftware Inc.

  Fixed EnrollFinger Event bug;	ZhangHonggen 2009.11.24
 *************************************************/


#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include <minigui/tftmullan.h>
#include <dirent.h>
#include "ssrpub.h"
#include "msg.h"
#include "options.h"
#include "flashdb.h"
#include "commu.h"
#include "ssrcommon.h"
#include "t9.h"
#include "sensor.h"
#include "main.h"
#include "net.h"
#include "iclock/pushapi.h"

#ifdef FACE
#include "facedb.h"
#endif

#define LB_ACNO 50
#define ID_USERACNO 51
#define LB_NAME 52
#define ID_NAME 53
#define LB_FP 54
#define LB2_FP 55
#define ID_FP 56
#define LB_PWD 58
#define LB2_PWD 59
#define ID_PWD 60
#define LB_AUTH 61
#define ID_AUTH 62
#define ID_SAVE 63
#define ID_EXIT 64
#define LB_CARD 65
#define ID_CARD 66
#define ID_SCARD 67
#define ID_RISFP 68
#define LB_USVF	69
#define ID_USVF	70
#define LB_CPMD 71
#define ID_CPMD 72
#define ID_CAPTURE 73

#ifdef FACE
#define ID_FACE 	74
#define ID_FACE_REG     75
#define ID_FACE_SG      76
#define ID_FACE_G       77
#define ID_FACE_SGR     78
#endif

char mycard[20];
U32 cardvalue=0;
int iscardreg=0;
int havecard=0;
HWND EdAcno,EdName,EdCard,EdPwd3,CbAuth, EdFp, CbCamera;
HWND btnfp,btnpwd,btncard,btnrisfp,btncapture,btnsave,btnexit;
HWND Edtmp, focuswnd;
BOOL ComListClose=TRUE;

TUser user;
int userenrollcount=0;
char tmpchar[48];
char tmpcharname[30];
char mypwd[10];
int oppass=0;//0 add 1 edit
U16 g_pin=0;
int fpregcount=0;
char fpcountchar[30]="0";
int g_authsel=0;
int isfpreg=0;
int ispwdreg=0;
int isname=0;
int isacno=0;
int tempauth=-1;
int isshowsavebox=0;
//BITMAP userbmp;
BITMAP pwdhintbmp;
int continueadd=0;
int havepwd=0;
BOOL PwdEnrolled;
int haveris=0;
int g_cpsel=0;
int tmp_cpsel=0;
int tmp_capturesel=-1;

//用户多种验证方式
HWND CbVS;
int g_vssel=0;
int tmp_vssel=-1;
int pwdbmp_height=0;					//密码图标显示高度
extern char* myVerifyStyle[];				//验证方式列表
extern int GetUserVerifyType(PExtUser pu, int* vtype);
/*用户照片U盘上传至服务器，add by yangxiaolong,2011-7-6,start*/
extern BOOL g_bIsRegUserPic;
/*用户照片U盘上传至服务器，add by yangxiaolong,2011-7-6,start*/
HWND EdGroup;
char uGroup[12];

static BOOL haveface=0;
static BOOL isregface=0;
static int f_group=0;
static BOOL DataChg=0;

extern int DelFingerFromCache(HANDLE pin);
extern int CreateFaceEnrollWindow(HWND hWnd, int pin);

static int switch_pri_for_compatibility(int privilege)
{
	switch (privilege){
		case PRIVILLEGE0: return 0; /*normal user item*/
		case PRIVILLEGE1: return 1; /*enroller item*/

		case PRIVILLEGE2:
		case PRIVILLEGE3:
		{
			return 2; /*administrator item*/
		}
		default: return 0;
	}
}

static int bChgGroup(void)
{
	char tmpgp[4];
	memset(tmpgp, 0, 4);
	GetWindowText(EdGroup, tmpgp, 4);
	if (tmpgp[0] && atoi(tmpgp)>0) {
		if (atoi(tmpgp)!=atoi(uGroup)) {
			return 1;
		}
	} else {
		return 2;
	}

	return 0;
}

static void SetUserVerify(int uPIN)
{
	TExtUser textuser;
	memset(&textuser, 0, sizeof(TExtUser));
	if(FDB_GetExtUser(uPIN, &textuser) != NULL)
	{
		GetUserVerifyType(&textuser, &g_vssel);
		g_cpsel = textuser.reserved[0];
	}
	else
	{
		g_vssel = 0;
		g_cpsel = 0;
	}

	if(gOptions.UserExtendFormat && !gOptions.LockFunOn)
		SendMessage(CbVS, CB_SETCURSEL, g_vssel, 0);
	if (gOptions.CameraOpen)
		SendMessage(CbCamera, CB_SETCURSEL, g_cpsel, 0);

	tmp_vssel = g_vssel;
	tmp_cpsel = g_cpsel;
}

static int SaveExtUserInfo(int uPIN)
{
	TExtUser textuser;
	memset(&textuser,0,sizeof(TExtUser));
	
	if(FDB_GetExtUser(uPIN, &textuser)!=NULL) {
		if(gOptions.LockFunOn > 1) {
			textuser.VerifyStyle = g_vssel;
		} else {
			textuser.VerifyStyle = g_vssel|0x80;
		}
		textuser.reserved[0] = g_cpsel;
		return FDB_ChgExtUser(&textuser);
	} else {
		textuser.PIN = uPIN;
		if(gOptions.LockFunOn > 1) {
			textuser.VerifyStyle = g_vssel;
		} else {
			textuser.VerifyStyle = g_vssel|0x80;
		}
		textuser.reserved[0] = g_cpsel;
		return FDB_AddExtUser(&textuser);
	}

}
//----------------------------------------------

extern void rotate_photo(char *input_file);
extern int GrabPicture(char *filename);
int SavePhoto(void)
{
	DIR *pDir;
	char photopath[50];
	char pname[100];
	int mount = -1;

	if (strncmp(USB_MOUNTPOINT, GetPhotoPath(""), strlen(USB_MOUNTPOINT))==0)
	{
		DoUmountUdisk();
		mount = DoMountUdisk();
		if(mount!=0) return 0;
	}

	sprintf(photopath, "%s", GetPhotoPath(""));
	pDir = opendir(photopath);
	if (pDir==NULL)		//文件夹不存在
	{
		sprintf(pname, "mkdir %s && sync", photopath);
		//		printf("pname:%s\n", pname);
		if (systemEx(pname) != EXIT_SUCCESS)
			return 0;
	}
	else
	{
		closedir(pDir);
	}

	memset(&pname, 0, sizeof(pname));
	sprintf(pname, "%s/%s.jpg", photopath, user.PIN2);
	//	printf("PhotoName:%s\n", pname);
	if (GrabPicture(pname) != 1)
	{
		ExPlayOtherVoice(VOICE_CAPTURE);
#ifdef ZEM600
		if (gOptions.IsRotatePhoto)
		{
			rotate_photo(pname);
		}
#endif
	}
	sync();

	if(mount==0) DoUmountUdisk();
	return 1;
}

static void GetFreeacno(U16 pin, char *useacno)
{
	//GetFreePIN2FromRam(useacno);
	GetFreePIN2Fast(useacno);
}

static void Initnewinfo(void)
{
	memset((void*)&user, 0, sizeof(TUser));
	memset((void*)usertmp,0,sizeof(usertmp));
	memset((void *)usertmplen,0,sizeof(usertmplen));
	memset((void *)userisenroll,0,sizeof(userisenroll));
	memset(tmpchar,0,sizeof(tmpchar));
	memset(tmpcharname,0,sizeof(tmpcharname));
	memset(mypwd,0,sizeof(mypwd));
	memset(mycard,0,sizeof(mycard));
	memset(uGroup, 0, sizeof(uGroup));
	isfpreg=0;
	isname=0;
	ispwdreg=0;
	havepwd=0;
	isacno=0;
	fpregcount=0;
	tempauth=-1;
	tmp_vssel=-1;
	userenrollcount=0;
	isshowsavebox=0;
	//For face
	haveface=0;
	isregface=0;
	f_group=0;
	DataChg=0;

	memset(fpcountchar,0,sizeof(fpcountchar));
	if (!oppass)
	{
		//printf("________%s%d\n", __FILE__, __LINE__);
		busyflag=1;
		user.PIN = 1;

		user.PIN=GetNextPIN(user.PIN, TRUE);
		memset(tmpchar, 0, sizeof(tmpchar));
		//U32 tick1 = GetTickCount1();
		GetFreeacno(user.PIN, tmpchar);
		//printf("----------time:%d\n", (GetTickCount1() - tick1));

		memcpy(user.PIN2,tmpchar,strlen(tmpchar));
		//printf(" %s user.PIN %d,pin2 %s\n", __FUNCTION__, user.PIN, user.PIN2);
		SetMenuTimeOut(time(NULL));
		sprintf(fpcountchar,"%s%d", LoadStrByID(MID_FPLEN), 0);
#ifdef FACE
		if(gOptions.FaceFunOn) // add by caona for face
		{
			GetUserFreePINAndFaceGroup(buffer,&f_group,1);
		}
		else
#endif
		{
			if (user.Group)
				sprintf(uGroup, "%d", user.Group);
			else
				sprintf(uGroup, "%d", gOptions.MachineGroup);
		}

		if (continueadd)
		{
			if(gOptions.UserExtendFormat && !gOptions.LockFunOn)
			{
				g_vssel=0;
				tmp_vssel=0;
				SendMessage(CbVS, CB_SETCURSEL, g_vssel, 0);            //新用户默认为第一种验证方式
			}

			if (gOptions.CameraOpen)
			{
				g_cpsel = 0;
				tmp_cpsel = 0;
				SendMessage(CbCamera, CB_SETCURSEL, g_cpsel, 0);
			}

			SendMessage(CbAuth,CB_SETCURSEL,0,0);
			SetWindowText(EdAcno,tmpchar);
			if(gOptions.IMEFunOn)
				SetWindowText(EdName,"");

			SetWindowText(EdFp,fpcountchar);
			SetWindowText(EdPwd3,"");

			if(gOptions.RFCardFunOn)
			{
				SetWindowText(EdCard,mycard);
			}

			if (IDTFlag)
				SetWindowText(EdGroup, uGroup);

			SetFocusChild(EdAcno);
			SendMessage(EdAcno,EM_SETCARETPOS,0,strlen(tmpchar));
#ifdef FACE
			if(gOptions.FaceFunOn) // add by caona for face
			{
				sprintf(buffer,"%d",f_group);
				SetWindowText(facegwnd,buffer);
				SendMessage(facegwnd,EM_SETCARETPOS,0,strlen(buffer));
			}
#endif
		}
		g_authsel=0;
		tempauth=0;
		havepwd=0;
		havecard=0;
		isacno=1;
	}
	else
	{
		FDB_GetUser(g_pin,&user);

		/*
		if (user.Privilege==PRIVILLEGE3) {//超级管理员
			tempauth=2;
		} else if (user.Privilege==PRIVILLEGE1) {//登记员
			tempauth=1;
		} else {
			tempauth=0;
		} */

		tempauth = switch_pri_for_compatibility(user.Privilege);

		if(gOptions.IMEFunOn==1) {
			memcpy(tmpchar, user.PIN2, strlen(user.PIN2));

			char mynamename[100];		//modify by jazzy 2008.07.26
			memset(mynamename, 0, sizeof(mynamename));
			Str2UTF8(tftlocaleid, (unsigned char *)user.Name, (unsigned char *)mynamename);
			sprintf(tmpcharname, "%s", mynamename);

			//zsliu change fix a bug, 解决掉显示姓名的时候乱码的问题 2009-05-09
			//strncpy(user.Name, mynamename, strlen(mynamename));	//zsliu
			/*dsl 2012.5.9 fix Thailand and Traditional Chinese language can not correct display*/
			if (gOptions.Language != 74 && gOptions.Language != 84) {
				strncpy(user.Name, mynamename, strlen(mynamename));
			}
		} else 	if(user.Name[0]) {
			char mynamename[100];
			memset(mynamename, 0, sizeof(mynamename));
			Str2UTF8(tftlocaleid, (unsigned char *)user.Name, (unsigned char *)mynamename);
			sprintf(tmpchar, "%s  %s  %s", user.PIN2, LoadStrByID(MID_NAME), mynamename);

			//zsliu change fix a bug, 解决掉显示姓名的时候乱码的问题 2009-05-09
			//strncpy(user.Name, mynamename, strlen(mynamename));	//zsliu
			/*dsl 2012.5.9 fix Thailand and Traditional Chinese language can not correct display*/
			if (gOptions.Language != 74 && gOptions.Language != 84) {
				strncpy(user.Name, mynamename, strlen(mynamename));
			}
		} else {
			memcpy(tmpchar, user.PIN2, strlen(user.PIN2));
		}

		if(gOptions.RFCardFunOn)
		{
			memcpy(&cardvalue, user.Card, 4);
			if(cardvalue)
			{	
				unsigned int aaa = htonl(cardvalue);
				sprintf(mycard, "%010u", aaa);
				havecard=1;
			}
			else
			{
				havecard=0;
			}
		}
		fpregcount = FDB_GetTmpCnt(user.PIN);
		//		printf("%s,pin %d fpregcount %d\n",__FUNCTION__, user.PIN, fpregcount);
		sprintf(fpcountchar,"%s%d",LoadStrByID(MID_FPLEN),fpregcount);
		if(user.Password[0])
		{
			nstrcpy(mypwd, user.Password, PWDLIMITLEN);
			havepwd=1;
		}

#ifdef FACE
		if(gOptions.FaceFunOn) //add by caona for face
		{
			if(FDB_GetFaceTmp(user.PIN,0,NULL))
				haveface=1;
			f_group=user.Group;
		}
		else
#endif
		{
			sprintf(uGroup, "%d", user.Group);
		}
	}
}

extern int myMessageBox1 (HWND hwnd, DWORD dwStyle, char * title, char * text, ...);
extern int  MessageBox1 (HWND hParentWnd, const char* pszText,const char* pszCaption, DWORD dwStyle);
static int ValidAcnoFromRemote(HWND hWnd)
{
	int pin2;
	unsigned char result;
	int ret=0;

	pin2=atoi(user.PIN2);
	//	printf("ris pin2:%d\n",pin2);
	if (!CheckIsIdleIDFromAuthServer(pin2, &result))
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_AUTHSERVER_ERROR));
		return 0;
	}
	if (result)
	{
		//ID 已存在
		if(MessageBox1(hWnd,LoadStrByID(REP_ATTLOG),LoadStrByID(MID_APPNAME),
					MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK)
		{
			pin2=0;
			if (!GetFreeIDFromAuthServer((U32*)&pin2))
			{
				myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),
						LoadStrByID(MID_AUTHSERVER_ERROR));
				return 0;
			}
			else
			{
				char risbuf[24];
				memset(risbuf,0,sizeof(risbuf));
				sprintf(risbuf,"%d",pin2);
				SetWindowText(EdAcno,risbuf);
				SetFocusChild(EdAcno);
				SendMessage(EdAcno,EM_SETCARETPOS,0,strlen(risbuf));
				return 0;
			}
		}
		else
		{
			if(!ismenutimeout) ret=pin2;
		}
	}
	else
	{
		ret=pin2;
	}

	return ret;
}

static int  processregfp(HWND hWnd)
{
	static int fingerid;
	int i, FPEnrolled,ret=News_ErrorInput;
	char IsNewEnroll;
	char fpcountbuf[20];	//modify by jazzy 2009.05.09 for length to short
	TUser u;
	int OldState=gMachineState;  //changed by cxp at 2010-04-22

	i=0;
	memset((void*)&u, 0, sizeof(TUser));

	if(((FDB_CntTmp()+userenrollcount)>=gOptions.MaxFingerCount*100)||
			(gOptions.IsOnlyRFMachine&&(FDB_CntUserByMap()>=gOptions.MaxUserCount*100)))
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_EXCEED));
		return News_CancelInput;
	}

	if(FDB_CntUserByMap()==0)
	{
		IsNewEnroll=TRUE;
	}
	else
	{
		fingerid = -1;
		for(i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			if(0!= FDB_GetTmp((U16)user.PIN,(char)i,NULL))    
			{
				fingerid=i;
				break;
			}
		}
		if(fingerid == -1)
			IsNewEnroll = TRUE;
		else
			IsNewEnroll = FALSE;
	}

	u.PIN=0;
	if(!IsNewEnroll) FDB_GetUser(user.PIN,&u);

	if(u.PIN!=0)
	{
		for(i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			if(0!=FDB_GetTmp((U16)u.PIN,(char)i,NULL))
			{
				userisenroll[i] = 1;
			}
		}
	}
	fingerid=0;
	if(1)
	{
		fingerid = -1;
		// must deside whether other finger already enrooled
		for (i=0;i<MAX_USER_FINGER_COUNT;i++)
		{
			if (userisenroll[i] == 0)
			{
				fingerid = i;
				break;
			}
		}

		if(fingerid==-1)
		{
			myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_EXCEEDFINGER));
			return News_CancelInput;
		}
	}

	FPEnrolled=0;
	if(1)
	{
		usertmplen[fingerid]=0;
		i=user.PIN;
		//printf("user.PIN %d PIN2 %s fingerid %d\n",user.PIN, user.PIN2,fingerid);
		int retCreatFp=0;
		gMachineState=STA_ENROLLING;  //changed by cxp at 2010-04-22
		retCreatFp = CreateFpEnrollWindow(hWnd,usertmp[fingerid],&usertmplen[fingerid],fingerid,user.PIN2,IsNewEnroll);
		gMachineState=OldState;  //changed by cxp at 2010-04-22

		if (ismenutimeout)
		{
			return 0;
		}

		//printf("CreatFPEnrollWindow return retCreatFp=%d\n", retCreatFp);
		FPEnrolled = retCreatFp;
		if (FPEnrolled)
		{
			int kk=fingerid;
			isfpreg=1;
			while (kk<10)
			{
				if (userisenroll[kk]==1 && usertmplen[kk])
				{
					userenrollcount++;
				}
				kk++;
			}
			if (fpregcount)
			{
				userenrollcount+=fpregcount;
				fpregcount=0;
			}
			sprintf(fpcountbuf,"%s%d",LoadStrByID(MID_FPLEN),userenrollcount);
			SetWindowText(EdFp,fpcountbuf);
		}
	}
	return ret;
}

extern int UTF8toLocal(int lid,const unsigned char *utf8, unsigned char *str);
extern int FPDBInit(void );
int SaveUserEnrolled(int IsNewEnroll,HWND hWnd)
{

	TUser u;
	int ret = 0;
	int j= 0;
	int i;
	int cur_sel;
	if (pushsdk_is_running() && (!gOptions.IsOnlyRFMachine) )
	{
		int fpCunt =0;
		for (i=0; i<10; i++)
		{
			fpCunt +=  userisenroll[i];
		}
		if (fpCunt > 0)
		{
			pushsdk_pause();
		}
	}

	memset((void *)&u,0,sizeof(TUser));
	if (isshowsavebox>1)
	{
		if(MessageBox1(hWnd,LoadStrByID(MID_SAVEDATA),LoadStrByID(MID_APPNAME),
					MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK)
		{
			isshowsavebox=0;
			DelFingerFromCache(fhdl);
			goto  End;
		}
	}

	if ((IsNewEnroll)&&(!(NULL==FDB_GetUserByCharPIN2(user.PIN2,NULL))))
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_ERRORPIN));
		if(!ismenutimeout) SetFocusChild(EdAcno);
		goto  End;
	}

	if(1)
	{
		j=FDB_OK;
		cur_sel=SendMessage(CbAuth,CB_GETCURSEL,0,0);
		if (cur_sel==2)	{
			user.Privilege=PRIVALUES[3];
		} else if (cur_sel==1) {
			user.Privilege=PRIVALUES[1];
		} else {
			user.Privilege=0;
		}

#ifdef FACE
		//add by caona for face
		if(gOptions.FaceFunOn)
		{
			if((isregface || haveface || (DataChg && (isregface || haveface))) 
					&& (gOptions.FaceRegMode||user.Privilege) 
					&& !isfpreg && !ispwdreg && !iscardreg && !fpregcount && !havepwd && !havecard)
			{
				myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_FACE_REGFP));
				if(!gOptions.IsOnlyRFMachine)
					SetFocusChild(btnfp);
				else
					SetFocusChild(btnpwd);
				isshowsavebox=0;
				ret = 3;
				goto End;
			}
		}
#endif

		//zsliu change fix a bug, 解决掉显示姓名的时候乱码的问题 2009-05-09
		//if (strlen(user.Name)) {
		/*dsl 2012.5.9 fix Thailand and Traditional Chinese language can not correct display*/
		if (strlen(user.Name) && gOptions.Language != 76 &&  gOptions.Language != 84) {
			char tmpstcode[100];
			memset(tmpstcode,0,sizeof(tmpstcode));
			UTF8toLocal(tftlocaleid,(unsigned char*)user.Name,(unsigned char*)tmpstcode);
			FDB_CreateUser(&u, user.PIN, tmpstcode, user.Password, user.Privilege);
		} else {
			FDB_CreateUser(&u, user.PIN, user.Name, user.Password, user.Privilege);
		}
#ifdef FACE
		if(gOptions.FaceFunOn) //add by caona for face group
		{
			char buffer[20];
			int group=0;
			int cnt=0;
			memset(buffer,0 ,sizeof(buffer));
			GetWindowText(facegwnd,buffer,2);
			group=atoi(buffer);

			if(group <= 0 || group > gFaceGroupNum )//|| 0==GetFaceGroupCnt(g))
			{
				myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_SAVE_HINT7));
				ret = 3;
				goto End;
			}
			else if(IsNewEnroll || (!IsNewEnroll && user.Group != group))
			{
				cnt=GetFaceGroupCnt(group);
				if((group==gOptions.DefFaceGroup && cnt>=gFaceDefGroupCnt) ||
						(group != gOptions.DefFaceGroup && cnt >= gFaceGroupCnt))
				{
					myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_EXCEED));
					ret = 3;
					goto End;
				}
			}
			user.Group=group;
			u.Group=group;
		}
#endif
		//-------------- zsliu change end-----------------

		nmemcpy(u.Card, user.Card, sizeof(u.Card));
		nmemcpy((unsigned char*)u.PIN2,(unsigned char*)user.PIN2,strlen(user.PIN2));
#ifdef FACE
		if(!gOptions.FaceFunOn)
#endif
		{
			if (IDTFlag)
			{
				char tmpgp[12];
				GetWindowText(EdGroup, tmpgp, 10);
				u.Group = atoi(tmpgp);
			}
			else
			{
				if(user.Group)
					u.Group = user.Group;	//保留原来的组
			}
			memcpy(u.TimeZones, user.TimeZones, sizeof(user.TimeZones));
		}

		//pwd
		if(nstrcmp(user.Password,u.Password,8))
		{
			memset(u.Password, 0, sizeof(u.Password));
			nstrcpy(u.Password,user.Password,8);
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_PWD, u.PIN, j, 0,0);
		}

		//process name
		if(gOptions.IMEFunOn==1 )	//
		{
			memset(tmpcharname,0,sizeof(tmpcharname));
			memset(u.Name,0,sizeof(u.Name));
			GetWindowText(EdName,tmpcharname,24);
			if (strlen(tmpcharname))
			{
				char tmpstcode[40];
				memset(tmpstcode,0,sizeof(tmpstcode));         //modify by jazzy 2008.12.15 for user name
				UTF8toLocal(tftlocaleid,(unsigned char*)tmpcharname,(unsigned char*)tmpstcode);
				//zsliu change fix a bug, 解决掉显示姓名的时候乱码的问题 2009-05-09
				//memcpy(u.Name,tmpstcode,strlen(tmpstcode));
				/*dsl 2012.5.9 fix Thailand and Traditional Chinese language can not correct display*/
				if (gOptions.Language == 76 || gOptions.Language == 84) {
					memcpy(u.Name,user.Name, strlen(user.Name));
				} else {
					memcpy(u.Name,tmpstcode,strlen(tmpstcode));
				}

			}
		}

		if(userenrollcount==0 && fpregcount==0 && havepwd==0 && havecard==0 && haveface==0 && user.Privilege!=0)
		{
			MessageBox1(hWnd,LoadStrByID(MID_ADUSER_HINT),
					LoadStrByID(MID_APPNAME), MB_OK|MB_ICONSTOP|MB_BASEDONPARENT);
			goto End;
		}

		if(IsNewEnroll)		//new Enroll
		{
			printf(">>>PIN=%d, PIN2=%s\n", u.PIN, u.PIN2);
			j=FDB_AddUser(&u);
			if(j==FDB_OK)
			{
				printf(">>>saved\n");
				g_pin=u.PIN;
				CheckSessionSend(EF_ENROLLUSER,(char*)&u, sizeof(u));
			}
			else
			{
				printf("save fail\n");
			}
			FDB_AddOPLog(ADMINPIN, OP_ENROLL_USER, u.PIN,j,0,0);
		}
		else 	//edit user
		{
			int is_chg_flag = 0;
			/*when change the name,sync to BS svr,20111205 */
			{
				TUser data_user;
				if (FDB_GetUser(u.PIN, &data_user) != NULL)
				{
					if (strlen(data_user.Name) != strlen(u.Name))
					{
						is_chg_flag= 1;
					}
					else
					{
						if (strcmp(data_user.Name,u.Name) != 0)
						{
							is_chg_flag= 1;
						}
					}
					if(data_user.Privilege != u.Privilege)
					{
						is_chg_flag = 2;
					}
				}
			}
			/*when change the name,sync to BS svr,20111205*/
			j=FDB_ChgUser(&u);
			if (ispwdreg)
				FDB_AddOPLog(ADMINPIN, OP_ENROLL_PWD, u.PIN, j, 0,0);
			if (iscardreg)
				FDB_AddOPLog(ADMINPIN, OP_ENROLL_RFCARD, u.PIN, j, 0, 0);    
			if (is_chg_flag==1)
			{
				FDB_AddOPLog(ADMINPIN, OP_CHG_USER_NAME, u.PIN, j, 0, 0);    
			}
			else if(is_chg_flag == 2)
			{
				FDB_AddOPLog(ADMINPIN, OP_CHG_USER_PRIVILEGE, u.PIN, j, 0, 0);
			}
		}
		if(j==FDB_OK)
		{
			if ((gOptions.UserExtendFormat && !gOptions.LockFunOn)/* || gOptions.CameraOpen*/) {
				SaveExtUserInfo(u.PIN);		//保存用户验证方式
			}

			if(!gOptions.IsOnlyRFMachine) {		//保存指纹
				TZKFPTemplate t;
				for(i=0;i<10;i++) {
					if (userisenroll[i] && usertmplen[i]) {
						if(gOptions.ZKFPVersion==ZKFPV10 && !fhdl) {
								break;
						}
						memset(&t,0,sizeof(t));
						j=FDB_AddTmp((char *)FDB_CreateTemplate((char*)&t, (U16)u.PIN, (char)i, 
									usertmp[i], usertmplen[i],1));
						if(j==FDB_OK) {
							char Buffer[6+4];
							((U16*)Buffer)[2]=0;
							memcpy(Buffer+4, &t, 6);
							/*send template head for event only,avoid memory overflow of event data field*/
							CheckSessionSend(EF_ENROLLFINGER,Buffer+2, 2+6);
						}
						FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, u.PIN,j,i,usertmplen[i]);
						sync();

						if(FDB_OK!=j){
							myMessageBox1(hWnd,MB_OK|MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_FLASHERROR));
							break;
						} else {
							FPDBInit();  //reload data
							isfpdbload=0;
						}
					}
				}
			}
		}
#ifdef FACE
		if( gOptions.FaceFunOn && isregface)
		{
			if(!IsNewEnroll) //change user face templates
				FDB_DeleteFaceTmps(regface[0].PIN);

			j= FDB_AddFaceTmp(&regface, FACE_NUM);
			if(FDB_OK!=j)
				printf("Save Face templates failed \n");
		}
		sync();
		isregface=0;

#endif
	}

	if (g_bIsRegUserPic)
	{
		g_bIsRegUserPic = FALSE;
		FDB_AddOPLog(ADMINPIN, OP_ENROLL_USERPIC, u.PIN, 0,0,0);
	}
	isfpreg=0;
	ispwdreg=0;
	iscardreg=0;
	isname=0;
	isacno=0;
	tempauth=-1;
	tmp_vssel=-1;
	tmp_capturesel=-1;
	isshowsavebox=0;
	continueadd=0;
	ret = News_CommitInput;
End:
	if (pushsdk_is_running() )
	{
		pushsdk_resume();
		pushsdk_data_new(FCT_OPLOG);
	}
	return ret;
}

int Invalidacno(HWND hWnd)
{
	if (oppass)
		return 1;

	if (strlen(user.PIN2)==0)
	{
		myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
		if(!ismenutimeout)
			SetFocusChild(EdAcno);
		return 0;
	}
	else
	{
		if (FDB_GetUserByCharPIN2(user.PIN2,NULL)!=NULL)
		{
			myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_ERRORPIN));
			if(!ismenutimeout) SetFocusChild(EdAcno);
			return 0;
		}
	}
	return 1;
}

extern HWND hIMEWnd;
static int  SelectNext(HWND hWnd,WPARAM wParam)
{
	Edtmp  = GetFocusChild(hWnd);
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN)
	{
		if (Edtmp == EdAcno)
		{
			if (Invalidacno(hWnd))
			{
				if (haveris)
					focuswnd=btnrisfp;
				else
				{
					//if(gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3)
					if(gOptions.IMEFunOn==1)
						focuswnd = EdName;
					else
					{
						if(!gOptions.IsOnlyRFMachine)
							focuswnd = btnfp;
						else
							focuswnd = btnpwd;
					}
				}
			}
			else
				return 1;
		}
		else if (Edtmp == btnrisfp)
		{
			if(gOptions.IMEFunOn==1)
				focuswnd = EdName;
			else
			{
				if(!gOptions.IsOnlyRFMachine)
					focuswnd = btnfp;
				else
					focuswnd = btnpwd;
			}

		}
		else if (Edtmp == EdName)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0L);
				hIMEWnd=HWND_INVALID;
			}
			if(!gOptions.IsOnlyRFMachine)
				focuswnd = btnfp;
			else
				focuswnd = btnpwd;
		}
		else if (Edtmp == btnfp)
			focuswnd = btnpwd;
		else if (Edtmp == btnpwd)
#ifdef FACE
		{
			if(gOptions.FaceFunOn) //add by caona for face
				focuswnd = facewnd;
			else if(gOptions.RFCardFunOn)
				focuswnd = btncard;
			else
				focuswnd = CbAuth;
		}
		else if(Edtmp == facewnd)
			focuswnd = facegwnd;
		else if(Edtmp == facegwnd)

#endif
		{
			if(gOptions.RFCardFunOn)
				focuswnd = btncard;
			else
				focuswnd = CbAuth;
		}
		else if(Edtmp == btncard)
			focuswnd = CbAuth;
		else if (Edtmp == CbAuth)
		{
			if(gOptions.UserExtendFormat && !gOptions.LockFunOn)
				focuswnd = CbVS;
			else
			{
				if (gOptions.CameraOpen)
					focuswnd = CbCamera;
				else
				{
					if (IDTFlag)
						focuswnd = EdGroup;
					else
						focuswnd = btnsave;
				}
			}
		}
		else if (Edtmp == CbVS)
		{
			if (gOptions.CameraOpen)
				focuswnd = CbCamera;
			else
			{
				if (IDTFlag)
					focuswnd = EdGroup;
				else
					focuswnd = btnsave;
			}
		}
		else if (IDTFlag && Edtmp==EdGroup)
			focuswnd = btnsave;
		else if (Edtmp == CbCamera)
			focuswnd = btncapture;
		else if (Edtmp == btncapture)
			focuswnd = btnsave;
		else if (Edtmp == btnsave)
			focuswnd = btnexit;
		else if (Edtmp == btnexit)
		{
			if (oppass)
			{
				if(gOptions.IMEFunOn==1)
					focuswnd = EdName;
				else
				{
					if(!gOptions.IsOnlyRFMachine)
						focuswnd = btnfp;
					else
						focuswnd = btnpwd;
				}
			}
			else
				focuswnd = EdAcno;
		}
		SetFocusChild(focuswnd);
#ifdef _TTS_
		if(gOptions.TTS_KEY)
		{
			char buffer[32];
			if(focuswnd==btnfp || focuswnd==btnpwd || focuswnd==btncard)
			{
				GetWindowText(focuswnd,buffer,32);
				TTS_Say(buffer);
			}
		}
#endif		
	}
	if (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP)
	{
		if (Edtmp == EdAcno)
		{
			if (Invalidacno(hWnd))
				focuswnd = btnexit;
			else
				return 1;
		}
		else if (Edtmp == EdName)
		{
			if(hIMEWnd != HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			if (oppass)
				focuswnd = btnexit;
			else
			{
				if (haveris)
					focuswnd=btnrisfp;
				else
					focuswnd = EdAcno;
			}
		}
		else if (Edtmp == btnrisfp)
		{
			focuswnd = EdAcno;
		}
		else if (Edtmp == btnfp)
		{
			if(gOptions.IMEFunOn==1)
				focuswnd = EdName;
			else
			{
				if (oppass)
					focuswnd = btnexit;
				else
					focuswnd = EdAcno;
			}
		}
		else if (Edtmp == btnpwd)
		{
			if(!gOptions.IsOnlyRFMachine)
				focuswnd = btnfp;
			else
			{
				if(gOptions.IMEFunOn==1)
					focuswnd = EdName;
				else
				{
					if(oppass)
						focuswnd = btnexit;
					else
						focuswnd = EdAcno;
				}
			}
		}
#ifdef FACE
		else if(Edtmp == facewnd)
		{
			focuswnd = btnpwd;
		}
		else if(Edtmp == facegwnd)
			focuswnd = facewnd;
#endif
		else if (Edtmp == CbAuth)
		{
			if(gOptions.RFCardFunOn)
				focuswnd = btncard;
#ifdef FACE
			else if(gOptions.FaceFunOn) //add by caona for face
				focuswnd = facegwnd;
#endif
			else
				focuswnd = btnpwd;
		}
		else if (Edtmp == btncard)
		{
#ifdef FACE
			if(gOptions.FaceFunOn) //add by caona for face
				focuswnd = facegwnd;
			else
#endif
				focuswnd = btnpwd;
		}
		else if (Edtmp == btnsave)
		{
			if (IDTFlag)
				focuswnd = EdGroup;
			else
			{
				if (gOptions.CameraOpen)
					focuswnd = btncapture;
				else
				{
					if(gOptions.UserExtendFormat && !gOptions.LockFunOn)
						focuswnd = CbVS;
					else
						focuswnd = CbAuth;
				}
			}
		}
		else if (IDTFlag && Edtmp==EdGroup)
		{
			if (gOptions.CameraOpen)
				focuswnd = btncapture;
			else
			{
				if(gOptions.UserExtendFormat && !gOptions.LockFunOn)
					focuswnd = CbVS;
				else
					focuswnd = CbAuth;
			}
		}
		else if (Edtmp == btncapture)
			focuswnd = CbCamera;
		else if (Edtmp == CbCamera)
		{
			if (gOptions.UserExtendFormat && !gOptions.LockFunOn)
				focuswnd = CbVS;
			else
				focuswnd = CbAuth;
		}
		else if (Edtmp == CbVS)
			focuswnd = CbAuth;
		else if (Edtmp == btnexit)
			focuswnd = btnsave;

		SetFocusChild(focuswnd);
#ifdef _TTS_
		if(gOptions.TTS_KEY)
		{
			char buffer[32];
			if(focuswnd==btnfp || focuswnd==btnpwd || focuswnd==btncard)
			{			
				GetWindowText(focuswnd,buffer,32);
				TTS_Say(buffer);
			}
		}
#endif		
	}

	//if(focuswnd == EdName) {
	//	SendMessage (EdName, EM_SETCARETPOS, 0, char_pos);
	//}
	return 1;
}

static int getHeight(row,offset)
{
	return (10+25*row-offset);
}

char* whint[18];
static void LoadWindowHint(void)
{
	whint[0]=LoadStrByID(MID_ACNO);
	whint[1]=LoadStrByID(MID_NAME);
	whint[2]=LoadStrByID(MID_FINGER);
	whint[3]=LoadStrByID(MID_REGFP);
	whint[4]=LoadStrByID(MID_PWD);
	whint[5]=LoadStrByID(MID_REGPWD);
	whint[6]=LoadStrByID(HID_CHGPWD);
	whint[7]=LoadStrByID(MID_CARD);
	whint[8]=LoadStrByID(MID_CHANGE_CARD);
	whint[9]=LoadStrByID(MID_ENROLL_CARD);
	whint[10]=LoadStrByID(MID_AUTH);
	whint[11]=LoadStrByID(MID_SAVE);
	whint[12]=LoadStrByID(MID_EXIT);
	whint[13]=LoadStrByID(MID_AUTHSERVER_REGISTER);
	whint[14]=LoadStrByID(MID_USER_VRTP);
	whint[15]=LoadStrByID(MID_CAMERA_MODE);
	whint[16]=LoadStrByID(MID_CAMERA_MODE2);
	whint[17]=LoadStrByID(MID_USER_GP);
}

static void InitWindow (HWND hWnd)
{
	int rows=0;
	int i;
	int posX,posX1,posX2,posX3,posX4,posX5,posX6,posX7,posX8,posX9;
	if (fromRight==1)  //modify by jazzy 2008.07.24
	{
		posX=30;
		posX1=275+gOptions.GridWidth*2;
		posX2=78+gOptions.GridWidth;
		posX3=5;
		posX4=55;
		posX5=115+gOptions.GridWidth*2;
		posX6=100;
		posX7=155+gOptions.GridWidth;
		posX8=40;
		posX9=10;
	}
	else
	{
		posX=0;
		posX1=10;
		posX2=60;
		posX3=203;
		posX4=170;
		posX5=85;
		posX6=94;
		posX7=60;
		posX8=0;
		posX9=220+gOptions.ControlOffset;
	}

	//printf("%s:fromRight:%d\n",__FUNCTION__,fromRight);
	CreateWindow(CTRL_STATIC, whint[0], WS_VISIBLE | SS_CENTER, LB_ACNO, posX1, getHeight(rows,0), 40, 23, hWnd, 0);
	if(!oppass)
	{
		if (!gOptions.IsSupportAuthServer)
		{
			EdAcno=CreateWindow(CTRL_SLEDIT,tmpchar,WS_VISIBLE|WS_TABSTOP|ES_AUTOSELECT|ES_BASELINE|WS_BORDER,ID_USERACNO,posX2,getHeight(rows,4),180,23,hWnd,0);
		}
		else
		{
			EdAcno=CreateWindow(CTRL_SLEDIT,tmpchar,WS_VISIBLE|WS_BORDER,ID_USERACNO,posX2+posX8,getHeight(rows,4),140,23,hWnd,0);
			if(CheckAllAuthServerRuning()&&(gOptions.AuthServerEnabled!=ONLY_LOCAL) && (gOptions.AuthServerCheckMode==0))
			{
				btnrisfp=CreateWindow(CTRL_BUTTON,whint[13],WS_VISIBLE|WS_BORDER|BS_PUSHLIKE,ID_RISFP,posX3,getHeight(rows,4),110,23,hWnd,0);
				haveris=1;
			}
		}
	}
	else
	{
		EdAcno = CreateWindow(CTRL_STATIC, tmpchar, WS_VISIBLE | SS_LEFT, ID_USERACNO, posX2, getHeight(rows,0), 180, 23, hWnd, 0);
	}
	rows++;

	//if(gOptions.IMEFunOn && gOptions.TFTKeyLayout!=3)
	if(gOptions.IMEFunOn)
	{
		CreateWindow(CTRL_STATIC,whint[1], WS_VISIBLE | SS_CENTER, LB_NAME,posX1,getHeight(rows,0),40,23,hWnd,0);
		EdName = CreateWindow(CTRL_SLEDIT,tmpcharname, WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER ,ID_NAME,posX2, getHeight(rows,4),180,23,hWnd,0);
		//SendMessage(EdName,EM_LIMITTEXT,24,0L);
		SendMessage(EdName,EM_LIMITTEXT,23,0L);
		rows++;
	}

	if(!gOptions.IsOnlyRFMachine)
	{
		CreateWindow(CTRL_STATIC, whint[2], WS_VISIBLE | SS_CENTER, LB_FP, posX1, getHeight(rows,0), 30, 23, hWnd, 0);
		btnfp = CreateWindow(CTRL_BUTTON,whint[3],WS_VISIBLE | WS_BORDER|BS_PUSHLIKE,ID_FP,posX7,getHeight(rows,4),100,23,hWnd,0);
		EdFp = CreateWindow(CTRL_STATIC, fpcountchar, WS_VISIBLE |SS_CENTER, LB2_FP, posX4, getHeight(rows,0), 90, 23, hWnd, 0);
		rows++;
	}

	CreateWindow (CTRL_STATIC, whint[4], WS_VISIBLE | SS_CENTER, LB_PWD, posX1, getHeight(rows,0), 60, 23, hWnd, 0);
	if (havepwd==0)
		btnpwd = CreateWindow(CTRL_BUTTON,whint[5],WS_VISIBLE | WS_BORDER,ID_PWD,posX7,getHeight(rows,4),100,23,hWnd,0);
	else
		btnpwd = CreateWindow(CTRL_BUTTON,whint[6],WS_VISIBLE | WS_BORDER,ID_PWD,posX7,getHeight(rows,4),100,23,hWnd,0);
	pwdbmp_height = getHeight(rows, 0);
	rows++;

#ifdef FACE
	if(gOptions.FaceFunOn) // add by caona for face
	{
		CreateWindow (CTRL_STATIC, LoadStrByID(MID_FACE), WS_VISIBLE | SS_CENTER, ID_FACE, posX1, getHeight(rows,0), 60, 23, hWnd, 0);
		facewnd=CreateWindow(CTRL_BUTTON,LoadStrByID(FACE_REG),WS_VISIBLE | WS_BORDER,ID_FACE_REG,posX7,getHeight(rows,4),100,23,hWnd,0);
		facebmp_height = getHeight(rows, 0);
		rows++;

		char buffer[20];
		CreateWindow (CTRL_STATIC, LoadStrByID(MID_DEF_FG), WS_VISIBLE | SS_CENTER, ID_FACE_SG, posX1, getHeight(rows,0), 60, 23, hWnd, 0);
		sprintf(buffer,"(1~%d)",gFaceGroupNum);
		CreateWindow (CTRL_STATIC, buffer, WS_VISIBLE | SS_CENTER, ID_FACE_SGR, posX4, getHeight(rows,0), 90, 23, hWnd, 0);
		sprintf(buffer,"%d",f_group);
		facegwnd=CreateWindow(CTRL_SLEDIT,buffer,WS_VISIBLE|WS_BORDER,ID_FACE_G,posX7,getHeight(rows,4),100,23,hWnd,0);
		SendMessage(facegwnd,EM_LIMITTEXT,2,0L);
		SetWindowText(facegwnd,buffer);
		SendMessage(facegwnd,EM_SETCARETPOS,0,strlen(buffer));
		rows++;
	}
#endif

	if(gOptions.RFCardFunOn)
	{
		CreateWindow(CTRL_STATIC, whint[7], WS_VISIBLE | SS_CENTER, LB_AUTH, posX1, getHeight(rows,0),60,23,hWnd,0);
		if(havecard)
			btncard = CreateWindow(CTRL_BUTTON, whint[8],WS_VISIBLE | WS_BORDER, ID_CARD,posX7,getHeight(rows,4),100,23,hWnd,0);
		else
			btncard = CreateWindow(CTRL_BUTTON, whint[9],WS_VISIBLE | WS_BORDER, ID_CARD,posX7,getHeight(rows,4),100,23,hWnd,0);
		EdCard = CreateWindow(CTRL_STATIC,mycard, WS_VISIBLE | SS_CENTER, ID_SCARD, posX4,getHeight(rows,0),140,23,hWnd,0);
		rows++;
	}

	CreateWindow (CTRL_STATIC, whint[10], WS_VISIBLE | SS_CENTER, LB_AUTH, posX1-posX, getHeight(rows,0), 60, 23, hWnd, 0);
	CbAuth = CreateWindow (CTRL_COMBOBOX, " ", WS_VISIBLE | CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,ID_AUTH, posX5, getHeight(rows,4), 120, 23, hWnd, 0);
	SendMessage(CbAuth, CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_PRI_NONE));
	/*dsl 2012.4.17*/
	SendMessage(CbAuth, CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_PRI_ENROLL));
	/*zxz 2012-10-12*/
	if (TESTPRIVILLEGE((PRI_SUPERVISOR|PRI_OPTIONS)) || (ADMINPIN==0))
	{
		SendMessage(CbAuth, CB_ADDSTRING, 0, (LPARAM)LoadStrByID(HID_PRI_ADMIN));
	}
	rows++;

	if(gOptions.UserExtendFormat && !gOptions.LockFunOn)		//用户验证方式(08.1.10)
	{
		CreateWindow(CTRL_STATIC, whint[14], WS_VISIBLE|SS_CENTER, LB_USVF, posX1-posX, getHeight(rows, 0), 65, 23, hWnd, 0);
		CbVS = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,ID_USVF, posX5, getHeight(rows, 4), 120, 23, hWnd, 0);

		for(i=0;i<VS_NUM;i++)
		{
			SendMessage(CbVS, CB_ADDSTRING, 0, (LPARAM)myVerifyStyle[i]);
		}
		rows++;
	}

	if (gOptions.CameraOpen)
	{
		CreateWindow(CTRL_STATIC, whint[15], WS_VISIBLE|SS_CENTER, LB_CPMD, posX1-posX, getHeight(rows, 0), 65, 23, hWnd, 0);
		CbCamera = CreateWindow(CTRL_COMBOBOX, "", WS_VISIBLE|CBS_READONLY|CBS_SPINLIST|CBS_SPINARROW_LEFTRIGHT|CBS_AUTOFOCUS,ID_CPMD, posX5, getHeight(rows, 4), 120, 23, hWnd, 0);
		//使用全局设置/不拍照/拍照/拍照并保存
		SendMessage(CbCamera, CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CAMERA_MODE5));
		for(i=0; i<3; i++)
		{
			SendMessage(CbCamera, CB_ADDSTRING, 0, (LPARAM)LoadStrByID(MID_CAMERA_MODE1+i));
		}

		btncapture = CreateWindow(CTRL_BUTTON, whint[16], WS_VISIBLE|BS_DEFPUSHBUTTON|WS_BORDER, ID_CAPTURE,posX9, 130, 85, 23, hWnd, 0);
		rows++;
	}

	if (IDTFlag)
	{
		CreateWindow(CTRL_STATIC, whint[17], WS_VISIBLE | SS_CENTER, 0x0010, posX1, getHeight(rows, 0), 80, 23, hWnd, 0);
		EdGroup = CreateWindow(CTRL_SLEDIT, uGroup, WS_VISIBLE | ES_AUTOSELECT | WS_BORDER, ID_USERACNO,
				posX6, getHeight(rows, 4), 102, 23, hWnd, 0);
		SendMessage(EdGroup, EM_LIMITTEXT, 10, 0L);
	}

	btnsave = CreateWindow(CTRL_BUTTON, whint[11], WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_SAVE,posX9,160,85,23,hWnd,0);
	btnexit = CreateWindow(CTRL_BUTTON, whint[12], WS_VISIBLE | BS_DEFPUSHBUTTON|WS_BORDER, ID_EXIT,posX9,190,85,23,hWnd,0);

	SendMessage(EdAcno,EM_LIMITTEXT,gOptions.PIN2Width,0L);
}

static int testwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	char buff3[100];
	int id,nc;
	HWND ttWnd;
	int ret=0;
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
			//		printf("%s  MSG_CREATE\n",__FUNCTION__);
			InitWindow(hWnd);
			UpdateWindow(hWnd,TRUE);

			if ((gOptions.UserExtendFormat && !gOptions.LockFunOn) || (gOptions.CameraOpen))
			{
				SetUserVerify(user.PIN);
			}

			/*dsl 2012.4.17*/
			//g_authsel = (user.Privilege>0)? 1:0;
			g_authsel = switch_pri_for_compatibility(user.Privilege);
			//printf("g_authsel:%d\n", g_authsel);
			SendMessage(CbAuth, CB_SETCURSEL, g_authsel, 0);

			if(!oppass)
			{
				SetFocusChild(EdAcno);
				SendMessage(EdAcno,EM_SETCARETPOS,0,strlen(tmpchar));
			}
			else
			{
				if(gOptions.IMEFunOn==1)
				{
					SetFocusChild(EdName);
				}
				else
				{
					if (!gOptions.IsOnlyRFMachine)
					{
						SetFocusChild(btnfp);
					}
					else
					{
						SetFocusChild(btnpwd);
					}
				}
			}
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

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

				if (havepwd)
				{
					if (fromRight==1)
					{
						FillBoxWithBitmap(hdc, 70, pwdbmp_height, 0, 0, &pwdhintbmp);
					}
					else
					{
						FillBoxWithBitmap(hdc, 170, pwdbmp_height, 0, 0, &pwdhintbmp);
					}
				}

#ifdef FACE
				if(haveface && gOptions.FaceFunOn)
				{
					if (fromRight==1)
					{
						FillBoxWithBitmap(hdc, 70, facebmp_height-4, 0, 0, &facebmp);
					}
					else
					{
						FillBoxWithBitmap(hdc, 170, facebmp_height-4, 0, 0, &facebmp);
					}
				}

#endif
				if (fGetDC)
				{
					ReleaseDC (hdc);
				}
				return 0;
			}

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
				if(1==keyupFlag)
				{
					keyupFlag=0;
				}
				else
				{
					break;
				}
			}
			if (gOptions.KeyPadBeep)
			{
				ExKeyBeep();
			}

			ttWnd=GetFocusChild(hWnd);
			if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
			{
				if (ttWnd==EdAcno)
				{
					if(Invalidacno(hWnd))
					{
						if(gOptions.IMEFunOn==1)
						{
							SetFocusChild(EdName);
						}
						else
						{
							if(!gOptions.IsOnlyRFMachine)
							{
								SetFocusChild(btnfp);
							}
							else
							{
								SetFocusChild(btnpwd);
							}
						}
					}
				}
				else if(ttWnd==EdName)
				{
					if(!gOptions.IsOnlyRFMachine)
					{
						SetFocusChild(btnfp);
					}
					else
					{
						SetFocusChild(btnpwd);
					}
				}
				else if(ttWnd==btnfp)
				{
					SendMessage(hWnd, MSG_COMMAND, ID_FP, 0);
				}
				else if(ttWnd==btnpwd)
				{
					SendMessage(hWnd, MSG_COMMAND, ID_PWD,0);
				}
#ifdef FACE
				else if(ttWnd==facewnd)
				{
					SendMessage(hWnd, MSG_COMMAND, ID_FACE_REG,0);
				}
#endif
				else if(ttWnd==btncard)
					SendMessage(hWnd, MSG_COMMAND, ID_CARD,0);
				else if(ttWnd==btnsave)
					SendMessage(hWnd, MSG_COMMAND, ID_SAVE,0);
				else if(ttWnd==btnexit)
					SendMessage(hWnd, MSG_COMMAND, ID_EXIT,0);
				else if(ttWnd==btnrisfp)
					SendMessage(hWnd, MSG_COMMAND, ID_RISFP,0);
				else if(ttWnd==btncapture)
					SendMessage(hWnd, MSG_COMMAND, ID_CAPTURE, 0);
				return 0;
			}
			else if(LOWORD(wParam)==SCANCODE_F9 ||
					((gOptions.TFTKeyLayout==0 || gOptions.TFTKeyLayout==4) && (LOWORD(wParam)==SCANCODE_F11)))
			{
				if((ttWnd==EdName) && (gOptions.IMEFunOn==1&& gOptions.TFTKeyLayout!=3))
				{
					T9IMEWindow(hWnd,0,200,gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
				}
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
			{
				if (SelectNext(hWnd,wParam))
				{
					return 0;
				}
			}
			else if((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT) || (LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
			{
				if((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)&& gOptions.TFTKeyLayout==3)
				{
					if((ttWnd==EdName) && (gOptions.IMEFunOn==1))
					{
						T9IMEWindow(hWnd,0,200, gOptions.LCDWidth, gOptions.LCDHeight,gOptions.HzImeOn);
					}
				}

				if(ttWnd==btnsave)
				{
					SetFocusChild(btnexit);
				}
				else if(ttWnd==btnexit)
				{
					SetFocusChild(btnsave);
				}
				else if(ttWnd==CbAuth)
				{
					g_authsel=SendMessage(CbAuth, CB_GETCURSEL,0,0);
					/*Only enroller privilege and Change mode*/
					if (TEST_ONLY_ENROLL_PRIVILLEGE && (oppass==1)) {
						if (tempauth < 2) { 	/*The current user is Administator*/
							g_authsel = (g_authsel==0)?1:0;
						}
					} else {
						switch (g_authsel) {
							case 0: 
								g_authsel=1;
								break;
							case 1:
								g_authsel=2;
								break;
							default:
								g_authsel=0;
						}
					}
					SendMessage(CbAuth,CB_SETCURSEL,g_authsel,0);
				}
				else if(ttWnd==CbVS)
				{
					if(LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT 
							|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
					{
						if(--g_vssel<0)
							g_vssel=VS_NUM-1;
					}
					else if(LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT)
					{
						if(++g_vssel>VS_NUM-1)
							g_vssel=0;
					}
					SendMessage(CbVS, CB_SETCURSEL, g_vssel, 0);
				}
				else if(ttWnd==CbCamera)
				{
					if (LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT
							|| ((gOptions.TFTKeyLayout==3) && LOWORD(wParam)==SCANCODE_BACKSPACE))
					{
						if(--g_cpsel < 0)
							g_cpsel = 3;
					}
					else
					{
						if(++g_cpsel > 3)
							g_cpsel = 0;
					}
					SendMessage(CbCamera, CB_SETCURSEL, g_cpsel, 0);
				} else {
					break;/*add by zxz 2013-01-10,姓名和工号左右键按默认方式处理*/
				}
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				memset(buff3,0,sizeof(buff3));
				GetWindowText(EdName, buff3, 24);
				if (
#ifdef FACE
						(gOptions.FaceFunOn && (isregface || DataChg)) || 
#endif
						g_bIsRegUserPic||isfpreg || ispwdreg || (gOptions.RFCardFunOn && iscardreg) || (tempauth != g_authsel)
						||(gOptions.UserExtendFormat && !gOptions.LockFunOn && (tmp_vssel!=g_vssel)) 
						||(nstrcmp(buff3, tmpcharname, 24)!=0) || (gOptions.CameraOpen && tmp_cpsel != g_cpsel))
				{
					if (isfpreg)
					{
						FPDBInit();
					}

					isshowsavebox+=2;
					ret=SaveUserEnrolled(!oppass,hWnd);
					if(ret==3)
					{
						return 0;
					}
				}
				PostMessage (hWnd, MSG_CLOSE, 0, 0L);
			}
			else if ((LOWORD(wParam)==SCANCODE_MENU))
			{
				SendMessage(hWnd,MSG_COMMAND,ID_SAVE,0);
				return 0;
			}
			else if ((LOWORD(wParam)==SCANCODE_BACKSPACE )&& (ttWnd==EdAcno))
			{
				char tmpacnobuf[30];
				GetWindowText(EdAcno,tmpacnobuf,24);
				if (tmpacnobuf && (strcmp(tmpacnobuf,tmpchar)==0))
				{
					SetWindowText(EdAcno,"");
				}
				//Liaozz 20081006 fix bug second 26
				if (tmpacnobuf && gOptions.IsSupportAuthServer) {
					SetWindowText(EdAcno,"");
				}
				//Liaozz end
			}
			break;

		case MSG_CLOSE:
#ifdef _TTS_
			if(gOptions.TTS_KEY)
			{
				TTS_Stop();
			}
#endif			
			//printf("%s hIMEWnd:%d\n",__FUNCTION__,hIMEWnd);
			if(hIMEWnd!=HWND_INVALID)
			{
				SendMessage(hIMEWnd,MSG_CLOSE,0,0);
				hIMEWnd=HWND_INVALID;
			}
			//UnloadBitmap(&userbmp);
			UnloadBitmap(&pwdhintbmp);
			//MainWindowCleanup (hWnd);
			DestroyMainWindow(hWnd);
			return 0;

		case MSG_KILLFOCUS:
			return 0;

		case MSG_CHAR:
			{
				HWND curWnd=GetFocusChild(hWnd);
				if(curWnd==EdName)
				{
					memset(buff3,0,sizeof(buff3));
					GetWindowText(EdName,buff3,24);
					if(strlen(buff3)==23 && wParam > 0xa0)
						return 0;
				}
				else if(curWnd==EdAcno)
				{
					if(wParam==0x20) return 0;
				}
#ifdef FACE
				else if(curWnd==facegwnd) //add by caona for face
				{
					memset(buff3,0,sizeof(buff3));
					GetWindowText(facegwnd,buff3,2);
					if(f_group != atoi(buff3))
					{
						DataChg=1;
						FaceDBChg=1;
					}
					if(wParam==0x20) return 0;
				}
#endif
				break;
			}

		case MSG_COMMAND:
			id = LOWORD(wParam);
			nc = HIWORD(wParam);
			if (id==ID_NAME && nc==EN_CHANGE)
			{
				memset(buff3,0,sizeof(buff3));
				GetWindowText(EdName,buff3,24);
				if(strlen(buff3))
					isname=1;
			}

			if (id==ID_USERACNO && nc==EN_CHANGE)
			{
				if (nc==EN_CHANGE)
				{
					memset(buff3,0,sizeof(buff3));
					GetWindowText (EdAcno, buff3, 24);
					if (strlen(buff3))
					{
						if (!oppass)
						{
							isacno=1;
						}
						memset(user.PIN2,0,24);
						if (buff3[0]=='0')
						{
							SetWindowText(EdAcno,"");
							break;
						}
						memcpy(user.PIN2,buff3,strlen(buff3));
					}
					else
					{
						memset(user.PIN2,0,24);
					}
				}
			}
			switch (id)
			{
				case ID_FP:
					if (strlen(user.PIN2)==0)
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
						SetFocusChild(EdAcno);
					}
					else
					{
						if ((!oppass)&&(!(NULL==FDB_GetUserByCharPIN2(user.PIN2,NULL))))
						{
							myMessageBox1(hWnd, MB_OK | MB_ICONSTOP, LoadStrByID(MID_APPNAME),
									LoadStrByID(HID_ERRORPIN));
							SetFocusChild(EdAcno);
						}
						else
						{
							processregfp(hWnd);
						}
					}
					break;

#ifdef FACE
				case ID_FACE_REG:  //add by caona for face 
					if (strlen(user.PIN2)==0)
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
						if(!ismenutimeout) SetFocusChild(EdAcno);
					}
					else
					{
						if(haveface && MessageBox1(hWnd,LoadStrByID(MID_DEL_ORGDATA), LoadStrByID(MID_APPNAME), 
									MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)!=IDOK)
						{
							break;
						}
						if(haveface)
						{
							FaceDBChg=1;
							FDB_DeleteFaceTmps(user.PIN); //del user old face templates
							haveface=0;
						}

						if(FDB_CntFaceUser()>=  gOptions.MaxFaceCount*100)
						{
							myMessageBox1(HWND_DESKTOP,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),
									LoadStrByID(HID_EXCEED));
							return 0;
						}
						//into register
						isregface=0;
						if(CreateFaceEnrollWindow(hWnd, user.PIN))  //reg face ok
						{
							FaceDBChg=1;
							isregface=1;
							haveface=1;
							printf("CreateFaceEnrollWindow  isregface %d \n", isregface);
						}
						UpdateWindow(hWnd,TRUE);
					}
					break;

#endif
				case ID_PWD:
					if (strlen(user.PIN2)==0)
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
						SetFocusChild(EdAcno);
					}
					else
					{
						PwdEnrolled=FALSE;
						if (!oppass)
						{
							memset(mypwd,0, sizeof(mypwd));
							if (CreatePwdWindow(hWnd,mypwd,TRUE))
							{
								ispwdreg=1;
								havepwd=1;
								PwdEnrolled=TRUE;
								memset(user.Password,0,PWDLIMITLEN);
								nstrcpy(user.Password,mypwd,PWDLIMITLEN);
								SetWindowText(EdPwd3,mypwd);
								UpdateWindow(hWnd,TRUE);
							}
						}
						else
						{
							if (CreatePwdWindow(hWnd,mypwd,FALSE))
							{
								ispwdreg=1;
								havepwd=1;
								PwdEnrolled=TRUE;
								memset(user.Password,0,PWDLIMITLEN);
								nstrcpy(user.Password,mypwd,PWDLIMITLEN);
								SetWindowText(EdPwd3,mypwd);
								UpdateWindow(hWnd,TRUE);
							}
						}
						//		printf("mypwd:%s\n", mypwd);
					}
					break;

				case ID_CARD:
					if (strlen(user.PIN2)==0)
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
						if(!ismenutimeout)
						{
							SetFocusChild(EdAcno);
						}
					}
					else
					{
						if(CreateRFCardWindow(hWnd,mycard,(int*)&cardvalue,!oppass))
						{
							iscardreg=1;
							havecard=1;
							memset(user.Card,0,4);
							memcpy(user.Card,&cardvalue,4);
							SetWindowText(EdCard,mycard);
							UpdateWindow(hWnd,TRUE);
						}
					}
					break;

				case ID_SAVE:
					if (strlen(user.PIN2)==0)
					{
						myMessageBox1(hWnd,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(MID_NOACNOHINT));
						if(!ismenutimeout)
						{
							SetFocusChild(EdAcno);
						}
					}
					else
					{
						if (IDTFlag && bChgGroup()==2)
						{
							break;
						}

						isshowsavebox++;
						memset(buff3,0,sizeof(buff3));
						GetWindowText(EdName, buff3, 24);

						if (
#ifdef FACE
								(gOptions.FaceFunOn && (isregface==0 && DataChg==0)) &&
#endif
								((!gOptions.RFCardFunOn && isacno==1 && isfpreg==0 
								  && ispwdreg==0 && (nstrcmp(buff3, tmpcharname, 24)==0)) ||
								 (gOptions.RFCardFunOn && iscardreg==0 && isacno==1 
								  && isfpreg==0 && ispwdreg==0  &&  (g_bIsRegUserPic==0) && (nstrcmp(buff3, tmpcharname, 24)==0))))
						{
							isshowsavebox++;
							PostMessage(hWnd,MSG_CLOSE,0,0L);
							break;
						}
						else if(
#ifdef FACE
								(gOptions.FaceFunOn && (isregface||DataChg) ) ||
#endif
								isfpreg || ispwdreg || (gOptions.RFCardFunOn && iscardreg) ||
								(tempauth!=g_authsel) ||
								(gOptions.UserExtendFormat && !gOptions.LockFunOn && (tmp_vssel!=g_vssel)) ||
								(gOptions.CameraOpen && tmp_cpsel != g_cpsel) ||
								(nstrcmp(buff3, tmpcharname, 24)!=0) ||
								(IDTFlag && bChgGroup()==1) ||
								(g_bIsRegUserPic==1))
						{	
							ret = SaveUserEnrolled(!oppass,hWnd);
							if(ret == News_CommitInput)
							{
								//DelayMS(10000);
								if (!oppass)
								{
									if(((FDB_CntUserByMap()>=gOptions.MaxUserCount*100)))
									{
										PostMessage(hWnd,MSG_CLOSE,0,0L);
										break;
									}
#if 1
									if(MessageBox1(hWnd,LoadStrByID(MID_GOONINPUT), 
												LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
									{
										continueadd=1;
										Initnewinfo();
										UpdateWindow(hWnd,TRUE);
									}
									else
									{
										if(!ismenutimeout)
										{
											PostMessage(hWnd,MSG_CLOSE,0,0L);
										}
									}
#endif
								}
								else
								{
									PostMessage(hWnd,MSG_CLOSE,0,0L);
								}
							}
							else if(ret==3)
							{
								return 1;
							}
							else
							{
								PostMessage(hWnd,MSG_CLOSE,0,0L);
							}
						}
						else
						{
							PostMessage(hWnd,MSG_CLOSE,0,0L);
						}
					}

					break;

				case ID_EXIT:
					//printf("xsen test isfpreg=%d,ispwdreg=%d,iscardreg=%d,tempauth=%d,g_authsel=%d,IDTFlag=%d\n",isfpreg,ispwdreg,iscardreg,tempauth,g_authsel,IDTFlag);
					memset(buff3,0,sizeof(buff3));
					GetWindowText(EdName, buff3, 24);
					if(
#ifdef FACE
							(gOptions.FaceFunOn && (isregface||DataChg) ) ||
#endif
							g_bIsRegUserPic ||   isfpreg || ispwdreg || (gOptions.RFCardFunOn && iscardreg)||
							(tempauth!=g_authsel) ||
							(gOptions.UserExtendFormat && !gOptions.LockFunOn && 
							 (tmp_vssel!=g_vssel)) ||
							(gOptions.CameraOpen && tmp_cpsel != g_cpsel)||
							nstrcmp(buff3, tmpcharname, 24)!=0||
							(IDTFlag && bChgGroup()==1))
					{
						isshowsavebox+=2;
						ret=SaveUserEnrolled(!oppass,hWnd);
						if(ret==3)
							return 0;
					}
					PostMessage (hWnd, MSG_CLOSE, 0, 0L);
					break;

				case ID_RISFP:
					{
						int rispin=0;
						rispin = ValidAcnoFromRemote(hWnd);
						if (rispin)
						{
							CreateRisFpEnrollWindow(hWnd,rispin,1);
						}
						break;
					}

				case ID_CAPTURE:
					{
						int b, c, q, s;
						b = gOptions.CameraBright;
						c = gOptions.CameraContrast;
						q = gOptions.PicQuality;
						s = gOptions.CameraScene;

						gOptions.enableVideoRotate=0;
						CreateVideoAdjustWindow(hWnd, &b, &c, &q, &s, 1);
						gOptions.enableVideoRotate=1;

						return 0;
					}
			}
			break;

	}
	return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

/*opmode: 0--new, 1--modify*/
int CreateAddUserWindow(HWND hOwner,int opmode,int *pin)
{
	MSG msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	//LoadBitmap(HDC_SCREEN,&userbmp,GetBmpPath("submenubg.jpg"));
	LoadBitmap(HDC_SCREEN,&pwdhintbmp,GetBmpPath("key.gif"));
#ifdef FACE
	if(gOptions.FaceFunOn) //add by caona for face
		LoadBitmap(HDC_SCREEN,&facebmp,GetBmpPath("faceok.gif"));
#endif

	/*When add a new user, check user capacity.*/
	if((!opmode)&&((FDB_CntUserByMap()>=gOptions.MaxUserCount*100)))
	{
		UnloadBitmap(&pwdhintbmp);
		myMessageBox1(hOwner,MB_OK | MB_ICONSTOP,LoadStrByID(MID_APPNAME),LoadStrByID(HID_EXCEED));
		return 0;
	}

	oppass = opmode;
	g_pin = *pin;
	continueadd=0;
	haveris=0;
	Initnewinfo();
	LoadWindowHint();

	hOwner = GetMainWindowHandle (hOwner);
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
	CreateInfo.dwExStyle = WS_EX_NONE;
	if (oppass) {
		CreateInfo.spCaption = LoadStrByID(MID_EDITUSER);
	} else {
		CreateInfo.spCaption = LoadStrByID(MID_AD_ADDUSER);
	}
	CreateInfo.hMenu = 0;
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = testwinproc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = gOptions.LCDWidth;
	CreateInfo.by = gOptions.LCDHeight;
	CreateInfo.iBkColor = 0x00FFA2BE;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hOwner;

	hMainWnd = CreateMainWindow(&CreateInfo);
	if (hMainWnd == HWND_INVALID) {
		return -1;
	}
	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,hMainWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup(hMainWnd);
	*pin=g_pin;
	return 0;
}

int CreateNewUserWindow(HWND hOwner)
{
	int addtmppin;
	addtmppin=0;
	CreateAddUserWindow(hOwner,0,&addtmppin);
	return 0;
}
#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

