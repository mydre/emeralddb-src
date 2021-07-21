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

#ifndef PMDEDU_HPP__
#define PMDEDU_HPP__

#include "core.hpp"
#include "pmdEDUEvent.hpp"
#include "ossQueue.hpp"//之前定义的，（互斥锁+条件变量）(这个是Event队列，不是线程池队列)
#include "ossSocket.hpp"

#define PMD_INVALID_EDUID 0
#define PMD_IS_EDU_CREATING(x)(PMD_EDU_CREATING == x)
#define PMD_IS_EDU_RUNNING(x)(PMD_EDU_RUNNING == x)
#define PMD_IS_EDU_WAITING(x)(PMD_EDU_WAITING == x)
#define PMD_IS_EDU_IDLE(x)(PMD_EDU_IDLE == x)
#define PMD_IS_EDU_DESTROY(x)(PMD_EDU_DESTROY == x)

typedef unsigned long long EDUID;

enum EDU_TYPES{//要么是监听，要么是代理
	EDU_TYPE_TCPLISTENER = 0,
	EDU_TYPE_AGENT,
	EDU_TYPE_UNKNOWN,
	EDU_TYPEMAXIMU = EDU_TYPE_UNKNOWN
};

enum EDU_STATUS{
	PMD_EDU_CREATING = 0,
	PMD_EDU_RUNNING,
	PMD_EDU_WAITING,
	PMD_EDU_IDLE,
	PMD_EDU_DESTROY,
	PMD_EDU_UNKNOWN,
	PMD_EDU_STATUS_MAXIMUM = PMD_EDU_UNKNOWN
};
class pmdEDUMgr;
class pmdEDUCB{
public:
	pmdEDUCB(pmdEDUMgr *mgr,EDU_TYPES type);
	inline EDUID getID(){
		return _id;
	}
	inline void postEvent(pmdEDUEvent const &data){
		_queue.push(data);
	}
	bool waitEvent(pmdEDUEvent &data,long long millsec){//等待成功了就返回true，否则返回false
		//if millsec is not 0,that means we want timeout
		bool waitMsg = false;
		if(PMD_EDU_IDLE != _status){
			_status = PMD_EDU_WAITING;//不是空闲状态，则设置为等待状态
		}
		if(0 > millsec){
			_queue.wait_and_pop(data);//阻塞等待
			waitMsg = true;
		}else{//超时等待
			waitMsg = _queue.timed_wait_and_pop(data,millsec);//定义了最多阻塞的时长
		}
		if(waitMsg){
			if(data._eventType == PMD_EDU_EVENT_TERM){//如果事件类型是terminate
				_isDisconnected = true;
			}else{//如果事件类型不是terminating，就把EDU设置为running状态
				_status = PMD_EDU_RUNNING;
			}
		}
		return waitMsg;//返回等待的结果
	}
	inline void force(){//向其他线程发送命令
		_isForced = true;
	}
	inline void disconnect(){
		_isDisconnected = true;
	}
	inline EDU_TYPES getType(){
		return _type;
	}
	inline EDU_STATUS getStatus(){
		return _status;
	}
	inline void setType(EDU_TYPES type){
		_type = type;
	}
	inline void setID(EDUID id){
		_id = id;
	}
	inline void setStatus(EDU_STATUS status){
		_status = status;
	}
	inline bool isForced(){
		return _isForced;
	}
	inline pmdEDUMgr *getEDUMgr(){
		return _mgr;
	}
private:
	EDU_TYPES _type;
	pmdEDUMgr *_mgr;
	EDU_STATUS _status;
	EDUID _id;
	bool _isForced;
	bool _isDisconnected;
	ossQueue<pmdEDUEvent> _queue;//ossQueue和事件(event)相关！！
};

typedef int (*pmdEntryPoint)(pmdEDUCB *,void *);
pmdEntryPoint getEntryFuncByType(EDU_TYPES type);

int pmdAgentEntryPoint(pmdEDUCB *cb,void *arg);
int pmdTcpListenerEntryPoint (pmdEDUCB *cb,void *arg);
int pmdEDUEntryPoint (EDU_TYPES type,pmdEDUCB *cb,void *arg);

int pmdRecv(char *pBuffer,int recvSize,ossSocket *sock,pmdEDUCB *cb);
int pmdSend(const char *pBuffer,int sendSize,ossSocket *sock,pmdEDUCB *cb);
#endif
