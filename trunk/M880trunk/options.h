/*************************************************

  ZEM 200

  options.h header file for options

  Copyright (C) 2003-2005, ZKSoftware Inc.

  $Log: options.h,v $
  Revision 5.19  2006/03/04 17:30:09  david
  Add multi-language function

  Revision 5.18  2005/12/22 08:54:23  david
  Add workcode and PIN2 support

  Revision 5.17  2005/11/06 02:41:34  david
  Fixed RTC Bug(Synchronize time per hour)

  Revision 5.16  2005/09/19 10:01:59  david
  Add AuthServer Function

  Revision 5.15  2005/08/13 13:26:14  david
  Fixed some minor bugs and Modify schedule bell

  Revision 5.14  2005/08/04 15:42:53  david
  Add Wiegand 26 Output&Fixed some minor bug

  Revision 5.13  2005/08/02 16:07:51  david
  Add Mifare function&Duress function

  Revision 5.12  2005/07/14 16:59:50  david
  Add update firmware by SDK and U-disk

  Revision 5.11  2005/07/07 08:09:05  david
  Fixed AuthServer&Add remote register

  Revision 5.10  2005/06/29 20:21:46  david
  Add MultiAuthServer Support

  Revision 5.9  2005/06/16 23:27:49  david
  Add AuthServer function

  Revision 5.8  2005/06/10 17:10:59  david
  support tcp connection

  Revision 5.7  2005/06/02 20:11:09  david
  Fixed SMS bugs and Add Power Button Control function

  Revision 5.6  2005/05/20 23:41:04  david
  Add USB support for SMS

  Revision 5.5  2005/04/27 00:15:34  david
  Fixed Some Bugs

  Revision 5.4  2005/04/24 11:11:28  david
  Add advanced access control function

  Revision 5.3  2005/04/07 17:01:43  david
  Modify to support A&C and 2 row LCD

 *************************************************/

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "arca.h"
#include "langres.h"

#define STA_IDLE	0	//	等待状态；
#define	STA_ENROLLING	1	//	登记指纹状态；
#define	STA_VERIFYING	2	//	识别指纹状态；
#define	STA_MENU	3	//	执行人机界面菜单
#define	STA_BUSY	4	//	正忙于处理其他工作
#define STA_WRITEMIFARE 5       // 	等待写卡状态

#define TYPE_SLAVE_ID   101

//iCLSRW
#define COM1 1
#define COM2 2
#define COM3 3

#define VALUE_BUFFERLEN 	1024
#define VALUE_BUFFERCACHE 	4096
#define MAX_OPTION_SIZE		(8*1024)
#define MAX_AUTOALARM_CNT 	8

#define ATTLOGLIMIT 		50000
#define USERLIMIT		3000

#ifdef ZEM600
#define VOICE_LOW       50
#define VOICE_MIDDLE    80
#define VOICE_HEIGH     99
#endif
extern int gMachineState;

extern int battery_fd;  // 文件句柄
extern unsigned char battery_state;  // 检测到电池变化时，为避免丢失消息，就多检测几次，另外，用于检测电池的最大值

/*
 *	1. Ver 6.50 （不包括Ver 6.50 )以前的，只支持9.0算法。
 *	2. Ver 6.50 （包括Ver 6.50 )到 Ver 6.60（不包括Ver 6.60）的，是旧的10.0支持固件。
 *	3. Ver 6.60 （包括Ver 6.60 )之后的，是支持新10.0的固件。
 *	新10.0和旧10.0的却别：
 *		旧10.0存储和传输指纹的时候，都是把一个用户的指纹模板的fingerID全部当做0来处理，
 *	也就是同一个用户的指纹模板实际上只是一个文件。
 *	  新10.0的指纹模板，把指纹模板处理方式和9.0一样，每个指纹都有一个fingerID来存储，这样
 *	方便统计指纹，也方便处理后台验证，胁迫指纹,ADMS传输等处理。
 *	备注：这个参数主要用于考勤软件以及门禁软件等软件部分处理10.0的指纹，并且兼容以前的固件。
 *	4.Ver 6.70 BS考勤固件完善项目新增BS方面功能
 */
//#define MAINVERSION "Ver 6.50 "__DATE__
// #define MAINVERSION "Ver 6.60 "__DATE__	//2009年11月17号更改
#define MAINVERSION "Ver 6.70 "__DATE__	//2011年8月30日


//#define MAINVERSIONTFT  "Ver 6.4.1(build 01)"  //add 3 and 3.5 in one code
//#define MAINVERSIONTFT  "Ver 6.4.1(build 02)" //fix some bug for test department 2009-11-25
//#define MAINVERSIONTFT  "Ver 6.4.1(build 03)" //add wiegand and fix hang device bug 2009-11-28
//#define MAINVERSIONTFT  "Ver 6.4.1(build 04)" //fix wiegand bug for verify
//#define MAINVERSIONTFT  "Ver 6.4.1(build 05)" //fix wiegand, default and other 2009-11-29
//#define MAINVERSIONTFT  "Ver 6.4.1(build 06)" //更改高速上传的时候，出现段错误的问题，以及其它一些问题2009-12-01
//#define MAINVERSIONTFT  "Ver 6.4.1(build 07)" //更改按键以及电池部分，并且解决了一些简单的Bug  2009-12-02
//#define MAINVERSIONTFT  "Ver 6.4.1(build 08)" //解决高速下载死机的问题，解决webserver以及固件吃掉大量内存的问题  2009-12-03-01
//#define MAINVERSIONTFT  "Ver 6.4.1(build 09)" //解决多种验证方式时考勤状态不对的情况，修改部分界面，集中在考勤记录查询，修改menu界面，去掉U盘的情况下无法使用快捷键的bug．2009-12-03-03
//#define MAINVERSIONTFT  "Ver 6.4.1(build 10)" //解决关于camera旋转的问题，还有就是图片会覆盖按钮的问题，还有就是算法库文件没有刷新指纹记录。．2009-12-03-04
//#define MAINVERSIONTFT  "Ver 6.4.1(build 11)"
//#define MAINVERSIONTFT  "Ver 6.4.1(build 12)"
//#define MAINVERSIONTFT  "Ver 6.4.1(build 13)"
//#define MAINVERSIONTFT  "Ver 6.4.1(build 14)"
//#define MAINVERSIONTFT  "Ver 6.4.1(build 15)"	//解决联机登记指纹删除不掉的问题，解决时间乱跳的问题。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 16)"	//解决wifi 死机，浏览图片显示重叠，考勤照片刷卡时内存泄漏，键盘自定义可以大于16个字符，高速上传用户混乱的问题
//解决联机登记的时候，如果指纹质量不够，这个时候，固件会提示请重按手指，但是界面没有推出的bug
//解决打开后台验证的时候，界面不刷新的bug, 这个问题具体是时间不能刷新，界面不动，这个问题现在已经解决。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 17)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 18)"	//解决纯9.0机器，升级新的固件，没有质量这个界面，3.5寸彩屏机器默认都有U盘功能。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 19)"	//解决联机登记问题不能退出的情况，解决查看考勤记录的时候，考勤状态有问题的bug,
//#define MAINVERSIONTFT  "Ver 6.4.1(build 20)"	//更改保存照片的时候，使用日期文件夹存储的方式保存，不再使用只有一个文件夹，解决死机问题。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 21)"	//
//#define MAINVERSIONTFT  "Ver 6.4.1(build 22)"    //change update firmware show lcd error
//#define MAINVERSIONTFT  "Ver 6.4.1(build 23)"   //解决后台验证的时候出错的地方20091221
//#define MAINVERSIONTFT  "Ver 6.4.1(build 24)" 	//解决sensor有黑边的问题增加参数 sensorOffsetY，默认为10
/*
   1、ADMS 上传用户指纹bug， 删除全部数据 bug
   2、修改 10.0指纹 bug
   3、修改 射频卡机器 可能出现的 bug
   4.	修改写指纹卡速度慢的问题
   */
//  #define MAINVERSIONTFT  "Ver 6.4.1(build 25)" 	//
/*
   1. GPRS
   2. ADMS
   3. Wiegand out 
   */
//#define MAINVERSIONTFT  "Ver 6.4.1(build 26)"		
//#define MAINVERSIONTFT  "Ver 6.4.1(build 27)"  //主要添加GPRS部分，移交测试部
//#define MAINVERSIONTFT  "Ver 6.4.1(build 28)" //解决B3机器按power键关机后，在按power键不能开机、解决定时响铃功能改变音量后不起作用Bug
//#define MAINVERSIONTFT  "Ver 6.4.1(build 29)" //解决带ADMS功能同时加TTS功能，在设置WEBSERVER的IP地址后返回主界面，当按menu键进入界面后死机问题
//#define MAINVERSIONTFT  "Ver 6.4.1(build 30)" //解决带TTS功能，启动时有杂音问题
//#define MAINVERSIONTFT  "Ver 6.4.1(build 31)" //解决内存溢出，机器反应越来越慢问题
//#define MAINVERSIONTFT  "Ver 6.4.1(build 32)" //fix a bug for Photo ID
//#define MAINVERSIONTFT  "Ver 6.4.1(build 33)" //20100221 解决wifi不通的问题，更改界面设置，解决工程部提出登记邋MF卡的时候，3寸屏机器显示图像有偏移的问题。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 34)" //20100223 更新wifi固件到测试部
//#define MAINVERSIONTFT  "Ver 6.4.1(build 35)" //20100224 更新wifi固件到测试部
//#define MAINVERSIONTFT  "Ver 6.4.1(build 36)" //20100225 更新wifi固件到测试部,解决设置成共享模式，没有密码不通的问题。
//#define MAINVERSIONTFT  "Ver 6.4.1(build 37)" //20100301 wifi设置部分在直连的情况下，有一个bug, 还有就是声音会断掉的问题
//#define MAINVERSIONTFT  "Ver 6.4.1(build 38)" //20100303 临时解决生产上蜂鸣器会发声的问题。编译给客户的是Ver 6.4.1(build 33)
//#define MAINVERSIONTFT  "Ver 6.4.1(build 39)" //20100316   add face(For FS1000 --------imagic)
//#define MAINVERSIONTFT  "Ver 6.4.1(build 40)" //20100316   fixed  FS1000 LED
//#define MAINVERSIONTFT  "Ver 6.4.1(build 41)" //20100311 fix extend bell for command 4
//#define MAINVERSIONTFT  "Ver 6.4.1(build 42)"
//#define MAINVERSIONTFT  "Ver 6.4.1(build 43)" //20100406 解决生产bug：1、3寸屏提示是否保存对话框左键失效2、菜单-系统设置-系统参数项修改设置后按OK提示"重启后生效"问题
/****************************************
 ** 提交者：刘振树
 ** 日期：2010年4月9日
 ** 解决ADMS上传一定用户数后不再上传的问题，这次也优化了摄像头驱动
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.4.1(build 44)"
/****************************************
 ** 姓名：刘振树
 ** 日期：2010年4月12日
 ** 解决俄语状态显示的问题，
 ** main.c 函数static int procmainstatekey(HWND hWnd,int index1)
 ** 更改 TextOut(hdc,x+(130-strlen((char *)temp)*8)/2,y+8,(char *)temp);
 ** 为TextOut(hdc,x+(130-strlen((char *)mystkey.stateName)*8)/2,y+8,(char *)temp);
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.4.1(build 45)" 
/****************************************
 ** 提交日期：2010年4月20日
 ** 提交者：王伟
 ** 主要目的：添加通过PUSH-SDK下载考勤照片功能。
 ** 版本号：Ver 6.4.1(build 46)
 ** 更改文件为：
 ** flashdb.h flashdb.c gridview.c libcam.h(排版的问题)，options.h options.c
 ** ssrvfwin.c ./iclock/checklog.c ./iclock/http.c ./iclock/iclok.h ./iclock/iclockprx.c
 ** SVN  -r 1158

 ****************************************/
/*******************************************************************
 ** 提交日期：2010年5月11日
 ** 提交者：王伟
 ** 主要目的：修改多国语言存在的问题
 **		1,阿拉伯语在某些某些界面显示不全或显示不正确的问题
 **		2,泰语考勤后按Muen查看考勤记录死机的问题
 ** 版本号：Ver 6.4.1(build 47)
 ** 更改文件为：
 ** logquery.c  ssrsystem1.c ssrcgroupmng.c main.c ssrsystem3.c  ssrstkey.c
 ** 
 ** SVN  -r 1264
 ****************************************/

//#define MAINVERSIONTFT  "Ver 6.4.1(build 47)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 54)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 57)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 58)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 61)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 62)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 63)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 64)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 65)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 66)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 67)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 68)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 69)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 70)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 71)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 72)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 73)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 74)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 75)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 92)" 
//#define MAINVERSIONTFT  "Ver 6.4.1(build 98)" 
/*******************************************************************
 ** 提交日期：2011年8月30日
 ** 提交者：杨小龙
 ** 主要目的：
 **		1.BS考勤固件完善项目新增功能
 **
 ** 版本号：Ver 6.5.0(build 99)
 ** 
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.5.1(build 99)" 
/*******************************************************************
 ** 提交日期：2011年9月17日
 ** 提交者：杨小龙
 ** 主要目的：
 **1.从根本解决指针被free两次的问题，程序报错为
 *** glibc detected *** ./main: double free or corruption (!prev): 0xxxxx ***
 Aborted
 **2.解决当网络异常或者服务器超时时，异地考勤和后台比对反应慢的问题
 **版本号：Ver 6.5.2(build 100)
 ** 
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.5.2(build 100)" 
/*******************************************************************
 ** 提交日期：2011年9月17日
 ** 提交者：杨小龙
 ** 主要目的：
 **1.BS考勤完善项目提交生产固件版本
 **版本号：Ver 6.5.2(build 100)
 ** 
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.5.3(build 102)" 
/*******************************************************************
 ** 提交日期：2011年9月17日
 ** 提交者：杨小龙
 ** 主要目的：
 **1.优化U盘上传用户信息速度
 **
 **2.优化设备处理服务器下发大数据量的速度
 **版本号：Ver 6.5.3(build 103)
 ** 
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.5.3(build 103)" 
/*******************************************************************
 ** 提交日期：2011年10月28日
 ** 提交者：罗小文
 ** 主要目的：
 **1.增加新3G功能，使用libmodem.so库支持GPRS、CDMA、WCDMA、TDS-CDMA
 **版本号：Ver 6.5.4(build 104)
 ** 
 ****************************************/
//#define MAINVERSIONTFT  "Ver 6.5.4(build 104)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 105)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 106)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 107)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 108)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 109)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 110)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 111)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 112)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 113)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 114)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 115)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 116)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 139)"
//#define MAINVERSIONTFT  "Ver 6.5.4(build 140)"
#define MAINVERSIONTFT  "Ver 6.5.4(build 142-A0433-02)"

#define DeviceVender "ZKSoftware Inc."
#define AuthorName "Chen Shukai"
#define ProductTime (LoadStrOld("~ProductTime")?LoadStrOld("~ProductTime"):"2004-01-01 20:20:20")
#define SerialNumber (LoadStrOld("~SerialNumber")?LoadStrOld("~SerialNumber"):"0")
#define OEMVendor (LoadStrOld("~OEMVendor")?LoadStrOld("~OEMVendor"):DeviceVender)
#define DeviceName (LoadStrOld("~DeviceName")?LoadStrOld("~DeviceName"):"SSR")
#define AlgVer (LoadStrOld("~AlgVer")?LoadStrOld("~AlgVer"):"ZKF2004-5.04")

//dsl 2011.5.7
//#define FORFACTORY

enum __SHORT_KEY_{
	STK_UNDEFINED=0,
	STK_STATE,
	STK_WORKCODE,
	STK_EMS,
	STK_HELP,
	STK_UPDTHEME,
	STK_USERMGMT,
	STK_NEWUSER,
	STK_MANAGEUSER,
	STK_COMMMGMT,
	STK_NETWORK,
	STK_RS232,
	STK_SECURITY,
	STK_SYSTEM,
	STK_SYSTEMSET,
	STK_DATA,
	STK_UPDATE,
	STK_KEYBOARD,
	STK_DISPLAY,
	STK_POWERMGMT,
	STK_RESET,
	STK_BELL,
	STK_DATETIME,
	STK_PENDRIVE,
	STK_DWNRECORD,
	STK_DWNUSER,
	STK_UPLUSER,
	STK_UPLPICTURE,
	STK_DWNEMS,
	STK_UPLEMS,
	STK_UPLTHEME,
	STK_AUTOTEST,
	STK_RECORD,
	STK_SYSINFO,
	STK_TIMEZONE,
	STK_HOLIDAY,
	STK_GROUP,
	STK_UNLOCKGROUP,
	STK_ACCESS,
	STK_DURESS=39,

	STK_FUN,	//40
	STK_VERIFY,
	STK_GMATCH,
	STK_GROUP1,
	STK_GROUP2,
	STK_GROUP3,
	STK_GROUP4,
	STK_GROUP5,

};
//==========================================================

#define TTS_OPTION

typedef struct __OPTIONS__{
	int Language;					//Language of display messages
	BYTE MAC[6];					//Ethrenet MAC
	BYTE CardKey[6];
	BYTE IPAddress[4];				//Network IP Address
	int DeviceID;					//Device ID for 485 connection
	int MThreshold, EThreshold, VThreshold;		//Fingerprint Matching threshold, Fingerprint Enroll threshold
	unsigned int LastAttLog;			//Last AttLog index
	int UDPPort;					//UDP Port Number for communication
	int OImageWidth, OImageHeight, OTopLine, OLeftLine;  //Original fingerprint size and corner for capturing
	int CPX[4],CPY[4];				//correct distorted image parameters
	int ZF_WIDTH,ZF_HEIGHT;
	int MSpeed;					//fingerprint match speed 0-low, 1-high, 2-auto
	int AttState;
	int MaxUserCount;				//unit is 100
	int MaxAttLogCount;				//unit is 10000
	int MaxFingerCount;				//unit is 100
	int AlarmAttLog;
	int AlarmOpLog;
	int AlarmReRec;					//Recheck time
	int RS232BaudRate;				//0x23
	int RS232CRC;
	int RS232Stop;
	int WEBPort;
	int ShowState;
	int KeyLayout;
	int VoiceOn;
	int AutoPowerOff; 				//自动关机时间
	int AutoPowerOn;				//自动开机时间
	int AutoPowerSuspend;				//自动待机时间
	int AutoAlarm[MAX_AUTOALARM_CNT];		//自动响铃时间
	int IdlePower;					//空闲自动待机-1，自动关机-0
	int IdleMinute;					//空闲时间
	int ShowScore;					//show the verification score for fingerprint matching.
	int NetworkOn, RS232On, RS485On;		//是否用以太网络、RS232、RS485功能
	int NetworkFunOn, LockFunOn, RFCardFunOn; 	//是否打开网络/门禁（0-无，1-简单门禁，2-高级门禁）/射频卡功能
	int One2OneFunOn, PowerMngFunOn;	 	//是否打开1:1功能,电源管理功能
	int NewFPReader;
	int ShowName;
	int UnlockPerson;				//同时开锁的人数
	int ShowCheckIn;				//是否显示上下班状态
	int OnlyPINCard;				//仅验证号码卡
	int IsTestMachine;				//是否测试用机器
	int MustChoiceInOut;				//是否必须选择进出状态
	//能够自动测试	int CMOSSensorDevAddr;		//CMOS Sensor Chip IIC Device Address: 0x42-OV7620, 0x22-Hy7131
	int HiSpeedNet;					//100M Ethernet
	int MenuStyle;					//菜单风格
	int CanChangeCardKey;				//是否允许改变卡密码
	int Must1To1;					//是否只允许一对一比对
	int LCDModify;
	int ComKey;					//连接密码
	int MustEnroll;					//必须是注册用户,比对后才能有效。用于别的机器上登记的指纹卡可以不在本机上注册即可使用
	int TimeOutMenu;				//菜单的超时时间
	int TimeOutPin;					//输考勤号码的超时时间
	int TimeOutState;				//考勤状态的超时时间
	int SaveAttLog;					//是否保存验证记录
	int RS232Fun;					//RS232接口功能：0-无；1-完整API通讯；2-简单ID输出
	int IsModule;					//是否模块
	int ShowSecond;					//是否显示秒
	int RFCardSecStart;				//Mifare Card 起始存放指纹数据的扇区
	int RFCardSecLen;				//Mifare Card 存放指纹数据的扇区数
	int RFCardFPC;					//Mifare Card 存放指纹的个数
	int PIN2Width;					//PIN2码的字符宽度 <=5表示不支持PIN2码  2147483647=0x7FFFFFF
	int DateFormat;					//Date Format
	int OPLogMask1;					//
	int OPLogMask2;					//
	int AutoState[4*4];				//自动状态转换的时间
	int DelayCount, IncThr, TopThr, MinThr;		//指纹检测的阀值参数
	int MaxNoiseThr, MinMinutiae, MaxTempLen;	//最大容许指纹图像噪音阀值,
	int SaveBitmap;					//Save bmp format finger image or not
	BYTE GATEIPAddress[4];				//Gateway IP Address
	int TCPPort;					//TCP port for communication
	int RISServerTCPPort;					// net TCP port
	BYTE NetMask[4];				//Network mask address
	BYTE AuthServerIP[4];				//Identification Server
	int AuthServerEnabled;				//Identification Server enabled 0-only local 1-network->local 2-only network 3-local->network
	int IsOnlyRFMachine;				//Only RF machine not fingerprint
	int IsOnlyOneSensor;				//Only One Sensor
	int OS;						//Current OS 0=NONE 1=LINUX
	int IsWiegandKeyPad;				//use wiegand keypad or not
	int AutoStateFunOn;				//自动状态转换功能
	int EnrollCount;  	                     	//register finger count
	int IsSupportSMS;				//SMS Enable or Disable
	int IsSupportUSBDisk;				//Support USB DISK
	int AdminCnt;                               	//同时验证管理员的个数
	int AutoAlarmDelay;				//Auto Alarm delay time(second)
	int MultiLanguage;				//support select language
	int LockPowerButton;				//Lock Power Button
	int ModemEnable;
	int IsSupportModem;				//support modem connection
	int ModemModule;					// 1 - Q24PL  2 - EM560  3 - EM660  4 - EM770/W
	int IsSupportAuthServer;			//support AuthServer
	int AuthServerTimeOut;				//TimeOut for AuthServer
	int IsSupportMF;				//support mifare
	int AutoAlarmTimes;				//Auto Alarm Times
	int IsACWiegand;				//whether output access control wiegand 26bit
	int DNSCheckInterval;				//Refresh AuthServer List interval times unit:minute
	int AutoUploadAttlog;				//Automate upload Attlog seconds
	int DisableNormalUser;				//Disable normal user
	int KeyPadBeep;					//press key to play beep voice.
	int WorkCode;					//support work code
	int MaxUserFingerCount;				//default=10
	int AttLogExtendFormat;				//Extend attendance logs storage format
	int CompressAttlog;				//whether compress attendance logs or not(only valid for old AttLog format)
	int ASDewarpedImage;				//whether dewarp the image for auth server
	int ProcessImage;				//processing image with light check
	int IsFlashLed;					//whether flash green led or not
	int DisableRootPassword;			//disabled Setup root password by CommKey
	int IsSupportMP3;				//support MP3
	int MifareAsIDCard;				//Mifare Card as ID Card
	int PlayGroupVoice;				//when verified play voice by its group
	int PlayTZVoice;				//when verified play voice by time zone
	BYTE WebServerIP[4];				//Network IP Address
	int WebServerPort;				//Network Port
	int AudioVol;					//Audio volumn
	int AutoAlarmAudioVol;				//Auto alarm audio volume
	int DHCP;					//DHCP enable or disable
	int AutoSyncTimeFunOn;				//Synchronize time from remote time server
	int AutoSyncTime;				//Schedule time for synchronize
	BYTE TimeServerIP[4];				//Time Server IP Address
	int CameraFunOn;				//take a photo from camera
	int StartUpNotify;                              //起动广播，BIT0-BT232, BIT1-NETWORK
	int AdvanceMatch;				//support 1:G 1:H
	int I1ToNFrom;                                  //1：N From (minimum user PIN for identification)
	int I1ToNTo;                                    //1：N To (maximum user PIN for identification)
	int I1ToH;                                    	//1：H
	int I1ToG; 					//1：G
	int TwoLineLCM;					//TWO LINE SMALL LCM
	int ErrTimes;					//Error times for pressing finger
	int UserExtendFormat;				//extend user data information
	U32 CMOSGC; 					//0-AUTO; 1-255 for RGB
	int RefreshUserData;				//Auto refresh user data from authserver
	int DisableAdminUser;				//Disable administrator verification
	int IClockFunOn;				//iClock functions
	BYTE ProxyServerIP[4];				//Proxy server ip address
	int ProxyServerPort;				//Proxy server port
	int IsSupportC2;				//support C2 controller or not
	int EnableProxyServer;				//Enable ProxyServer
	int WorkCodeFunOn;
	int MustChoiceWorkCode;				//验证时是否必须选择WorkCode(liming)
	int ViewAttlogFunOn;                            //View attlog funtion
	int IsInit;
	int Saved;
	int SSRFunOn;					//whether have ssr funtion
	int HzImeOn;					//t9 hz ime
	int PinPreAdd0;					//号码位数不足是否显示0
	int SetGatewayWaitCount;                        //Wait seconds to retry setup gateway
	int IsSupportHID;                               //support iClass
	int FPRetry;					//1:1指纹验证重试次数
	int AdmRetry;					//管理员验证重试次数
	int PwdRetry;					//密码验证重试次数
	int InterStyle;					//界面风格
	int LogoTime;					//图片循环间隔
	int ClockTime;					//时钟显示延时
	int SMSTime;
	//liming
	int ShortKeyCount;				//快捷键数量
	int AlarmMaxCount;
	int AlarmPlayTimes;
	int AutoAlarmFunOn;				//Auto alarm function enabled or disable
	int SMSCount;
	int TFTKeyLayout;				//0:iclock200/300,1:hit-1-2,2:iclock400/500
	int MustCheckWorkCode;				//输入的WORKCODE是否必须在表中
	int USB232On;					//
	int USB232FunOn;				//
	int IMEFunOn;					//1:开启输入法，0:关闭输入法
	int Brightness;					//屏幕亮度
	int LcdMode;					//显示屏模式 1:支持背光调节(iF4),0:不支持背光调节
	int OpenDoorDelay;                              //门磁延时
	int DuressHelpKeyOn;                            //“~K”键求助 是/否
	int Duress1To1;                                 //1：1方式报警 是/否
	int Duress1ToN;                                 //1：N方式报警 是/否
	int DuressPwd;                                  //Password方式产生报警
	int DuressAlarmDelay;                           //自动报警时间 0～255秒
	int DoorSensorMode;                             //door sensor 门磁开关方式 0-NO 1-NC 其他表示不检测门磁
	int LockOn;                                     //设置锁控时长
	int DoorSensorTimeout;                          //门磁报警延时Alarm signal will be raise when door sensor open
	int DoorCloseTimeZone;				//常闭时间段
	int DoorOpenTimeZone;				//常开时间段
	int IsHolidayValid;				//节假日是否有效
	int ExtWGInFunOn;
	int WiegandID;
	int iCLASSAsIDCard;				//ICLASS卡作为普通ID卡
	int IsSupportiCLSRW;				//是否支持iclass卡
	int iCLASSCardType;				//iclass卡类型
	int AntiPassbackFunOn;                          //0支持反潜,1不支持反潜
	int AntiPassbackOn;                             //0 不反潜;1、出反潜，必须有入才能出；2、入反潜，必须有出才能入；
	int MasterState; //主从机通讯中主机的状态,若主机为0,则从机为1,若主机为1则从机为0,若为-1则主从都为主机当前的状态gOptions.AttState。
	int IsSupportWIFI;      //ccc
	int wifidhcpfunon;      //ccc
	int PrinterFunOn;                       //是否有打印机功能
	int PrinterOn;                          //打印机功能：0：无；1：输出到串口；2：ESC、Star打印
	int AuthServerREQ;      		//检查服务器是否联机时间间隔
	//        int ModemKeepLive;      		//是否保持连线状态 0=挂断 1=保持
	//	int GPRSFrequency;			//GPRS频率
	int AuthServerCheckMode;        	//服务器检查类型,0:TCP/UDP通信校验，1:GSM通信传输校验，2:pushsdk通信方式
	int AuthServerIPType;			//服务器地址类型
	int RedialTimeInterval; 		//重拨时间间隔
	int RedialCount;                	//重拨次数
	//        int isgprstest;
	int ReconnectInterval;
	int ShowPhoto;				//是否显示用户照片
	int IsSupportPhoto;			//支持照片显示功能
	int WiegandOutType;			//Wiegand输出类型：0=PIN；1=CardNumber
	int WiegandInType;			//Wiegand输入类型：0=PIN; 1=CardNumber
	int isSupportAlarmExt;			//是否开启外部闹铃
	int CameraOpen;				//是否支持Camera
	int CameraBright;                         //摄像头亮度
	int CameraContrast;                       //摄像头对比度
	int PicQuality;                           //摄像头抓图质量
	int CameraScene;                         //摄像头环境设置（室内，室外)
	int CapturePic;                         //抓拍图像
	int CapturevfTimes;                     //抓拍验证不通过次数
	int CaptureAlarm;               	//拍照空间警告
	BYTE pingtestip[4];
	int IMESwitchKey;			//输入法开关键
	int SwitchAttStateByTimeZone;		//
	int NotShowRecordInfo;			//不显示记录信息
	int DaylightSavingTimeFun;                       //是否显示支持夏令时制,
	int DaylightSavingTimeOn;                      //是否支持夏令时制
	int CurTimeMode;			//1当前为夏令时，2当前不是夏令时
	int DaylightSavingTime;			//进入夏令时的时间
	int StandardTime;			//进入非夏令时的时间
	int FpSelect;				//是否显示指纹图像
	int AlwaysShowState;			//永远显示状态图
	int AttUseTZ;				//考勤用时间段限制打卡
	int MachineGroup;			//机器组(IDT Limited定制)
	int HasFPThreshold;
	int NoFPThreshold;
	int UseNewBell;				//是否使用新闹铃模式
	int HavePowerKey;			//是否有电源键
	int LockPowerKey;			//是否锁定关机键
	int LimitFpCount;			//系统指纹最大数,当指纹数量超出此数时只能1:1比对(<MaxFingerCount)
	int ExAlarmDelay;			//外部响铃持续时间(秒)
	int DelRecord;				//Del Record Count if full
	int ApiPort;				//HTTP_SDK DataApi Service Port
	int RSize;
	int ShowStyle;	//add by jazzy 2008.07.28 for arabic right to left show
	//zsliu add
	int isUseHejiraCalendar;//user Hejira Calendar's flag 判断是否使用伊朗日历
	int IR_SensorOn;                        // 是否支持IR Sensor
	int IRSS_BLSwitch;			//背光开关
	int IRSS_BLOffR;                                //背光暗值
	int IRSS_BLOnR;                                 //背光亮值
	int IRSS_BLDlyT;                                //背光延时时间（自动背光打开时）
	int IRSS_AutoCon;                               //自动控制 [7:5] 检测间隔 [4:3] 检测次数 [2] 自动采样[1] 自动控制背光 [0] 上电状态
	int IRSS_Range;                                 // 判断幅度（自动控制采样）
	int BatteryInfo;	//add cn 2009-03-04
	/*
	 * ScreenAddress=2 表示确定不需要翻转。
	 * ScreenAddress=0或者1表示翻转的情况，如果出现倒屏，则需要赋值为1或者0进行翻转，
	 * 如果翻转不成功的话，则更换参数0和1进行操作处理。
	 * 注意：翻转后，需要重新启动机器两次后方可生效。
	 */
	int ScreenAddress;	// add cn 2009-03-04
	int isRotateCamera;   	//add by cn
	int IsSupportExtBell;     //add by lyy
	int DetectFpAlg;
	int ZKFPVersion;
	int FingerSensitivity;	//0,1,2->low,middle,high
	int FPRotation;

	//欧版考勤机的参数。判断是否是欧版考勤机
	int EuropeDevice;//add by mjh
	int RotateDev;//for iclcok990 camera


	//zsliu add for 3寸和3.5寸屏的合并工作
	/*
	 * ～MulAlgVer=0	//仅支持单一指纹算法
	 * ～MulAlgVer=1	//支持多种指纹算法，目前支持9.0和10.0算法自由切换
	 * 如果仅仅需要支持9.0的固件，配置如下：ZKFPVersion=9，并且～MulAlgVer=0
	 * 如果仅仅需要支持10.0的固件，配置如下：ZKFPVersion=10，并且～MulAlgVer=0
	 * 如果需要支持10.0/9.0之间可以切换的固件，则配置如下：～MulAlgVer=1
	 */
	int MulAlgVer;
	int FP10DBVersion;
	/*
	 * supportLCDType=0  //支持3寸彩屏固件
	 * supportLCDType=1  //支持3.5寸彩屏固件
	 * supportLCDType=2  //等方便扩展处理
	 */
	int supportLCDType;
	//菜单界面的偏移量,不需要保存，全局临时变量
	int MenuOffset;
	//控制界面的偏移量,不需要保存，全局临时变量
	int ControlOffset;
	//屏幕宽度,不需要保存，全局临时变量
	int LCDWidth;
	//屏幕高度,不需要保存，全局临时变量
	int LCDHeight;
	//这个是在主界面显示的偏移量,不需要保存，全局临时变量
	int MainVerifyOffset;
	//主要针对指纹在模板框中显示的Y位置的偏移量，默认是10
	int sensorOffsetY;
	//列表框的间距
	int GridWidth;
	//键盘类型
	int keyboardStyle;

	// GPRS  的一些参数
	int EchoFailRestart;
	int EchoInterval;
	int EchoAction;
	int EchoType;
	int WGPassThrough;

	//ADMS
	int IclockSvrFun;       	//charge open ADMS or not; 1 is open, 0 is close.
	int iClockServerStatus;
	int iClockErrorStatus;


	//写屏显示的delay时长，以秒为单位
	int WriteLCDDelay;              //add by zhc 2008.11.25 

	int ttsIntegralPointOpen;	//TTS integral point give the correct time

	//拍照机器，能保存考勤照片的最大值
	int maxCaptureCount;		//max capture picture, default 8000

	TTS_OPTION int TTS_S;
	TTS_OPTION int TTS_VERIFY;
	TTS_OPTION int TTS_SMS;
	TTS_OPTION int TTS_LOGO;
	TTS_OPTION int TTS_ENROLL;
	TTS_OPTION int TTS_REC;
	TTS_OPTION int TTS_KEY;
	TTS_OPTION int TTS_STATE;
	TTS_OPTION int TTS_TIME;
	TTS_OPTION int TTS_MENU;

	int FreeTime;		//free time
	//这个参数用于连接断开的时候，是否需要关闭beep声
	int EnableCommEndBeep;	//enable commu end beep
	//这是视频显示是否旋转的参数，主要是为了在登记用户的时候，
	//不支持此功能，需要屏蔽掉,但是其它设置界面需要此功能，默认为1；
	int enableVideoRotate;
	/*
	 * 键盘类型配置主要针对彩屏中比较特殊的键盘而设置的，在固件中做键盘映射功能：
	 * KeyType=1 代表B3键盘类型的机器，这些是定制
	 * KeyType=2 代表ICS01键盘类型的机器，这些是定制
	 * 
	 * 
	 */
	int KeyType;

	//add by caona for face
	int FaceFunOn;
	int DefFaceGroup;
	int FaceVThreshold;
	int FaceMThreshold;
	int MaxFaceCount;
	int FaceRegMode;
	int FaceExposoure;
	int FaceExposRang;
	int VideoGain;
	int VideQuality;
	//int UploadPhoto;	//seiya dlphoto end
	int IsUploadPhotoOnly;  //when open ADMS, if 1, All functions of ADMS except upload photo can be used; if 0, all can be used.
	BYTE DNS[4];				//DNS
	int IsSupportDNS;
	int RS232FunOn;
	int RS485FunOn;
	BYTE IclockSvrURL[128];
	int WebServerURLModel;
	int IsSupportFlowCtrl;
	BYTE OptionsPassword[64];
	int IsRotatePhoto;
	/*异地考勤,add by yangxiaolong,2011-06-14,start*/
	int RemoteAttFunOn;		//异地考勤开关，1:open，0:close.默认为开
	int RmUserSaveTime;		//异地考勤用户保存时间，以天为单位。默认为5天
	/*异地考勤,add by yangxiaolong,2011-06-14,end*/
	int USBCheckTime; //add by lihm	 几秒进行一次U盘检测
	int USBFileCount; //add by lihm 针对ZEM510 /proc/bus/usb/001 在没有插U盘时有几个目录
	int USBCheckOn; //add by lihm  U盘检测功能开关
	/*增加pushsdk对高级门禁的支持，add by yangxiaolong,2011-9-26,start*/
	int PushLockFunOn;	// 1,支持；0不支持
	/*增加pushsdk对高级门禁的支持，add by yangxiaolong,2011-9-26,end*/
	int DownloadNewLog;
	/*support RS485 reader*/
	int RS485Port;
	int InBIOComType;
	int Reader485FunOn;
	int Reader485On;
	/*end rs485*/
	int ZEM800Platform;
	int VideoWidth;
	int VideoHeight;
	int WifiModule;
	int LoopWriteRecordOn; 		/*When the record capacity is full, Overwrite the old record*/
	int BitsInvertCardModule;
	int CardNumberParity;
	/**************************************************
	 *Number printed on the card . 0010930095  166.51119
	 *Number Reading by want : 16651119
	 *parameter explain:
	 *cardNum int : the card number is 0010930095
	 *readCardStyle int : the style for read cardnum;
	 *explain:
	 *IDCardStyle=0 read card num is 0010930095
	 *IDCardStyle=1 read card num is 16651119
	 *IDCardStyle=2 read card num is 51119
	 ***************************************************/
	int IDCardStyle;
	int StartPosition;		// The start position of the card number,1 means staring from the first digit in a selected
	int CardNumberLen;		// The length of the card number.
	int DelPictureCnt;		//When picture capacity is full, auto delete xxx the old picture
	int IsWGSRBFunOn;		//support srb fun
	int SRBOn;				//srb on
	int SRBType;			//srb type
	int FingerMaxRotation;	//支持指纹旋转的最大度数,范围是0～180，不旋转则默认值30，支持360度旋转则设置值为180.
	int RegistOpenFlag;
	int MFCardError;
	int ReadCardInterval;
}TOptions, *POptions;

#define optoffset(field) offsetof(TOptions, field)

POptions LoadOptions(POptions opts);
POptions SaveOptions(POptions opts);
POptions GetDefaultOptions(POptions opts);

typedef int (*StrConvertFun)(char *str, BYTE *value);
typedef char *(*StrFormatFun)(char *str, BYTE *value);
//new options
typedef struct __OPTIONSRESSTR__{
	char *OptionName;
	int OptionLong;
	char DefaultValue[256];
	int IsNeedRestoreFactory;
	int Offset;
	StrConvertFun Convertor;
	StrFormatFun Formator;
	int IsEncrypt;
}TOptionsResStr,*POptionsResStr;
typedef struct __OPTIONSRESINT__{
	char *OptionName;
	int DefaultValue;
	int IsNeedRestoreFactory;
	int Offset;
	int MenuResID;
	int MaxValue;
	int MinValue;
	int IsEncrypt;
}TOptionsResInt,*POptionsResInt;

#define OPTIONSRESSTRCOUNT		(sizeof(OptionsResStr)/sizeof(OptionsResStr[0]))
extern TOptionsResStr OptionsResStr[];
#define OPTIONSRESINTCOUNT		(sizeof(OptionsResInt)/sizeof(OptionsResInt[0]))
extern TOptionsResInt OptionsResInt[];
POptionsResInt QueryOptResByOffset(int Offset);

BOOL SaveStr(const char *name, const char *value, int SaveTrue);
BOOL RemoteSaveStr(const char *name, const char *value, int SaveTrue);
BOOL LoadStr(const char *name, char *value);
BOOL LoadCaseStr(const char *name, char *value);
int LoadStrFromFile(int fd, const char *name, char *value, BOOL ExitSign, int offset);
char *LoadStrOld(const char *name);
char* LoadStrByID(int ID);
//char* LoadStrByIDDef(int ID, const char *defStr);
char* LoadStrByIDDef(int ID, char *defStr);
char* LoadStrByIDPad(int ID, int Len);
char* LoadStringLng(const char *Name, int LngID);
int LoadInteger(const char *Name, int DefaultValue);
int SaveInteger(const char *Name, int Value);
//从内存串列表中删除指定字符串
U32 PackStrBuffer(char *Buffer, const char *name, int size);
char* GetYesNoName(int Yes);
int InitOptions(void);

extern TOptions gOptions;

#define IsNetSwitch "IsNetSwitch"
#define PLATFORM "~Platform"
#define LIMITWIDTH 5
#define OldLog 1
#define OldExlog 2
#define PWDLIMITLEN 8
#define PINLIMITLEN 23
#define VERIFYONETOONE 1
#define VERIFYPWD 2

int FormatDate(char *buf, int index, int y, int m, int d);
int FormatDate2(char *buf, int index, int m, int d);
extern char *DateFormats[];

int GetFileLength(int fd);
void TruncOptionAndSaveAs(char *buffer, int size);

void SelectLanguage(char Language);
int GetDefaultLocaleID(void);
char *GetLangName(char LngID);
int GetSupportedLang(int *LngID, int MaxLngCnt);

char *GetSMS(int UserID);

BOOL ReadOneLine(int fd, char *dest, int *size);
void SplitByChar(char *buffer, char *name, char * value, char DeliChar);

int ClearAllACOpt(int All);
int ClearOptionItem(char *name);

BOOL UpdateNetworkInfoByDHCP(char *dhcp);

#ifndef URU
int ReadSensorOptions(POptions opts);
int WriteSensorOptions(POptions opts, int Rewrite);
int EEPROMWriteOpt(BYTE * data, int size, int Rewrite);
#endif

char *GetCardKeyStr(char *Buffer, BYTE *Key);
//2007.07.23
int tftnewlng;
int tftlocaleid;	//add by jazzy 2008.07.24 for mul language

int issimcardright; //add by lyy for check sim card 2009.06.12

//add by jazzy 2009.05.06
#define DELALLPASSPIC "rm -rf %scapture/pass && sync && mkdir %scapture/pass && sync"
#define DELALLBADPIC  "rm -rf %scapture/bad && sync &&  mkdir %scapture/bad && sync"
#define DELALL  "rm -rf %s && sync &&  mkdir %s && sync"

#define MAX_CHAR_WIDTH 100

int initLCDTypeOptions();
//if no language will load this defaultlanguage, name "DefaultLanguage"
char* LoadDefaultLanguageByID(int LanguageID);
void ExecuteActionForOption(const char *name, const char *value);

char * ipformat(char *str, BYTE *value);

#define LOG_VALID               0
#define LOG_REPEAT              1
#define LOG_INVALIDTIME         2
#define LOG_INVALIDCOMBO        4
#define LOG_INVALIDUSER         8
#define LOG_ANTIPASSBACK        512             //反潜

#endif

