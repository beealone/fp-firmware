#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mywindows.h>
#include <minigui/mgext.h>
#include "ssrpub.h"
#include "options.h"
#include "msg.h"
#include "sensor.h"
#include "commu.h"
#include "exfun.h"
#include "ssrcommon.h"
#include "main.h"
#include "arca.h"

//#include "serial.h"
//#include "lcdmenu.h"
#include "flashdb.h"
#include "finger.h"
//#include "zkfp.h"
//#include "net.h"
//#include "utils.h"
#include "zlg500b.h"
//#include "lcm.h"
//#include "kb.h"
#include "convert.h"
#include "iclsrw.h"
#include <minigui/tftmullan.h>
#include "tftmsgbox.h"

static void BeginWriteCard(HWND hWnd);

#define IDC_TIMER_MF 902
enum _MFCardOptStyle
{
	EnrollPIN,
	EnrollFP,
	ClearInfo,
	CopyInfo,
	CopyOnLine,
};

enum _MFCardOptState
{
	MFReady,
	MFSuccess,
	MFFaild,
};

BITMAP mfbmpbkg;
BITMAP mfbmpok;
BITMAP mfbmpcard;
BITMAP mfbmpfail;
BITMAP mfbmphint;

BITMAP tmpbkg;

typedef struct _EnrollFpSession_{
        BYTE *(Tmps[3]);
        BYTE *GTmp;
        int Index;
        int len;
}TEnrollFpSession, *PEnrollFpSession;

#define LB_STATIC       1000
#define ID_PIN2         1271
#define LB_PIN2		1002

HWND EdPIN2,LbPIN2,btnSave1,btnSave2;

RECT BmpRec={170,50,155,180};

PSensorBufInfo FpSensorInfo;
int OptStyle;           //Operation model
extern int gMachineState;
TFPCardOP tmpFPCard;
TUser tmpUser;
static TEnrollFpSession fpes;

int tmplens[4];
//char tmps[4][1024];
char tmps[4][2048];
static BYTE tmpfp[3][2048];
U8 TMPBUF[10240];

int fpindex=0;
int isenrolling=0;
int EnrollNext=0;
int copyoperate=0;
int copycontent=0;		//复制内容（0:只复制用户信息，1:复制用户信息和指纹）
int isnewUser=0, isnewTemplate=0;

HWND MFCardWnd = (HWND)HWND_INVALID;	//add by cxp at 2010-04-20
int result;
//把 Templates 指定的数个指纹，压缩放到Temp中，然后返回新的长度
int PackTemplate(U8 *Temp, U8 *Templates[], int TempCount, int ResSize)
{
	int TempLen[10],NewLen[10];
	int i, size=0, newsize=0;
	//U8 tmp[1024];
	U8 tmp[2048];
	for(i=0;i<TempCount;i++)
	{
		TempLen[i]=BIOKEY_TEMPLATELEN(Templates[i]);
		NewLen[i]=TempLen[i];
		size+=TempLen[i];
	}
	newsize=0;
	if(size>ResSize)
	{
		for(i=0;i<TempCount;i++)
		{
			NewLen[i]=TempLen[i]*ResSize/size;
			memcpy(tmp, Templates[i], TempLen[i]);
			NewLen[i]=BIOKEY_SETTEMPLATELEN(tmp, NewLen[i]);
			memcpy(Temp+newsize, tmp, NewLen[i]);
			newsize+=NewLen[i];
		}
	}
	else
		for(i=0;i<TempCount;i++)
		{
			memcpy(Temp+newsize, Templates[i], TempLen[i]);
			newsize+=TempLen[i];
		}
	return newsize;
}

/***************************************************************************/
static void ShowInfoBox(HWND hWnd, char* msgstr)
{
	HDC hdc;

	hdc=GetClientDC(hWnd);
	GetBitmapFromDC(hdc, 0,60,gOptions.LCDWidth, gOptions.LCDHeight-60, &tmpbkg);

	MessageBox1(hWnd, msgstr, LoadStrByID(MID_APPNAME), MB_OK | MB_ICONINFORMATION | MB_BASEDONPARENT);
	if(!ismenutimeout)
		FillBoxWithBitmap(hdc,0,60,0,0,&tmpbkg);
	UnloadBitmap(&tmpbkg);
	ReleaseDC(hdc);
}

static int ShowQuestionBox(HWND hWnd, char* msgstr, int defbtn)
{
	int res=0;
	HDC hdc;

	hdc=GetClientDC(hWnd);
	GetBitmapFromDC(hdc, 0,60,gOptions.LCDWidth, gOptions.LCDHeight-60, &tmpbkg);

	if(defbtn==0)
		res=(MessageBox1(hWnd,msgstr,LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1|MB_BASEDONPARENT)==IDOK);
	else if(defbtn==1)
		res=(MessageBox1(hWnd,msgstr,LoadStrByID(MID_APPNAME),MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2|MB_BASEDONPARENT)==IDOK);
	if(!ismenutimeout)
		FillBoxWithBitmap(hdc,0,60,0,0,&tmpbkg);
	UnloadBitmap(&tmpbkg);
	ReleaseDC(hdc);
	return res;

}
//显示提示图片
static void ShowOptBmp(HWND hWnd, BITMAP bmp)
{
	int tmpvalue = 0;
        HDC hdc=GetClientDC(hWnd);
        tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc,PIXEL_lightwhite);
        InvalidateRect(hWnd, &BmpRec, TRUE);
        FillBoxWithBitmap(hdc, 170+gOptions.MainVerifyOffset, 50, 0, 0, &bmp);
        ReleaseDC(hdc);
}
//显示信息
static void ShowOptInfo(HWND hWnd, char* str, int row)
{
	int tmpvalue = 0;
	RECT InfoRec={10,80,120,100};
        HDC hdc=GetClientDC(hWnd);
        tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc,PIXEL_lightwhite);
	if(!row) InvalidateRect(hWnd, &InfoRec, TRUE);
        TextOut(hdc, 10, 80+(row*18), str);
        ReleaseDC(hdc);
}
//显示操作提示
static void ShowOptHint(HWND hWnd, char* str, int bicon)
{
	int tmpvalue = 0;
	RECT HintRec={0,170,180,230};
        HDC hdc=GetClientDC(hWnd);
        tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc,PIXEL_lightwhite);
	InvalidateRect(hWnd, &HintRec, TRUE);
        if(bicon) FillBoxWithBitmap(hdc,0,180,0,0,&mfbmphint);
        TextOut(hdc, 35, 185, str);
        ReleaseDC(hdc);
}

static int RegistCardInfo(BYTE* card)
{
        TUser cuser;
        memset(&cuser, 0, sizeof(TUser));
        if (FDB_GetUserByCard(card, &cuser)!=NULL)
        {
                memset(cuser.Card, 0, 4);
                FDB_ChgUser(&cuser);
        }

        memset(tmpUser.Card, 0, 4);
        memcpy(tmpUser.Card, card, 4);
        FDB_ChgUser(&tmpUser);
	return 0;
}


//创建临时号码卡
static void CreatePINCard(char* pin2)
{
	tmpUser.PIN=1;
	memcpy(tmpUser.PIN2, pin2, gOptions.PIN2Width);

	memset(TMPBUF,0,sizeof(TMPBUF));
	tmpFPCard.PIN=atoi(tmpUser.PIN2);
	memset(tmpFPCard.Finger,0xFF,4);
	tmpFPCard.TempSize=0;
	tmpFPCard.Templates=TMPBUF;
	tmpFPCard.OP=OP_WRITE;
}

//创建临时指纹卡
static void CreateFPCard(HWND hWnd)
{
	U8 *(t[4]);
	//char buf[20];
	//char fmt[20];

	t[0]=(U8*)tmps[0]; t[1]=(U8*)tmps[1]; t[2]=(U8*)tmps[2]; t[3]=(U8*)tmps[3];
	//tmpFPCard.Templates=(BYTE *)tmps;
	tmpFPCard.Templates=TMPBUF;
	tmpFPCard.TempSize = PackTemplate((U8*)tmpFPCard.Templates, t, fpindex, MFGetResSize()-8);
	tmpFPCard.OP=OP_WRITE;

//	printf("tmpFPCard.PIN:%d,fpcount:%d, tmpFPCard.TempSize:%d\n",tmpFPCard.PIN,fpindex,tmpFPCard.TempSize);
/*
	sprintf(fmt, "%%0%dd:%%d", gOptions.PIN2Width);
	sprintf(buf, fmt, tmpUser.PIN, fpindex);
	ShowOptInfo(hWnd, buf,0);
*/
}

static void ShowFpImage(HWND handle)
{
	HDC dc1;
    BITMAP mybmp;
    BLOCKHEAP my_cliprc_heap;
    BOOL ch_inited = FALSE;

    CLIPRGN my_cliprgn1;
    CLIPRGN my_cliprgn2;
    CLIPRGN my_cliprgn3;

    write_bitmap(TMPBMPFILE, FpSensorInfo->DewarpedImgPtr, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
	 LoadBitmap(HDC_SCREEN,&mybmp, TMPBMPFILE);
    dc1=GetClientDC(handle);
	if(!ch_inited)
	{
        	InitFreeClipRectList (&my_cliprc_heap, 100);
		ch_inited = TRUE;
	}

	InitClipRgn (&my_cliprgn1, &my_cliprc_heap);
	InitClipRgn (&my_cliprgn2, &my_cliprc_heap);
	InitClipRgn (&my_cliprgn3, &my_cliprc_heap);

	InitEllipseRegion (&my_cliprgn2, 85, 110, 68, 90);
	SubtractRegion (&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion (dc1, &my_cliprgn1);
	OffsetRegion (&my_cliprgn2, 150+gOptions.MainVerifyOffset, 0);
	XorRegion (&my_cliprgn1, &my_cliprgn1, &my_cliprgn2);
	SelectClipRegion (dc1, &my_cliprgn1);

#ifdef URU
	FillBoxWithBitmap(dc1, 150+gOptions.MainVerifyOffset, 0, 180, 200,&mybmp);
#else
        if(gOptions.FpSelect==0 || gOptions.FpSelect==2)
	{
	//	RotateScaledBitmapVFlip(dc1, &mybmp, 90, -80, 1, 180, 200);
	    	FillBoxWithBitmap(dc1, 150+gOptions.MainVerifyOffset, 20, 180, 200,&mybmp);
	}
        else
        {
                BITMAP finger;
                LoadBitmapFromFile(HDC_SCREEN,&finger, GetBmpPath("pic3.gif"));
                FillBoxWithBitmap(HDC_SCREEN, 160+gOptions.MainVerifyOffset, 35, 0, 0, &finger);
                UnloadBitmap(&finger);
        }
//    	FillBoxWithBitmap(dc1, 150, 20, 180, 200,&mybmp);
#endif
        UnloadBitmap(&mybmp);
        ReleaseDC(dc1);

}

static void ProcessFpEnroll(HWND hWnd)
{
        int qlt=0;
        PEnrollFpSession es1 = &fpes;

        if(BIOKEY_EXTRACT(fhdl, FpSensorInfo->DewarpedImgPtr, es1->Tmps[es1->Index], EXTRACT_FOR_IDENTIFICATION))
	{
		ShowFpImage(hWnd);		//Display FpImage
		if(++(es1->Index)>=gOptions.EnrollCount)
		{
			es1->len=BIOKEY_GENTEMPLATE(fhdl, es1->Tmps, gOptions.EnrollCount, es1->GTmp);
			if (es1->len > 0)
			{
				EnableMsgType(MSG_TYPE_FINGER, 0);
				tmplens[fpindex]=fpes.len;
				isfpdbload=1;
	                        tmpFPCard.Finger[fpindex]=fpindex;
				fpindex++;
				if(fpindex>=gOptions.RFCardFPC)
				{
					UpdateWindow(hWnd, TRUE);
					EnableMsgType(MSG_TYPE_FINGER,0);
					CreateFPCard(hWnd);
					BeginWriteCard(hWnd);
				}
				else
				{
					isenrolling=1;
					ShowOptHint(hWnd, LoadStrByID(MID_MF_OK),1);
				}
			}
			else
			{
				es1->len = 0;
				es1->Index = 0;
				ShowOptHint(hWnd, LoadStrByID(HID_VFFAIL),1);
				DelayMS(1000);

				InvalidateRect(hWnd, &BmpRec, TRUE);
				ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER),1);
			}
		}
		else if(es1->Index==1)
		{
			InvalidateRect(hWnd, &BmpRec, TRUE);
			ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER2), 1);
		}
		else if(es1->Index==2)
		{
			InvalidateRect(hWnd, &BmpRec, TRUE);
			ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER3), 1);
		}
		else
		{
			InvalidateRect(hWnd, &BmpRec, TRUE);
                	ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER),1);
		}

		qlt=100;
	}
	else
	{
		ShowOptHint(hWnd, LoadStrByID(HID_VFFAIL),1);
		ShowOptBmp(hWnd,mfbmpfail);
	}
        CheckSessionSend(EF_FPFTR, (void*)&qlt, 1);

}

//=================================================================================
static void InitEnrollParam(HWND hWnd)
{
	char mycharchar[30];
	EnrollNext=0;
	isenrolling=0;
	if (OptStyle!=CopyOnLine)  //changed by cxp at 2010-04-22
	{
		memset(TMPBUF, 0, sizeof(TMPBUF));
		memset(&tmpFPCard, 0, sizeof(TFPCardOP));
	}
	memset(&tmpUser, 0, sizeof(TUser));
	memset(tmps,0,sizeof(tmps));
	memset(tmplens,0,sizeof(tmplens));
	memset(tmpfp,0,sizeof(tmpfp));

	if(OptStyle==EnrollPIN || OptStyle==EnrollFP)
	{
		ShowWindow(EdPIN2, SW_SHOW);
		ShowWindow(LbPIN2, SW_HIDE);
		SetFocusChild(EdPIN2);
	}
	UpdateWindow(hWnd,TRUE);

	switch(OptStyle)
	{
		case EnrollPIN:
		case EnrollFP:
			memset(mycharchar,0,30);
			//strcpy(mycharchar,CheckFreePIN2());

			GetFreePIN2FromRam(mycharchar);
			//SetWindowText(EdPIN2, CheckFreePIN2());
			SetWindowText(EdPIN2, mycharchar);

			ShowOptHint(hWnd, LoadStrByID(MID_INPUT_CODE),1);
			SetFocusChild(EdPIN2);
			isnewUser=0;
			isnewTemplate=0;
			EnableMsgType(MSG_TYPE_MF,0);
			EnableMsgType(MSG_TYPE_ICLASS,0);
			break;
		case ClearInfo:
			ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);
			tmpFPCard.Templates=TMPBUF;
			tmpFPCard.OP=OP_READ;
			EnableMsgType(MSG_TYPE_MF,1);
			EnableMsgType(MSG_TYPE_ICLASS,1);
			break;

		case CopyInfo:
			if(copyoperate)
			{
				ShowWindow(btnSave1, SW_HIDE);
				ShowWindow(btnSave2, SW_HIDE);
				ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);
				tmpFPCard.Templates=TMPBUF;
				tmpFPCard.OP=OP_READ;
				EnableMsgType(MSG_TYPE_MF,1);
				EnableMsgType(MSG_TYPE_ICLASS,1);
			}
			else
			{
				ShowWindow(btnSave1, SW_SHOW);
				ShowWindow(btnSave2, SW_SHOW);
				SetFocusChild(btnSave1);
				EnableMsgType(MSG_TYPE_MF,0);
				EnableMsgType(MSG_TYPE_ICLASS,0);
			}
			break;
		case CopyOnLine:
		{
			ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);
			BeginWriteCard(hWnd);
		}
		break;
	}
	EnableMsgType(MSG_TYPE_FINGER,0);
}

static void InitMFWindow(HWND hWnd)
{
	int posX1,posX2,posX3;

	posX1=2;
	posX2=10;
	posX3=5;
	
	if(OptStyle==EnrollPIN || OptStyle==EnrollFP)
	{
	        CreateWindow(CTRL_STATIC, LoadStrByID(HID_PIN2), WS_VISIBLE | SS_LEFT, LB_STATIC, posX1, 8, 90, 23, hWnd, 0);
	        EdPIN2 = CreateWindow(CTRL_SLEDIT,"", WS_VISIBLE | WS_TABSTOP | ES_AUTOSELECT | ES_BASELINE | WS_BORDER,
              			ID_PIN2, posX2, 30, 90, 26, hWnd, 0);
		LbPIN2 = CreateWindow(CTRL_STATIC, "", SS_LEFT, LB_PIN2, posX2, 34, 90, 26, hWnd, 0);

	        SendMessage(EdPIN2, EM_LIMITTEXT, gOptions.PIN2Width, 0L);

	}
	else if(OptStyle==CopyInfo)
	{
		btnSave1 = CreateWindow(CTRL_BUTTON,LoadStrByID(MID_MFCP_OP1), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
                                ID_PIN2+1, posX3, 140, 150, 30, hWnd, 0);
                btnSave2 = CreateWindow(CTRL_BUTTON,LoadStrByID(MID_MFCP_OP2), WS_VISIBLE | BS_DEFPUSHBUTTON | WS_BORDER,
                                ID_PIN2+2, posX3, 180, 150, 30, hWnd, 0);
		copyoperate=0;			//开始复制操作标志
	}

	InitEnrollParam(hWnd);
}

static int CheckCardInfo(HWND hWnd, BYTE* cvalue)
{
	char tmppin2[24];
	int ckrs=0;

        memset(&tmpUser, 0, sizeof(TUser));
//	printf("tmpFPCard.PIN:%d\n",tmpFPCard.PIN);
        sprintf(tmppin2, "%d", tmpFPCard.PIN);
        if(!FDB_GetUserByCharPIN2(tmppin2, &tmpUser))
        {
            int userCurCount = FDB_CntUserByMap();
            if(userCurCount >= gOptions.MaxUserCount*100)
            {
                return 0;
            }
            
            FDB_CreateUser(&tmpUser, GetNextPIN(1, TRUE), NULL, NULL, 0);
            memcpy(tmpUser.PIN2, tmppin2, gOptions.PIN2Width);
            if(FDB_AddUser(&tmpUser)==FDB_OK)
    		{
    			RegistCardInfo(cvalue);
    			ckrs=1;				//创建了新用户
    		}
        }
        else
        {
		if(ShowQuestionBox(hWnd, LoadStrByID(MID_MFCP_QUEST2),0))
		{
			if(FDB_ChgUser(&tmpUser)==FDB_OK)
			{
				RegistCardInfo(cvalue);
				ckrs=2;				//用户存在,覆盖用户指纹
			}
		}
        }

	sync();
	return ckrs;
}

//判断指纹是否存在
//0:指纹不存在
//1:与同一用户的指纹重复
//2:与不同用户的指纹重复
static int isExistTemplate(int index)
{
	int ret=0;
	PUser pu;
        int result,score=55;
        PEnrollFpSession esck = &fpes;

        fpes.GTmp=(BYTE*)tmps[index];
	
	if(BIOKEY_IDENTIFYTEMP(fhdl, (BYTE*)esck->GTmp, &result, &score))
	{
		pu=FDB_GetUser(result,NULL);

		//zsliu change 防止为空的时候，固件about
		//if(strcmp(pu->PIN2, tmpUser.PIN2)==0)
		if((pu != NULL) && (strcmp(pu->PIN2, tmpUser.PIN2)==0))
			ret=1;
		else
			ret=2;
	}	return ret;
}

//获取用户可用指纹ID
static int GetValidFingerIndex(U16 UID)
{
	TZKFPTemplate ttp;
	int i,id=-1;
	for(i=0; i<gOptions.MaxUserFingerCount; i++)
	{
		memset(&ttp,0,sizeof(ttp));
		if(!FDB_GetTmp(UID, (char) i, (char*)&ttp))
		{
			id=i;
			break;
		}
	}
	return id;
}

static int CopyMFCardInfo(HWND hWnd, BYTE* cvalue)
{
	TZKFPTemplate t;
	int i,offset=0,len,fc=0;
	int rc,sc;
	char cpinfo[50];
	char fmt[20];
	int fpidx=0;

	rc=CheckCardInfo(hWnd, cvalue);		//用户信息
//	printf("rc:%d,tmpUser.PIN:%d, tmpUser.PIN2:%s\n",rc,tmpUser.PIN, tmpUser.PIN2);

	if(rc)
	{
		if(copycontent==0)
		{
			sprintf(fmt, "%%s:%%0%dd", gOptions.PIN2Width);
			sprintf(cpinfo, fmt, LoadStrByID(HID_PIN2), tmpFPCard.PIN);
			ShowOptInfo(hWnd, cpinfo,0);
			return 0;		//只复制用户信息
		}

		for(i=0;i<=gOptions.RFCardFPC;i++)
		{
			if(tmpFPCard.TempSize>0)
			{
				len=BIOKEY_TEMPLATELEN(tmpFPCard.Templates+offset);
				FDB_CreateTemplate((char*)&t, tmpUser.PIN, (char)tmpFPCard.Finger[i], (char*)tmpFPCard.Templates+offset, len, 1);
				if(len)
				{
					memset(tmps[tmpFPCard.Finger[i]],0,sizeof(tmps[tmpFPCard.Finger[i]]));
					memcpy(tmps[tmpFPCard.Finger[i]],t.Template,len);
					sc=isExistTemplate(tmpFPCard.Finger[i]);
//					printf("sc:%d\n",sc);

					if(sc!=2)
					{
						if(rc==2)
						{
							if(sc==1)	//重复指纹
								 FDB_DelTmp(tmpUser.PIN, tmpFPCard.Finger[i]);
							else		//新指纹
							{
                                int fPCurCount = FDB_CntTmp();
                                if(fPCurCount >= gOptions.MaxFingerCount*100)
                                {
                                    break;
                                }
							
								fpidx = GetValidFingerIndex(tmpUser.PIN);
//								printf("fpidx:%d\n",fpidx);
								if(fpidx<0)	//指纹满，覆盖旧指纹
									FDB_DelTmp(tmpUser.PIN, t.FingerID);
								else
									t.FingerID=fpidx;
							}
						}
						if(FDB_OK==FDB_AddTmp((char*)&t)) fc++;
					}
					offset+=len;
					tmpFPCard.TempSize-=len;
				}
			}
			else
				break;
		}

//		printf("fc:%d\n",fc);

		FDB_AddOPLog(ADMINPIN, OP_MF_DUMP, tmpFPCard.PIN, 0, fc, 0);
        FPDBInit();
		sync();
		if(fc)
		{
			if(gOptions.PIN2Width==PIN_WIDTH2)
				sprintf(cpinfo,"%s:%05d", LoadStrByID(HID_ENROLLNUM), tmpFPCard.PIN);
			else
			{
				sprintf(fmt, "%%s:%%0%dd", gOptions.PIN2Width);
				sprintf(cpinfo, fmt, LoadStrByID(HID_PIN2), tmpFPCard.PIN);
			}
			ShowOptInfo(hWnd, cpinfo,0);
			sprintf(cpinfo,"%s: %d",LoadStrByID(MID_IR_FINGER), fc);
			ShowOptInfo(hWnd, cpinfo,1);
			return 0;
		}
		else
			return 2;	//指纹数为0
	}
	else
	{
		FDB_AddOPLog(ADMINPIN, OP_MF_DUMP, tmpFPCard.PIN, 1, 0, 0);
		sync();
		return 1;		//创建用户信息失败
	}

}

static int SaveTemplates(HWND hWnd, int index)
{
        TZKFPTemplate t;
        int j;
        PEnrollFpSession es3 = &fpes;
        //int rc=0;

	fpes.GTmp=(BYTE*)tmps[index];
	BIOKEY_DB_ADD(fhdl, tmpUser.PIN|(index<<16), fpes.len-6, (BYTE*)es3->GTmp);

	j = FDB_AddTmp((char*)FDB_CreateTemplate((char*)&t,(U16)tmpUser.PIN, (char)index, tmps[index], tmplens[index], 1));
	if(j==FDB_OK)
	{
		char Buffer[sizeof(TZKFPTemplate)+4];
		((U16*)Buffer)[2]=0;
		memcpy(Buffer+4, &t, sizeof(t));
		CheckSessionSend(EF_ENROLLFINGER,Buffer+2, sizeof(t)+2);
	}
	FDB_AddOPLog(ADMINPIN, OP_ENROLL_FP, tmpUser.PIN, j, index, tmplens[index]);
	sync();
	return j;
}

static int SaveUserInfo(HWND hWnd, BYTE* cvalue)
{
	int i,j=FDB_OK;
	int scnt=0;
	char sfmt[50];
	int ts;

	if(!FDB_GetUserByCharPIN2(tmpUser.PIN2, NULL))
	{
        int userCurCount = FDB_CntUserByMap();
        if(userCurCount >= gOptions.MaxUserCount*100)
        {
            return 0;
        }
	
		tmpUser.PIN=GetNextPIN(tmpUser.PIN, TRUE);

	        //默认分配到存在的第一个组
	        if(gOptions.LockFunOn)
	        {
			TGroup tgp;
	                for(i=0;i<GP_MAX;i++)
	                {
	                        memset(&tgp,0,sizeof(TGroup));
	                        if(FDB_GetGroup(i+1,&tgp)!=NULL)
	                        {
	                                tmpUser.Group=tgp.ID;
	                                break;
	                        }
	                }
	        }

		j = FDB_AddUser(&tmpUser);
		FDB_AddOPLog(ADMINPIN, OP_ENROLL_USER, tmpUser.PIN, j, 0, 0);
		sync();
	}

	if(j==FDB_OK)
	{
                //将卡ID值写入用户Card字段
		RegistCardInfo(cvalue);

		if(OptStyle==EnrollFP)
		{
			for(i=0;i<fpindex;i++)
			{
				if(tmplens[i])
				{
					ts = isExistTemplate(i);
					if(ts)
					{
						if(ts==1)
						{
							if(ShowQuestionBox(hWnd, LoadStrByID(MID_MFCP_QUEST),0))
							{
								FDB_DelTmp(tmpUser.PIN,i);
								if(SaveTemplates(hWnd, i)==FDB_OK)
									scnt++;
							}
						}
						else
						{
							ShowOptHint(hWnd,LoadStrByID(HID_REFINGER),1);
						}
					}
					else
					{
//						printf("Template index:%d\n", i);

                        int fPCurCount = FDB_CntTmp();
                        if(fPCurCount >= gOptions.MaxFingerCount*100)
                        {
                            return 0;
                        }
                                        
						if(SaveTemplates(hWnd, i)==FDB_OK)
							scnt++;
					}
				}
			}
			sprintf(sfmt,LoadStrByID(MID_MFCP_HINT),scnt);
			ShowOptInfo(hWnd,sfmt,0);
			FPDBInit();
			sync();
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

static void BeginCreateTemplate(HWND hWnd)
{
	//char tmpstr[20];
	char tmpstr[50];

	fpindex=0;		//指纹编号
	fpes.Index=0;
	fpes.len=0;
	fpes.Tmps[0]=tmpfp[0];fpes.Tmps[1]=tmpfp[1];fpes.Tmps[2]=tmpfp[2];
	fpes.GTmp=(BYTE*)tmps[0];

	sprintf(tmpstr,"%s(%s-%d)", LoadStrByID(MID_DATA_EU_FINGER),tmpUser.PIN2, fpindex);
	ShowOptInfo(hWnd, tmpstr,0);
	ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER),1);
	EnableMsgType(MSG_TYPE_FINGER,1);
}

static void BeginWriteCard(HWND hWnd)
{
	ShowOptBmp(hWnd, mfbmpcard);
	ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);       //提示出示指纹卡
	EnableMsgType(MSG_TYPE_MF, 1);
	EnableMsgType(MSG_TYPE_ICLASS,1);
}

static int GetUserTemplate(PUser u)
{
	int fid, fc=0;
	TZKFPTemplate Temp;
	tmpFPCard.Templates=TMPBUF;
	tmpFPCard.OP=OP_WRITE;

	tmpFPCard.PIN=atoi(u->PIN2);
	memset(tmpFPCard.Finger, 0xFF, 4);
	for(fid=0; fid<gOptions.MaxUserFingerCount; fid++)
	{
		printf("u->PIN:%d\n",u->PIN);
		printf("user FP count:%d\n",FDB_GetTmpCnt(u->PIN));
		if(FDB_GetTmp(u->PIN, (char)fid, (char*)&Temp))
		{
			memcpy(tmps[fc], Temp.Template, Temp.Size);
			tmpFPCard.Finger[fc]=fid;
			fc++;
			if(fc>=gOptions.RFCardFPC) break;
		}
	}
	printf("fc:%d\n",fc);
	if(fc>0)
	{
		U8 *(t[4]);
		t[0]=(U8*)tmps[0];t[1]=(U8*)tmps[1];t[2]=(U8*)tmps[2];t[3]=(U8*)tmps[3];
		tmpFPCard.TempSize=PackTemplate(tmpFPCard.Templates, t, fc, MFGetResSize()-8);
		return 1;
	}
	else
		return 0;

}

static int mfcardwinproc(HWND  hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int i=0;
	char tmpstr[50];
	static char keyupFlag=0;
	switch (message)
	{
		case MSG_CREATE:
	                LoadBitmap (HDC_SCREEN, &mfbmpbkg, GetBmpPath("fp.jpg"));
	                LoadBitmap (HDC_SCREEN, &mfbmpok, GetBmpPath("ok.gif"));
	                LoadBitmap (HDC_SCREEN, &mfbmpfail, GetBmpPath("fail.gif"));
	                LoadBitmap (HDC_SCREEN, &mfbmpcard, GetBmpPath("card.gif"));
	                LoadBitmap (HDC_SCREEN, &mfbmphint, GetBmpPath("warning.gif"));

			InitMFWindow(hWnd);
			SetTimer(hWnd,IDC_TIMER_MF,1);
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

			FillBoxWithBitmap(hdc,0,0,gOptions.LCDWidth, gOptions.LCDHeight,&mfbmpbkg);
                        if(fGetDC) ReleaseDC (hdc);
                        break;
                }

		case MSG_PAINT:
			hdc=BeginPaint(hWnd);
			EndPaint(hWnd,hdc);
			break;
		case MSG_KEYUP:
			if(3==gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));
			if(3==gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
			if(wParam==SCANCODE_ESCAPE)
			{
				if(isenrolling)
				{
					isenrolling=0;
					CreateFPCard(hWnd);
					EnableMsgType(MSG_TYPE_MF, 1);
					EnableMsgType(MSG_TYPE_ICLASS, 1);
					EnableMsgType(MSG_TYPE_FINGER,0);
					UpdateWindow(hWnd, TRUE);
					ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);	//提示出示指纹卡
				}
				else
					SendMessage(hWnd, MSG_CLOSE, 0, 0);
				break;
			}

			if(wParam==SCANCODE_CURSORBLOCKUP || wParam==SCANCODE_CURSORBLOCKDOWN)
			{
				if(OptStyle==CopyInfo && !copyoperate)
				{
					if(GetFocusChild(hWnd)==btnSave1)
						SetFocusChild(btnSave2);
					else
						SetFocusChild(btnSave1);
				}
			}

			if(wParam==SCANCODE_ENTER || (LOWORD(wParam)==SCANCODE_F10))
			{
				if(EnrollNext)				//继续进行卡操作
					InitEnrollParam(hWnd);
				else if (isenrolling)			//登记下一枚指纹
				{
					fpes.Index=0;
			                fpes.len=0;
			                fpes.GTmp=(BYTE*)tmps[fpindex];
			                isenrolling=0;
			                FlushSensorBuffer(); //Clear sensor buffer

			                EnableMsgType(MSG_TYPE_FINGER, 1);
					UpdateWindow(hWnd, TRUE);
					ShowOptHint(hWnd, LoadStrByID(HID_PLACEFINGER),1);
		                        sprintf(tmpstr,"%s(%s-%d)", LoadStrByID(MID_DATA_EU_FINGER),tmpUser.PIN2, fpindex);
					ShowOptInfo(hWnd, tmpstr, 0);
			        }
				else if(OptStyle==CopyInfo && !copyoperate)		//选择复制内容
				{
					if(GetFocusChild(hWnd)==btnSave1)
						copycontent=0;
					else if(GetFocusChild(hWnd)==btnSave2)
						copycontent=1;

					copyoperate=1;
					InitEnrollParam(hWnd);
				}
				else if((OptStyle==EnrollPIN || OptStyle==EnrollFP) && GetFocusChild(hWnd)==EdPIN2)
				{
					memset(tmpstr, 0, sizeof(tmpstr));
					GetWindowText(EdPIN2, tmpstr, gOptions.PIN2Width);
					if(!tmpstr || strlen(tmpstr)<1 || atoi(tmpstr)<=0)				//PIN2输入为空
					{
						ShowInfoBox(hWnd,LoadStrByID(HID_OS_MUST1TO1));
						SetFocusChild(EdPIN2);
					}
					else
					{
						SetWindowText(LbPIN2, tmpstr);
						ShowWindow(EdPIN2, SW_HIDE);
						ShowWindow(LbPIN2, SW_SHOW);
						SetFocusChild(LbPIN2);

						memset(&tmpUser,0,sizeof(TUser));
						if(FDB_GetUserByCharPIN2(tmpstr, &tmpUser))			//用户信息存在
						{
							if(ShowQuestionBox(hWnd,LoadStrByID(MID_CRTMF_HINT1),0))
							{
								isnewUser=0;
								if(OptStyle==EnrollPIN)
								{
        								memset(TMPBUF,0,sizeof(TMPBUF));
								        tmpFPCard.PIN=atoi(tmpUser.PIN2);
								        memset(tmpFPCard.Finger,0xFF,4);
								        tmpFPCard.TempSize=0;
								        tmpFPCard.Templates=TMPBUF;
								        tmpFPCard.OP=OP_WRITE;
									BeginWriteCard(hWnd);
								}
								else
								{
									if(GetUserTemplate(&tmpUser))
									{
										isnewTemplate=0;
										BeginWriteCard(hWnd);
									}
									else
									{
										if(ShowQuestionBox(hWnd,LoadStrByID(MID_CRTMF_HINT2),0))
										{
											isnewTemplate=1;
											BeginCreateTemplate(hWnd);
										}
										else
										{
											ExBeep(1);
											ShowOptBmp(hWnd, mfbmpfail);
											ShowOptHint(hWnd, LoadStrByID(MID_CRTMF_HINT3),1);
											DelayMS(1000);
											InitEnrollParam(hWnd);		//重新开始
										}
									}
								}
							}
							else
							{
								CreatePINCard(tmpstr);
	                                                        if(OptStyle==EnrollPIN)
        	                                                        BeginWriteCard(hWnd);
                	                                        else
								{
									isnewTemplate=1;
                        	                                        BeginCreateTemplate(hWnd);
								}
							}

						}
						else								//用户信息不存在
						{
							isnewUser=1;
							CreatePINCard(tmpstr);
							if(OptStyle==EnrollPIN)
								BeginWriteCard(hWnd);
							else
								BeginCreateTemplate(hWnd);
						}
					}
				}
			}
			break;

		case MSG_TYPE_ICLASS:
		case MSG_TYPE_MF:
		{
			int OldState=gMachineState;
			char ClearPIN2[24];		//被清空卡的PIN2
			int opst=0;

			BYTE card[4];
			//memcpy(card, (void*)&lParam, 4);
			memcpy(card, (BYTE *)&lParam, 4);

			SetMenuTimeOut(time(NULL));
			ShowOptBmp(hWnd, mfbmpcard);
			ShowOptInfo(hWnd, LoadStrByID(HID_WORKING),0);

			switch(OptStyle)
			{
				case EnrollPIN:
				case EnrollFP:
//					printf("tmpFPCard.PIN:%d,TempSize:%d,OP:%d\n",tmpFPCard.PIN,tmpFPCard.TempSize,tmpFPCard.OP);
					if(tmpFPCard.PIN)
					{
						gMachineState=STA_WRITEMIFARE;
						ShowOptHint(hWnd, LoadStrByID(HID_DOWNLOADING_DATA),1);

						ExLightLED(LED_RED, FALSE);
						ExLightLED(LED_GREEN, FALSE);
						if(tmpFPCard.OP==OP_WRITE)
						{
//							tmpFPCard.Templates=tmps;
							if(gMFOpened)
								i=MFWrite(&tmpFPCard);
							else if(giCLSRWOpened)
								i=iCLSWrite(&tmpFPCard);
						}
						#ifdef ZEM500
						if(gMachineState==STA_WRITEMIFARE)
						#endif
						        gMachineState=OldState;
		                                FDB_AddOPLog(ADMINPIN, OP_MF_CREATE, tmpFPCard.PIN, i, 0, 0);
					}
					break;

				case ClearInfo:
				{
					char fms[20], ifs[50];
					result=-1;
					#if 1
					if((gMFOpened && MFRead(&tmpFPCard, FALSE)>=0) || (giCLSRWOpened && iCLSRead(&tmpFPCard, FALSE)>=0))
					{
						memset(ClearPIN2, 0, 24);
						sprintf(ClearPIN2, "%d", tmpFPCard.PIN);
		                                sprintf(fms, "%%s:%%0%dd", gOptions.PIN2Width);
		                                sprintf(ifs, fms, LoadStrByID(HID_PIN2), tmpFPCard.PIN);
						ShowOptInfo(hWnd, ifs,0);
					}
					if(giCLSRWOpened) DelayMS(1000);
					#endif

					ShowOptHint(hWnd, LoadStrByID(MID_CLNMF_HINT4),1);
					if(gMFOpened)
					{
						i=MFEmpty();
						result=i;
					}
					else if(giCLSRWOpened)
					{
						i=iCLSEmpty();
						result=i;
					}

					tmpFPCard.OP=OP_EMPTY;
					break;
				}
				case CopyOnLine:
				{
					result=-1;
					gMachineState=STA_WRITEMIFARE;
					ShowOptHint(hWnd, LoadStrByID(HID_DOWNLOADING_DATA),1);
					ExLightLED(LED_RED, FALSE);
					ExLightLED(LED_GREEN, FALSE);
					if(tmpFPCard.OP==OP_WRITE)
					{
						if(gMFOpened)
						{
							i=MFWrite(&tmpFPCard);
							result=i;
						}
						else if(giCLSRWOpened)
						{
							i=iCLSWrite(&tmpFPCard);
							result=i;
						}
					}
#ifdef ZEM500
						if(gMachineState==STA_WRITEMIFARE)
#endif
						gMachineState=OldState;
						FDB_AddOPLog(ADMINPIN, OP_MF_CREATE, tmpFPCard.PIN, i, 0, 0);
				}
				break;
				case CopyInfo:
					if(tmpFPCard.OP==OP_READ)
					{
						if(gMFOpened)
							i=MFRead(&tmpFPCard, FALSE);
						else if(giCLSRWOpened)
							i=iCLSRead(&tmpFPCard, FALSE);
					}
					tmpFPCard.OP=OP_READ;
					break;

			}

			DelayMS(1000);
			EnableMsgType(MSG_TYPE_MF, 0);		//Close Mifare Card Function.
			EnableMsgType(MSG_TYPE_ICLASS,0);
			if(i>=0)
			{
				UpdateWindow(hWnd, TRUE);
				ExLightLED(LED_GREEN, TRUE);
				if(gMFOpened) MFFinishCard();
				ExBeep(1);

				if(tmpFPCard.OP!=OP_READ)
				{
					ShowOptBmp(hWnd, mfbmpok);
					ShowOptInfo(hWnd,LoadStrByID(HID_WRITE_OK),0);	//Enroll PIN Card Ok!
					if((OptStyle==EnrollPIN || OptStyle==EnrollFP) && tmpUser.PIN)
					{
						if(isnewUser || isnewTemplate)
						{
							int mbfg=(gOptions.MustEnroll) ? 0:1;
							if(ShowQuestionBox(hWnd, LoadStrByID(MID_CARD_SUC),mbfg))
							{
								if(SaveUserInfo(hWnd, card))
									ShowOptHint(hWnd, LoadStrByID(MID_MF_SAVEOK),1);
								else
									ShowOptHint(hWnd, LoadStrByID(MID_MF_SAVEFL),1);
							}
						}
						else
						{
							RegistCardInfo(card);
						}
					}
					else if(OptStyle==ClearInfo)
					{
						memset(&tmpUser,0,sizeof(TUser));
						if(ClearPIN2 && FDB_GetUserByCharPIN2(ClearPIN2,&tmpUser))
						{
							if(ShowQuestionBox(hWnd, LoadStrByID(MID_CLNMF_HINT3),0))
							{
//								printf("tmpUser.PIN:%d\n",tmpUser.PIN);
								if(FDB_DelUser(tmpUser.PIN)==FDB_OK)
								{
									ShowOptHint(hWnd, LoadStrByID(MID_CLNMF_HINT1), 1);
									FPDBInit();
									sync();
								}
								else
									ShowOptHint(hWnd, LoadStrByID(MID_CLNMF_HINT2), 1);
							}
							else
							{
								memset(tmpUser.Card, 0, 4);
								FDB_ChgUser(&tmpUser);
							}
						}
					}
        	                }
				else	//复制卡内信息
				{
					opst = CopyMFCardInfo(hWnd, card);
					if(opst==0)	//复制成功
					{
						ShowOptBmp(hWnd,mfbmpok);
						ShowOptHint(hWnd, LoadStrByID(MID_MFCP_OK),1);
					}
					else
					{
						ShowOptBmp(hWnd,mfbmpfail);
						if(opst==1)
							ShowOptHint(hWnd, LoadStrByID(MID_MFCP_HINT1),1);
						else
							ShowOptHint(hWnd, LoadStrByID(MID_MFCP_HINT2),1);
					}

					copyoperate=0;
					DelayMS(1000);
					if(opst>0)
					{
						InitEnrollParam(hWnd);          //重新开始
						break;
					}
				}

				EnrollNext=1;
				ShowOptHint(hWnd, LoadStrByID(MID_MF_OK),1);
			}
			else
			{
				ExLightLED(LED_RED, TRUE);
				UpdateWindow(hWnd, TRUE);
				ShowOptBmp(hWnd, mfbmpfail);
				if(tmpFPCard.OP!=OP_READ)
					sprintf(tmpstr, "%s", LoadStrByID(HID_WRITE_ERROR));
				else
					sprintf(tmpstr, "%s", LoadStrByID(HID_INVALIDCARD));
				ShowOptHint(hWnd, tmpstr,1);
				ExBeep(1);
                               	DelayMS(1000);
				UpdateWindow(hWnd, TRUE);
				ShowOptHint(hWnd, LoadStrByID(MID_PLACE_CARD),1);
			        ExLightLED(LED_RED, FALSE);
				EnableMsgType(MSG_TYPE_MF,1);
				EnableMsgType(MSG_TYPE_ICLASS,1);
                	}

			break ;
		}

		case MSG_TYPE_FINGER:
                        FpSensorInfo=(PSensorBufInfo)lParam;
			ProcessFpEnroll(hWnd);
			break;

                case MSG_TIMER:
                        //scanfp 10ms unit loop
                        if(wParam==IDC_TIMER_MF)
                        {
				if(m && (Fwmsg.Message==MSG_TYPE_MF || Fwmsg.Message==MSG_TYPE_FINGER || Fwmsg.Message==MSG_TYPE_ICLASS))
                                {
                                        SendMessage(hWnd,Fwmsg.Message,Fwmsg.Param1,Fwmsg.Param2);
                                        m=0;
                                }
                        }
			break;

		case MSG_CLOSE:
                        KillTimer(hWnd,IDC_TIMER_MF);
			UnloadBitmap(&mfbmpok);
			UnloadBitmap(&mfbmpfail);
			UnloadBitmap(&mfbmpbkg);
			UnloadBitmap(&mfbmpcard);
			UnloadBitmap(&mfbmphint);

			DestroyMainWindow(hWnd);
			break;

		default:
			return DefaultMainWinProc(hWnd,message,wParam,lParam);

	}
	return (0);
}


int CreateMFCardWindow(HWND hOwner, int opMode)
{
	MSG msg;
	MAINWINCREATE CreateInfo;

	EnableMsgType(MSG_TYPE_MF, 0);	//打开Mifare卡
	EnableMsgType(MSG_TYPE_ICLASS,0);
	OptStyle=opMode;

	hOwner = GetMainWindowHandle (hOwner);
        CreateInfo.dwStyle = WS_VISIBLE| WS_CAPTION|WS_BORDER;
        CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption=LoadStrByID(MID_CARD_OP1+OptStyle);
        CreateInfo.hMenu = 0;
        //CreateInfo.hCursor = GetSystemCursor(0);
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = mfcardwinproc;
        CreateInfo.lx = 0;
        CreateInfo.ty = 0;
        CreateInfo.rx = g_rcScr.right;
        CreateInfo.by = g_rcScr.bottom;
        CreateInfo.iBkColor = COLOR_lightwhite;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = hOwner;

	MFCardWnd = CreateMainWindow(&CreateInfo);   //changed by cxp
	if (MFCardWnd == HWND_INVALID)
			return -1;
	ShowWindow(MFCardWnd, SW_SHOWNORMAL);

	while (GetMessage(&msg,MFCardWnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    MainWindowThreadCleanup(MFCardWnd);
	EnableMsgType(MSG_TYPE_MF, 0);
	EnableMsgType(MSG_TYPE_ICLASS,0);
	MFCardWnd=HWND_INVALID;
	return result;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

