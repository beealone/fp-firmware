/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/

 
#ifndef __CWX_GUI_DATAPICDLG

#define __CWX_GUI_DATAPICDLG

#include "ssrcommon.h"
#include "ssrpub.h"

static BITMAP up,down;
static int picitem=0;
static HWND PicItemWnd[2];
static int PicItem;
int pictype=0;	//图片类型
#define IDC_UPLOADPIC 8888
extern RECT gMainMenu_rc;

static DLGTEMPLATE DataPicDlgDlgBox =

{
    WS_BORDER,
    WS_EX_NONE,
    1, 25, 319, 210, 
    "",
    0, 0,
    3, NULL,
    0
};

static CTRLDATA DataPicDlgCtrl [] =
{ 
    {
        CTRL_STATIC,
        WS_TABSTOP | WS_VISIBLE, 
        1, 1, 70, 25,
        0x11050, 
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 180, 80, 25,
        IDCANCEL,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
        5, 180, 80, 25,
        IDC_UPLOADPIC,
        "",
        0
    }
};

//dsl 2011.9.29
int copy_file(char *dest_file,char *src_file)
{
	int size;
	FILE *srcfp,*destfp;
	char buff[1024];
	int ret=0;
	
	if ((srcfp=fopen(src_file,"r")) == NULL)
		return -1;
	if ((destfp=fopen(dest_file,"wb")) == NULL)
	{
		fclose(srcfp);
		return -2;
	}

	memset(buff, 0, sizeof(buff));
	while ((size=fread(buff,sizeof(char),sizeof(buff),srcfp)) > 0)
	{
		if (fwrite(buff,sizeof(char),size,destfp) != size)
		{
			ret=-3;
			break;
		}
	}
	fclose(srcfp);
	fclose(destfp);
	return ret;
}


int CopyPic(HWND hWnd,int pp)
{
	//dsl 2011.9.29
	//char sys[256];
	char dest[128], src[128];
	int ret, tmpvalue = 0;
	HDC hdc;
	hdc=GetClientDC(hWnd);

	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
        SetTextColor(hdc,PIXEL_lightwhite);

	FillBoxWithBitmap(hdc,100,60,200,40,get_submenubg_jgp());

        TabbedTextOut(hdc,120,70,LoadStrByID(HIT_UPIC7));

	ReleaseDC(hdc);
//dsl 2011.9.29
#if 0
        sprintf(sys,"cp -rf %s/%s %s",USB_MOUNTPOINT,uPicList[pp],GetPicPath("adpic"));
        system(sys);
#endif
	memset(dest, 0, sizeof(dest));
	memset(src, 0, sizeof(src));
	sprintf(src, "%s/%s", USB_MOUNTPOINT,uPicList[pp]);
	sprintf(dest, "%s/%s", GetPicPath("adpic"), uPicList[pp]);
	printf("file:%s, %s\n", src, dest);
	ret=copy_file(dest, src);
	sync();
	return ret;
}

long DataPicDlgShowInfo(HWND hDlg,HDC hdc,int item)
{
	char str[255];
	int tmpvalue = 0;
	BITMAP mybmp;

	tmpvalue = SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,PIXEL_lightwhite);

	TabbedTextOut(hdc,5,120,LoadStrByID(HIT_UPIC4));
	TabbedTextOut(hdc,5,145,LoadStrByID(HIT_UPIC5));

	FillBoxWithBitmap(hdc,55,120,15,15,&up);
	FillBoxWithBitmap(hdc,55,145,15,15,&down);

	sprintf(str,LoadStrByID(HIT_UPIC2),item+1,uPic);
	TabbedTextOut(hdc,5,60,str);

	TabbedTextOut(hdc,120+ gOptions.ControlOffset, 165,uPicList[item]);

	sprintf(str,"%s/%s",USB_MOUNTPOINT,uPicList[item]);

	if(LoadBitmap(HDC_SCREEN,&mybmp,str))
        	return 0;
	FillBoxWithBitmap(hdc,90 + gOptions.ControlOffset, 5,220,160,&mybmp);
	UnloadBitmap(&mybmp);
	return 1;
}

static int DataPicDlgDialogBoxProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static char keyupFlag=0;
	switch (message) 
	{
		case MSG_INITDIALOG:
			//if (LoadBitmap(HDC_SCREEN,&upicbk,GetBmpPath("submenubg.jpg")))
                	//        return 0;
			if (LoadBitmap(HDC_SCREEN,&up,GetBmpPath("up.gif")))
                	        return 0;
			if (LoadBitmap(HDC_SCREEN,&down,GetBmpPath("down.gif")))
        	                return 0;

			SetWindowText(GetDlgItem(hDlg,0x11050),LoadStrByID(HIT_UPIC1));
	        	SetWindowText(GetDlgItem(hDlg,IDC_UPLOADPIC),LoadStrByID(HIT_UPIC3));
        		SetWindowText(GetDlgItem(hDlg,IDCANCEL),LoadStrByID(HIT_CANCEL));

			PicItemWnd[0] = GetDlgItem (hDlg, IDC_UPLOADPIC);
		        PicItemWnd[1] = GetDlgItem (hDlg, IDCANCEL);
			SetFocus(PicItemWnd[0]);
			PicItem=0;
			picitem=0;
	        	return 1;

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
			if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKDOWN))
                        {
				SetFocus(PicItemWnd[0]);
				if(++picitem>uPic-1)
					picitem=0;
				InvalidateRect(hDlg,&gMainMenu_rc,TRUE);
				return 1;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKUP))
                        {
				SetFocus(PicItemWnd[0]);
				if(--picitem<0)
					picitem=uPic-1;
				InvalidateRect(hDlg,&gMainMenu_rc,TRUE);
				return 1;
                        }
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKRIGHT))
			{	
				if(++PicItem>1)	PicItem=0;
				SetFocus(PicItemWnd[PicItem]);
				return 1;
			}
			else if ((LOWORD(wParam)==SCANCODE_CURSORBLOCKLEFT))
			{
				if(--PicItem<0) PicItem=1;
				SetFocus(PicItemWnd[PicItem]);
				return 1;
			}
			else if ((LOWORD(wParam)==SCANCODE_ENTER)||(LOWORD(wParam)==SCANCODE_F10))
                        {
				SetFocus(PicItemWnd[0]);
				if(PicItem==0)
				{
					CopyPic(hDlg,picitem);
					MessageBox1(hDlg,LoadStrByID(HIT_UPIC6),LoadStrByID(HIT_RUN),MB_OK| MB_ICONINFORMATION);
					if(!ismenutimeout)
						SendMessage(hDlg,MSG_KEYDOWN,SCANCODE_CURSORBLOCKDOWN,0);
				}
				else if(PicItem==1)
					SendMessage(hDlg,MSG_CLOSE,0,0);
				return 1;	
                        }
			else if ((LOWORD(wParam)==SCANCODE_ESCAPE))
			{
				SendMessage(hDlg,MSG_CLOSE,0,0);
				return 1;
			}
                        break;

		case MSG_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;
			const RECT* clip = (const RECT*) lParam;
			BOOL fGetDC = FALSE;
			RECT rcTemp;

			if(hdc == 0) 
			{
				hdc = GetClientDC (hDlg);
				fGetDC = TRUE;
			}

			if (clip) 
			{
				rcTemp = *clip;
				ScreenToClient (hDlg, &rcTemp.left, &rcTemp.top);
				ScreenToClient (hDlg, &rcTemp.right, &rcTemp.bottom);
				IncludeClipRect (hdc, &rcTemp);
			}

			FillBoxWithBitmap (hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());

			if (fGetDC)
				ReleaseDC (hdc);
			return 0;
		}

		case MSG_PAINT:
	                hdc=BeginPaint(hDlg);
			DataPicDlgShowInfo(hDlg,hdc,picitem);
	                EndPaint(hDlg,hdc);
	                return 0;

		case MSG_CLOSE:
			//UnloadBitmap(&upicbk);
			UnloadBitmap(&up);
			UnloadBitmap(&down);
		        EndDialog (hDlg, IDCANCEL);
		        return 0;
        
	}
    
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

void SSR_MENU_DATAPICDLG(HWND hWnd)
{
	gMainMenu_rc.right = gOptions.LCDWidth-1;
	gMainMenu_rc.bottom = gOptions.LCDHeight-1;

	DataPicDlgDlgBox.w = gOptions.LCDWidth;
	DataPicDlgDlgBox.h = gOptions.LCDHeight;
	DataPicDlgCtrl[1].x = 230+gOptions.ControlOffset;
    DataPicDlgDlgBox.controls = DataPicDlgCtrl;
    DialogBoxIndirectParam (&DataPicDlgDlgBox, hWnd, DataPicDlgDialogBoxProc, 0L);
}


#endif
