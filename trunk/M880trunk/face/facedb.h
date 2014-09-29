#ifndef _FACE_DB_H_
#define _FACE_DB_H_

#include <time.h>
#include "ccc.h"
#include "zkface.h"



#define FCT_FACE     	(U8)60
#define FCT_FACEPOS     (U8)61


//Face FDB Error
#define FDB_FACE_LIMIT          1

#define FACE_NUM                15
#define FACE_LEARN_NUM          3
#define  FACE_LEARN_ID		31

#define FACETEMPS_NUM           (FACE_LEARN_NUM+FACE_NUM)
#define MAX_BUFFER_SIZE         (FACETEMPS_NUM*sizeof(TFaceTmp))

#define FACE_MEM_MAX            sizeof(TFaceTmp)*60

#define FACE_TMP_MAXSIZE        (1024*2+512)

#define FACE_IMG_W	480
#define FACE_IMG_H	640
#define FACE_BMP_SIZE           (FACE_IMG_W*FACE_IMG_H)

extern int fdface;
extern int fdfacepos;

extern int gFaceGroupCnt;
extern int gFaceDefGroupCnt;
extern int gFaceGroupNum;
extern int CurFaceGroup;
extern unsigned char MulGroup;
extern unsigned char FaceDBChg;
extern BOOL FaceInit;



typedef struct _FaceTmp_{
        U16 Size;                       //face template size
        U16 PIN;                        //user ID
        BYTE FaceID;                  //Face id
        BYTE Valid;                     // flag
        U16 Reserve;                    //Reserve
        int ActiveTime;                 //Last active time 
        int VfCount;                    // Verify Count
        BYTE Face[FACE_TMP_MAXSIZE];    //maximize template length
}GCC_PACKED TFaceTmp, *PFaceTmp;



typedef struct _FacePos_{
        U16 PIN;
        BYTE FaceID;
        BYTE Valid;
        int RecordNum;
}GCC_PACKED TFacePos, *PFacePos;


typedef struct _UserCash_{
        U8 Group;	//Group
	char PIN2[24];	//pin2
}GCC_PACKED TUserCash, *PUserCash;


typedef struct _FGHandle__{
        void* Handle;	
	BYTE Group;
	int Count;
}GCC_PACKED TFGHandle, *PFGHandle;


int FaceDBInit(void);
int FaceDBFree(void);
int FaceDBReset(void);
int Cache_filter(void *handle, const char *id);
U32 FDB_GetFaceRecord(U16 UID, char FaceID , PFacePos tmp);
int DeleteFaceRecordData(PFacePos Facepos);
U32 FDB_DelFaceRecord(U16 UID, char FaceID);
U32 FDB_DelUserFaceRecords(U16 UID);
int ReadFaceTmpData(char*buf, int size, int offset);
int WriteFaceTmpData(char*buf, int size, int offset);
U32 FDB_GetFaceTmp(U16 UID, char FaceID , PFaceTmp tmp);
int FDB_AddFaceTmp(void* tmp, int Cnt);
int FDB_DeleteFaceTmps(U16 UID);
int ChangeFaceTmp(U16 pin, char id, PFaceTmp tmp);
int FDB_ClrFaceTmp(void);
int FDB_CntFaceUser(void);
int GetUserFreePINAndFaceGroup(char* Freepin2, int* FreeGroup,int mode);
int GetFaceGroupCnt(int Group);
int FDB_LoadUserFaceTmps(int pin, void* Handle);
int FDB_LoadFaceDefGroup(void);
int FDB_LoadFaceGroupTmp(int Group);
int FDB_LoadFaceTmpByGroup(int Group, void* Handle);
int FDB_LoadAllFaceTmp(void);
int VerifyFaceTemps(char* SFace, int size, PUser user);
int IdentifyFaceTemp(void* Handle, char * SFace , int size);
int ChangeFaceTmp(U16 pin, char id, PFaceTmp tmp);
int FDB_GetUserFaceNum(void);
int FDB_GetUserFaceTemps(int pin, char*buf);
int GetUserFaceInfoForListview(PUserlb header);



/***************************************************************
*?û??ǼǵĴ???????????
*
*****************************************************************/
#define  RET_CONTINUE	1
#define  RET_FAILED	-1
#define  RET_SUCCESS	2
#define	 RET_SAME	3
#define	 RET_ERROR	0

#define	 REG_PREPARE	0
#define  REG_START	1
#define	 REG_MERGE_OK	2


extern char Extracttmp[FACE_TMP_MAXSIZE];
extern TFaceTmp regface[FACE_NUM];
extern unsigned char FaceCount;
extern int FaceLen;
extern int RegBegin;
extern int RegCnt;

extern unsigned short face_bmp[FACE_BMP_SIZE];

/********   Face Enroll   ************/
int  InitFaceEnroll(void);
int FreeFaceEnroll(void);
int RegFaceAloneTemplate(int pin);
int RegFaceTmpProc(int pin);

/***************************************************************
*?û?ʶ???Ĵ???à?غ???
* Face Verify
*****************************************************************/
#define HINT_FPRE  	      	1
#define HINT_FGOING         	2
#define HINT_FCHG        	3
#define HINT_FFAR       	4
#define HINT_FNEAR	       	5
#define HINT_REGOK	       	6
#define HINT_REGFAIL	       	7
#define HINT_VERIFY	       	8
#define HINT_VERIFYP		9
#define HINT_FTESTOK		10
#define HINT_FTESTFAILED	11
#define HINT_CHG		0xff

extern int admflag;
extern int FaceVScore;
extern int FacePin;

#if 0
extern int Quality[3];
extern int positions[MAX_POSITION_NUMBER];
#endif

/**********  face verify  ************/
void LoadFaceWindowHint(void);
int VerifyFaceWithTmp( char* pin2 ,char* tmpface , int size);



/************  Face Extract ***************/
#define MENUBAR_HEIGHT		30
#define IDC_TIMER_NUM		60

extern void* hFaceExtact;
extern BOOL FaceTest;
extern int FaceStatus;
extern int FaceDistance;
extern int FaceQuality;
extern int Position[MAX_POSITIOV_NUMBER];
extern int Quality[3];
extern int FaceGray;

int CameraOpen(int size);
int FaceExtactDBInit(void);
void ChangeExposal( int mode);
int CaptureSensorFace(char *Buffer, BOOL Sign,int* ImageSize);
int CheckFaceDistance(int* Distance, int* positions, int gray ,int quality);
int FaceDetection(char* face_img);
int FaceExtractProc(void);
void SetFaceExtactParams(void);

/**********  face other extern *************/
#define FACE_ENROLL_PROC        0
#define FACE_VERIFY_PROC        1

#define FACE_PROCTYPE_SHOW	1
#define FACE_PROCTYPE_HIDE	0

#define FACE_NO_FACE		0
#define FACE_NO_EYES		1
#define FACE_NO_TMP		2
#define FACE_FETCH_OK		3

extern unsigned char facevfwin;

/**********  other  *****************/
int Bmp8ToRGB565(unsigned char* bmpfile, int w, int h);
int FPBmpToRGB565(char* bmpfile, int w, int h);
void FreeFaceVerifyBMP(void);
int LoadFaceVerifyBMP(void);
int Cache_filter(void *handle, const char *id);

#define IFACE_OUTDOOR_EXP	280	//100
#define IFACE_OUTDOOR_GAIN	80
#define IFACE_INDOOR_EXP	180*2
#define IFACE_INDOOR_GAIN	100

#endif





