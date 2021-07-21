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

#ifndef OSSLATCH_HPP__
#define OSSLATCH_HPP__
#include<pthread.h>
enum OSS_LATCH_MODE{
	SHARED,
	EXCLUSIVE
};
class ossXLatch{
private:
	pthread_mutex_t _lock;
public:
	ossXLatch(){
		pthread_mutex_init(&_lock,0);
	}
	~ossXLatch(){
		pthread_mutex_destroy(&_lock);
	}
	void get(){
		pthread_mutex_lock(&_lock);
	}
	void release(){
		pthread_mutex_unlock(&_lock);
	}
	bool try_get(){
		return (pthread_mutex_trylock(&_lock) == 0);
	}
};

class ossSLatch{
private:
	pthread_rwlock_t _lock;
public:
	ossSLatch(){
		pthread_rwlock_init(&_lock,0);
	}
	~ossSLatch(){
		pthread_rwlock_destroy(&_lock);//析构函数中释放锁
	}
	void get(){
		pthread_rwlock_wrlock(&_lock);
	}
	void release(){
		pthread_rwlock_unlock(&_lock);
	}
	bool try_get(){//之后write lock需要try get
		return (pthread_rwlock_trywrlock(&_lock) == 0);
	}
	void get_shared(){//共享锁直接get
		pthread_rwlock_rdlock(&_lock);
	}
	void release_shared(){
		pthread_rwlock_unlock(&_lock);
	}
};


#endif
