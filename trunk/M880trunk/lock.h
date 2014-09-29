#ifndef __LOCK_H__
#define __LOCK_H__

#define DB_LOCK "db.lock"

int lockStart(const char *lockName);
int lockEnd(const char *lockName);

#endif
