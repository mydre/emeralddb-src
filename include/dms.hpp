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

#ifndef DMS_HPP__
#define DMS_HPP__

#include "ossLatch.hpp"
#include "ossMmapFile.hpp"
#include "bson.h"
#include "dmsRecord.hpp"
#include <vector>

#define DMS_EXTEND_SIZE 65536 //2的16次方
// 4MB for page size
#define DMS_PAGESIZE 4194304 //pow(2,22)
#define DMS_MAX_RECORD DMS_PAGESIZE-sizeof(dmsHeader) - sizeof(dmsRecord) - sizeof(SLOTOFF)
#define DMS_MAX_PAGES
typedef unsigned int SLOTOFF;
#define DMS_INVALID_SLOTID 0xFFFFFFFF
#define DMS_INVALID_PAGEID 0xFFFFFFFF

#define DMS_KEY_FILEDNAME "_id"
extern const char *gKeyFieldName;

//each record has the following header,include 4bytes size and 4 byte flag
#define DMS_RECORD_FLAG_NORMAL 0
#define DMS_RECORD_FLAG_DROPPED 1
struct dmsRecord{
	unsigned int _size;
	unsigned int _flag;
	char _data[0];
};

// dms header
#define DMS_HEADER_EYECATCHER "DMSH"
#define DMS_HEADER_EYECATCHER_LEN 4
#define DMS_HEADER_FLAG_NORMAL 0
#define DMS_HEADER_FLAG_OPENED 1

#define DMS_HEADER_VERSION_0 0
#define DMS_HEADER_VERSION_CURRENT DMS_HEADER_VERSION_0

struct dmsHeader{
	char _eyeCatcher[DMS_HEADER_EYECATCHER_LEN];
	unsigned int _size;
	unsigned int _flag;
	unsigned int _version;
};

//page structure
/****
---------------------------
|	PAGE HEADER		|
----------------------
|	Slot List		|
-----------------------
|	Free Space		|
----------------------
|	Data			|
------------------------
 * ***/
#define DMS_PAGE_EYECATCHER "PAGH"
#define DMS_PAGE_EYECATCHER_LEN 4
#define DMS_PAGE_FLAG_NORMAL 0
#define DMS_PAGE_FLAG_UNALLOC 1
#define DMS_SLOT_EMPTY 0xFFFFFFFF

struct dmsPageHeader{
	char _eyeCatcher[DMS_PAGE_EYECATCHER_LEN];
	unsigned int _size;
	unsigned int _flag;
	unsigned _numSlots;
	unsigned int _slotOffset;
	unsigned int _freeSpace;
	unsigned int _freeOffset;
	char _data[0];
};

#define DMS_FILE_SEGMENT_SIZE 134217728 //128M
#define DMS_FILE_HEADER_SIZE 65536 //64k
#define DMS_PAGES_PER_SEGMENT (DMS_FILE_SEGMENT_SIZE/DMS_PAGESIZE)
#define DMS_MAX_SEGMENTS (DMS_MAX_PAGES/DMS_PAGES_PER_SEGMENT)

class dmsFile :public ossMmapFile{
private:
	// points to memory where header is located
	dmsHeader *_header;
	std::vector<char *> _body;//指针，指向每个setment的起始位置
	// free space to page id map
	std::multimap<unsigned int,PAGEID> _freeSpaceMap;//一个key对应好几个value
	ossSLatch _mutex; //读写锁
	ossXLatch _extendMutex;//互斥锁
	char *_pFileName;
public:
	dmsFile();
	~dmsFile();
	// initialize the dms file
	int initialize(const char *pFileName);
	// insert into file
	//第一个和第二个参数的内容相同，地址不同
	int insert(bson::BSONObj &record,bson::BSONObj &outRecord,dmsRecordID &rid);
 	int remove(dmsRecordID &rid);
 	int find(dmsRecordID &rid,bson::BSONObj &result);
private:
 	// create a new segmentfor the current file
 	int _extendSegment();
 	// int from empty file,createing header only
 	int _initNew();
 	// extend  the file for given bytes
 	int _extendFile(int size);
 	// load data from beginning
 	int _loadData();
 	// search slot
 	int _searchSlot(char *page,dmsRecordID &recordID,SLOTOFF &slot);
 	// reorg
 	void _recoverSpace(char *page);
 	// udpate free space
 	void _updateFreeSpace(dmsPageHeader *header,int changeSize,PAGEID pageID);
 	// find a page id to insert ,return invalid_pageid if there's no page can be found for required size bytes
 	PAGEID _findPage(size_t requireSize);
public:
 	inline unsigned int getNumSegments(){
 		return _body.size();
 	}
 	inline unsigned int getNumPages(){
 		return getNumSegments() * DMS_PAGES_PER_SEGMENT;
 	}
 	inline char *pageToOffset(PAGEID pageID){
 		if(pageID >= getNumPages()){
 			return NULL;
 		}
 		return _body[pageID/DMS_PAGES_PER_SEGMENT] + DMS_PAGESIZE *(pageID % DMS_PAGES_PER_SEGMENT);
 	}
 	inline bool validSize(size_t size){
 		if(size <DMS_FILE_HEADER_SIZE){
 			return false;
 		}
 		size = size - DMS_FILE_HEADER_SIZE;
 		if(size % DMS_FILE_SEGMENT_SIZE != 0){
 			return false;
 		}
 		return true;
 	}
};

#endif
