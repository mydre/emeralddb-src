/*********************************************************************************************
The GNU Affero General Public License is a free, copyleft license for software and other kinds of works,
specifically designed to ensure cooperation with the community in the case of network server software.
The licenses for most software and other practical works are designed to take away your freedom to share and
change the works. By contrast, our General Public Licenses are intended to guarantee your freedom to share
and change all versions of a program--to make sure it remains free software for all its users.
When we speak of free software, we are referring to freedom, not price. Our General Public Licenses are
designed to make sure that you have the freedom to distribute copies of free software
(and charge for them if you wish), that you receive source code or can get it if you want it,
that you can change the software or use pieces of it in new free programs, and that you know you can do
these things.Developers that use our General Public Licenses protect your rights with two steps:
(1) assert copyright on the software, and (2) offer you this License which gives you legal permission to copy,
distribute and/or modify the software.
**********************************************************************************************/

//时间信息，当前进程的id。
#include "core.hpp"
#ifndef OSSUTIL_HPP__
#define OSSUTIL_HPP__
inline void ossSleepmicros(unsigned int s)//等待多少个微秒
{
	struct timespec t;//第一个参数是秒，第二个参数是微秒
	t.tv_sec = (time_t) (s/10000000);//设置秒
	t.tv_nsec = 1000 * (s%1000000);//设置微秒
	while(nanosleep(&t,&t) == -1 && errno == EINTR);
}

inline void ossSleepmillis(unsigned int s){//毫秒
	ossSleepmicros(s * 1000);
}

typedef pid_t 		OSSPID;//进程id
typedef pthread_t	OSSTID;//线程id

inline OSSPID ossGetParentProcessID(){
	return getpid();
}

inline OSSTID ossGetCurrentThreadID(){
	return syscall(SYS_gettid);//获取tid的系统调用
}
#endif
