#ifndef _LANGRES_H__
#define _LANFRES_H__

/*
L/_XXX_=RRR

        L为多国语言资源标识符，用于定义一种语言字符串资源。不同的语言使用不同的标识符
作为一个字符串的引导字符。其范围是：0x21~0x7E 中，除去["],[/],[=]三个字符以外的92
个字符；XXX 是字符串编号；RRR 是对应的字符串。

下面是已经分配的标识符
*/

#define LanguageTraditionalChinese 	'T'  //BIG5内码
#define LanguageSimplifiedChinese 	'S'
#define LanguageEnglish			'E'
#define LanguageThailand		'L'
#define LanguageIndonesian 		'I'
#define LanguageJapanese 		'J'
#define LanguageKorean 			'K'
#define LanguageMalaysia 		'M'
#define LanguageVietnamese 		'V'
#define LanguageTurkish			't'
#define LanguageSpa                     'a'
#define LanguageSpaHit                  'b'
#define LanguageArab                    'B'
#define LanguageHebRew                  'H'
#define LanguagePers                    'A'
#define LanguageFra                     'F'
#define LanguageCze                     'Z'


//字符串编号表
#define MSG_LANG_ID 		0       //该资源的语言内码编号，参见"locale.h"
#define MSG_LANG_NAME           1       //该资源的语言名称，<=4 BYTE
#define MSG_LANG_NAME_ENG       2       //该资源的语言名称的英文表示，<=4 BYTE
//#define MSG_LANG_REPLACE        3       //该语言的首要替换语言。
#define MID_OS_LANGUAGE 5		//"语言\t%s"
#define HID_RESTART				6        	//重新启动才能生效

//LCD Hint messages
//#define HINT_START	10
//#define HID_SYSTEM	10+1)		//"指纹考勤系统"
#define HID_CANCEL      11          //"取消"    //modify by jazzy 2008.12.02 for 479--确定 11－取消  63－是 64－否
#define HID_PLACEFINGER	12		//"请按手指"
//#define HID_HWID		13		//"HardWare ID"
//#define HID_LICENSE		14		//"Please input license"
//#define HID_LICENSEFAIL	15		//"license check failed"
#define HID_QUALITY		16		//"指纹质量"
//#define HID_DELALLDATA	17	
//#define HID_NOLICENSE   18      //"no license for ZKFinger10.0"
#define HID_PIN2		19		//"号码"
//#define HID_PLACECNT	10+10)		//"第 %d 次"
//#define HID_FINGEREXISTS	10+11)
//#define HID_ENROLLFAIL	10+12)
//#define HID_ENROLLOK	10+13)
//#define HID_NOSPACE	10+14)
//#define HID_UIDERROR	10+15)		//"错误！"
//#define HID_YESNO	10+16)		//"否ESC   是OK"
//#define HID_SAVECANCEL	10+17)		//"无效ESC  保存OK"
//#define HID_OKCANCEL	10+18)		//退出 ESC  设置OK
#define HID_WELCOME	29		//"欢迎"
#define HID_DAY0	30		//"星期日"
#define HID_DAY1	31		//"星期一"
#define HID_DAY2	32		//"星期二"
#define HID_DAY3	33		//"星期三"
#define HID_DAY4	34		//"星期四"
#define HID_DAY5	35		//"星期五"
#define HID_DAY6	36		//"星期六"
#define HID_VADMIN	37		//"管理者确认"
//#define HID_ENROLLNEWQ	10+28)		//"新登记？"
//#define HID_ENROLLNEW	10+29)		//"新登记"
#define HID_ENROLLNUM		40		//"登记号码"
//#define HID_LEAVEFINGER	41		//"请离开手指"
#define HID_PLACEFINGER2	42	//"请第二次按手指"
#define HID_PLACEFINGER3	43	//"请第三次按手指"
//#define HID_FINGERIDFMT	44		//"%05d-%d"
//#define HID_CONTINUE	10+35)		//"继续？"
//#define HID_UIDFMT	10+36)		//"%05d"
#define HID_ENROLLBACKUP	47	//"备份登记"
//#define HID_USERPWDFMT	10+38		//"%05d-P"
#define HID_INPUTPWD		49		//"输入密码"
#define HID_VERINPUTPWD		50		//"密码确认"
//#define HID_ADMINPRIV		51		//"管理者授权"
#define HID_VERDELETE		52		//"删除？"
#define HID_NOTENROLLED		53		//"无登记数据!"
#define HID_NOATTLOG		54		//"无考勤记录"
//#define HID_HINTTIME		55		//"设置ESC  上一个OK"
#define HID_WAITING			56		//"工作中，请稍等"
#define HID_ERRORPIN		57		//"登记号码出错"
#define HID_REFINGER		58		//"指纹重复"
#define HID_INPUTAGAIN		59		//"请重新输入！"
//#define HID_LNGSCH			10+50)		//"简体"
//#define HID_LNGTCH			10+51)		//"繁体"
//#define HID_LNGENG			10+52)		//"ENG"
#define HID_YES				63		//"是"
#define HID_NO				64		//"否"
#define HID_NOTHING			65		//"无"
//#define HID_THANK			66		//"谢谢！"
#define HID_ALREADY			67		//"已签到，谢谢！"
//#define HID_AGAIN			68		//"请重新输入！"
#define HID_EXCEEDFINGER	69	//"已登记10枚指纹"
#define HID_SCIN			70		//"上班签到
#define HID_SCOUT			71		//"下班签退"
#define HID_SOUT			72		//"外出"
#define HID_SBACK			73		//"外出返回"
#define HID_SOCIN			74		//"加班签到"
#define HID_SOCOUT			75		//"加班签退"
//#define HID_SOPENLOCK		76		//"开门"
#define HID_SVERIFY			77		//"验证"
//#define HID_VF				78			//"指纹确认"
#define HID_VFFAIL			79		//"请重新按手指！"
//#define HID_VP			80			//"密码确认"
#define HID_VPFAIL			81		//"密码错误！"
//#define HID_VREP			82		//"已确认！"
//#define HID_CONTINUEESC	83		//"退出ESC 继续OK"
//#define HID_SAVEQ			84		//"保存？"
#define HID_VSUCCESS		85		//"确认成功！"
#define HID_ACCESSDENY		86		//"非法管理！"
//#define HID_POWEROFF		87		//"关机\t%s"
#define HID_SUSPEND			88		//"待机\t%s"
//#define HID_ESC			10+79)			//"退出ESC"
//#define HID_TSTLCD		10+80)		//当前LCD
//#define HID_TSTLCD_LF		10+81)		//左屏充满
//#define HID_TSTLCD_RF		10+82)		//右屏充满
//#define HID_TSTLCD_UF		10+83)		//上屏充满
//#define HID_TSTLCD_DF		10+84)		//下屏充满
//#define HID_TSTLCD_LE		10+85)		//左屏空
//#define HID_TSTLCD_RE		10+86)		//右屏空
//#define HID_TSTLCD_UE		10+87)		//上屏空
//#define HID_TSTLCD_DE		10+88)		//下屏空
//#define HID_CONTINUEOK	10+89)		//继续OK

//#define HID_KEYNAME_MENU	10+90)	//MENU
//#define HID_KEYNAME_OK	10+91)		//OK
//#define HID_KEYNAME_ECS	10+92)		//ESC
//#define HID_KEYNAME_UP	10+93)		//上箭头
//#define HID_KEYNAME_DOWN	10+94)	//下箭头
//#define HID_KEYNAME_LEFT	10+95)	//左箭头
//#define HID_KEYNAME_RIGHT	10+96)	//右箭头
//#define HID_KEYNAME_POWER	10+95)	//按下电源
//#define HID_KEYNAME_ANYKEY	10+99)	//按任意键显示
//#define HID_KEYNAME_NUMBER	10+100)//数字
//#define HID_PLAYVOICEINDEX  10+101)	//播放第 %d 段声音
//#define HID_TEST_FLASH_CQ1   10+102)	//测试FLASH时请
//#define HID_TEST_FLASH_CQ2   10+103)	//不要按电源按钮
//#define HID_TESTING   10+104)		//正在测试……
//#define HID_TEST_FLASH_RES   10+105)	//总数:%d坏块:%d
//#define HID_FINISH   10+106)		//完成！
//#define HID_TEST_OK    10+107)		//正常！
//#define HID_TEST_BAD    10+108)	//不能正常工作！
#define HID_PREPOWEROFF		119	//准备关机……
#define HID_PRI_NONE		120		//普通用户
//#define HID_PRI_ENROLL    	121	//登记员
#define HID_PRI_ADMIN		122	//管理员
#define HID_PRI_SUPERVISOR	123	//超级管理员
#define HID_PRI_INVALID		124	//禁止用户
//#define HID_PRI   10+115)	//权限
#define HID_1TO1			126		//1:1 Verifying
#define HID_EXCEED			127	//Exceed space
#define HID_LEFTSPACE		128		//left space %d
//#define HID_OKPWD 10+119) 		//OK for pwd
//#define HID_OTAPOWER 10+120) 		//设置定时功能吗？
#define HID_DEL_FP			131		//Delete Fingerprint
#define HID_DEL_PWD			132 		//Delete Password
//#define HID_DEL_USR			133 		//Delete User
#define HID_WORKING			134	//工作中....
//#define HID_CARDFP			135	//比对卡上指纹
#define HID_INVALIDCARD		136	//无效卡
//#define HID_CANCELKEY		137	//任意键取消
#define HID_SHOWCARD		138	//请出示卡
#define HID_WRITE_OK		139	//写卡成功
#define HID_WRITE_ERROR		140	//写卡失败
//#define HID_NOFINGER		141	//未登记指纹
#define HID_FLASHERROR		142	//存储器错误
#define HID_CARD			143	//卡验证
#define HID_MUSTINOUT		144	//必须选择进出状态
//#define HID_TZDEF			10+135	//时间段%d定义
//#define HID_GTZ			10+136		//组%d默认时间段
//#define HID_ULG			10+137		//组合%d
//#define HID_TZI			10+138		//时间段%d
//#define HID_UAOPT	10+139)	//用户%05d门禁
//#define HID_SHORTWEEK	10+140)	//星期的短名称
//#define HID_TZNUM	10+141)	//时间段编号
//#define HID_GRPNUM	10+142)	//组编号acc
#define HID_INVALIDTIME		153	//非法时段访问
#define HID_OS_MUST1TO1		154	//必须输入ID
//#define HID_NEWENROLLNUM 10+145)	//新号码
#define HID_CARD_NOTENROLLED 156	//卡未登记
//#define HID_CARD_ENROLLED 10+147)	//卡已登记
//#define HID_CARD_NO	10+148)	//卡号:
//#define HID_AUTO_STATE	10+149)
//#define HID_NUM	10+150)
//#define HID_STATE	10+151)
//#define HID_LNGTHAI	10+152)	//泰语
//#define HID_SHUTDOWNING 10+153)        //%s 秒后关机
//#define HID_SUSPENDING 10+154) 	//%s 秒后待机
#define HID_ALARM_STRIPE		165       //机器被拆除
#define HID_ALARM_DOOR			166        	//门被意外打开
//#define HID_ALARM_LOWBATTERY 10+157) 	//电池电压过低
//#define HID_ALARM_INDOOROPEN 10+158)   //出门开关开门
//#define HID_DSM 10+159)  		//门磁开关模式：常开;常闭;无
#define HID_DOOR_OPEN			170  	//门已打开
#define HID_DOOR_CLOSE			171 	//门已关闭
#define HID_MUSER_OPEN1			172        //多用户进入
#define HID_MUSER_OPEN2			173        //非法分组组合
//#define HID_UNREGALL 10+165)   	//全部清除吗
//#define HID_LNGINDON 10+166)   	//印尼语
//#define HID_TSTLCD_1 10+167) 	//LCD测试第一屏
//#define HID_TSTLCD_2 10+168) 	//LCD测试第一屏
//#define HID_1TOG     10+169) 	//1:G比对

//#define MENU_START 200
//#define MID_MENU 200+1)	//"菜单"
//#define MID_DATA 200+2)	//"数据管理"
//#define MID_HIDE 200+3)	
//#define MID_OPTIONS 200+4)	//"设置"
//#define MID_SYSINFO 200+5)	//"系统信息"
//#define MID_DATA_EUSER 200+6)	//"用户登记"
//#define MID_DATA_EADMIN 200+7)	//"管理者登记"
//#define MID_DATA_DEL 200+8)		//"删除登记数据"
#define MID_DATA_EU_FINGER		209	//"指纹登记"
#define MID_DATA_EU_PWD			210		//"密码登记"
//#define MID_DATA_EU_FP 200+11)		//"指纹及密码"
//#define MID_OPTIONS_SYSTEM 200+12)	//"系统设置"
//#define MID_OPTIONS_REC 200+13)		//"记录设置"
//#define MID_OPTIONS_COMM 200+14)	//"通讯设置"
//#define MID_OS_ADMINNUMBER 200+15)	//"管理者总数\t%d"
//#define MID_OS_DEVNUMBER		216	//"机号\t%d"
//#define MID_OS_TIME 200+17)		//"时间设置"
#define MID_OS_LOCK			219		//"锁驱动\t%s"
//#define MID_OS_AUTOPOWER 200+20)	//"自动电源管理\t%s"
//#define MID_OR_AADMINLOG 200+21)	//"管理记录警告\t%d"
//#define MID_OR_AATTLOG 200+22)		//"考勤记录警告\t%d"
//#define MID_OR_REREC 200+23)		//"重复确认时间\t%d"
//#define MID_OC_BAUDRATE 200+24)		//"波特率\t%d"
//#define MID_OC_CRC 200+25)		//"奇偶校验\t%s"
//#define MID_OC_STOP 200+26)		//"停止位\t%d"
//#define MID_SYSINFO_USER 200+27)	//"用户登记\t%d"
//#define MID_SYSINFO_FINGER 200+28)	//"指纹登记\t%d"
//#define MID_SYSINFO_ATTLOG 200+29)	//"考勤记录\t%d"
//#define MID_SYSINFO_ADMIN 200+30)	//"管理者登记\t%d"
//#define MID_SYSINFO_PWD 200+31)		//"密码登记\t%d"
//#define MID_SYSINFO_ADMINLOG 200+32)	//"管理记录\t%d"
//#define MID_DATA_VATTLOG 200+33)	//"查看考勤记录"
//#define MID_DATA_VADMINLOG 200+34)	//"查看管理记录"
//#define MID_DATA_DELLOG 200+35)		//"删除全部记录"
//#define MID_NET_UDPPORT 200+36)		//"服务端口"
//#define MID_NET_WEBPORT 200+37)		//"Web端口"
//#define MID_NET_MAC 200+38)		//"MAC地址"
#define MID_NET_IP				239		//"IP地址"
//#define MID_OPTIONS_NET 200+40)		//"网络设置"
//#define MID_OSA_POWEROFF		241	//"关机\t%s"
//#define MID_OSA_SUSPEND			242	//"待机\t%s"
//#define MID_OSA_POWERON			243	//"开机\t%s"
//#define MID_OSA_IDLE			244	//"空闲设置\t%s
//#define MID_OSA_IDLETIME		245	//"空闲时间\t%s"
//#define HMID_SHOWSCORE			246	//显示分数
//#define HMID_MATCHSCORE 200+47) //匹配阀值
//#define HMID_NOISETHRESHOLD 200+48)//噪音阀值
//#define HMID_MATCHSPEED 200+49)	//高速比对
//#define HMID_VOICEON			250	//语音提示
//#define HMID_NOFINGER_THRESHOLD 200+51)		//无指纹阀值
//#define HMID_HASFINGER_THRESHOLD 200+52)	//有指纹阀值
//#define HMID_TOPFINGER_THRESHOLD 200+53)	//高指纹阀值
//#define MID_AUTOTEST 200+54)	//自动检测机器
//#define MID_AT_FLASH 200+55)	//FLASH检测
//#define MID_AT_LCD 200+56)	//液晶检查
//#define MID_AT_VOICE 200+57)	//语音检测acc
//#define MID_AT_KEYPAD 200+58)	//键盘检测
//#define MID_AT_ALL 200+59)	//ALL
//#define MID_AT_RTC 200+60)	//实时时钟
//#define MID_AT_FINGER 200+61)	//指纹采集器
//#define MID_OC_NETOFF			262	//禁用以太网络
//#define MID_OC_RS232OFF			263	//禁用RS232通信
//#define MID_OC_RS485OFF			264	//禁用RS485通信
//#define MID_OS_ADVANCE 200+65)	//高级设置
//#define MID_OS_INIT	200+66)		//出厂设置
//#define MID_OI_ENNUM	200+67)		//可登记指纹
//#define MID_OI_ALNUM	200+68)		//可保存记录
//#define MID_OI_NET				269		//网络功能
//#define MID_OI_LOCK	200+70)		//锁控功能
//#define MID_OI_RFCARD	200+71)		//射频卡功能
//#define MID_OI_PROTIME	200+72)		//出厂时间
//#define MID_OI_INITDEV	200+73)		//设备初始化
//#define MID_CLEAR_DATA	200+74)		//Clear Data
//#define MID_OS_RESTORE	200+75)		//Restore Default Options
//#define MID_OS_VERSCORE	200+76)		//1:1 Verify Score
//#define MID_CLEAR_ADMIN	200+77)		//清除管理员
//#define MID_INFO_RES	200+78)		//剩余容量信息
#define MID_IR_FINGER			279		//指纹数AttLog
//#define MID_IR_ATTLOG	200+80)		//出入记录
//#define MID_INFO_DEV 	200+81)		//设备信息
//#define MID_OI_PT	200+82)		//出厂日期
//#define MID_OI_SN	200+83)		//序列号
//#define MID_OI_OEM	200+84)		//制造商
//#define HMID_TDFINGER_THRESHOLD 200+85)		//HasFingerThresholdDiff
//#define HMID_NEWFPR	200+86)		//侧光采集器
//#define HMID_DEVTYPE 200+87)	//机型
//#define MID_OI_1TO1 200+88)	//1:1功能
//#define MID_OI_ATTSTATE 200+89)	//考勤状态功能
//#define MID_OI_SHOWNAME 200+90)	//显示姓名功能
//#define MID_OI_POWERMNG 291	//电源管理功能
//#define MID_OI_PN	200+92)	//Device Name
//#define MID_OS_LOCKUSERS 200+93)//unLock Users
//#define MID_DC_ENROLL	200+94)	//登记指纹卡
//#define MID_DC_REG	200+95)	//注册指纹卡
//#define MID_DC_CREATE	200+96)	//生成指纹卡
//#define MID_DC_EMPTY	200+97)	//清空指纹卡
//#define MID_DC_UNREG	200+98)	//注销指纹卡
//#define MID_DATA_CARD	200+99)	//指纹卡管理

//#define MID_DC_DUMPFP	200+100)//复制卡内指纹
//#define MID_DC_MOVEFP	200+101)//转移指纹到卡内
//#define MID_DC_PIN	200+102)//号码卡
//#define MID_OC_PINCARD	303			//仅验证号码卡
//#define MID_OI_FWVER	200+104)//固件版本号
//#define MID_OI_ALGVER	200+105)//算法版本号
//#define MID_OA_FPKEY	200+106)//指纹卡密码
#define MID_OS_CUST	307//其他设置
//#define MID_OS_HIGHSPEED	200+108)//高速网络
//#define MID_OA_OPTION	200+109)	//门禁功能设置
//#define MID_OA_TZDEF	200+110)	//时间段定义
//#define MID_OA_ULGRP	200+111)	//开锁组合定义
//#define MID_OA_UAOPT	200+112)	//用户门禁设置
//#define MID_OA_GTZ	200+113)	//组默认时间段
//#define MID_OA_GRP	200+114)	//所属分组
//#define MID_OS_MUST1TO1 315	//必须输入ID
//#define MID_OS_COMKEY	200+116)	//连接密码
//#define MID_OC_MUSTENROLL	317//禁止未注册卡
//#define MID_OA_UDT	200+118)//使用组时段
//#define MID_CARD_REG	200+119)	//射频卡登记
//#define MID_CARD_UNREG	200+120)	//射频卡注销
//#define MID_OSA_ALARM	200+121)	//定时响铃
//#define MID_AUTO_STATE	200+124)	//定时状态装换
//#define MID_AO_IMGCOH 	200+125)    //指纹图像清晰度
//#define MID_AO_ALARMOFF 200+126)    //解除报警
#define MID_AC_DSD 	327    //门磁延时
#define MID_AC_DSM 	328    //门磁开关模式
//#define MID_OSA_ALARM_DELAY 200+129) //
//#define MID_OSA_ALARM_NAME 200+130) //
//#define MID_AD_DURESSFINGER 200+131)    //胁迫指纹管理
#define MID_AD_DURESSHELP 332     //“~K”键求助 是/否
#define MID_AD_DURESS11  333        //1：1方式报警 是/否
#define MID_AD_DURESS1N  334        //1：N方式报警 是/否
#define MID_AD_DURESSAD     335
//#define MID_ADF_ENROLL 200+136) 	//新登记胁迫指纹
//#define MID_ADF_REG 200+137)    	//胁迫指纹定义
//#define MID_ADF_UNREG 200+138)  	//取消胁迫指纹
//#define MID_ADF_UNREGALL 200+139)       //取消全部
//#define MID_AD_DURESS 200+140)  	//胁迫报警
#define MID_AD_DURESSPWD  341       //PWD方式报警 是/否
#define MID_LOCK_POWER	  342    	//LOCK POWER BUTTON
#define MID_POWER_OFF	 343    	//Shut Down
//#define MID_OS_1TON_FROM 200+144)       //1:N From
//#define MID_OS_1TON_TO 	200+145) 	//1:N To
//#define MID_OS_1TOH 	346 	//"S/_346_=前部分ID号",
//#define MID_OS_1TOG 	347 	//"S/_347_=允许1:G",
//#define MID_OI_1TON 	348 	//"S/_348_=1：N指纹",

#define MID_USERMNG 359
#define MID_AD_ADDUSER 360
//#define MID_AD_BROWSEUSER 200 + 161)
//#define MID_AD_DEPTMNG 200 + 162)
#define MID_SETTING 363
//#define MID_AD_SCHCLASS 364
//#define MID_AD_ATTRULE 200 + 165)
//#define MID_AD_CPNAME 366
#define MID_ATTQUERY  367
//#define MID_AD_LOGQUERY 200 + 168)
//#define MID_AD_ATTEXPQUERY 200 + 169)
//#define MID_AD_DEVINFOQ 200 + 170)
//#define MID_ATTPRINT 200 + 171)
//#define MID_OKBACK 372
//#define MID_VIEW 200 + 173)
#define MID_EDIT 374
//#define MID_NEW 375
//#define MID_NEWSCH 200 + 176)
//#define MID_SCH1 200 + 177)
//#define MID_SCH2 200 + 178)
//#define MID_SCH3 200 + 179)
//#define MID_SCH4 200 + 180)
//#define MID_SCHNAME 200+181)
//#define MID_DUTYONTM 200 + 182)
//#define MID_DUTYOFFTM 200 + 183)
#define MID_DEL 384
//#define MID_ALLDEL 385
#define MID_ACNO 386
#define MID_NAME 387
//#define MID_DEPT 388
#define MID_AUTH 389
//#define MID_APPLYTODEP 200 + 190)
//#define MID_APPLYTOONE 200 + 191)
//#define MID_APPLYTOALL 200 + 192)
#define MID_NONESET 393
//#define MID_OPSET 394)
//#define MID_HADSET 200 + 195)
#define MID_FINGER 396
#define MID_PWD 397
//#define MID_APPLYTO 398
#define MID_ALL 399

//#define MID_ENROLL_SUCESS 200 + 200)
//#define MID_DELALLUSER 200 + 201)
//#define MID_DEL_SCH 200 + 202)
//#define MID_DEL_ALLSCH 200 + 203)
//#define MID_SCHHADUSE 200 + 204)
//#define MID_CANNOTDEL 200 + 205)
//#define MID_SETCPNAME 200 + 206)
//#define MID_LATETM 200 + 207)
//#define MID_EARLYTM 200 + 208)
//#define MID_CALUUNIT 200 + 209)
//#define MID_DUTYUNIT 200 + 210)
//#define MID_LATEUNIT 200 + 211)
//#define MID_EARLYUNIT 200 + 212)
//#define MID_LOSTUNIT 200 + 213)
//#define MID_UNITTIMES 200 + 214)
//#define MID_UNITMINUTE 200 + 215)
//#define MID_UNITHOUR 200 + 216)
//#define MID_UNITDAY 200 + 217)
//#define MID_UNITWORKDAY 200 + 218)
//#define REP_TOPREP 200 + 219)
#define REP_ATTLOG 420
//#define REP_ATTABNOR 200 +221)
//#define REP_TIMEQSET 200 + 222)
#define REP_ATTTMST  423
#define REP_ATTTMED 424
//#define REP_ATTDATASEL 200 + 225)
//#define REP_BYONEUSER 200 + 226)
//#define REP_OUTTOUDISK 200 +227)
//#define T9_TOPHINT 200 +228)
//#define T9_BOTTOMHINT 200 +229)
//#define MID_OTHERSET 200+230)
//#define MID_EDITNAME 200+231)
//#define REP_RPOUT 200+232)
//#define MID_ONEDEPMOVE 200+233)
//#define MID_DEPMOVE 200 +234)
#define MID_NOACNOHINT	435
#define REP_DATE		436
//#define REP_CINTM 200 + 237)
//#define REP_COUTTM 200 + 238)
//#define REP_ABNOR 200 + 239)
//#define REP_TMLENG 200 + 240)
//#define REP_OT 441
//#define REP_SPEYDAY 200 + 242)
//#define REP_OTHER 200 + 243)
//#define REP_MUSTIN 200 + 244)
//#define REP_MUSTOUT 200 + 245)
//#define REP_ATTSTAT 200 + 246)
#define MID_DELUSER 447
#define MID_EDITUSER 448
//#define MID_DATETO 200 + 249)
//#define MID_USERQUERY 200+250)
//#define MID_ZHIDAN 451
//#define MID_ALLDELUSER 200+252)
//#define MID_ALLDELSCH 200+253)
//#define MID_SCHERROR 200+254)
//#define REP_MONTH 455
//#define MID_SCHHADDEL 200+256)
//#define MID_USERHADDEL 200+257)
//#define MID_ACNOWIDTH 200+258) //工号位数
//#define MID_ACNOPRE0 200+259)  //工号前缀补0
//#define MID_NOSPACE 200+260)   //输出数据量太大
//#define MID_QYAGAIN 200+261) //请重新查询
//#define MID_RFCARD 200+262) //感应卡
#define MID_ATT  463 //考勤
//#define MID_STAFF 200+264) //员工
//#define MID_CALU 465 //统计
//#define MID_CANCELENROLL 200+266)//取消登记
#define MID_REGFP 467//登记指纹
#define MID_REGPWD 468//登记密码
#define MID_SAVE 469//保存
#define MID_EXIT 470//退出
#define MID_SAVEDATA 471//保存提示
#define MID_GOONINPUT		472//保存，是否继续操作
//#define MID_OK 473//确定
#define MID_EMPTYPWD 474//请输入密码
#define MID_NOTSAMEPWD 475//密码不一致
#define MID_CHOOSEUSER 476//请选择用户
//#define MID_CHOOSEDEL 200+277)//删除数据选择
#define MID_USEBASE 478//用户基本信息
#define MID_REMOVEHINT 479//确定删除以下数据吗
//#define MID_REMOVEGOON 200+280)//操作成功 是否继续?
#define MID_QUERYUSER 481//查找用户
#define MID_QUERYNONE 482//无更多查找结果
#define MID_STATUS 483//状态
#define MID_ERRORTIME 484//状态
//#define MID_VERIFYMODE 200+285)//验证方式选择
#define MID_DAY 486//日
#define MID_HOUR 487//小时
#define MID_RECTOTAL 488//记录数
//#define MID_ONETOONEHINT 200+289)//1:1提示
//#define MID_PWDHINT 200+290)//密码验证提示
#define MID_PWDLEN 491//密码长度
#define MID_FPLEN 492//指纹数
#define MID_FPOK 493//登记成功,继续登记
#define MID_FPFULL 494//登记满
#define MID_APPNAME 495//系统名称
//#define MID_EXITASK 200+296)//保存退出吗?
#define MID_WAIT 497//正在载入数据
#define MID_FPHADDEL 498//指纹已删除
#define MID_PWDHADDEL 499//密码已删除 

#define MID_MON		500		//月 
#define MID_WORKCODE	501	//WORK CODE
//#define MID_BUTTONBEEP 	500+2) 	//button beep
//#define MID_AT_MP3	500+3)	//mp3 hint
//#define MID_ADV_VOICETZ	500+4)	//voice time zone 
//#define MID_OSA_WEBSERVERIP 500+5) 	//web server ip
//#define MID_ADV_AUDIOVOL 500+6)	//adjust audio volume
//#define HID_PLUGPENDRIVE 500+7)	//PLS PLUG PEN DRIVE
#define HID_DOWNLOADING_DATA 508       //Downloading DATA
//#define HID_PENDRIVE_NOEXIST 500+9)	//PEN DRIVER NO EXIST
//#define HID_COPYDATA_FAILURE 500+10)	//COPY DATA FAILURE
//#define HID_COPYDATA_SUCCEED 500+11)	//COPY DATA SUCCEED
//#define HID_MOUNTING_PENDRV 500+12)	//Mounting PenDrive
//#define HID_FIRMWARE_ITEM 500+13)	//Firmware item(files)
//#define HID_SELECT_ITEM 500+14)	//select item for update
#define HID_PLACEFINGER4 515    	//"请第四次按手指"
//#define HID_CONTINUECANCEL 500+16)	//退出ESC 继续OK
#define HID_CHGPWD 517          	//change password
//#define HID_CHGCARDNUM 500+18) 	//change card number
//#define MID_DOWNLOAD_ATT 500+19)	//download attlog
//#define MID_DOWNLOAD_USR 500+20)	//download usr info
//#define MID_UPLOAD_USR	500+21)	//upload usr info
//#define MID_PENDRV_MNG	500+22)	//pen drive manage
#define MID_GATEWAY_IP	523	//gateway
#define MID_NETMASK_ADDR 524	//Network mask
//#define MID_OI_USERNUM	500+25)	//Users
//#define MID_TWOSENSOR	500+26)	//Two sensor
//#define MID_UPDATE_FIRMWARE 500+27)	//update firmware
//#define MID_UPLOAD_SMS 	500+28)    	//upload SMS
//#define MID_DOWNLOAD_SMS 500+29)   	//download SMS
//#define MID_MODEM	500+30) 	//use Modem or not
#define MID_AUTHSERVER	531     	//use AuthServer or not
#define MID_AUTHSERVER_IP 532    	//AuthServer IP
#define MID_AUTHSERVER_REGISTER	533 	//REMOTE REGISTER
#define MID_AUTHSERVER_ERROR	534 	//REMOTE AUTHSERVER ERROR
//#define MID_AUTOBELL_DELAY	500+35) 	//Auto ALARM delay times
#define MID_NET_DHCP		536 	//Auto ALARM delay times
//#define HID_NET_DHCPHINT	537 	//DHCP Running
//#define MID_TIME_SET		500+38) 	//Set time Manualy
//#define MID_TIME_SYNC		500+39)	//Synchronize time
//#define MID_TIME_SERVER		500+40)	//Time server ip address
//#define HID_TIME_SYNCHINT	500+41)	//Time synchronize hint
//#define MID_AD_ERRPRESS		542	//press finger n times failed to Alarm
//#define MID_OA_VERIFYTYPE	500+43)	//User verify type
//#define MID_OA_GVERIFYTYPE	500+44)	//Group verify type
//#define MID_OA_VSHINT		500+45)	//
//#define HID_OA_NOEQUAL		500+46)	//
//#define MID_OA_GRPVS		500+47)	//whether use Group Verify Type or not
//#define MID_PROXY_IP		500+48)	//proxy server ip address
//#define MID_PROXY_PORT		500+49)	//proxy server port
//#define MID_PROXY_SERVER	500+50)	//PROXY SERVER enable
#define MID_FINGER_SENSITIVITY	572	//Finger Sensitivity

#define MID_ENROLLING		600    //正校验指纹
#define MID_PAGEUP			601       //上一页:
#define MID_PAGEDOWN		602     //下一页:
#define MID_DETAILREC		603   //明细记录
#define HIT_USER1           604     //新增用户
#define HIT_USER2           605     //管理用户
#define HIT_USER            606     //User Manager
#define HIT_COMM            607     //Comm.
#define HIT_SYSTEM          608     //System
#define HIT_DATETIME        609     //Date/Time
#define HIT_DATA            610     //Data Manager
#define HIT_AUTO            611     //Auto Test
#define HIT_RECORD          612     //record
#define HIT_INFO            613     //System Info
#define HIT_COMM1           614     //Network
#define HIT_COMM2           615     //RS232/485
#define HIT_COMM3           616     //Security
#define HIT_SYSTEMSET       617     //Setting
#define HIT_SYSTEM1         618     //System
#define HIT_SYSTEM2         619     //Date Manager
#define HIT_SYSTEM4         620     //Update
#define HIT_SYSTEM5         621     //Keyboard
#define HIT_SYSTEM6         622     //Display
#define HIT_SYSTEM7         623     //Misc Setting
#define HIT_SYSTEM8         624     //Reset
#define HIT_OK              625     //OK(M/<-)
#define HIT_CANCEL          626     //Back(ESC)
#define HIT_RUN             627     //Message
#define HIT_RIGHT           628     //Change Saved Successfully!
#define HIT_ERR             629     //Warning
#define HIT_ERROR0          630     //The input data error!
#define HIT_NETERROR1       631     //IP address error!
#define HIT_NETERROR2       632     //Subnet Mask error
#define HIT_NETERROR3       633     //Gateway error!
#define HIT_RSERROR         634     //RS232 AND RS485 select error
#define HIT_DATE            635     //Date & Time Setting
#define HIT_DATE1           636     //Date
#define MID_YEAR			637	//年
#define MID_MONTH			638	//月
#define MID_DAY1			639	//日
#define MID_TIME			640	//时间
#define MID_HOUR1			641	//时
#define MID_MINUTE			642	//分
#define HIT_DATE8           643     //Second
#define HIT_UDATA           644     //PenDrive
#define HIT_UDATA1          645     //Download Attlog
#define HIT_UDATA2          646     //Download User
#define HIT_UDATA3          647     //Upload User
#define HIT_UDATA4          648     //Upload Picture
#define HIT_AUTO0           650     //All Test
#define HIT_AUTOS           651     //Auto Test
#define HIT_AUTO1           652     //TFT Test
#define HIT_AUTO2           653     //Audio Test
#define HIT_AUTO3           654     //Sensor Test
#define HIT_AUTO4           655     //keyboard Test
#define HIT_AUTO5           656     //RTC Test

#define MID_LOCAL_IP		658     //IP Address
#define HIT_COMM1NET2       659     //Subnetmask
#define HIT_COMM1NET3       660     //Gateway
#define HIT_COMM1NET4       661     //NetSpeed

#define HIT_COMM2SER1       663     //Baudrate

#define HIT_COMM3LINK1      665     //DeviceID
#define HIT_COMM3LINK2      666     //Password
#define HIT_COMM3LINK3      667     //(1-254)
#define HIT_COMM3LINK4      668     //(Max 6 digit)
#define HIT_SYSTEM1SET1     669     //Threshold(1:1)
#define HIT_SYSTEM1SET2     670     //Date Fmt
#define HIT_SYSTEM1SET5     671     //Keybeep
#define HIT_SYSTEM1SET6     672     //Voice
#define HIT_SYSTEM1SET7     673     //Vol.
#define HIT_SYSTEM2INFO     674     //Are you sure?

#define HIT_SYSTEM2DATA2    677     //Delete Attlog
#define HIT_SYSTEM2DATA3    678     //Delete All
#define HIT_SYSTEM2DATA4    679     //Clear Purview
#define HIT_SYSTEM2DATA5    680     //Deleted
#define HIT_SYSTEM2DATA6    681     //Clear successful!
#define HIT_SYSTEM2DATA7    682     //Delete picture

#define HIT_SYSTEM3ITEM1    684     //Sleep Time
#define HIT_FREETIME        685     //M(0:Always On)

#define HIT_SYSTEM3ITEM6    687     //Log Alert
#define HIT_SYSTEM3ITEM7    688     //ReCheck Min
#define HIT_SYSTEM5KEY      689     //Key State
#define HIT_SYSTEM5KEY1     690     //Check-In
#define HIT_SYSTEM5KEY2     691     //CheckOut
#define HIT_SYSTEM5KEY3     692     //OT-In
#define HIT_SYSTEM5KEY4     693     //OT-Out
#define HIT_SYSTEM5KEY5     694     //BreakOut
#define HIT_SYSTEM5KEY6     695     //Break-In
#define HIT_SPEED1          696     //Auto
#define HIT_SPEED2          697     //100M
#define HIT_SPEED3          698     //10M
#define HIT_SWITCH1         699     //ON

#define HIT_SWITCH2         700     //OFF
#define HIT_FREESET1        701     //Sleep
#define HIT_FREESET2        702     //Shutdown
#define HIT_AUTOTESTINFO    703     //Press'OK'Continue,'ESC'quit
#define HIT_AUTOTESTINFOEX  704     //ESC:quit

#define HIT_INFO2           711     //Records
#define HIT_INFO3           712     //Device
#define HIT_INFO4           713     //SysInfo
#define HIT_INFO5           714     //DeviceName
#define HIT_INFO6           715     //Serial Num
#define HIT_INFO7           716     //Alg Version
#define HIT_INFO8           717     //Firmware Ver
#define HIT_INFO9           718     //Vendor
#define HIT_INFO10          719     //Manu Time

#define HIT_INFO12          721     //Used
#define HIT_INFO13          722     //Admin:
#define HIT_INFO14          723     //Pwd:
#define HIT_INFO15          724     //Free
#define HIT_INFO16          725     //FP:
#define HIT_INFO17          726     //Record:

#define HIT_INFO19          728     //User:
#define HIT_UINFO3          729     //Downloading,wait....
#define HIT_UINFO4          730     //Uploading,wait....
#define HIT_UINFO5          731     //Download Complate!
#define HIT_UINFO6          732     //Upload Complate!
#define HIT_UINFO7          733     //PenDrive Not find!

#define HIT_UINFO9          735     //Updateing,wait....

#define HIT_UINFO13         737     //Update Successfully!
#define HIT_UINFO14         738     //Data fail!
#define HIT_UINFO15         739     //Processing Picture,wait....
#define HIT_UINFO16         740     //Picture Not find!

#define HIT_KEY1            743     //ShutDown Key
#define HIT_KEY2            744     //BackSpace Key
#define HIT_KEY3            745     //Up Cursor
#define HIT_KEY4            746     //Down Cursor
#define HIT_KEY5            747     //Left Cursor
#define HIT_KEY6            748     //Right Cursor
#define HIT_NULLKEY         749     //None
#define HIT_KEYINFO         750     //Key set error!
#define HIT_UPIC1           751     //Browse
#define HIT_UPIC2           752     //Curr(%d/%d)
#define HIT_UPIC3           753     //Update(OK)
#define HIT_UPIC4           754     //Next
#define HIT_UPIC5           755     //Prev
#define HIT_UPIC6           756     //Upload Complate!
#define HIT_UPIC7           757     //Uploading picture.
#define HIT_SNDINFO         758     //Play Voice
#define HIT_DPIC1           759     //Delete
#define HIT_DPIC2           760     //Delete All
#define HIT_DPIC3           761     //Delete Complate!
#define HIT_DPIC4           762     //Deleting...
#define HIT_DPICINFO        763     //Not find pictrue!
#define HIT_UPDATEINFO      764     //Reset Complate!
#define HIT_MACINFO         765     //MAC Address
#define HIT_INFO20          766     //User:
#define MID_DOWNLOAD_PHOTO	767
#define MID_LOAD_PHOTO		768
#define HIT_UPEKINITOK      769     //Sensitivity
#define HIT_UPEKINITFAIL    770     //InitFPD

#define MID_INTERFACE		800 //界面风格
#define MID_FPRETRY			801 //1:1重试次数
#define MID_PWDRETRY		802  //密码重试次数
#define MID_CLOCKSTYLE		803 //时钟选择
#define MID_PICTIME			804 //图片循环间隔
#define MID_CLOCKTIME		805 //时钟显示延时
#define MID_RIGHT			806 //成功提示
#define MID_RUN				807 //设置成功
#define MID_WARNING			808 //警告
#define MID_INVALIDDATA		809 //数据非法提示
#define MID_DATARANGE		810 //数据范围 
#define MID_INVALIDDATA2	811     //数据非法提示2
#define MID_RETRYPWD		812	//请重新输入密码
#define MID_SECOND			813	//秒
#define MID_ALARM			814	//闹铃
#define MID_ALARMTIME		815	//响铃时间
#define MID_ALARMWAVE		816	//铃声
#define MID_ALARMSTART		817	//启用
#define MID_ALARMEDIT		818	//闹铃编辑
#define MID_ALARMWVSEL		819	//铃声选择
#define MID_ALARMSTATUS		820	//闹铃状态
#define MID_ALARMSTART1		821	//开启
#define MID_ALARMSTOP		822	//关闭
#define MID_TIMEERROR		823	//闹铃时间重复，请重新设置！
#define MID_ALARMSETTING	824 //闹铃设置
#define MID_ALARMVOL		825 //音量设置
#define HIT_USER3           826     //短消息
#define HIT_USER5           827     //Access
#define MID_SMSCHECK		828 //查看
#define MID_SMSMENU			829 //菜单
#define MID_SMSPRIVATE		830 //个人
#define MID_SMSCOMMON		831 //公共
#define MID_SMSADD			832 //新增
#define MID_SMSEDIT			833 //编辑
#define MID_SMSDEL			834 //删除

#define MID_SMSSTARTTIME	836 //起始时间
#define MID_SMSMINUTE		837 //分钟
#define MID_SMSTYPE			838 //类型
#define MID_SMSEDITCAPTION	839 //编辑闹铃
#define MID_SMSTIMELEN		840	//有效时长
#define MID_SMSTYPE1		841 //消息类型
#define MID_SMSEDITCAPTION1 842 //短消息
#define MID_SMSRESERVED		843 //保留
#define MID_SMSSEND			844 //指定用户
#define MID_SMSDELETEHIT	845 //确定要删除此消息吗？
#define MID_SMSNOTSEND		846 //未分配
#define MID_SMSCONTENTHIT	847 //短消息内容
#define MID_SMSSENDCAPTION	848 //分发短消息

#define MID_SMSSELECT		850 //选择/取消
#define MID_SMSCANCEL		851 //退出
//#define MID_SMSNOUSERHINT 800+52) //没有选择任何用户...
//#define MID_MIFCARDLABEL 800+53) //ID卡
//#define MID_MIFCARDREG 800+54) //登记卡
#define MID_WKCDEDIT		855 //编辑workcode
#define MID_WKCDADD			856	//新增workcode
#define MID_WKCDCODE		857	//代码
#define MID_WKCDKEY			858	//快捷键
#define MID_WKCDNAME		859 //名称
#define MID_WKCDCHGTIME		860 //自动切换
//#define MID_WKCDDOORFUN 800+61) //门禁功能
#define MID_WKCDDOORON		862 //开启
#define MID_WKCDDOOROFF		863 //关闭
#define MID_SHORTKEY01		864	//F1 
#define MID_SHORTKEY02		865 	//F2
#define MID_SHORTKEY03		866 	//F3
#define MID_SHORTKEY04		867 	//F4
#define MID_SHORTKEY05		868 	//F5
#define MID_SHORTKEY06		869 	//F6
#define MID_SHORTKEY07		870 	//F7
#define MID_SHORTKEY08		871 	//F8
#define MID_SHORTKEY09		872      	//*键(pageup)
#define MID_SHORTKEY10		873      	//#键(pagedown)
#define MID_SHORTKEY11		874      	//退格键
#define MID_SHORTKEY12		875       //上光标键
#define MID_SHORTKEY13		876      	//下光标键
#define MID_SHORTKEY14		877      	//左光标键
#define MID_SHORTKEY15		878     	//右光标键
#define MID_WKCDKEY71		879	//PageUp 
#define MID_WKCDKEY72		880 	//pageDown
#define MID_WKCDDELETE 881 //确定删除此workcode项吗？
#define MID_ALARM_TIMES 882 //响铃次数
//#define MID_WKCDKEYREPEAT 800+83) //快捷键重复，请重新选择!
#define MID_WKCDINPUT 884 //请输入workcode
#define MID_WKCDACTIMEHINT 885 
#define MID_WKCDCODEREPEAT 886
#define MID_WKCDNAMEREPEAT 887
#define MID_SMSCONTENTEMPTY 888
#define MID_ALARMVOLHINT 889
#define MID_ALARMVOLADV 890
//#define MID_WKCDDELHINT 800+91)
#define MID_SMSCOUNTHINT 892
#define MID_SMSTIMEHINT 893
#define MID_SMSCOUNT 894
#define MID_SMSTIAO 895
#define MID_SMSCURINDEX 896
#define MID_SMSALLHINT 897	//通 知
//#define MID_SMSUSERHINT 800+98)	//短消息
#define MID_KEYTESTHINT1 899
#define MID_KEYTESTHINT2 900
#define MID_SMSCHECKHINT 901
#define MID_DATASMSDOWNLOAD 902
#define MID_DATASMSLOAD 903
#define MID_WKCDEMPTYHINT 904
#define MID_LOGHINTOTHER 905
#define MID_NOWKCDHINT 906
#define MID_SHORTKEYFUN 907
#define MID_STKEYFUN1 908	//未定义
#define MID_STKEYFUN2 909	//功能键
#define MID_STKEYFUN3 910	//WorkCode
#define MID_STKEYFUN4 911	//查看短消息
#define MID_STATECODEERROR 912
#define MID_STATECODEREPEAT 913
#define MID_STATENAMEREPEAT 914
#define MID_STKEYFUNREPEAT 915
#define MID_IMEMETHOD1 916	//拼音
#define MID_IMEMETHOD2 917	//英文
#define MID_IMEMETHOD3 918	//符号
#define MID_BRIGHT_ADV 919      //屏幕亮度调整
#define MID_BRIGHT_HINT 920     //范围10%～100%
#define MID_WKCDKEY73 921 	//Esc
#define MID_WKCDKEY74 922 	//M/Enter
#define MID_WKCDKEY75 923 	//0
#define MID_PRINTER     924     //打印
#define MID_PT_MODE     925     //模式
#define MID_PT_HINT     926     //是否打印考勤记录?

//IKIOSK定制=============================================
#define MID_IKIOSK			930
#define MID_IKIOSK_SMSDELAY		970
#define MID_IKIOSK_FUNLIMIT		971
#define MID_IKIOSK_THEME		972
#define MID_IKIOSK_REBOOT		973
#define IKIOSK_STKEY_COUNT		40		//快捷功能数量
//#define IKIOSK

#ifdef IKIOSK
#define HIT_IKIOSK_KEYFUN0    930
#define HIT_IKIOSK_KEYFUN1  931
#define HIT_IKIOSK_KEYFUN2  932
#define HIT_IKIOSK_KEYFUN3  933
#define HIT_IKIOSK_KEYFUN4  934
#define HIT_IKIOSK_KEYFUN5  935
#define HIT_IKIOSK_KEYFUN6  936
#define HIT_IKIOSK_KEYFUN7  937
#define HIT_IKIOSK_KEYFUN8  938
#define HIT_IKIOSK_KEYFUN9  939
#define HIT_IKIOSK_KEYFUN10 940
#define HIT_IKIOSK_KEYFUN11 941
#define HIT_IKIOSK_KEYFUN12 942
#define HIT_IKIOSK_KEYFUN13 943
#define HIT_IKIOSK_KEYFUN14 944
#define HIT_IKIOSK_KEYFUN15 945
#define HIT_IKIOSK_KEYFUN16 946
#define HIT_IKIOSK_KEYFUN17 947
#define HIT_IKIOSK_KEYFUN18 948
#define HIT_IKIOSK_KEYFUN19 949
#define HIT_IKIOSK_KEYFUN20 950
#define HIT_IKIOSK_KEYFUN21 951
#define HIT_IKIOSK_KEYFUN22 952
#define HIT_IKIOSK_KEYFUN23 953
#define HIT_IKIOSK_KEYFUN24 954
#define HIT_IKIOSK_KEYFUN25 955
#define HIT_IKIOSK_KEYFUN26 956
#define HIT_IKIOSK_KEYFUN27 957
#define HIT_IKIOSK_KEYFUN28 958
#define HIT_IKIOSK_KEYFUN29 959
#define HIT_IKIOSK_KEYFUN30 960
#define HIT_IKIOSK_KEYFUN31 961
#define HIT_IKIOSK_KEYFUN32 962
#define HIT_IKIOSK_KEYFUN33 963
#define HIT_IKIOSK_KEYFUN34 964
#define HIT_IKIOSK_KEYFUN35 965
#define HIT_IKIOSK_KEYFUN36 966
#define HIT_IKIOSK_KEYFUN37 967
#define HIT_IKIOSK_KEYFUN38 968
#define HIT_IKIOSK_KEYFUN39 969
#endif



//门禁功能<liming>
//#define SSR_DOOR	1000
#define MID_USER_DOOR	1001	//用户门禁
#define MID_USER_ID		1002	//用户编号
#define MID_USER_GP		1003	//所属分组
#define MID_USER_VRTP	1004	//验证方式
#define MID_USER_GVRTP	1005	//组验证方式
#define MID_USER_USETZ	1006	//使用时间段
#define MID_USER_GTZ	1007	//组时间段
#define MID_USER_DTZ	1008	//自定义时间段
#define MID_USER_TZ		1009	//时间段
#define MID_USER_DURFP	1010	//胁迫指纹
#define MID_USER_DURFPM	1011	//管理胁迫指纹
#define MID_USER_FPS	1012	//指纹数
#define MID_FP_HINT1	1013	//一般指纹
#define MID_FP_HINT2	1014	//胁迫指纹
#define MID_LIST_CAPTION 1015	//编号    描述
#define MID_DF_CC		1016	//定义/取消
#define MID_ENROLL_FP	1017	//登记指纹
#define MID_CANCEL_ALL	1018	//取消全部
#define MID_DFP_SAVE	1019	//保存
#define MID_SAVE_HINT1	1020	//您选择的组没有定义，是否定义？
#define MID_SAVE_HINT2	1021	//您选择的时间段没有定义，是否定义？
#define MID_LOCK_OP1	1022	//时间段设置
#define MID_LOCK_OP2	1023	//节假日设置
#define MID_LOCK_OP3	1024	//组时间段设置
#define MID_LOCK_OP4	1025	//开锁组合设置
#define MID_LOCK_OP5	1026	//门禁管理参数
#define MID_LOCK_OP6	1027	//胁迫报警参数
#define MID_TZ_SY		1028	//:
#define MID_TZ_TO		1029	//至
#define MID_SAVE_HINT3	1030	//数据已保存，是否继续?
#define MID_SAVE_HINT4	1031	//数据已更改，是否保存?
#define MID_SAVE_HINT5	1032	//时间段编号不正确！
#define MID_SAVE_HINT6	1033	//没有定义时间段，是否使用组时间段？
#define MID_GP_INDEX	1034	//编号
#define MID_GP_DEFTZ	1035	//默认时间段
#define MID_GP_DELHINT	1036	//确定要删除此组吗？
#define MID_GP_ADD	1037	//新增组
#define MID_GP_EDIT	1038	//编辑组
#define MID_GP_HD	1039	//节假日
#define MID_HD_VALID	1040	//有效
#define MID_HD_INVALID	1041	//无效
#define MID_SAVE_HINT7	1042	//组编号设置错误
#define MID_SAVE_HINT8	1043	//组已存在，是否覆盖
#define MID_SAVE_HINT9	1044	//没有设置时间段
#define MID_HTZ_DATES	1045	//起始日期
#define MID_HTZ_DATEE	1046	//终止日期
#define MID_HTZ_DELHINT	1047	//确定删除此节日吗?
#define MID_SAVE_HINT10	1048	//编号已存在！
#define MID_SAVE_HINT11	1049	//节假日编号设置错误！
#define MID_HTZ_ADD	1050	//新增节假日
#define MID_HTZ_EDIT	1051	//编辑节假日
#define MID_HTZ_DATE	1052	//起止日期
#define MID_REC_FULL	1053	//记录数已达到最大值！
#define MID_LOCK_GP	1054	//组合
#define MID_GROUP	1055	//组
#define MID_MEMBER	1056	//成员
#define MID_SAVE_HINT12	1057	//组合已存在！
#define MID_SAVE_HINT13	1058	//组合编号设置错误！
#define MID_SAVE_HINT14	1059	//没有设置成员
#define MID_SAVE_HINT15	1060	//不存在,是否添加?
#define MID_LOCKGP_ADD	1061	//添加开锁组合
#define MID_LOCKGP_EDIT	1062	//编辑开锁组合
#define MID_ALARM_DELAY	1063	//门磁报警延时
#define MID_ALARM_COUNT	1064	//按键报警次数
#define MID_CLOSE_TZ	1065	//常闭
#define MID_OPEN_TZ	1066	//常开
#define MID_HTZ_VALID	1067	//节假日是否有效
#define MID_TZ_SAME	1068	//时间段冲突！
#define MID_TIMES	1069	//次
#define MID_STKEYFUN5	1070	//求助键
#define MID_CLEARALARM	1071	//是否解除报警?
#define MID_GP_DELHINT1	1072	//此组正被使用,不能删除!
#define MID_ANTIPASSBACK 1073	//非法出入

//#define HID_CARD_INTERFACE      1080
#define MID_CARD                1080    //卡
#define MID_ENROLL_CARD         1081  //登记卡
#define MID_CHANGE_CARD         1082  //更改卡
#define MID_REGED_CARD          1083  //卡已登记
#define MID_PLACE_CARD          1084  //请出示卡
#define MID_ERROR_CARD          1085  //卡错误
#define MID_CARD_NUM            1086  //ID:
#define MID_CARD_SUCC           1087  //登记成功
#define MID_CARD_OPT            1088 //退出，保存
#define MID_RF_CARD             1089  //RFC
#define MID_DEL_CARD            1090 //Delete RFCard number
#define MID_DEL_RF              1091 //Del RFCard
#define MID_HIDHADDEL			1092 //卡已删除

#define SSR_CARD	1100
#define MID_CARD_MNG	1100	//卡管理
#define MID_CARD_OP1	1101	//登记号码卡
#define MID_CARD_OP2	1102	//登记指纹卡
#define MID_CARD_OP3	1103	//清空卡信息
#define MID_CARD_OP4	1104	//复制卡信息
#define MID_CARD_OP5	1105	//卡参数设置
#define MID_INPUT_CODE	1106	//请输入卡号码，OK确定
#define MID_CARD_CODE	1107	//卡号
#define MID_CARD_SUC	1108	//登记成功，是否保存数据?
#define MID_MF_OK	1109	//继续[OK],退出[ESC]
#define MID_MFCP_OK	1110	//复制数据成功
#define MID_MF_SAVEOK	1111	//保存成功！
#define MID_MF_SAVEFL	1112	//保存失败！
#define MID_MFCP_FAIL	1113	//复制数据失败！
#define MID_MFCP_HINT	1114	//成功复制%d枚指纹
#define MID_MFCP_QUEST	1115	//指纹重复，是否覆盖?
#define MID_MFCP_QUEST2	1116	//用户存在，是否覆盖数据?
#define MID_ONLY_PIN	1117	//只验证号码
#define MID_MFC_PWD	1118	//指纹卡密码
#define MID_MFCP_OP1	1119	//只复制用户信息
#define MID_MFCP_OP2	1120	//复制用户信息和指纹
#define MID_CRTMF_HINT1	1121	//用户信息存在,是否复制到卡?
#define MID_CRTMF_HINT2	1122	//无指纹数据,是否登记新指纹?
#define MID_CLNMF_HINT1	1123	//成功清除用户信息
#define MID_CLNMF_HINT2	1124	//清除用户信息失败
#define MID_CRTMF_HINT3	1125	//登记指纹卡失
#define MID_CLNMF_HINT3	1126	//是否清除本机内与此卡对应的用户信息?
#define MID_MFCP_HINT1	1127	//创建用户信息失败
#define MID_MFCP_HINT2	1128	//复制指纹失败
#define MID_SAVE_LOCAL	1129	//必须保留信息到本机
#define MID_VF_FAILED	1130	//验证失败
#define MID_GP_MEMBER	1131	//开锁组合成员
#define MID_GP_VF	1132	//组合验证
#define MID_FPCARD_VF	1133	//指纹卡
#define MID_ADUSER_HINT 1134	//无验证信息，不能设为管理员!
#define MID_CLNMF_HINT4	1135	//正在清除...

//#define SSR_WIFI		1139
#define MID_WIFI_SETTING	1139	//无线网络设置		
#define MID_WIFI_ID			1140	//网络识别ID
#define MID_WIFI_MODE		1141	//网络模式
#define MID_WIFI_INFRA		1142	//Infra
#define MID_WIFI_ADHOC		1143	//Adhoc
#define MID_WIFI_AUTH		1144	//认证类型
#define MID_WIFI_OPEN		1145	//OPEN
#define MID_WIFI_SHARED		1146	//SHARED
#define MID_WIFI_WEPAUTO	1147	//WEPAUTO
#define MID_WIFI_WPAPSK		1148	//WPAPSK
#define MID_WIFI_WPA2PSK	1149	//WPA2PSK
#define MID_WIFI_WPANONE	1150	//WPANONE
#define MID_WIFI_ENCRYP		1151	//加密方式
#define MID_WIFI_NONE		1152	//NONE
#define MID_WIFI_WEP		1153	//WEP
#define MID_WIFI_TKIP		1154	//TKIP
#define MID_WIFI_AES		1155	//AES
#define MID_WIFI_PWDTYPE	1156	//密码类型
#define MID_WIFI_PWDFMT1	1157	//64bit(104+24) 10位16进制数字
#define MID_WIFI_PWDFMT2	1158	//128bit(104+24) 26位16进制数字
#define MID_WIFI_PWDFMT3	1159	//64bit(40+24) 5位ASCII字符
#define MID_WIFI_PWDFMT4	1160	//128bit(104+24) 13位ASCII字符
#define MID_WIFI_PWDWEP		1161	//WEP
#define MID_WIFI_PWDWPA		1162	//WPA
#define MID_WIFI_PWDSET		1163	//设置密码
#define MID_WIFI_MANUIP		1164	//手动分配
#define MID_WIFI_IP			1165	//指定IP
#define MID_WIFI_PWDIDX		1166	//密码序号

//#define SSR_ANTIPB		1170	
#define MID_ANTI_FUN		1170	//反潜功能
#define MID_ANTI_WAY		1171	//反潜方向
#define MID_ANTI_NONE		1172	//不反潜
#define MID_ANTI_OUT		1173	//出反潜			//fix by luoxw
#define MID_ANTI_IN			1174	//入反潜			//fix by luoxw
#define MID_ANTI_LOCAL		1175	//本机状态
#define MID_CTRL_OUT		1176	//控制出门
#define MID_CTRL_IN			1177	//控制入门
#define HIT_COMM4           1178    //WiFi Setup
#define HIT_COMM5           1179    //Wirless LAN

#define MID_WFINFO_NAME		1180	//可用无线网络
#define MID_WFINFO_SSID		1181	//网络名称(SSID)
#define MID_WFINFO_SINGLE	1182	//信号强度
#define MID_WFINFO_HINT		1183	//正在搜索可用网络...
#define MID_WFINFO_REFRESH	1184	//刷新列表
#define MID_WIFI_ERR1		1185	//网络识别ID不能为空!
#define MID_WIFI_ERR2		1186	//密码长度不正确!
#define MID_WIFI_HINT2		1187 //8～64位ASCII字符或16进制数
#define MID_WIFI_ERR3		1188	//密码字符类型不正确!
#define MID_WIFI_DHCPERR	1189	//获取IP地址失败!

#define MID_GPRS		1190
#define MID_GPRS_MODEM		1191	//Modem类型
#define MID_GPRS_CONNECT	1192	//是否连接
#define MID_GPRS_APN		1193	//APN名称
#define MID_GPRS_USER		1194	//用户名称
#define MID_GPRS_PWD		1195	//连接密码
#define MID_GPRS_NMB		1196	//接入号码
#define MID_GPRS_RCT		1197	//重拨间隔
#define MID_GPRS_RCC		1198	//重拨次数
#define MID_GPRS_ONLINE		1199	//保持在线
#define MID_GPRS_UPT		1200	//上传记录间隔
#define MID_GPRS_CHKMODE	1201	//检索方式
#define MID_GPRS_REQT		1202	//响应时间
#define MID_GPRS_TOT		1203	//超时时间
#define MID_GPRS_SAT		1204	//服务器地址类型
#define MID_GPRS_IP		1205	//IP
#define MID_GPRS_DNS		1206	//DNS
#define MID_GPRS_URL		1207	//URL
#define MID_GPRS_TYPE1		1208	//GPRS
#define MID_GPRS_TYPE2		1209	//CDMA
#define MID_GPRS_WIN		1210	//拨号设置
#define MID_GPRS_ADVANCE	1211	//高级设置
#define MID_GPRS_ERR		1212	//APN名称不能为空!
#define MID_GPRS_TYPE3		1213	//WAN
#define MID_GPRS_DNSERR		1214	//DNS地址不合法!

#define MID_WG_SETTING		1215	//韦根设置
#define MID_WG_SETOUT		1216	//输出配置
#define MID_WG_SETIN		1217	//输入配置
#define MID_WG_DEFFMT		1218	//已定义格式
#define MID_WG_FMT1		1219	//Wiegand 26 with sitecode
#define MID_WG_FMT2		1220	//Wiegand 34 with sitecode
#define MID_WG_FMT3		1221	//Wiegand 26 without sitecode
#define MID_WG_FMT4		1222	//Wiegand 34 without sitecode
#define MID_WG_FAILID		1223	//失败ID
#define MID_WG_SITECODE		1224	//区位码
#define MID_WG_DURESSID		1225	//胁迫ID
#define MID_WG_OEMCODE		1226	//OEM码
#define MID_WG_PULSWIDTH	1227	//脉冲宽度
#define MID_WG_PULSINTERVAL	1228	//脉冲间隔
#define MID_WG_US		1229	//微秒
#define MID_WG_DEFAULT		1230	//默认值
#define MID_WG_USRFMT		1231	//自定义格式
#define MID_WG_BITCOUNT		1232	//Bit位数
#define MID_WG_OUTPUT		1233	//输出内容
#define MID_WG_INPUT		1234	//输入内容
#define MID_WG_INOUT		1235	//出入反潜
#define MID_GPRS_FREQUENCY	1236	//频率
#define MID_BELL_EXT		1237	//外部响铃
#define MID_GPRS_ON		1238	//GPRS在线
#define MID_QUERY_ATTLOG	1239	//查询考勤记录
#define MID_QUERY_SET		1240	//查询考勤照片
#define MID_QUERY_SET1		1241	//删除考勤照片
#define MID_QUERY_SET2		1242	//查询失败照片
#define MID_QUERY_SET3		1243	//删除失败照片
#define MID_PHOTO_DEL		1244	//全部删除
#define MID_PHOTO_HINT1		1245	//正在删除照片,请稍后...
#define MID_PHOTO_HINT2		1246	//正在读取照片,请稍后...
#define MID_PHOTO_VIEW		1247	//预览
#define MID_CAMERA_EN		1248	//环境
#define MID_CAMERA_IN		1249	//室内
#define MID_CAMERA_OUT		1250	//室外
#define MID_CAMERA_MODE		1251	//拍照模式
#define MID_CAMERA_MODE1	1252	//不拍照
#define MID_CAMERA_MODE2	1253	//拍照
#define MID_CAMERA_MODE3	1254	//拍照并保存
#define MID_CAMERA_MODE4	1255	//不通过保存
#define MID_CAMERA_MODE5	1256	//使用全局设置
#define MID_CAMERA_WARN		1257	//拍照空间警告
#define MID_CAMERA_WARN1	1258	//拍照存储空间不足
#define MID_CAMERA_WARN2	1259	//拍照存储空间满
#define MID_VIDEO_SETTING	1260	//视频设置
#define MID_VIDEO_ADJUST	1261	//调整视频设置
#define MID_VIDEO_BRIGHTNESS	1262	//亮度
#define MID_VIDEO_CONTRAST	1263	//对比度
#define MID_VIDEO_QULITY	1264	//质量
#define MID_VIDEO_QLOW		1265	//低
#define MID_VIDEO_QMID		1266	//中
#define MID_VIDEO_QHIGHT	1267	//高
#define MID_PATH_ERROR		1268	//创建照片文件夹失败!
#define MID_CAPTURE_HINT	1269	//拍照成功!
#define MID_DELPHOTO_HINT	1270	//照片已删除
#define MID_DOWN_ATTPIC		1271	//下载考勤照片
#define MID_DOWN_BADPIC		1272	//下载黑名单照片
#define MID_CLEAR_DOWN		1273	//删除已下载照片
#define MID_NOCLEAR_DOWN	1274	//不删除已下载照片
#define MID_USB_UPLOAD		1275	//上传数据
#define MID_USB_DOWNLOAD	1276	//下载数据
#define MID_DOWNALL_PIC		1277	//下载全部照片
#define MID_FPIMG		1278	//指纹图像显示
#define MID_FPREGSHOW		1279	//登记显示
#define MID_FPVFSHOW		1280	//比对显示
#define MID_FPSHOW		1281	//登记,比对显示
#define MID_FPNONESHOW		1282	//指登记,比对不显示

#define MID_DAYLIGHTSAVINGTIMEON        1283  //夏令时
#define MID_DAYLIGHTSAVINGTIME          1284    //夏令时开始
#define MID_STANDARDTIME                1285    //夏令时结束
#define MID_RESTOREPARA                1286    //恢复全部出厂设置
#define MID_RESTORESHORTKEY		1287	//恢复键盘定义设置)
#define MID_RESTOREALARM		1288	//恢复闹铃设置
#define MID_RESTORELOCKFUN		1289	//恢复门禁设置
#define MID_RESTOREOTHER		1290	//恢复其他设置

#define MID_MUSTCHOICESTATE		1300	//请先选择工作状态!
#define MID_TIMEOFFOUT			1301	//TimeOff-OT
#define MID_TIMEOFFIN			1302	//TimeOff-IN
#define HIT_ERR_SELUSER     1303
#define MID_UPDATE_OPT      1334
#define MID_UPDATE_OPTFIR   1336
#define MID_UPFAILD_OPTFW   1337

#define MID_WIFI_ERR4		1339	//请设置密码 Liaozz 20081009 fix bug 1008:3
#define MID_WIFI_ERR5		1340	//请设置IP Liaozz 20081011 fix bug 1010:1

#define MID_MUST121	1400
#define MID_MUST121_NO	1401
#define MID_MUST121_YES 1402


//#define MID_IRSENSOR_BL_SWITCH  1403  //  采集器设置
//#define MID_IRSENSOR_BL_ON      1404  //  自动感应
//#define MID_IRSENSOR_BL_OFF     1405  //  常亮背光
#define MID_VIDEO_ROTATE 	1406

#define MID_TTS_OPEN        	1408	//TTS Open State when 2:00 or 3:00

#define MID_GPRS_SETTING        1415    //GPRS设置
#define MID_GPRS_FLOW           1416
#define MID_GPRS_AUTH_NORMAL    1417    //普通
#define MID_GPRS_AUTH_SECURE    1418    //安全
#define GPRS_INFO               1419
#define MID_MODEM_MODEL         1420    //Modem 型号
#define MID_MODEM_COPS          1421    //移动运行商
#define MID_MODEM_REGSTATUS     1422    //注册状态
#define MID_MID_PPP_FLOW        1349    //GPRS流量
#define MID_PPP_RESETFLOW       1424    //GPRS流量清零

#define MID_SIM_REG_NOT_SEARCH  1425
#define MID_SIM_REG_OK          1426
#define MID_SIM_REG_SEARCHING   1427
#define MID_SIM_REG_DENIED      1428
#define MID_SIM_REG_UNKNOWN     1429
#define MID_SIM_REG_ROAMING     1430
#define MID_SIM_REMOVED         1431
#define MID_SIM_PIN_ERROR       1432

#define MID_GPRS_DETACH         1433
#define MID_GPRS_DETACH_SURE    1434
#define MID_GPRS_ATTATCH        1435
#define MID_GPRS_ATTATCH_SURE   1436
#define MID_GPRS_RESTART        1437
#define MID_GPRS_RESTART_SURE   1438
#define MID_GPRS_TX             1365		//发送
#define MID_GPRS_RX             1366		//接收
#define MID_GPRS_DETACHED       1441
#define MID_PPP_FLOW_RESET      1442
#define MID_GPRS_ATTACHING      1443
#define MID_GPRS_SUCCESS        1444
#define MID_GPRS_FAIL           1445
#define MID_GPRS_RESTARTING     1446
#define MID_GPRS_NORNETWORK     1447
#define MID_GPRS_INIT_MODEM     1448
#define MID_GPRS_OPEN_ERR       1449
#define MID_GPRS_ATCMD_ERR      1450
#define MID_GPRS_DISCONNECTED   1451
#define MID_GPRS_DETACH_FAIL    1452
#define MID_GPRS_SIGNAL		1453

#define MID_DNS			1500
#define MID_DNS_ERROR		1501

#define MID_IP_MODEL		1502
#define MID_URL_MODEL		1503

//FOR 3g
#define MID_MODEM_TX             		1439		//发送
#define MID_MODEM_RX            	 	1440		//接收
#define MID_MODEM_MOBILENET		2003		//移动网络
#define MID_MODEM_HTSERVER		2004		//心跳服务器
#define MID_MODEM_UNAVAILABE		2005		//模块不可用
#define MID_MODEM_INITING			2006		//模块初始化中
#define MID_MODEM_INITFAIL			2007		//模块初始化失败
#define MID_MODEM_MODULE_READY	2008		//模块已就绪
#define MID_MODEM_SIM_UNREADY	2009		//SIM卡未就绪
#define MID_MODEM_SIM_UNDETECTE	2010		//未检测到SIM卡
#define MID_MODEM_SIM_LOCED		2011		//SIM被锁住
#define MID_MODEM_SIM_READY		2012		//SIM已就绪
#define MID_MODEM_CONNECTING		2013		//连接中。。。
#define MID_MODEM_UNKNOWNSTATUS	2014		//未知状态
#define MID_MODEM_NOSERVICE		2015		//无服务
#define MID_MODEM_CDMA20001X		2016		//CDMA20001X
#define MID_MODEM_CDMA2000EVDO	2017		//CDMA2000EVDO
#define MID_MODEM_MIXMODE			2018		//混合模式
#define MID_MODEM_UNKNOWE		2019		//未知
#define MID_MODEM_GSM				2020		//GSM
#define MID_MODEM_EDGE			2021		//EDGE
#define MID_MODEM_WCDMA			2022		//WCDMA
#define MID_MODEM_HSDPA			2023		//HSDPA
#define MID_MODEM_HSUPA			2024		//HSUPA
#define MID_MODEM_HSDPAHSUPA		2025		//HSDPAHSUPA
#define MID_MODEM_TDSCDMA		2026		//TDSCDMA
#define MID_MODEM_ONLINETIME		2027		//在线时间
#define MID_MODEM_DAY				2028		//天
#define MID_MODEM_HOUR			2029		//小时
//end 3g

//For face
#define MID_FACE                2119    //face
#define MID_FACE_VERIFY         2120    //face identify
#define FACE_REG                2121    //face register
#define FACE_REG_PREPARE        2122    //face register ,please prepare
#define FACE_CHG_POS            2123    //Please change posture or change distance
#define FACE_NEAR               2124    //Please near
#define FACE_FAR                2125    //Please far
#define FACE_EXPOSOURE          2126    //face exposoure
#define FACE_GLOBALGAIN         2127    //video gain
#define FACE_REG_FRONT          2128    //
#define FACE_REG_SCREEN         2129
#define FACE_REG_LEFT           2130
#define FACE_REG_RIGHT          2131
#define FACE_REG_CAMERA         2132
#define FACE_FACESET            2133    //face param setting
#define MID_STKEYFUN6           2134    //switch workmode
#define MID_G_FACE              2135    //1:G face
#define MID_DEF_FG              2136    //default face group
#define MID_ONE_FACE            2137    //1:1 face
#define MID_VF_MAIN             2138    //input group or pin2
#define MID_STKEYFUN10          2139    //face group 1
#define MID_STKEYFUN11          2140    //face group 2
#define MID_STKEYFUN12          2141    //face group 3
#define MID_STKEYFUN13          2142    //face group 4
#define MID_STKEYFUN14          2143    //face group 5
#define FACE_CUR_GROUP          2144    //cur group
#define MID_FACE_FINGER         2145    //face +finger
#define MID_FACE_REGMODE        2146    //face register
#define MID_FACE_REGFP          2147    //must register finger
#define MID_FACE_INFO           2148    //face info
#define MID_DEL_FACE            2149    //del facetemp
#define MID_FACEHADDEL          2150    //del facetemp ok
#define MID_DEL_ORGDATA         2151    //Wether to del. the original data
#define MID_FACE_TEST           2152    //face test
#define MID_FACE_SCORE          2153    //score
#define MID_FACE_SET            2154    //face param setting
#define MID_FACE_VTH            2155    //face 1:1 score
#define MID_FACE_MTH            2156    //face 1:G score
//end face

#define HIT_BAT_STR1        	2110
#define HIT_BAT_STR2        	2111
#define HIT_BAT_STR3        	2112
#define HIT_BAT_STR5        	2114
#define HIT_BAT_STR6        	2115
//web server
#define MID_WENSERVER_SETUP 	2116
#define MID_WENSERVER       	2117
#define MID_WENSERVER_PROXY 	2118


#define HID_PRI_ENROLL    	2802	//登记员
//485 Reader
#define HID_485READER           2896 //
#define HID_NOTOFF              2897 //can not trun off hint.

#define PID_PHOTO_MNG		2898//考勤照片管理
#define PID_PHOTO_DEL_TIME	2899//删除设定时间点之前照片
#define PID_PHOTO_DEL		2900//删除照片
#define PID_PHOTO_AUTO_CNT	2901//自动删除照片数
#define PID_PHOTO_NOTICE	2902//照片达到最大容量时自动删除照片

#define MID_SRB_FUN         2903//SRB功能

#define MID_PHOTO_UPLOAD	2904//照片上传
#define MID_PHOTO_SERVER	2905//照片服务器
#endif

