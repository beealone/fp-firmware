#ifndef _FLASHDB2_H_
#define _FLASHDB2_H_
//给出一个用户的字符串格式的考勤号码(用于显示和数据交换)
//不同的TUser数据格式请修改该函数
char *fmtPin(TUser *user);

//给出一个用户的字符串格式的考勤号码(用于显示和数据交换)
//不同的TLog数据格式请修改该函数
char *fmtPinLog(TAttLog *log);

//遍历一个表时，用于操作记录的函数
//typedef int (*ForARecFun)(void *rec, int count, void *param);

//遍历一个表，对每一条记录进行操作
//fun 指定了对记录操作的函数
//param 用于向操作函数传递参数
#define NO_LIMIT	-1
//int FDB_ForAllData(int ContentType, ForARecFun fun, int MaxRecCount, void *param);

int FDB_DelOldAttLog(int delCount);

int IsUserDataNeedDownload(char *pin, TUser *user);

int IsUserDataNeedUpload(TUser *user);
#endif //#ifndef _FLASHDB_H_
