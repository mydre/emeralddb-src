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

#include "dms.hpp"
#include "pd.hpp"
using namespace bson;

const char *gKeyFieldName = DMS_KEY_FILEDNAME;


dmsFile::dmsFile():_header(NULL),_pFileName(NULL){

}

dmsFile::~dmsFile(){
	if(_pFileName)
		free(_pFileName);
	close();
}

int dmsFile::insert(BSONObj &record,BSONObj &outRecord,dmsRecordID &rid){
	int rc = EDB_OK;
	PAGEID pageID = 0;
	char *page = NULL;
	dmsPageHeader *pageHeader = NULL;
	int recordSize = 0;
	SLOTOFF offsetTemp = 0;
	const char *pGKeyFieldName = NULL;
	dmsRecord recordHeader;

	recordSize = record.objsize();
	//When we attempt to insert record,first we have to verify it include _id field
	if(recordSize > DMS_MAX_RECORD){
		rc = EDB_INVALIDARG;
		PD_LOG(PDERROR,"record cannot bigger than 4MB");
		goto error;
	}
	pGKeyFieldName = gKeyFieldName;

	// make sure _id exists
	if(record.getFieldDottedOrArray(pGKeyFieldName).eoo()){//检测一个BsonObj对象中是否含有_id这个key
		rc = EDB_INVALIDARG;
		PD_LOG(PDERROR,"record must be with _id");
		goto error;
	}

retry:
	//lock the database
	_mutex.get();
	// and then we should get the required record size
	pageID = _findPage(recordSize + sizeof(dmsRecord));
	// if there's not enough space in any existing pages,let's release db lock
	if(DMS_INVALID_PAGEID == pageID){
		_mutex.release();
		//if there's not enoush space in any existing pages,let's release db lock and
		// try to allocate a new segment by calling _extendSegment
		if(_extendMutex.try_get()){
			//calling _extendSegment
			rc = _extendSegment();
			if(rc){
				PD_LOG(PDERROR,"Failed to extend segment, rc = %d",rc);
				_extendMutex.release();
				goto error;
			}
		}else{
			// if we cannot get the extendmutex, that means someone else is trying to extend
			// so let's wait until getting the mutex,and release it and try again
			_extendMutex.get();
		}
		_extendMutex.release();
		goto retry;
	}
	// find the in-momory offset for the page
	page = pageToOffset(pageID);
	// if something wrong,le'ts return error
	if(!page){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Failed to find the page");
		goto error_releasemutex;
	}
	// set page header
	pageHeader = (dmsPageHeader *)page;
	if(memcmp(pageHeader->_eyeCatcher,DMS_PAGE_EYECATCHER,DMS_PAGE_EYECATCHER_LEN) != 0){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Inlid page header");
		goto error_releasemutex;
	}
	// slot offset is the last byte of slots
	// free offset is the first byte of data
	// so freeOffset - slotOffset is the actual free space excluding holes
	if(
	// make sure tere's still holes to recover
	(pageHeader->_freeSpace > pageHeader->_freeOffset - pageHeader->_slotOffset) &&
	// if there's no free space excluding holes
	(pageHeader->_slotOffset + recordSize + sizeof(dmsRecord) + sizeof(SLOTID) > pageHeader->_freeOffset)
	){
		// recover empty hole from page
		_recoverSpace(page);
	}
	if(
	// make sure there si enough free space
	(pageHeader->_freeSpace < recordSize + sizeof(dmsRecord) + sizeof(SLOTID) || (pageHeader->_freeOffset -pageHeader->_slotOffset < recordSize + sizeof(dmsRecord) + sizeof(SLOTID)))
	){
		PD_LOG(PDERROR,"Something big wrong!!");
		rc = EDB_SYS;
		goto error_releasemutex;
	}
	//找偏移，在什么地方插入数据
	offsetTemp = pageHeader->_freeOffset - recordSize - sizeof(dmsRecord);
	recordHeader._size = recordSize + sizeof(dmsRecord);
	recordHeader._flag = DMS_RECORD_FLAG_NORMAL;
	// copy the slot
	*(SLOTOFF *)(page + sizeof(dmsPageHeader) + pageHeader->_numSlots * sizeof(SLOTOFF)) = offsetTemp;//相当于是。。。。。。。。。。。。。。。。自己理解（顶部的索引）
	//copy the record header
	memcpy(page + offsetTemp,(char *)&recordHeader,sizeof(dmsRecord));
	//copy the record body
	memcpy(page + offsetTemp + sizeof(dmsRecord),record.objdata(),recordSize);
	outRecord = BSONObj(page+offsetTemp + sizeof(dmsRecord));
	rid._pageID = pageID;
	rid._slotID = pageHeader->_numSlots;
	// modify metadata in page
	pageHeader->_numSlots ++;
	pageHeader->_slotOffset += sizeof(SLOTID);
	pageHeader->_freeOffset = offsetTemp;
	//modify database metadata
	_updateFreeSpace(pageHeader,-(recordSize + sizeof(SLOTID) + sizeof(dmsRecord)),pageID);
	// release lock for database
	_mutex.release();
done:
	return rc;
error_releasemutex:
	_mutex.release();
error:
	goto done;
}

int dmsFile::remove(dmsRecordID &rid){
	int rc = EDB_OK;
	SLOTOFF slot = 0;
	char *page = NULL;
	dmsRecord *recordHeader = NULL;
	dmsPageHeader *pageHeader = NULL;
	std::pair<std::multimap<unsigned int,PAGEID>::iterator,std::multimap<unsigned int,PAGEID>::iterator> ret;
	_mutex.get();
	// find the page in memory
	page = pageToOffset(rid._pageID);
	if(!page){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Failed to find the page for %u;%u",rid._pageID,rid._slotID);
		goto error;
	}
	// search the given slot
	rc = _searchSlot(page,rid,slot);
	if(rc){
		PD_LOG(PDERROR,"Failed to search slot,rc = %d",rc);
		goto error;
	}
	if(DMS_SLOT_EMPTY == slot){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"The record is droped");
		goto error;
	}
	// set page header
	pageHeader = (dmsPageHeader *)page;
	// set slot to empty
	*(SLOTID *)(page + sizeof(dmsPageHeader) + rid._slotID *sizeof(SLOTID)) = DMS_SLOT_EMPTY;
	// set record header
	recordHeader = (dmsRecord *)(page + slot);
	recordHeader->_flag = DMS_RECORD_FLAG_DROPPED;
	// update database metadata
	_updateFreeSpace(pageHeader,recordHeader->_size,rid._pageID);
done:
	_mutex.release();
	return rc;
error:
	goto done;
}
int dmsFile::find(dmsRecordID &rid,BSONObj &result){
	//查找的时候需要返回一个result
	int rc = EDB_OK;
	SLOTID slot = 0;
	char *page = NULL;
	dmsRecord *recordHeader = NULL;
	//S lock the database
	_mutex.get_shared();
	// goto the page and verify the slot is valid
	page = pageToOffset(rid._pageID);
	if(!page){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Failed to find the page");
		goto error;
	}
	rc = _searchSlot(page,rid,slot);
	if(rc){
		PD_LOG(PDERROR,"Failed to search slot,rc = %d",rc);
		goto error;
	}
	// if slot is empty,something big wrong
	if(DMS_SLOT_EMPTY == slot){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"The record si dropped");
		goto error;
	}
	// get the record header
	recordHeader = (dmsRecord *)(page + slot);
	// if recordheader->flag is dropped ,this record is dropped already
	if(DMS_RECORD_FLAG_DROPPED == recordHeader->_flag){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"This data is dropped");
		goto error;
	}
	result = BSONObj(page + slot + sizeof(dmsRecord)).copy();
done:
	_mutex.release_shared();
	return rc;
error:
	goto done;
}
void dmsFile::_updateFreeSpace(dmsPageHeader *header,int changeSize,PAGEID pageID){
	unsigned int freeSpace = header->_freeSpace;
	std::pair<std::multimap<unsigned int,PAGEID>::iterator,std::multimap<unsigned int,PAGEID>::iterator> ret;
	ret = _freeSpaceMap.equal_range(freeSpace);
	for(std::multimap<unsigned int ,PAGEID>::iterator it = ret.first;it!=ret.second;++it){
		if(it->second == pageID){
			_freeSpaceMap.erase(it);
			break;
		}
	}
	// increase page free space
	freeSpace += changeSize;
	header->_freeSpace = freeSpace;
	// insert into free space map
	_freeSpaceMap.insert(pair<unsigned int,PAGEID>(freeSpace,pageID));
}

int dmsFile::initialize(const char *pFileName){
	offsetType offset = 0;
	int rc = EDB_OK;
	//duplicate filename
	_pFileName = strdup(pFileName);
	if(!_pFileName){
		rc = EDB_OOM;
		PD_LOG(PDERROR,"Failed to duplicte file name");
		goto error;
	}
	// open file
	rc = open(_pFileName,OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS);
	PD_RC_CHECK(rc,PDERROR,"Failed to open file %d, rc = %d",_pFileName,rc);
getfilesize:
	// get file size
	rc = _fileOp.getSize(&offset);
	PD_RC_CHECK(rc,PDERROR,"Failed to get file size, rc = %d",rc);
	// if ifle size is 0,that mean it's newly created ifle and we shoud initialize it
	if(!offset){
		rc = _initNew();
		PD_RC_CHECK(rc,PDERROR,"Failed to initialize file,rc = %d",rc);
		goto getfilesize;
	}
	// load data
	rc = _loadData();
	PD_RC_CHECK(rc,PDERROR,"Failed to laod data,rc = %d",rc);
done:
	return rc;
error:
	goto done;
}

// caller must hold extend latch,同事只有一个县城在扩展段
int dmsFile::_extendSegment(){
	// extend  a new segment
	int rc = EDB_OK;
	char *data = NULL;
	int freeMapSize = 0;
	dmsPageHeader pageHeader;
	offsetType offset = 0;
	// first let's get the size of file before extend
	rc = _fileOp.getSize(&offset);
	PD_RC_CHECK(rc,PDERROR,"Failed to get file size, rc = %d",rc);
	// extend the file
	rc = _extendFile(DMS_FILE_SEGMENT_SIZE);
	PD_RC_CHECK(rc,PDERROR,"Failed to extend setment rc = %d",rc);
	//mep from original end to new end
	rc = map(offset,DMS_FILE_SEGMENT_SIZE,(void **)&data);
	PD_RC_CHECK(rc,PDERROR,"Failed to map FIle,rc = %d",rc);
	//create page header structure and we are going to copy to each page
	strcpy(pageHeader._eyeCatcher,DMS_PAGE_EYECATCHER);
	pageHeader._size = DMS_PAGESIZE;
	pageHeader._flag = DMS_PAGE_FLAG_NORMAL;
	pageHeader._numSlots = 0;
	pageHeader._slotOffset = sizeof(dmsPageHeader);
	pageHeader._freeSpace = DMS_PAGESIZE - sizeof(dmsPageHeader);
	pageHeader._freeOffset = DMS_PAGESIZE;
	//copy header to each page（这一步不能出错）
	for(int i = 0;i<DMS_FILE_SEGMENT_SIZE;i+=DMS_PAGESIZE){
		memcpy(data+i,(char *)&pageHeader,sizeof(dmsPageHeader));
	}
	_mutex.get();
	//free space handling
	freeMapSize = _freeSpaceMap.size();
	//insert into free space map
	for(int i = 0;i<DMS_PAGES_PER_SEGMENT;i++){
		_freeSpaceMap.insert(pair<unsigned int,PAGEID>(pageHeader._freeSpace,i+freeMapSize));
	}
	// push the segment into body list
	_body.push_back(data);
	_header->_size += DMS_PAGES_PER_SEGMENT;
	_mutex.release();
done:
	return rc;
error:
	goto done;
}

int dmsFile::_initNew(){
	// initialize a newly created file,let's append DMS_FILE_HEADER_SIZE btes and the initialize the header
	int rc = EDB_OK;
	rc = _extendFile(DMS_FILE_HEADER_SIZE);
	PD_RC_CHECK(rc,PDERROR,"Failed to extend file,rc = %d",rc);
	rc = map(0,DMS_FILE_HEADER_SIZE,(void **)&_header);
	PD_RC_CHECK(rc,PDERROR,"Failed to map, rc = %d",rc);
	strcpy(_header->_eyeCatcher,DMS_HEADER_EYECATCHER);//这个时候拷贝到内存当中就相当于是拷贝到了文件当中。
	_header->_size = 0;
	_header->_flag = DMS_HEADER_FLAG_NORMAL;
	_header->_version = DMS_HEADER_VERSION_CURRENT;
done:
	return rc;
error:
	goto done;
}

PAGEID dmsFile::_findPage(size_t requiredSize){
	std::multimap<unsigned int ,PAGEID>::iterator findIter;
	findIter = _freeSpaceMap.upper_bound(requiredSize);
	if(findIter!=_freeSpaceMap.end()){
		return findIter->second;
	}
	return DMS_INVALID_PAGEID;
}

int dmsFile::_extendFile(int size){
	int rc = EDB_OK;
	char temp[DMS_EXTEND_SIZE] = {0};//65536等于64KB
	memset(temp,0,DMS_EXTEND_SIZE);
	if(size % DMS_EXTEND_SIZE != 0){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Invaild extend size,must be multitile of %d",DMS_EXTEND_SIZE);
		goto error;
	}
	//write file
	for(int i = 0;i<size;i+=DMS_EXTEND_SIZE){
		_fileOp.seekToEnd();
		rc = _fileOp.Write(temp,DMS_EXTEND_SIZE);
		PD_RC_CHECK(rc,PDERROR,"Failed to write file,rc = %d",rc);
	}
done:
	return rc;
error:
	goto done;
}

int dmsFile::_searchSlot(char *page,dmsRecordID &rid,SLOTOFF &slot){
	int rc = EDB_OK;
	dmsPageHeader *pageHeader = NULL;
	if(!page){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"page is NULL");
		goto error;
	}
	// let's firsst verify the rid is valid
	if(0> rid._pageID || 0 > rid._slotID){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Invalid RID:%d.%d",rid._pageID,rid._slotID);
		goto error;
	}
	pageHeader = (dmsPageHeader *)page;
	if(rid._slotID > pageHeader->_numSlots){
		rc = EDB_SYS;
		PD_LOG(PDERROR,"SLot is out of range,provided:%d, max:%d",rid._slotID,pageHeader->_numSlots);
		goto error;
	}
	slot = *(SLOTOFF *)(page + sizeof(dmsPageHeader) + rid._slotID * sizeof(SLOTOFF));
done:
	return rc;
error:
	goto done;
}

int dmsFile::_loadData(){
	int rc = EDB_OK;
	int numPage = 0;
	int numSegments = 0;
	dmsPageHeader *pageHeader = NULL;
	char *data = NULL;
	SLOTID slotID = 0;
	SLOTOFF slotOffset = 0;
	dmsRecordID recordID;
	BSONObj bson;
	// check if header is valid
	if(!_header){
		rc = map(0,DMS_FILE_HEADER_SIZE,(void **)&_header);//从文件映射进内存获取header
		PD_RC_CHECK(rc,PDERROR,"Failed to map file header,rc = %d",rc);
	}
	numPage = _header->_size;//总页面的个数
	if(numPage % DMS_PAGES_PER_SEGMENT){//128M,4M,32个页面
		rc = EDB_SYS;
		PD_LOG(PDERROR,"Failed to load data,partial segments detected");
		goto error;
	}
	numSegments = numPage/DMS_PAGES_PER_SEGMENT;//总segment的个数
	//get the segments number
	if(numSegments > 0){
		for(int i = 0;i<numSegments;++i){
			//map each segment into memory
			rc = map(DMS_FILE_HEADER_SIZE+DMS_FILE_SEGMENT_SIZE * i,DMS_FILE_SEGMENT_SIZE,(void **)&data);
			PD_RC_CHECK(rc,PDERROR,"Failed to map segment %d, rc = %d",i,rc);
			_body.push_back(data);
			//initialize each page into freeSpaceMap
			for(unsigned int k = 0;k<DMS_PAGES_PER_SEGMENT;++k){
				pageHeader = (dmsPageHeader *)(data + k*DMS_PAGESIZE);
				_freeSpaceMap.insert(pair<unsigned int,PAGEID>(pageHeader->_freeSpace,k));
				slotID = (SLOTID)pageHeader->_numSlots;
				recordID._pageID = (PAGEID)k;
				// for each record in the page,let's insert into index
//				for(unsigned int s = 0;s < slotID;++s){
//					slotOffset = *(SLOTOFF *)(data + K*DMS_PAGEISZE + sizeof(dmsPageHeader) + s * sizeof(SLOTID));
//					if(DMS_SLOT_EMPTY == slotOffset){
//						continue;
//					}
//					bson = BSONObj
//				}
			}
		}
	}
done:
	return rc;
error:
	goto done;
}

void dmsFile::_recoverSpace(char *page){//页内充足
	char *pLeft = NULL;
	char *pRight = NULL;
	SLOTOFF slot = 0;
	int recordSize = 0;
	bool isRecover = false;
	dmsRecord *recordHeader = NULL;
	dmsPageHeader *pageHeader = NULL;

	pLeft = page + sizeof(dmsPageHeader);
	pRight = page + DMS_PAGESIZE;

	pageHeader = (dmsPageHeader *)page;
	//recover space
	for(unsigned int i = 0;i<pageHeader->_numSlots;i++){
		slot = *(SLOTOFF *)(pLeft+sizeof(SLOTOFF)*i);
		if(DMS_SLOT_EMPTY != slot){
			recordHeader = (dmsRecord *)(page + slot);
			recordSize = recordHeader->_size;
			pRight -= recordSize;
			if(isRecover){
				memmove(pRight,page+slot,recordSize);
				*((SLOTOFF*)(pLeft + sizeof(SLOTOFF)*i)) = (SLOTOFF)(pRight-page);
			}
		}else{
			isRecover = true;
		}
	}
}
