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

#include "rtn.hpp"
#include "pd.hpp"
#include "core.hpp"
#include "pmd.hpp"

using namespace bson;

rtn::rtn():
_dmsFile(NULL){
}

rtn::~rtn(){
	if(_dmsFile){
		delete _dmsFile;
	}
}

int rtn::rtnInitialize(){
	int rc = EDB_OK;
	_dmsFile = new(std::nothrow)dmsFile();
	if(!_dmsFile){
		rc = EDB_OOM;
		PD_LOG(PDERROR,"Failed to new dms file");
		goto error;
	}
	// init dms
	rc = _dmsFile->initialize(pmdGetKRCB()->getDataFilePath());
	if(rc){
		PD_LOG(PDERROR,"Failed to call dms initialize,rc = %d",rc);
		goto error;
	}
done:
	return rc;
error:
	goto done;
}

int rtn::rtnInsert(BSONObj &record){
	int rc = EDB_OK;
	dmsRecordID recordID;
	BSONObj outRecord;//outRecord是值-value。
	// write data into file
	rc = _dmsFile->insert(record,outRecord,recordID);
	if(rc){
		PD_LOG(PDERROR,"Failed to call dms insert, rc = %d",rc);
		goto error;
	}
done:
	return rc;
error:
	goto done;
}

int rtn::rtnFind(BSONObj &record){
	return EDB_OK;
}

int rtn::rtnRemove(BSONObj &record){
	return EDB_OK;
}

