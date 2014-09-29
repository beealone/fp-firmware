#ifndef _Web_Firmware
#define _Web_Firmware
//PZKFPTemplate FDB_CreateTemplate(char* tmp, U16 pin, char FingerID, char *Template, int TmpLen,int Valid)
typedef struct _tag_FWCBT
{
	int (*SaveStr)(const char *name, const char *value, int SaveTrue);
	void (*ExAuxOut)(int AuxOnTime, int OpenDoorDelay);
	int (*ExPowerOff)(int Cmd);
	int (*FDB_InitDBs)(int OpenSign);
	int (*FDB_ClrUser)(void);
	int (*FDB_ClrAdmin)(void);
	int (*FDB_ClrAttLog)(void);	
	int (*FDB_ClrTmp)(void);
	int (*FDBClrOPLog)(void);
	int (*FDB_CntAdmin)(int Privillege);
	int (*FDB_CntUser)(void);
	int (*FDB_CntAdminUser)(void);
	int (*FDB_CntPwdUser)(void);
	int (*FDB_CntTmp)(void);
	int (*FDB_CntAttLog)(void);
	int (*FDB_CntOPLog)(void);
	unsigned int (*FDB_GetTmpCnt)(unsigned short UID);
	int (*FDB_DeleteTmps)(unsigned short PIN);
	int (*FDB_DelUser)(unsigned short PIN);
	int (*FDB_AddUser)(PUser user);
	int (*FDB_ChgUser)(PUser user);
	PUser (*FDB_GetUser)(unsigned short PIN,PUser user);
	PZKFPTemplate (*FDB_CreateTemplate)(char *tmp, unsigned short PIN, char FingerID, char *Template, int TmpLen,int Valid);
	int (*FDB_AddTmp)(char *tmp);
	unsigned int (*FDB_GetTmp)(unsigned short PIN, char FingerID, char* tmp);
	int (*FPDBInit)(void);
	struct tm* (*OldDecodeTime)(time_t t, struct tm *ts);
	void (*RebootMachine)(void);
	time_t (*OldEncodeTime)(struct tm *t);
	unsigned short (*GetNextPIN)(int From, int Free);
	unsigned long (*GetLastAttLog)(TAttLog **OutBuffer,time_t dt);
	void (*ClearLastAttLog)(void);
	int (*AppendUserTemp)(int pin, char *name, int fingerid, char *temp, int tmplen, int valid);
	int (*SetRTCClock)(struct tm *tm);
	int (*ReadRTCClockToSyncSys)(struct tm *tm);
}
FWCBT;

#endif

