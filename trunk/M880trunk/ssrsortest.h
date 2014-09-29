/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录: 
* 编译环境:mipsel-gcc
*/


#ifndef __CWX_GUI_SORTEST
#define __CWX_GUI_SORTEST
#include "ssrcommon.h"
#include "sensor.h"
#include "options.h"
#include "ssrpub.h"
#include "main.h"
#ifdef UPEK
#include "upek/upek.h"
#endif

extern void write_bitmap(char *imgout, unsigned char *buffer, int nWidth, int nHeight);
static DLGTEMPLATE SorTestDlgBox =
{
    WS_VISIBLE | WS_CAPTION,
    WS_EX_NONE,
    1, 1, 319, 239, 
    "",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA SorTestCtrl [] =
{ 
    {
        CTRL_STATIC,
        WS_TABSTOP | WS_VISIBLE, 
        1, 1, 319, 239,
        IDC_TESTINFO, 
        "",
        0
    },
};

#ifdef UPEK
static void UpekInit(HWND hdlg)
{
        sint32 rett;
        HWND stahwnd;

        stahwnd = createStatusWin1(hdlg , 300 , 50 , LoadStrByID(MID_APPNAME) , LoadStrByID(HID_WAITING));
        rett= calibrate();
        destroyStatusWin1 (stahwnd);
        if(rett==0)
        {
                if(MessageBox1(hdlg,LoadStrByID(HIT_UPEKINITOK),LoadStrByID(HIT_RUN),MB_OKCANCEL|MB_ICONQUESTION|MB_BASEDONPARENT)==IDOK)
                {
                        systemEx("reboot");
                }
        }
        else
        {
                MessageBox1 (hdlg ,LoadStrByID(HIT_UPEKINITFAIL) ,LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
        }
}
#endif

void SorShowTestInfo(HDC hdc,int item)
{
	int tmpvalue = 0;
        SetTextColor(hdc,0x00990000);
        tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	TabbedTextOut(hdc,195,160,LoadStrByID(HIT_AUTOTESTINFOEX));
}

TSensorBufInfo SensorBufInfo;
char *myImageBuffer;
BITMAP mybmp,fpbg;
extern unsigned char *fingerbuf;
static int loop;

static int SorTestDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
	static char keyupFlag=0;
    switch (message) 
    {
    case MSG_INITDIALOG:

		if (LoadBitmap(HDC_SCREEN,&fpbg,GetBmpPath("fpbg.gif")))
                	return 0;
		SetTimer(hDlg,0x80080,10);
        myImageBuffer=(char *)MALLOC(5*gOptions.OImageWidth*gOptions.OImageHeight);
		TestItem=0;
		TestFlage=0;
		loop=0;
        	return 0;
     case MSG_TIMER:
        if(wParam==0x80080)
        {
            if(CaptureSensor(myImageBuffer, SENSOR_CAPTURE_MODE_STREAM, &SensorBufInfo))
            {
				char *tmpbuf;
				char* fingerbuf = gImageBuffer + gOptions.ZF_WIDTH*gOptions.ZF_HEIGHT;
				int i=0;
                memcpy(fingerbuf,SensorBufInfo.DewarpedImgPtr,SensorBufInfo.DewarpedImgLen);
				tmpbuf=MALLOC(gOptions.ZF_WIDTH);
				for(i=0;i<gOptions.ZF_HEIGHT/2;i++)
        		{
					memcpy(tmpbuf, fingerbuf+i*gOptions.ZF_WIDTH, gOptions.ZF_WIDTH);
					memcpy(fingerbuf+i*gOptions.ZF_WIDTH,fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH,gOptions.ZF_WIDTH);
					memcpy(fingerbuf+(gOptions.ZF_HEIGHT-i-1)*gOptions.ZF_WIDTH, tmpbuf, gOptions.ZF_WIDTH);
        		}
				FREE(tmpbuf);
				write_bitmap(TMPBMPFILE, (unsigned char *)fingerbuf, gOptions.ZF_WIDTH, gOptions.ZF_HEIGHT);
				PostMessage(hDlg,0x8898,0,0);
			}	
		}
		break;
     case 0x8898:
		 SetMenuTimeOut(time(NULL));
		LoadBitmap(HDC_SCREEN, &mybmp, TMPBMPFILE);
                 hdc=GetClientDC(hDlg);
		 //FillBoxWithBitmapPart(hdc, 15, 15, 160, 180,160,200,&mybmp,0,20);
		 FillBoxWithBitmapPart(hdc, 15, 15+gOptions.sensorOffsetY, 160, 180,160,200,&mybmp,0,20);
		 
                 UnloadBitmap(&mybmp);
                 ReleaseDC(hdc);
                 break;
    		case MSG_KEYUP:
			if(3 == gOptions.TFTKeyLayout)
			{
				keyupFlag=1;
			}
			break;
     case MSG_KEYDOWN:
			if(3 == gOptions.TFTKeyLayout)
			{
				if(1==keyupFlag)
					keyupFlag=0;
				else
					break;
			}
                        if ((LOWORD(wParam)==SCANCODE_ENTER) || (LOWORD(wParam)==SCANCODE_F10))
                        {
				PostMessage(hDlg,MSG_CLOSE,0,0);
				return 1;	
                        }
			else
			if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				TestFlage=-1;
				PostMessage(hDlg,MSG_CLOSE,0,0);
				return 1;
			}
#ifdef UPEK
			else if (LOWORD(wParam)==SCANCODE_MENU)
			{
				UpekInit(hDlg);
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
			}
#endif
                        break;
     case MSG_PAINT:
                hdc=BeginPaint(hDlg);
		FillBoxWithBitmap(hdc,0,0,190,210,&fpbg);
		SorShowTestInfo(hdc,TestItem);
                EndPaint(hDlg,hdc);
                return 0;
    case MSG_CLOSE:
	UnloadBitmap(&fpbg);
	KillTimer(hDlg,0x80080);
	FREE(myImageBuffer);
        EndDialog (hDlg, IDCANCEL);
        return 0;
        
    case MSG_COMMAND:
        break;
    }
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_AUTO_SORTEST(HWND hWnd)
{
//extern void* fpHandle;
    SorTestDlgBox.w = gOptions.LCDWidth;
    SorTestDlgBox.h = gOptions.LCDHeight;

    SorTestDlgBox.caption = LoadStrByID(HIT_AUTO3);
    SorTestDlgBox.controls = SorTestCtrl;
    DialogBoxIndirectParam (&SorTestDlgBox, hWnd, SorTestDialogBoxProc, 0L);
}


#endif
