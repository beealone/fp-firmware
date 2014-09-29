/*
 * SSR 2.0 Self Service Record 主入口头文件
 * 设计：CWX        2007.1.5
 * 原始版本:1.0.0
 * 修改记录:
 * 编译环境:mipsel-gcc
 */


#ifndef __CWX_GUI_DATADLG
#define __CWX_GUI_DATADLG

#include <dirent.h>
#include <asm/unaligned.h>
#include <sys/stat.h>
#include "ssrcommon.h"
#include "options.h"
#include "flashdb.h"
#include "usb_helper.h"
#include "updatepic.h"
#include "ssrdatapic.h"
#include "ssrpub.h"
#include "finger.h"
#include "exfun.h"
#include "main.h"

#ifdef FACE
#include "facedb.h"
#endif

#define PHOTO_SIZE_MAX (30*1024)

static int gDataDlgItem = 0;
static int gRun = 0;
//static BITMAP ubk;

static DLGTEMPLATE DataDlgDlgBox =
{
	WS_BORDER,
	WS_EX_NONE,
	20, 70, 279, 80,
	"",
	0, 0,
	1, NULL,
	0
};

static CTRLDATA DataDlgCtrl [] =
{
	{CTRL_STATIC, WS_TABSTOP | WS_VISIBLE, 1, 1, 259, 60, IDC_STATIC, "", 0},
};

static int IsValidFile(char* filename)
{
	int i = 0, j = 0;
	int getextra = 0;
	char tmpname[30];
	char tmpextra[8];

	//printf(" %s name %s\n", __FUNCTION__, filename);
	sprintf(tmpname, "%s", filename);
	for (i = 0;i < strlen(tmpname);i++)
	{
		if (tmpname[i] == '.')
		{
			getextra = 1;
			break;
		}
	}

	if (getextra)
	{
		i++;
		memset(tmpextra, 0, sizeof(tmpextra));
		while (i < strlen(tmpname))
			tmpextra[j++] = tmpname[i++];

		if (strncmp(tmpextra, "jpg", strlen(tmpextra)) == 0)
			return 1;
	}
	return 0;
}

static int CopyImageResource(char* path)
{
	int fd = -1;
	char cmdbuf[80];
	char despath[80];
	int rc;

	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf(cmdbuf, "%s/%s", USB_MOUNTPOINT, "img.bz2");
	if ((fd = open(cmdbuf, O_RDONLY)) != -1)	//文件存在
	{
		close(fd);

		memset(cmdbuf, 0, sizeof(cmdbuf));
		memset(despath, 0, sizeof(despath));
		if (!LoadStr("IMAGEFILEPATH", despath))
			sprintf(despath, "%s", "/mnt/mtdblock/data/");

		sprintf(cmdbuf, "cp %s/%s %simg.bz2 -rf && sync", USB_MOUNTPOINT, "img.bz2", despath);
		//		printf("UpdateTheme:%s\n", cmdbuf);
		rc = systemEx(cmdbuf);
		clearSystemCaches(); //zsliu add clear system cache
		return (rc == EXIT_SUCCESS);
	}
	return 0;
}
/*用户照片U盘上传至服务器，add by yangxiaolong,2011-7-6,start*/
void  UserPicUploadToSvr(char * strUserPicName)
{
	char *pstr;
	TUser user;

	pstr = strstr(strUserPicName, ".");
	*pstr = '\0';
	memset(&user, 0, sizeof(user));
	if (FDB_GetUserByCharPIN2(strUserPicName, &user)) {
		FDB_AddOPLog(ADMINPIN, OP_ENROLL_USERPIC, user.PIN, 0,0,0);
	}
}
/*用户照片U盘上传，add by yangxiaolong,2011-7-6,end*/
static int CopyPhoto(char* path)
{
	struct stat photosize;
	struct dirent* ent = NULL;
	DIR *pDir;
	char temp[80];
	char command[255];
	int cpcount = 0;
	BOOL pflag = FALSE;

	//目标文件夹是否存在
	pDir = opendir(GetPhotoPath(""));
	if (pDir)
	{
		pflag = TRUE;
		closedir(pDir);
	}

	pDir = opendir(path);
	if (pDir)
	{
		while (NULL != (ent = readdir(pDir)))
		{
			if (strlen(ent->d_name) <= gOptions.PIN2Width + 4 && ent->d_type == 8 && IsValidFile(ent->d_name))		//非子目录
			{
				memset(temp, 0, sizeof(temp));
				sprintf(temp, "%s/%s", path, ent->d_name);
				stat(temp, &photosize);
				if (photosize.st_size <= PHOTO_SIZE_MAX)
				{
					if (pflag == FALSE)
					{
						memset(command, 0, sizeof(command));
						sprintf(command, "mkdir %s && sync", GetPhotoPath(""));
						if (systemEx(command) == EXIT_SUCCESS)
						{
							UserPicUploadToSvr(ent->d_name);
							pflag = TRUE;
						}

					}
					if (pflag == TRUE)
					{
						memset(command, 0, sizeof(command));
						sprintf(command, "cp %s %s -f && sync", temp, GetPhotoPath(""));
						if (systemEx(command) == EXIT_SUCCESS)
						{
							UserPicUploadToSvr(ent->d_name);
							cpcount++;
						}
						clearSystemCaches(); //zsliu add clear system cache
					}
				}
			}
		}
		closedir(pDir);
	}
	return cpcount;
}

#define upuserbuf (200*1024)
#define uptmpopbuf (100*1024)
#define uptmpsbuf (400*1024)
#define Maxupsize (600*1024)
#define MaxUpusersize (300*1024)
int UploadUserData(void)
{
	char buffer[80];
	int sign = FALSE;
	//int ret;
	int fd;
	TUser tmp;
	TTemplate curtmp;

	char *TmpOpBuf, *UserInfoOpBuf, *TmpsBuf;
	int  TmpOpBufLen, UserInfoOpBufLen, TmpsBufLen;
	char *Updata = NULL;
	int len = 0;

	//ini batchbuf;
	UserInfoOpBufLen = 0;
	UserInfoOpBuf = NULL;
	TmpOpBufLen = 0;
	TmpOpBuf = NULL;
	TmpsBufLen = 0;
	TmpsBuf = NULL;

	//start upload data
	sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "user.dat");
	if ((fd = open(buffer, O_RDONLY)) != -1)
	{
		lseek(fd, 0, SEEK_SET);
		UserInfoOpBuf = MALLOC(upuserbuf);
		memset(UserInfoOpBuf, 0, upuserbuf);
		while (TRUE)
		{
			if ((read(fd, &tmp, sizeof(TUser)) == sizeof(TUser)) && (tmp.PIN > 0))
			{
				*(U8*)(UserInfoOpBuf + UserInfoOpBufLen) = 2;   //操作标志
				memcpy(UserInfoOpBuf + UserInfoOpBufLen + 1, &tmp, sizeof(TUser));//用户信息
				UserInfoOpBufLen += sizeof(TUser) + 1;
				len = 12 + UserInfoOpBufLen;
				if (len >= (MaxUpusersize - 512))
				{
					Updata = MALLOC(MaxUpusersize);
					memcpy(Updata, &UserInfoOpBufLen, 4);
					memcpy(Updata + 4, &TmpOpBufLen, 4);
					memcpy(Updata + 4 + 4, &TmpsBufLen, 4);
					memcpy(Updata + 12, UserInfoOpBuf, UserInfoOpBufLen);
					BatchOPUserData((BYTE *)Updata);
					FREE(UserInfoOpBuf);
					UserInfoOpBuf = NULL;
					UserInfoOpBufLen = 0;
					FREE(Updata);
					UserInfoOpBuf = MALLOC(upuserbuf);
					len = 0;
				}
			}
			else
			{
				if (len > 0)
				{
					Updata = MALLOC(600 * 1024);
					memcpy(Updata, &UserInfoOpBufLen, 4);
					memcpy(Updata + 4, &TmpOpBufLen, 4);
					memcpy(Updata + 4 + 4, &TmpsBufLen, 4);
					memcpy(Updata + 12, UserInfoOpBuf, UserInfoOpBufLen);
					BatchOPUserData((BYTE *)Updata);
					FREE(UserInfoOpBuf);
					UserInfoOpBuf = NULL;
					UserInfoOpBufLen = 0;
					FREE(Updata);
					len = 0;
				}
				else
					break;
			}
		}
		close(fd);

		if (!gOptions.IsOnlyRFMachine)
		{
			sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.dat");
			if ((fd = open(buffer, O_RDONLY)) != -1)
			{
				lseek(fd, 0, SEEK_SET);
				TmpOpBuf = MALLOC(uptmpopbuf);
				TmpsBuf = MALLOC(uptmpsbuf);
				len = 0;
				while (TRUE)
				{
					if ((read(fd, &curtmp, sizeof(TTemplate)) == sizeof(TTemplate)) && curtmp.Valid)
					{
						(TmpOpBuf + TmpOpBufLen)[0] = 2;//操作标志
						memcpy(TmpOpBuf + TmpOpBufLen + 1, &(curtmp.PIN), 2); //UserID
						(TmpOpBuf + TmpOpBufLen)[3] = (char)curtmp.FingerID;    //FingerID
						memcpy(TmpOpBuf + TmpOpBufLen + 4, &TmpsBufLen, 4);//Offset
						TmpOpBufLen += 8;
						memcpy(TmpsBuf + TmpsBufLen, &curtmp.Size - 6, 2);    //模板长度
						memcpy(TmpsBuf + TmpsBufLen + 2, curtmp.Template, (curtmp.Size - 6));//模板
						TmpsBufLen += (curtmp.Size - 6) + 2;                    //模板区总长度
						len = 12 + UserInfoOpBufLen + TmpOpBufLen + TmpsBufLen;
						if (len >= (uptmpsbuf - 1024))
						{
							Updata = MALLOC(Maxupsize);
							memset(Updata, 0, Maxupsize);
							memcpy(Updata, &UserInfoOpBufLen, 4);
							memcpy(Updata + 4, &TmpOpBufLen, 4);
							memcpy((Updata + 4 + 4), &TmpsBufLen, 4);
							memcpy(Updata + 12 + UserInfoOpBufLen, TmpOpBuf, TmpOpBufLen);
							memcpy(Updata + 12 + UserInfoOpBufLen + TmpOpBufLen, TmpsBuf, TmpsBufLen);
							BatchOPUserData((BYTE *)Updata);
							FREE(TmpOpBuf);
							FREE(TmpsBuf);
							FREE(Updata);
							TmpOpBufLen = 0;
							TmpsBufLen = 0;
							TmpOpBuf = MALLOC(uptmpopbuf);
							TmpsBuf = MALLOC(uptmpsbuf);
							len = 0;
						}
					}
					else
					{
						if (len > 0)
						{
							Updata = MALLOC(Maxupsize);
							memset(Updata, 0, Maxupsize);
							memcpy(Updata, &UserInfoOpBufLen, 4);
							memcpy((Updata + 4), &TmpOpBufLen, 4);
							memcpy((Updata + 4 + 4), &TmpsBufLen, 4);
							memcpy(Updata + 12 + UserInfoOpBufLen, TmpOpBuf, TmpOpBufLen);
							memcpy(Updata + 12 + UserInfoOpBufLen + TmpOpBufLen, TmpsBuf, TmpsBufLen);
							BatchOPUserData((BYTE *)Updata);
							FREE(TmpOpBuf);
							FREE(TmpsBuf);
							FREE(Updata);
							TmpOpBufLen = 0;
							TmpsBufLen = 0;
							TmpOpBuf = MALLOC(uptmpopbuf);
							TmpsBuf = MALLOC(uptmpsbuf);
							len = 0;
						}
						else
							break;
					}
				}
				sign = TRUE;
				close(fd);
			}
		}
		else
			sign = TRUE;
	}

	if (sign)
	{
		FDB_InitDBs(FALSE);
		if (gOptions.ZKFPVersion != ZKFPV10)
			FPInit(NULL);
	}
	return sign;
}

extern int downclearflag;
extern int gCurCaptureCount;

long DataDlgShowInfo(HWND hDlg, HDC hdc, int item)
{
	char buffer[80];
	int mount;
	int fd;
	//TUser tmp;
	TTemplate curtmp;
	TSms sms;
	TUData udata;
	int err = 0, tmpvalue = 0;
	BITMAP bmp;
#ifdef FACE
	TFaceTmp ftmp;
#endif

	if (gRun == 0)
		gRun = 1;
	else
		return 1;

	DoUmountUdisk();
	sync();
	if (LoadBitmap(HDC_SCREEN, &bmp, GetBmpPath("info.gif")))
		return 0;

	tmpvalue = SetBkMode(hdc, BM_TRANSPARENT);
	SetTextColor(hdc, PIXEL_lightwhite);

	mount = DoMountUdisk();

	if (mount == 0)
	{
		err = 0;
		memset(buffer, 0, sizeof(buffer));
		switch (item)
		{
			case 0:		//下传考勤记录
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));
				sprintf(buffer, "%s/%d_%s", USB_MOUNTPOINT, gOptions.DeviceID, "attlog.dat");
				if (!FDB_Download(FCT_ATTLOG, buffer)) err = 1;
				break;

			case 1:		//下传用户信息
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));
				sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "user.dat");
				if (FDB_Download(FCT_USER, buffer))
				{
					if (!gOptions.IsOnlyRFMachine)
					{
						int flag = 0;

						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer, "rm %s/%s %s/%s -f", USB_MOUNTPOINT, "template.dat", USB_MOUNTPOINT, "template.fp10");
						systemEx(buffer);
						sprintf(buffer, "rm %s/%s -f", USB_MOUNTPOINT, "template.fp10.1");
						systemEx(buffer);

						memset(buffer, 0, sizeof(buffer));
						if (gOptions.ZKFPVersion == ZKFPV09)
						{
							sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.dat");
							FDB_Download(FCT_FINGERTMP, buffer);
						}
						else if (gOptions.ZKFPVersion == ZKFPV10)
						{
							flag = ZKFPV10_1_FORMAT;
							sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.fp10");
							ProcessDownload(buffer, flag);

							flag = ZKFPV10_2_FORMAT;
							sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.fp10.1");
							ProcessDownload(buffer, flag);
						}
					}
#ifdef FACE
					if (gOptions.FaceFunOn)
					{
						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "ssrface.dat");
						FDB_Download(FCT_FACE, buffer);
					}
#endif
				}
				else err = 1;
				break;

			case 2:		//下传短消息
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));
				sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "sms.dat");
				if (FDB_CntSms() > 0)	//是否存在短消息
				{
					if (FDB_Download(FCT_SMS, buffer))
					{
						sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "udata.dat");
						FDB_Download(FCT_UDATA, buffer);
					}
					else err = 1;
				}
				else
					err = 1;
				break;

			case 3:		//下传用户照片
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));
				sprintf(buffer, "cp %s %s/ -rf && sync", GetPhotoPath(""), USB_MOUNTPOINT);
				if (systemEx(buffer) != EXIT_SUCCESS)
					err = 1;
				clearSystemCaches(); //zsliu add clear system cache

				break;

			case 4:		//上传用户信息
			{
				U16 adduserPIN[65535];
				//U16 UDiskUserPIN[65535];
				//U16 AdminUserPINFlag[65535];
				int AdminCount = 0;
				U16 *pAdminPin=NULL;
				int returnValue=0;

				memset(adduserPIN,0,sizeof(adduserPIN));
				if (pushsdk_is_running())
				{
					pushsdk_pause();
				}
				//memset(UDiskUserPIN, 0, sizeof(UDiskUserPIN));
				//memset(AdminUserPINFlag, 0, sizeof(AdminUserPINFlag));
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO4));

				sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "user.dat");
				if ((fd = open(buffer, O_RDONLY)) != -1)
				{
					int datainlen ;
					char *datain=NULL;
					datainlen = lseek(fd, 0, SEEK_END);
					if (datainlen >= sizeof(TUser))
					{
						datain  = (char*)malloc(datainlen);
						if (!datain) {
							err=1;
							close(fd);
							break;
						}

						lseek(fd, 0, SEEK_SET);
						if (read(fd, datain, datainlen) == datainlen)
						{
							pAdminPin=ProcessUploadUDiskUser(datain,datainlen,adduserPIN,&returnValue,
									&AdminCount);

						}
						FREE(datain);
					}

					close(fd);

					if (!gOptions.IsOnlyRFMachine)
					{
						int i = 0; 
						memset(buffer, 0, sizeof(buffer));
						if (gOptions.ZKFPVersion == ZKFPV09)
						{
							sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.dat");
							if ((fd = open(buffer, O_RDONLY)) != -1) {
								lseek(fd, 0, SEEK_SET);
								while (TRUE)
								{
									if ((read(fd, &curtmp, sizeof(TTemplate)) == sizeof(TTemplate)) 
											&& curtmp.Valid) {
										if ((U16)adduserPIN[curtmp.PIN] > 0) {
											AppendUserTemp((U16)adduserPIN[curtmp.PIN], 
													"NONE", curtmp.FingerID, 
													(char*)curtmp.Template, 
													curtmp.Size - 6, curtmp.Valid);
										}
										if (pAdminPin) {
											for(i=0; i<AdminCount; i++ ) {
												if (pAdminPin[i]==curtmp.PIN) {
													pAdminPin[i]=0;
												}
											}
										}
									} else {
										printf("read template fail,curtmp.Valid=%d\n",curtmp.Valid);
										break;
									}
								}
								close(fd);
							}
							FPInit(NULL);
						}
						else if (gOptions.ZKFPVersion == ZKFPV10)
						{
							int flag =  ZKFPV10_2_FORMAT;
							unsigned char *p;
							char *tmp;
							sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.fp10.1");
							fd = open(buffer, O_RDONLY);
							if (fd < 0)
							{
								//for compatible
								memset(buffer, 0, sizeof(buffer));
								sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "template.fp10");
								flag = ZKFPV10_1_FORMAT;
								fd = open(buffer, O_RDONLY);
							}

							if (fd != -1)
							{
								int tmpsize = 0;//,i;
								U16 pin = 0;
								BYTE fingerindex = 0, valid = 0;
								lseek(fd, 0, SEEK_SET);
								p = (unsigned char *)MALLOC(6);
								if (p==NULL) {
									break;
								}

								while (TRUE)
								{
									memset(p, 0, 6);
									if (!(read(fd, p, 6) == 6))
									{
										printf("read head of template failed\n");
										break;
									}

									pin = GETWORD(p + 2);
									fingerindex = *(p + 4);
									valid = *(p + 5);
									if (valid == 0)
										valid = 1;
									tmpsize = GETWORD(p) - 6;

									tmp = (char *)MALLOC(tmpsize);
									if (tmp==NULL) {
										break;
									}
									if (read(fd, tmp, tmpsize) == tmpsize)
									{
										AppendUserTemp((U16)adduserPIN[pin], NULL, fingerindex, tmp, 
												tmpsize, valid);
										FREE(tmp);
										if (pAdminPin) {
											for(i=0; i<AdminCount; i++ ) {
												if (pAdminPin[i]==pin) {
													pAdminPin[i]=0;
												}
											}
										}

									}
									else
									{
										printf("read template failed\n");
										FREE(tmp);
										break;
									}
								}
								FREE(p);
								close(fd);
							}
						}

					}
					if (pushsdk_is_running())
					{
						pushsdk_resume();
						pushsdk_data_new(FCT_OPLOG);
					}

#ifdef FACE
					if (gOptions.FaceFunOn)
					{
						int Cnt = FDB_CntFaceUser();
						int Curid = 0;
						sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "ssrface.dat");
						if ((fd = open(buffer, O_RDONLY)) != -1)
						{
							U16 newPIN;
							lseek(fd, 0, SEEK_SET);
							while (TRUE)
							{
								if ((read(fd, &ftmp, sizeof(TFaceTmp)) == sizeof(TFaceTmp)))
								{
									newPIN = adduserPIN[ftmp.PIN];
									if (newPIN != Curid) {
										Curid = newPIN;
										Cnt = FDB_CntFaceUser();
										if ((Cnt >= (gOptions.MaxFaceCount)*100) && \
												NULL == FDB_GetFaceRecord(newPIN, 0, NULL)) {
											break;
										}
									}
									if (pAdminPin) {
										for(i=0; i<AdminCount; i++ ) {
											if (pAdminPin[i]==curtmp.PIN) {
												pAdminPin[i]=0;
											}
										}
									}

									ftmp.PIN = newPIN;
									ChangeFaceTmp((U16)newPIN, ftmp.FaceID, &ftmp);
								} else {
									break;
								}
							}
							close(fd);
						}
					}

#endif
					FDB_InitDBs(FALSE);
					if (pAdminPin) {
						int admincounts = 0;
						for (admincounts = 0; admincounts < AdminCount; admincounts++)
						{
							if (pAdminPin[admincounts] > 0) {
								FDB_ChgUserByPIN(pAdminPin[admincounts], 0);
							}
						}
						free(pAdminPin);
					}

				} else {
					err = 1;
				}
				//				printf(" end upload user, use time %d ms\n", GetTickCount1()-msc);
				break;
			}
			case 5:		//上传短消息
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO4));
				sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "sms.dat");
				if ((fd = open(buffer, O_RDONLY)) != -1)
				{
					lseek(fd, 0, SEEK_SET);
					while (TRUE)
					{
						if (read(fd, &sms, sizeof(TSms)) == sizeof(TSms))
						{
							if (FDB_ChgSms(&sms) == FDB_ERROR_NODATA)
							{
								if (FDB_AddSms(&sms) != FDB_OK) break;
							}
						}
						else break;
					}
					close(fd);

					sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "udata.dat");
					if ((fd = open(buffer, O_RDONLY)) != -1)
					{
						lseek(fd, 0, SEEK_SET);
						while (TRUE)
						{
							if (read(fd, &udata, sizeof(TUData)) == sizeof(TUData))
							{
								if (FDB_DelUData(udata.PIN, 0) == FDB_OK)
									if (FDB_AddUData(&udata) != FDB_OK) break;
							}
							else break;
						}
						close(fd);
					}
				}
				else
					err = 1;
				if (gOptions.IsSupportSMS) CheckBoardSMS();
				break;

			case 6:		//上传用户照片
				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO4));

				//上传照片必须存放在U盘"photo"目录中
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%s/%s", USB_MOUNTPOINT, "photo");
				//printf("buffer %s\n",buffer);
				if (CopyPhoto(buffer) <= 0)
					err = 1;
				break;
			case 7:		//上传宣传图片
				{
					int i;
					for (i = 0;i < 16;i++)
						//memset(&uPicList[i],0,128);
						memset(&uPicList[i], 0, 16);
					FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, gOptions.LCDHeight, get_submenubg_jgp());
					FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
					TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO15));

					for (uPic = 0;uPic < 16;uPic++)
						memset(uPicList[uPic], 0, 16);
					uPic = 0;
					uPic = UpdatePic(USB_MOUNTPOINT, "ad_", uPicList);
					if (uPic)
						SSR_MENU_DATAPICDLG(hDlg);
					else
						MessageBox1(hDlg, LoadStrByID(HIT_UINFO16), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
					break;
				}
			case 11:	//上传img.bz2
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO4));
				if (!CopyImageResource(USB_MOUNTPOINT))
					err = 1;
				break;
			case 8:		//下载全部照片
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));

				sprintf(buffer, "%s/pic_%d", USB_MOUNTPOINT, gOptions.DeviceID);
				mkdir(buffer, 0);
				sync();
				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "cp -a %s/* %s/pic_%d && sync", GetCapturePath("capture"), USB_MOUNTPOINT, gOptions.DeviceID);
				if (systemEx(buffer) != EXIT_SUCCESS)
					err = 1;
				clearSystemCaches(); //zsliu add clear system cache

				break;

			case 9:		//下载考勤照片
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));

				sprintf(buffer, "%s/pic_%d", USB_MOUNTPOINT, gOptions.DeviceID);
				mkdir(buffer, 0);
				sync();
				memset(buffer, 0, sizeof(buffer));
				//修改u盘上传照片无法覆盖问题add 2012-09-08
				sprintf(buffer, "cp -a %s %s/pic_%d && sync", GetCapturePath("capture/pass"), USB_MOUNTPOINT, gOptions.DeviceID);
				if (systemEx(buffer) != EXIT_SUCCESS)
					err = 1;
				clearSystemCaches(); //zsliu add clear system cache


				break;

			case 10:	//下载黑名单照片
				FillBoxWithBitmap(hdc, 0, 0, 0, 0, get_submenubg_jgp());
				FillBoxWithBitmap(hdc, 5, 5, 35, 35, &bmp);
				TabbedTextOut(hdc, 10, 45, LoadStrByID(HIT_UINFO3));

				sprintf(buffer, "%s/pic_%d", USB_MOUNTPOINT, gOptions.DeviceID);
				mkdir(buffer, 0);
				sync();
				memset(buffer, 0, sizeof(buffer));
				//修改u盘上传照片无法覆盖问题add 2012-09-08
				sprintf(buffer, "cp -a %s %s/pic_%d && sync", GetCapturePath("capture/bad"), USB_MOUNTPOINT, gOptions.DeviceID);
				if (systemEx(buffer) != EXIT_SUCCESS)
					err = 1;
				clearSystemCaches(); //zsliu add clear system cache


				break;
		}

		DoUmountUdisk();
		if (err == 0)
		{
			if (item < 4 || (item > 7 && item < 11))
				MessageBox1(hDlg, LoadStrByID(HIT_UINFO5), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
			else if (item > 3 && item < 7)
				MessageBox1(hDlg, LoadStrByID(HIT_UINFO6), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
			else if (item == 11)		//上传图片资源包成功
				MessageBox1(hDlg, LoadStrByID(MID_IKIOSK_REBOOT), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);

			if (downclearflag)            //删除已下载照片
			{
				char buf[100];

				memset(buf, 0, sizeof(buf));
				if (item == 8)
				{
					//lyy
					//sprintf(buf, "rm -rf %s/* && sync", GetCapturePath("capture/pass"));
					//Modified for bug "rm , lyy 20090616
					sprintf(buf, DELALLPASSPIC, GetCapturePath(""), GetCapturePath(""));
					if (systemEx(buf) == EXIT_SUCCESS)
						FDB_ClearIndex(0);

					memset(buf, 0, sizeof(buf));
					//					sprintf(buf, "rm -rf %s/* && sync", GetCapturePath("capture/bad"));
					//Modified for bug "rm , lyy 20090616
					sprintf(buf, DELALLBADPIC, GetCapturePath(""), GetCapturePath(""));
					if (systemEx(buf) == EXIT_SUCCESS);
					FDB_ClearIndex(1);
					sync();
				}
				else if (item == 9)
				{
					//sprintf(buf, "rm -rf %s/* && sync", GetCapturePath("capture/pass"));
					sprintf(buf, DELALLPASSPIC, GetCapturePath(""), GetCapturePath(""));
					if (systemEx(buf) == EXIT_SUCCESS)
						FDB_ClearIndex(0);
				}
				else if (item == 10)
				{
					//		            sprintf(buf, "rm -rf %s/* && sync", GetCapturePath("capture/bad"));
					//Modified for bug "rm , lyy 20090616
					sprintf(buf, DELALLBADPIC, GetCapturePath(""), GetCapturePath(""));
					if (systemEx(buf) == EXIT_SUCCESS)
						FDB_ClearIndex(1);
				}
				gCurCaptureCount = FDB_CntPhotoCount();	//rest capture picture flag

				//	printf (" %s -----%s\n",__FUNCTION__,buf);
			}
		}
		else
		{
			MessageBox1(hDlg, LoadStrByID(HIT_UINFO14), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
		}
	}
	else
	{
		MessageBox1(hDlg, LoadStrByID(HIT_UINFO7), LoadStrByID(HIT_RUN), MB_OK | MB_ICONINFORMATION);
	}

	UnloadBitmap(&bmp);
	if (!ismenutimeout) PostMessage(hDlg, MSG_KEYDOWN, SCANCODE_ESCAPE, 0);
	return 1;
}

static int DataDlgDialogBoxProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	switch (message)
	{
		case MSG_INITDIALOG:
			gRun = 0;
			//if (LoadBitmap(HDC_SCREEN, &ubk, GetBmpPath("submenubg.jpg")))
			//	return 0;
			break;

		case MSG_KEYDOWN:
			SetMenuTimeOut(time(NULL));

			if (LOWORD(wParam) == SCANCODE_ENTER || LOWORD(wParam) == SCANCODE_F10 || LOWORD(wParam) == SCANCODE_ESCAPE)
			{
				PostMessage(hDlg, MSG_CLOSE, 0, 0);
				return 0;
			}
			break;

		case MSG_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				const RECT* clip = (const RECT*) lParam;
				BOOL fGetDC = FALSE;
				RECT rcTemp;

				if (hdc == 0)
				{
					hdc = GetClientDC(hDlg);
					fGetDC = TRUE;
				}

				if (clip)
				{
					rcTemp = *clip;
					ScreenToClient(hDlg, &rcTemp.left, &rcTemp.top);
					ScreenToClient(hDlg, &rcTemp.right, &rcTemp.bottom);
					IncludeClipRect(hdc, &rcTemp);
				}

				FillBoxWithBitmap(hdc, 0, 0, gOptions.LCDWidth, 0, get_submenubg_jgp());

				if (fGetDC)
					ReleaseDC(hdc);
				return 0;
			}

		case MSG_PAINT:
			hdc = BeginPaint(hDlg);
			busyflag = 1;
			DataDlgShowInfo(hDlg, hdc, gDataDlgItem);
			SetMenuTimeOut(time(NULL));
			EndPaint(hDlg, hdc);
			return 0;

		case MSG_CLOSE:
			//UnloadBitmap(&ubk);
			DestroyAllControls(hDlg);
			EndDialog(hDlg, IDCANCEL);
			return 0;
	}

	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

void SSR_MENU_DATADLG(HWND hWnd, int i)
{
	DataDlgDlgBox.x = 20 + gOptions.GridWidth;
	DataDlgDlgBox.w = 279 + gOptions.GridWidth * 2;

	gDataDlgItem = i;
	DataDlgDlgBox.controls = DataDlgCtrl;
	DialogBoxIndirectParam(&DataDlgDlgBox, hWnd, DataDlgDialogBoxProc, 0L);
}


#endif
