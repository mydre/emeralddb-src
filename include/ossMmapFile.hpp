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

#ifndef OSSMMAPFILE_HPP__
#define OSSMMAPFILE_HPP__

#include "core.hpp"
#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"

class _ossMmapFile{
protected:
	class _ossMmapSegment{
	public:
		void *_ptr;
		unsigned int _length;
		unsigned long long _offset;
		_ossMmapSegment(void *ptr,unsigned int length,unsigned long long offset){
			_ptr = ptr;
			_length = length;
			_offset = offset;
		}
	};//就是一个结构体
	typedef _ossMmapSegment ossMmapSegment;
	ossPrimitiveFileOp _fileOp;
	ossXLatch _mutex;
	bool _opened;
	std::vector<ossMmapSegment> _segments;
	char _fileName[OSS_MAX_PATHSIZE];
public:
	typedef std::vector<ossMmapSegment>::const_iterator CONST_ITR;
	inline CONST_ITR begin(){
		return _segments.begin();
	}
	inline CONST_ITR end(){
		return _segments.end();
	}
	inline unsigned int segmentSize(){
		return _segments.size();
	}
public:
	_ossMmapFile(){
		_opened = false;
		memset(_fileName,0,sizeof(_fileName));
	}
	~_ossMmapFile(){
		close();
//		_mutex.get();
//		if(_opened){
//
//			_fileOp.Close();
//			_opened = false;
//		}
//		_mutex.release();
	}
	int open(const char *pFilename,unsigned int options);
	void close();
	int map(unsigned long long offset,unsigned int length,void **pAddress);

};
typedef class _ossMmapFile ossMmapFile;
#endif
