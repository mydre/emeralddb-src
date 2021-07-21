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

#ifndef OSSQUEUE_HPP__
#define OSSQUEUE_HPP__

#include<queue>
#include<boost/thread.hpp>//使用boost中的互斥量和条件变
#include<boost/thread/thread_time.hpp>


#include "core.hpp"
template<typename Data>
class ossQueue{
private:
	std::queue<Data> _queue;
	boost::mutex _mutex;
	boost::condition_variable _cond;
public:
	unsigned int size(){
		boost::mutex::scoped_lock lock(_mutex);
		return (unsigned int) _queue.size();
	}
	void push(Data const &data){
		boost::mutex::scoped_lock lock(_mutex);//初始化一个锁，（lock）
		_queue.push(data);
		lock.unlock();//解锁（push到队列之后就立即解锁）
		_cond.notify_one();

	}
	bool empty() {
		boost::mutex::scoped_lock lock(_mutex);
		return _queue.empty();
	}
	bool try_pop(Data &value){//传引用的方式
		boost::mutex::scoped_lock lock(_mutex);//初始化一个锁
		if(_queue.empty()){
			return false;
		}
		value = _queue.front();
		_queue.pop();
		return true;
	}
	void wait_and_pop(Data &value){
		boost::mutex::scoped_lock lock(_mutex);
		while(_queue.empty()){
			_cond.wait(lock);
		}
		value = _queue.front();
		_queue.pop();
	}

	bool timed_wait_and_pop(Data &value,long long milsec){
		boost::system_time const timeout  = boost::get_system_time() + boost::posix_time::milliseconds(milsec);
		boost::mutex::scoped_lock lock(_mutex);
		// if timed wait return false,that means we failed by timeout
		while(_queue.empty()){
			if(!_cond.timed_wait(lock,timeout)){
				return false;
			}
		}
		value = _queue.front();
		_queue.pop();
		return true;
	}
};

#endif
