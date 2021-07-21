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
#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include "core.hpp"
#include <bson/src/util/json.h>
#include "ossSocket.hpp"

#define COMMAND_QUIT "quit"
#define COMMAND_INSERT "insert"
#define COMMAND_QUERY "query"
#define COMMAND_DELETE "delete"
#define COMMAND_HELP "help"
#define COMMAND_CONNECT "connect"
#define COMMAND_TEST "test"
#define COMMAND_SNAPSHOT "snapshot"

#define RECV_BUF_SIZE 4096
#define SEND_BUF_SIZE 4096

#define EDB_QUERY_INVALID_ARGUMENT -101
#define EDB_INSERT_INVALID_ARGUMENT -102
#define EDB_DELETE_INVALID_ARGUMENT -103

#define EDB_INVALID_RECORD -104
#define EDB_RECV_DATA_LENGTH_ERROR -107

#define EDB_SOCK_INIT_FAILED -113
#define EDB_SOCK_CONNECT_FAILED -114
#define EDB_SOCK_NOT_CONNECT -115
#define EDB_SOCK_REMOTE_CLOSED -116
#define EDB_SOCK_SEND_FAILED -117

#define EDB_MSG_BUILD_FAILED -119
class ICommand{
	typedef int (*OnMsgBuild) (char **ppBuffer,int *pBufferSize,bson::BSONObj &obj);
public:
	virtual int execute(ossSocket &sock,std::vector<std::string> &argVec);//virtual可以结合多态来使用
	int getError(int code);
	virtual ~ICommand(){}//虚析构函数
protected:
	int recvReply(ossSocket & sock);
	int sendOrder(ossSocket &sock,OnMsgBuild onMsgBuild);
	int sendOrder(ossSocket &sock,int opCode);
protected:
	virtual int handleReply(){return EDB_OK;}
protected:
	char _recvBuf[RECV_BUF_SIZE];
	char _sendBuf[SEND_BUF_SIZE];
	std::string _jsonString;
};
class ConnectCommand : public ICommand{
public:
	int execute(ossSocket &sock,std::vector<std::string>&argVec);//虽然前面没有声明virtual，但是确实是一个virtual函数
	~ConnectCommand(){}
private:
	std::string _address;
	int _port;
};

class QuitCommand : public ICommand{
public:
	int execute(ossSocket &sock,std::vector<std::string> &argVec);
	~QuitCommand(){}
protected:
	int handleReply();
};

class HelpCommand : public ICommand{
public:
	int execute(ossSocket & sock,std::vector<std::string> &argVec);
	~HelpCommand(){}//虚析构函数
};

class InsertCommand : public ICommand{
public:
	int execute(ossSocket & sock,std::vector<std::string> &argVec);
	~InsertCommand(){}//不写也行，应该是继承了
protected:
	int handleReply();
};

class DeleteCommand : public ICommand{
public:
	int execute(ossSocket &sock,std::vector<std::string> &argVec);
	//~QueryCommand(){}(由于会继承父类的虚构函数，所以这里不需要写)
protected:
	int handleReply();
};

class QueryCommand : public ICommand{
public:
	int execute(ossSocket &sock,std::vector<std::string> &argVec);
	//~QueryCommand(){}(由于会继承父类的虚构函数，所以这里不需要写)
protected:
	int handleReply();
};

class SnapshotCommand : public ICommand{
public:
	int execute(ossSocket &sock,std::vector<std::string> &argVec);
protected:
	int handleReply();
};
#endif
