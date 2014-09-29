/* 
* SSR 2.0 Self Service Record 主入口头文件
* 设计：CWX        2007.1.5
* 原始版本:1.0.0   
* 修改记录:
* 编译环境:mipsel-gcc
*/


#include "msg.h"

long HootMsg(long *msg,long *wp,long *lp)
{
	TMsg Fwmsg;
	if(GatherMsgs(&Fwmsg))
	{
		*msg=Fwmsg.Message;
		*wp=Fwmsg.Param1;
		*lp=Fwmsg.Param2;
		return 1;
	}
	return 0;
}

