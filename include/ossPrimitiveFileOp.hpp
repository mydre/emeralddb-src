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

#ifndef OSSPRIMITIVEFILEOP_HPP__
#define OSSPRIMITIVEFILEOP_HPP__

#include "core.hpp"

#ifdef _WINDOWS

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define OSS_F_GETLK        F_GETLK64
#define OSS_F_SETLK        F_SETLK64
#define OSS_F_SETLKW       F_SETLKW64

#define oss_struct_statfs  struct statfs64
#define oss_statfs         statfs64
#define oss_fstatfs        fstatfs64
#define oss_struct_statvfs struct statvfs64
#define oss_statvfs        statvfs64
#define oss_fstatvfs       fstatvfs64
#define oss_struct_stat    struct _stati64
#define oss_struct_flock   struct flock64
#define oss_stat           stat64
#define oss_lstat          lstat64
#define oss_fstat          _fstati64
#define oss_open           _open
#define oss_lseek          _lseeki64
#define oss_ftruncate      ftruncate64
#define oss_off_t          __int64
#define oss_close          _close
#define oss_access         access
#define oss_chmod
#define oss_read           read
#define oss_write          write

#define O_RDWR				_O_RDWR
#define O_RDONLY			_O_RDONLY
#define O_WRONLY			_O_WRONLY
#define O_CREAT				_O_CREAT
#define O_TRUNC				_O_TRUNC

#define OSS_HANDLE			int
#define OSS_INVALID_HANDLE_FD_VALUE (OSS_HANDLE(-1))
#else

#define OSS_HANDLE		   int
#define OSS_F_GETLK        F_GETLK64
#define OSS_F_SETLK        F_SETLK64
#define OSS_F_SETLKW       F_SETLKW64

#define oss_struct_statfs  struct statfs64
#define oss_statfs         statfs64
#define oss_fstatfs        fstatfs64
#define oss_struct_statvfs struct statvfs64
#define oss_statvfs        statvfs64
#define oss_fstatvfs       fstatvfs64
#define oss_struct_stat    struct stat64
#define oss_struct_flock   struct flock64
#define oss_stat           stat64
#define oss_lstat          lstat64
#define oss_fstat          fstat64
#define oss_open           open64
#define oss_lseek          lseek64
#define oss_ftruncate      ftruncate64
#define oss_off_t          off64_t
#define oss_close          close
#define oss_access         access
#define oss_chmod          chmod
#define oss_read           read
#define oss_write          write

#define OSS_INVALID_HANDLE_FD_VALUE (-1)

#endif // _WINDOWS

#define OSS_PRIMITIVE_FILE_OP_FWRITE_BUF_SIZE 2048
#define OSS_PRIMITIVE_FILE_OP_READ_ONLY     (((unsigned int)1) << 1)
#define OSS_PRIMITIVE_FILE_OP_WRITE_ONLY    (((unsigned int)1) << 2)
#define OSS_PRIMITIVE_FILE_OP_OPEN_EXISTING (((unsigned int)1) << 3)
#define OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS   (((unsigned int)1) << 4)
#define OSS_PRIMITIVE_FILE_OP_OPEN_TRUNC    (((unsigned int)1) << 5)

typedef oss_off_t offsetType ;

class ossPrimitiveFileOp
{
public :
   typedef  OSS_HANDLE    handleType ;
private :
   handleType _fileHandle = 0;
   ossPrimitiveFileOp( const ossPrimitiveFileOp & ) {}
   const ossPrimitiveFileOp &operator=( const ossPrimitiveFileOp & ) ;
   bool _bIsStdout = false;//定义变量的同时进行初始化

protected :
   void setFileHandle( handleType handle ) ;

public :
   ossPrimitiveFileOp() ;
   int Open
   (
      const char * pFilePath,
      unsigned int options = OSS_PRIMITIVE_FILE_OP_OPEN_ALWAYS
   ) ;
   void openStdout() ;
   void Close() ;
   bool isValid( void ) ;
   int Read( const size_t size, void * const pBuf, int * const pBytesRead ) ;

   int Write( const void * pBuf, size_t len = 0 ) ;

   int fWrite( const char * fmt, ... ) ;

   offsetType getCurrentOffset (void) const ;

   void seekToOffset( offsetType offset ) ;

   void seekToEnd( void ) ;

   int getSize( offsetType * const pFileSize ) ;

   handleType getHandle( void ) const
   {
      return _fileHandle ;
   }
} ;

#endif
