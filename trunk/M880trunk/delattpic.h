/*************************************************
*************************************************/

#ifndef _DELATTPIC_H_
#define _DELATTPIC_H_

#include "arca.h"
#include "ccc.h"
#include "commu.h"


//define function
int FDB_CntPhotoCountByPar(int flag, PCmdHeader chdr);
int FDB_GetPhotoByName(char *p, PCmdHeader chdr, PCommSession session);
int FDB_ClearPhotoByTime(char *p, PCmdHeader chdr, PCommSession session);
int FDB_GetAllPhotoNamesByTime(char *p, PCmdHeader chdr, PCommSession session);

#endif
