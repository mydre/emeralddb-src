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

#ifndef MSG_HPP__
#define MSG_HPP__

#include "bson.h"

#define OP_REPLY 1
#define OP_INSERT 2
#define OP_DELETE 3
#define OP_QUERY 4
#define OP_COMMAND 5
#define OP_DISCONNECT 6
#define OP_CONNECT 7
#define OP_SNAPSHOT 8

#define RETURN_CODE_STATE_OK 1

struct MsgHeader{
	int messageLen;
	int opCode;
};

struct MsgReply{
	MsgHeader header;
	int returnCode;
	int numReturn;
	char data[0];//标记，不占位，真实的用户数据是从哪个数据开始的
};

struct MsgInsert{
	MsgHeader header;
	int numInsert;
	char data[0];
};

struct MsgDelete{
	MsgHeader header;
	char key[0];
};

struct MsgQuery{
	MsgHeader header;
	char key[0];
};
struct MsgCommand{
	MsgHeader header;
	int numArgs;
	char data[0];
};

int msgBuildReply(char **ppBuffer,int *pBufferSize,int returnCode,bson::BSONObj *objReturn);

int msgExtractReply(char *pBuffer,int &returnCode,int &numReturn,const char **ppObjStart);

int msgBuildInsert(char **ppBuffer,int *pBufferSize,bson::BSONObj &obj);

int msgBuildInsert(char **ppBuffer,int *pBufferSize,vector<bson::BSONObj*> &obj);

int msgExtractInsert(char *pBuffer,int &numInsert,const char **ppObjStart);

int msgBuildDelete(char **pBuffer,int *pBufferSize,bson::BSONObj &key);

int msgBuildQuery(char **ppBuffer,int *pBufferSize,bson::BSONObj &key);

int msgExtractQuery(char *pBuffer,bson::BSONObj &key);

int msgBuildCommand(char **ppBuffer, int *pBufferSize,bson::BSONObj &obj);

int msgBuildCommand(char **ppBuffer, int *pBufferSize,vector<bson::BSONObj *> &obj);

int msgExtractCommand(char *pBuffer, int &numArgs,const char **ppObjStart);

#endif
