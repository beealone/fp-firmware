#ifndef __SSRACC__H__
#define __SSRACC__H__

int TestUserTZ(int UserID, TTime gCurTime);
int TestOpenLock(int UID, TTime t, int VerifyMethod);
int SaveUserGrp(int UserID, int GrpID);
int GetUserGrp(int UserID);
int GetFirstTimeZone(void);
int GetFirstGroup(void);
void ClearMemberRecord(void);
#endif
