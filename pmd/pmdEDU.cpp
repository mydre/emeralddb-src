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

#include "pmdEDU.hpp"

#include "pmdEDUMgr.hpp"
#include "pmd.hpp"
#include "pd.hpp"

static std::map<EDU_TYPES,std::string> mapEDUName;
static std::map<EDU_TYPES,EDU_TYPES> mapEDUTypeSys;

int registerEDUName(EDU_TYPES type,const char *name,bool system){
	int rc = EDB_OK;
	std::map<EDU_TYPES,std::string>::iterator it = mapEDUName.find(type);
	if(it != mapEDUName.end()){
		PD_LOG(PDERROR,"EDU Type conflick[type:%d, %s<->%s]",(int)type,it->second.c_str(),name);
		rc = EDB_SYS;
		goto error;
	}
	mapEDUName[type] = std::string(name);
	if(system){
		mapEDUTypeSys[type] = type;
	}
done:
	return rc;
error:
	goto done;

}

const char *getEDUName(EDU_TYPES type){
	std::map<EDU_TYPES,std::string>::iterator it = mapEDUName.find(type);
	if(it != mapEDUName.end()){
		return it->second.c_str();
	}
	return "Unknown";
}

bool isSystemEDU(EDU_TYPES type){//找到了就是系统EDU
	std::map<EDU_TYPES,EDU_TYPES>::iterator it = mapEDUTypeSys.find(type);
	return it == mapEDUTypeSys.end()?false:true;
}

pmdEDUCB::pmdEDUCB(pmdEDUMgr *mgr,EDU_TYPES type):
		_type(type),
		_mgr(mgr),
		_status(PMD_EDU_CREATING),
		_id(0),
		_isForced(false),
		_isDisconnected(false)
		{}

struct _eduEntryInfo{//每种不同的EDU类型的入口函数
	EDU_TYPES type;
	int regResult;
	pmdEntryPoint entryFunc;
};

//使用宏定义映射为struct结构体中的是三个元素。采用大括号{}包围的内容大括号包围的内容就是struct的各个元素
#define ON_EDUTYPE_TO_ENTRY1(type,system,entry,desp)			\
{type,registerEDUName(type,desp,system),entry}


pmdEntryPoint getEntryFuncByType(EDU_TYPES type){//内部使用了静态局部变量
	pmdEntryPoint rt = NULL;
	static const _eduEntryInfo entry[] = {//由于元素的个数比较少，因此遍历开销不大，直接采用array的方式
			ON_EDUTYPE_TO_ENTRY1(EDU_TYPE_AGENT,false,pmdAgentEntryPoint,"Agent"),
			ON_EDUTYPE_TO_ENTRY1(EDU_TYPE_TCPLISTENER,true,pmdTcpListenerEntryPoint,"TCPListener"),
			ON_EDUTYPE_TO_ENTRY1(EDU_TYPEMAXIMU,false,NULL,"Unknown")
	};

	static const unsigned int number = sizeof(entry)/sizeof(_eduEntryInfo);//3
	unsigned int index = 0;
	for(;index <number;++index){
		if(entry[index].type == type){
			rt = entry[index].entryFunc;
			goto done;
		}
	}
done:
	return rt;
}

int pmdRecv ( char *pBuffer, int recvSize,
              ossSocket *sock, pmdEDUCB *cb )
{
   int rc = EDB_OK ;
   EDB_ASSERT ( sock, "Socket is NULL" ) ;
   EDB_ASSERT ( cb, "cb is NULL" ) ;
   while ( true )
   {
      if ( cb->isForced () )
      {
         rc = EDB_APP_FORCED ;
         goto done ;
      }
      rc = sock->recv ( pBuffer, recvSize ) ;
      if ( EDB_TIMEOUT == rc )
         continue ;
      goto done ;
   }
done :
   return rc ;
}
int pmdSend ( const char *pBuffer, int sendSize,
              ossSocket *sock, pmdEDUCB *cb )
{
   int rc = EDB_OK ;
   EDB_ASSERT ( sock, "Socket is NULL" ) ;
   EDB_ASSERT ( cb, "cb is NULL" ) ;
   while ( true )
   {
      if ( cb->isForced () )
      {
         rc = EDB_APP_FORCED ;
         goto done ;
      }
      rc = sock->send ( pBuffer, sendSize ) ;
      if ( EDB_TIMEOUT == rc )
         continue ;
      goto done ;
   }
done :
   return rc ;
}

int pmdEDUEntryPoint(EDU_TYPES type,pmdEDUCB *cb,void *arg){
	int rc = EDB_OK;
	//EDB_KRCB *krcb = pmdGetKRCB();
	EDUID myEDUID = cb->getID();
	pmdEDUMgr *eduMgr = cb->getEDUMgr();
	pmdEDUEvent event;
	bool eduDestroyed = false;
	bool isForced = false;
	//main loop
	while(!eduDestroyed){
		type  = cb->getType();
		//wait for 1000 milisecnds for event
		if(!cb->waitEvent(event,1000)){//有一个大的while循环，可以一直不停等待时间！！！，在idle状态的时候进行等待！！！
			if(cb->isForced()){
				PD_LOG(PDEVENT,"EDU %lld is forced",myEDUID);
				isForced = true;
			}else{
				continue;
			}
		}
		if(!isForced && PMD_EDU_EVENT_RESUME == event._eventType){//如果事件的状态是RESUME
			//set EDU status to wait
			eduMgr->waitEDU(myEDUID);
			//run the main function
			pmdEntryPoint entryFunc = getEntryFuncByType(type);
			if(!entryFunc){
				PD_LOG(PDERROR,"EDU %lld type %d entry point func is NULL",myEDUID,type);
				EDB_SHUTDOWN_DB
				rc = EDB_SYS;
			}else{
				rc = entryFunc(cb,event._Data);
			}
			// sanity check
			if(EDB_IS_DB_UP){
				//system EDU should neve exit when db is still running
				if(isSystemEDU(cb->getType())){
					PD_LOG(PDSEVERE,"SystemEDU:%lld,type %s exits with %d",myEDUID,getEDUName(type),rc);
					EDB_SHUTDOWN_DB
				}else if(rc){
					PD_LOG(PDWARNING,"EDU %lld, type %s, exits with %d",myEDUID, getEDUName(type),rc);
				}
			}
			eduMgr->waitEDU(myEDUID);
		}
		else if(!isForced && PMD_EDU_EVENT_TERM  != event._eventType){
			PD_LOG(PDERROR, "Receive the wrong event %d in EDU %lld,type %s",event._eventType,myEDUID,getEDUName(type));
			rc = EDB_SYS;
		}else if(isForced && PMD_EDU_EVENT_TERM == event._eventType && cb->isForced()){
			PD_LOG(PDEVENT, "EDU %lld, type %s is forced",myEDUID,type);
			isForced = true;
		}
		//release th event data,释放事件
		if(!isForced && event._Data && event._release){
			free(event._Data);
			event.reset();
		}

		rc = eduMgr->returnEDU(myEDUID,isForced,&eduDestroyed);//本线程处理完了之后，返回给线程池，设置为idle状态!!!
		if(rc){
			PD_LOG(PDERROR,"Invalid EDU Status for EDU: %lld,type=%s",myEDUID,getEDUName(type));
		}
		PD_LOG(PDDEBUG,"Terminating thread for EDU: %lld,type=%s",myEDUID,getEDUName(type));
	}
	return 0;
}
