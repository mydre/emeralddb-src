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
#include "pd.hpp"
#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"

const static char *PDLEVELSTRING[] =
{
   "SEVERE",
   "ERROR",
   "EVENT",
   "WARNING",
   "INFO",
   "DEBUG"
};

const char* getPDLevelDesp ( PDLEVEL level )
{
   if ( (unsigned int)level > (unsigned int)PDDEBUG )
   {
      return "Unknow Level" ;
   }
   return PDLEVELSTRING[(unsigned int)level] ;
}

#ifdef _WINDOWS

const static char *PD_LOG_HEADER_FORMAT="%04d-%02d-%02d-%02d.%02d.%02d.%06d\
               \
	Level:%s"OSS_NEWLINE"PID:%-37luTID:%lu"OSS_NEWLINE"Function:%-32sLine:%u"\
	OSS_NEWLINE"File:%s"OSS_NEWLINE"Message:"OSS_NEWLINE"%s"OSS_NEWLINE OSS_NEWLINE;

#else

const static char *PD_LOG_HEADER_FORMAT="%04d-%02d-%02d-%02d.%02d.%02d.%06d \
								\
	Level:%s" OSS_NEWLINE"PID:%-37dTID:%d" OSS_NEWLINE"Function:%-32sLine:%d"\
	OSS_NEWLINE"File:%s" OSS_NEWLINE"Message:" OSS_NEWLINE"%s" OSS_NEWLINE OSS_NEWLINE;
#endif

PDLEVEL _curPDLevel = PD_DFT_DIAGLEVEL ;
char _pdDiagLogPath [ OSS_MAX_PATHSIZE+1 ] = {0} ;

ossXLatch _pdLogMutex ;
ossPrimitiveFileOp _pdLogFile ;

// open log file
static int _pdLogFileReopen ()//定义static类型的函数
{
   int rc = EDB_OK ;
   _pdLogFile.Close() ;
   rc = _pdLogFile.Open ( _pdDiagLogPath ) ;
   if ( rc )
   {
      printf ( "Failed to open log file, errno = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   _pdLogFile.seekToEnd () ;
done :
   return rc ;
error :
   goto done ;
}

// write log file
static int _pdLogFileWrite ( const char *pData )//写入文件
{
   int rc = EDB_OK ;
   size_t dataSize = strlen ( pData ) ;
   _pdLogMutex.get() ;//获取锁（文件打开操作是static方法，只实例化一次，）
   if ( !_pdLogFile.isValid() )
   {//第一次到这会执行
      // open the file
      rc = _pdLogFileReopen () ;
      if ( rc )
      {
         printf ( "Failed to open log file, errno = %d" OSS_NEWLINE,
                  rc ) ;
         goto error ;
      }
   }
   // write into the file
   rc = _pdLogFile.Write ( pData, dataSize ) ;
   if ( rc )
   {
      printf ( "Failed to write into log file, errno = %d" OSS_NEWLINE,
               rc ) ;
      goto error ;
   }
done :
   _pdLogMutex.release() ;//确保最后一定会执行
   return rc ;
error :
   goto done ;
}

// log
void pdLog ( PDLEVEL level, const char *func, const char *file,
             unsigned int line, const char *format, ... )
{
   int rc = EDB_OK ;
   if ( _curPDLevel < level )
      return  ;
   va_list ap ;//将最后面的参数放进了va_list当中了
   char userInfo[PD_LOG_STRINGMAX] ; // for user defined message
   char sysInfo[PD_LOG_STRINGMAX] ;  // for log header

   // create user information
   va_start ( ap, format ) ;
   vsnprintf ( userInfo, PD_LOG_STRINGMAX, format, ap ) ;
   va_end ( ap ) ;

#ifdef _WINDOWS
   SYSTEMTIME systime;
   GetLocalTime(&systime);

   snprintf ( sysInfo, PD_LOG_STRINGMAX, PD_LOG_HEADER_FORMAT,	//%04d-%02d-%02d-%02d.%02d.%02d.%06d
	   systime.wYear,
	   systime.wMonth,
	   systime.wDay ,
	   systime.wHour ,
	   systime.wMinute ,
	   systime.wSecond ,
	   systime.wMilliseconds*1000 ,
	   PDLEVELSTRING[level],
	   getpid(),
	   pthread_self(),
	   func,
	   line,
	   file,
	   userInfo
	   ) ;

#else
   struct tm otm ;
   struct timeval tv ;
   struct timezone tz ;
   time_t tt ;

   gettimeofday ( &tv, &tz ) ;
   tt = tv.tv_sec ;
   localtime_r ( &tt, &otm ) ;
   snprintf ( sysInfo, PD_LOG_STRINGMAX, PD_LOG_HEADER_FORMAT,
              otm.tm_year+1900,
              otm.tm_mon+1,
              otm.tm_mday,
              otm.tm_hour,
              otm.tm_min,
              otm.tm_sec,
              tv.tv_usec,
              PDLEVELSTRING[level],
              getpid(),
              syscall(SYS_gettid),
              func,
              line,
              file,
              userInfo
   ) ;
#endif // _WINDOWS
   printf ( "%s" OSS_NEWLINE, sysInfo ) ;//将日志信息打印到屏幕上面
   if ( _pdDiagLogPath[0] != '\0' )
   {//如果需要写写入到日志文件
      rc = _pdLogFileWrite ( sysInfo ) ;
      if ( rc )
      {
         printf ( "Failed to write into log file, errno = %d" OSS_NEWLINE, rc ) ;
         printf ( "%s" OSS_NEWLINE, sysInfo ) ;
      }
   }
   return ;
}
