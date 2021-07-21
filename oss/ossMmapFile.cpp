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

#include "ossMmapFile.hpp"
#include "pd.hpp"

using namespace std;
int ossMmapFile::open(const char *pFileName,unsigned int options){
	int rc = EDB_OK;
	_mutex.get();
	rc = _fileOp.Open(pFileName,options);
	if(EDB_OK == rc){
		_opened = true;
	}else{
		PD_LOG(PDERROR,"Failed to open file, rc = %d",rc);
		goto error;
	}
	strncpy(_fileName,pFileName,sizeof(_fileName));
done:
	_mutex.release();
	return rc;
error:
	goto done;
}

void ossMmapFile::close(){
	_mutex.get();
		for(vector<ossMmapSegment>::iterator it = _segments.begin();it!=_segments.end();it++){
			munmap((void *)(*it)._ptr,(*it)._length);
		}
		_segments.clear();
		if(_opened){
			_fileOp.Close();
			_opened = false;
		}
		_mutex.release();
}

int ossMmapFile::map(unsigned long long offset,unsigned int length,void **ppAddress){
	_mutex.get();
	int rc = EDB_OK;
	ossMmapSegment _seg(0,0,0);
	unsigned long long fileSize = 0;
	void *segment = NULL;
	if(0 == length)
		goto done;
	rc = _fileOp.getSize((off_t *)&fileSize);
	if(rc){
		PD_LOG(PDERROR,"Failed to get file size,rc = %d",rc);
		goto error;
	}
	if(offset + length > fileSize){
		PD_LOG(PDERROR,"Offset is greater than file size");
		rc = EDB_INVALIDARG;
		goto error;
	}
	//map region into memory
	segment = mmap(NULL,length,PROT_READ | PROT_WRITE,MAP_SHARED,_fileOp.getHandle(),offset);
	if(MAP_FAILED == segment){
		PD_LOG(PDERROR,"Failed to map offset %ld length %d,errno = %d",offset,length,errno);
		if(ENOMEM == errno)
			rc = EDB_OOM;
		else if(EACCES == errno)
			rc = EDB_PERM;
		else
			rc = EDB_SYS;
		goto error;
	}
	_seg._ptr = segment;
	_seg._length = length;
	_seg._offset = offset;
	_segments.push_back(_seg);
	if(ppAddress){
		*ppAddress = segment;
	}
done:
	_mutex.release();
	return rc;
error:
	goto done;
}
