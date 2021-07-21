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

#ifndef RTN_HPP__
#define RTN_HPP__

#include "bson.h"
#include "dms.hpp"
// define the storage file name
#define RTN_FILE_NAME "data.1"

class rtn{
private:
	dmsFile *_dmsFile;
public:
	rtn();
	~rtn();
	int rtnInitialize();
	int rtnInsert(bson::BSONObj &record);
	int rtnFind(bson::BSONObj &inRecord,bson::BSONObj &outRecord);
	int rtnFind(bson::BSONObj &record);
	int rtnRemove(bson::BSONObj &record);
};
#endif
