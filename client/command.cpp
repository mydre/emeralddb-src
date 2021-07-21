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
#include "core.hpp"
#include "command.hpp"
#include "commandFactory.hpp"
#include "pd.hpp"
#include "msg.hpp"

COMMAND_BEGIN
COMMAND_ADD(COMMAND_CONNECT,ConnectCommand) //add的时候会创建新的Command类
COMMAND_ADD(COMMAND_QUIT,QuitCommand)
COMMAND_ADD(COMMAND_HELP,HelpCommand)
COMMAND_ADD(COMMAND_INSERT,InsertCommand)
COMMAND_ADD(COMMAND_DELETE,DeleteCommand)
COMMAND_ADD(COMMAND_QUERY,QueryCommand)
COMMAND_ADD(COMMAND_SNAPSHOT,SnapshotCommand)
COMMAND_END

extern int gQuit;

int ICommand::execute(ossSocket &sock,std::vector<std::string> &argVec){
	return EDB_OK;
}

int ICommand::getError(int code){
	switch(code){
	case EDB_OK:
		break;
	case EDB_IO:
		myP("io error is occurred");
		break;
	case EDB_INVALIDARG:
		myP("invalid argument");
		break;
	case EDB_PERM:
		myP("edb_perm");
		break;
	case EDB_OOM:
		myP("edb_oom");
		break;
	case EDB_SYS:
		myP("edb_sys");
		break;
	case EDB_QUIESCED:
		myP("edu_quiesced");
		break;
	case EDB_PMD_HELP_ONLY:
		myP("edb_network_close");
		break;
	case EDB_PMD_FORCE_SYSTEM_EDU:
		myP("edb_pmd_force_system_edu");
		break;
	case EDB_TIMEOUT:
		myP("edb_timeout");
		break;
	case EDB_EDU_INVAL_STATUS:
		myP("edb_edu_inval_status");
		break;
	case EDB_NETWORK:
		myP("edb_network");
		break;
	case EDB_NETWORK_CLOSE:
		myP("edb_network_close");
		break;
	case EDB_APP_FORCED:
		myP("edb_app_forced");
		break;
	case EDB_IXM_ID_EXIST:
		myP("edb_ixm_id_exist");
		break;
	case EDB_HEADER_INVALID:
		myP("edb_header_invalid");
		break;
	case EDB_IXM_ID_NOT_EXIST:
		myP("edb_ixm_id_not_exist");
		break;
	case EDB_QUERY_INVALID_ARGUMENT:
		myP("edb_query_invalid_argument");
		break;
	case EDB_INSERT_INVALID_ARGUMENT:
		myP("edb_insert_invalid_argument");
		break;
	case EDB_DELETE_INVALID_ARGUMENT:
		myP("edb_delete_invalid_argument");
		break;
	case EDB_INVALID_RECORD:
		myP("edb_invalid_record");
		break;
	case EDB_RECV_DATA_LENGTH_ERROR:
		myP("edb_recv_data_length_error");
		break;
	case EDB_SOCK_INIT_FAILED:
		myP("edb_sock__init_failed");
		break;
	case EDB_SOCK_CONNECT_FAILED:
		myP("edb_sock_connect_failed");
		break;
	case EDB_SOCK_NOT_CONNECT:
		myP("edb_sock_not_connect");
		break;
	case EDB_SOCK_REMOTE_CLOSED:
		myP("edb_sock_remote_closed");
		break;
	case EDB_SOCK_SEND_FAILED:
		myP("edb_sock_send_failed");
		break;
	case EDB_MSG_BUILD_FAILED:
		myP("edb_msg_build_failed");
		break;
	default:
		break;
	}
	return code;
}

int ICommand::recvReply(ossSocket &sock){
	//define message data length
	int length = 0;
	int ret = EDB_OK;
	//fill receive buffer with 0
	memset(_recvBuf,0,RECV_BUF_SIZE);
	if(!sock.isConnected()){
		return getError(EDB_SOCK_NOT_CONNECT);
	}
	while(1){
		ret = sock.recv(_recvBuf,sizeof(int));//第一步先接收4个字节
		if(EDB_TIMEOUT == ret){
			continue;
		}else if(EDB_NETWORK_CLOSE == ret){
			return getError(EDB_SOCK_REMOTE_CLOSED);
		}else{//正常接收信息，则跳出循环
			break;
		}
	}
	//get the value of length
	length = *(int *)_recvBuf;
	//judge the length is valid or not
	if(length > RECV_BUF_SIZE){
		return getError(EDB_RECV_DATA_LENGTH_ERROR);
	}
	//receive data from the server.second receive the last data.
	while(1){
		ret = sock.recv(&_recvBuf[sizeof(int)],length-sizeof(int));
		if(ret == EDB_TIMEOUT){
			continue;
		}else if(EDB_NETWORK_CLOSE == ret){
			return getError(EDB_SOCK_REMOTE_CLOSED);
		}else{
			break;
		}
	}
	return ret;
}


int ICommand::sendOrder(ossSocket &sock,OnMsgBuild onMsgBuild){//传引用&sock
	int ret = EDB_OK;
	bson::BSONObj bsonData;
	try{
		bsonData = bson::fromjson(_jsonString);//转成bson对象
	}catch(std::exception &e){
		return getError(EDB_INVALID_RECORD);
	}
	memset(_sendBuf,0,SEND_BUF_SIZE);//发送缓冲区清空
	int size = SEND_BUF_SIZE;
	char *pSendBuf = _sendBuf;
	ret = onMsgBuild(&pSendBuf,&size,bsonData);
	if(ret){
		return getError(EDB_MSG_BUILD_FAILED);
	}
	ret = sock.send(pSendBuf,*(int*)pSendBuf);
	if(ret){
		return getError(EDB_SOCK_SEND_FAILED);
	}
	return ret;
}

int ICommand::sendOrder(ossSocket &sock,int opCode){//这个是之前创建的假的sendOrder（做个msgHeader）
	int ret = EDB_OK;
	memset(_sendBuf,0,SEND_BUF_SIZE);
	char *pSendBuf = _sendBuf;
	//const char *pStr = "hello world";
	//*(int *)pSendBuf = strlen(pStr) + 1 + sizeof(int);//构造第一个int，表示长度
//	memcpy(&pSendBuf[4],pStr,strlen(pStr)+1);
	MsgHeader *header = (MsgHeader *)pSendBuf;
	header->messageLen = sizeof(MsgHeader);
	header->opCode = opCode;
	ret = sock.send(pSendBuf,*((int *)pSendBuf));
	return ret;
}

int ConnectCommand::execute(ossSocket &sock,std::vector<std::string> &argVec){
	int ret = EDB_OK;
	_address = argVec[0];
	_port = atoi(argVec[1].c_str());
	//printf("address:%s\n",_address.c_str());转化为c_str()之后不乱码
	//printf("port:%d\n",_port);
	sock.close(); // 来一个新的connect命令之后，我都会断开连接
	sock.initSocket();//这一步不可少。
	sock.setAddress(_address.c_str(),_port);
	if(ret){
		return getError(EDB_SOCK_INIT_FAILED);
	}
	ret = sock.connect();
	if(ret){
		return getError(EDB_SOCK_CONNECT_FAILED);
	}
	sock.disableNagle();
	return ret;
}

int QuitCommand::handleReply(){
	int ret = EDB_OK;
	gQuit = 1;
	return ret;
}
int QuitCommand::execute(ossSocket &sock,std::vector<string> & argVec){
	int ret = EDB_OK;
	if(!sock.isConnected()){
		printf("quitcommand failed,havn't connected\n");
		return getError(EDB_SOCK_NOT_CONNECT);
	}
	ret = sendOrder(sock,OP_DISCONNECT);
	sock.close();
	ret = handleReply();
	return ret;
}
/******************************InsertCommand**********************************************/
int InsertCommand::handleReply()
{
   MsgReply * msg = (MsgReply*)_recvBuf;
   int returnCode = msg->returnCode;
   int ret = getError(returnCode);
   return ret;
}

int InsertCommand::execute( ossSocket & sock, std::vector<std::string> & argVec )
{
   int rc = EDB_OK;
   if( argVec.size() <1 )
   {
      return getError(EDB_INSERT_INVALID_ARGUMENT);
   }
   _jsonString = argVec[0];
     if( !sock.isConnected() )
   {
      return getError(EDB_SOCK_NOT_CONNECT);
   }

   rc = sendOrder( sock, msgBuildInsert);
   PD_RC_CHECK ( rc, PDERROR, "Failed to send order, rc = %d", rc ) ;

   rc = recvReply( sock );
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
   rc = handleReply();
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
done :
   return rc;
error :
   goto done ;
}

int QueryCommand::handleReply()
{
   MsgReply * msg = (MsgReply*)_recvBuf;
   int returnCode = msg->returnCode;
   int ret = getError(returnCode);
   if(ret)
   {
      return ret;
   }
   if ( msg->numReturn )
   {
      bson::BSONObj bsonData = bson::BSONObj( &(msg->data[0]) );
      std::cout << bsonData.toString() << std::endl;
   }
   return ret;
}

int QueryCommand::execute( ossSocket & sock, std::vector<std::string> & argVec )//第二个参数是参数数组
{
   int rc = EDB_OK;
   if( argVec.size() <1 )
   {
      return getError(EDB_QUERY_INVALID_ARGUMENT);
   }
   _jsonString = argVec[0];//通过参数构建jsonstring
   if( !sock.isConnected() )
   {
      return getError(EDB_SOCK_NOT_CONNECT);
   }

   rc = sendOrder( sock, msgBuildQuery );//msgBuildQuery是一个函数指针（指针，指向的是一个函数）
   PD_RC_CHECK ( rc, PDERROR, "Failed to send order, rc = %d", rc ) ;
   rc = recvReply( sock );
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
   rc = handleReply();
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
done :
   return rc;
error :
   goto done ;
}

int DeleteCommand::handleReply()
{
   MsgReply * msg = (MsgReply*)_recvBuf;
   int returnCode = msg->returnCode;
   int ret = getError(returnCode);
   return ret;
}

int DeleteCommand::execute( ossSocket & sock, std::vector<std::string> & argVec )
{
   int rc = EDB_OK;
   if( argVec.size() < 1 )
   {
      return getError(EDB_DELETE_INVALID_ARGUMENT);
   }
   _jsonString = argVec[0];
   if( !sock.isConnected() )
   {
      return getError(EDB_SOCK_NOT_CONNECT);
   }
   rc = sendOrder( sock, msgBuildDelete );
   PD_RC_CHECK ( rc, PDERROR, "Failed to send order, rc = %d", rc ) ;
   rc = recvReply( sock );
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
   rc = handleReply();
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
done :
   return rc;
error :
   goto done ;
}

int SnapshotCommand::handleReply()
{
   int ret = EDB_OK;
   MsgReply * msg = (MsgReply*)_recvBuf;
   int returnCode = msg->returnCode;
   ret = getError(returnCode);
   if(ret)
   {
      return ret;
   }
   bson::BSONObj bsonData = bson::BSONObj( &(msg->data[0]) );
   printf( "insert times is %d\n", bsonData.getIntField("insertTimes") );
   printf( "del times is %d\n", bsonData.getIntField("delTimes") );
   printf( "query times is %d\n", bsonData.getIntField("queryTimes") );
   printf( "server run time is %dm\n", bsonData.getIntField("serverRunTimes") );

   return ret;
}

int SnapshotCommand::execute( ossSocket & sock, std::vector<std::string> &argVec)
{
   int rc = EDB_OK;
   if( !sock.isConnected() )
   {
      return getError(EDB_SOCK_NOT_CONNECT);
   }

   rc = sendOrder( sock, OP_SNAPSHOT );
   PD_RC_CHECK ( rc, PDERROR, "Failed to send order, rc = %d", rc ) ;
   rc = recvReply( sock );
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
   rc = handleReply();
   PD_RC_CHECK ( rc, PDERROR, "Failed to receive reply, rc = %d", rc ) ;
done :
   return rc;
error :
   goto done ;
}

int HelpCommand::execute(ossSocket &sock,std::vector<std::string> &argVec){
	int ret = EDB_OK;
	printf("List of classes of commands:\n\n");
	printf("%s [server] [port] -- connecting emeralddb server\n",COMMAND_CONNECT);
	printf("%s -- sending a insert command to emeralddb server\n",COMMAND_INSERT);
	printf("%s --sending a query command to emeralddb server\n",COMMAND_QUERY);
	printf("%s --sending a delete command to emeralddb server\n",COMMAND_DELETE);
	printf("%s [number] -- sending a test command to emeralddb server\n",COMMAND_TEST);
	printf("%s -- providing current number of record inserting\n",COMMAND_SNAPSHOT);
	printf("%s -- quiting command\n\n",COMMAND_QUIT);
	printf("Type \"help\" command for help\n");
	return ret;
}
