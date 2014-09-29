#ifndef _TTS_H
#define _TTS_H

#include <stdio.h>

#define TTS_API

void TTS_API TTS_SetV(int value);
void TTS_API TTS_SetE(int value);
void TTS_API TTS_SetS(int value);
void TTS_API TTS_SetT(int value);
void TTS_API TTS_SetChinese(void);
void TTS_API TTS_Say(char* lpChsTxt);
void TTS_API TTS_Stop(void);
void TTS_API TTS_Init(char *Name);
void TTS_API TTS_Free(void);
void TTS_API TTS_SetVol(int vol);
void TTS_API TTS_Wait(void);
void TTS_API TTS_PlayWav(char *Name);

//Add New Function By 2009-10-13
long TTS_API TTS_OpenSound(void);
void TTS_API TTS_CloseSound(void);


#define AdjTTSVol(AudioVol) ((AudioVol)>40?40:(AudioVol))

//#define OpenSoundDevice()
//#define CloseSoundDevice()

#define MaxTTSID 129

#define TTS_CAPTION 128
#define TTS_LOGO_VOICE 127
#define TTS_SMS_VOICE 126
#define TTS_SHUTDOWN_VOICE 125

#define TTS_SELADMIN_VOICE 124
#define TTS_TMPFULL_VOICE 123
#define TTS_INPUTPIN_VOICE 122
#define TTS_INPUTNAME_VOICE 121
#define TTS_ENROLLFP_VOICE 120
#define TTS_ENROLLPWD_VOICE 119
#define TTS_ENROLLOK_VOICE 118
#define TTS_PRESSFP_VOICE 117
#define TTS_LEAVEFP_VOICE 116
#define TTS_PRESSFP1_VOICE 115
#define TTS_PRESSFP2_VOICE 114
#define TTS_ENROLLSAVE_VOICE 113
#define TTS_REFP_VOICE 112
#define TTS_AATTLOG_VOICE 111
#define TTS_T9INFO_VOICE 110

#define TTS_SETTING 586
#define TTS_INFO_S 587
#define TTS_INFO_T 588
#define TTS_INFO_E 589
#define TTS_VERIFY_INFO 590
#define TTS_VERIFY_1 591
#define TTS_VERIFY_2 592
#define TTS_VERIFY_3 593
#define TTS_VERIFY_4 594
#define TTS_SMS_INFO 595
#define TTS_LOGO_INFO 596
#define TTS_ENROLL_INFO 597
#define TTS_REC_INFO 598
#define TTS_KEY_INFO 599
#define TTS_STATE_INFO 600
#define TTS_TIME_INFO 601
#define TTS_TIME_INFO1 602
#define TTS_TIME_INFO2 603
#define TTS_TS_INFO 604
#define TTS_TS_NEW 605
#define TTS_TS_DEL 606
#define TTS_TS_NAME 607
#define TTS_TS_START 608
#define TTS_TS_END 609
#define TTS_TS_CONTENT 610
#define TTS_TS_FULL 611
#define TTS_TS_DEL_INFO 612
#define TTS_MENU_INFO 613
#define TTS_VOICE_DOWNLOAD 614
#define TTS_VOICE_UPLOAD 615
#define TTS_VOICE_REST 616
#define TTS_REST_INFO 617
#define TTS_TS_ITEM 618

#endif 
