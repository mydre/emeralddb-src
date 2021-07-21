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

#ifndef PMD_HPP__
#define PMD_HPP__

#include "core.hpp"
#include "pmdEDUMgr.hpp"
#include "rtn.hpp"

enum EDB_DB_STATUS
{
   EDB_DB_NORMAL = 0,
   EDB_DB_SHUTDOWN,
   EDB_DB_PANIC
} ;

#define EDB_IS_DB_NORMAL ( EDB_DB_NORMAL == pmdGetKRCB()->getDBStatus () )
#define EDB_IS_DB_DOWN   ( EDB_DB_SHUTDOWN == pmdGetKRCB()->getDBStatus () || \
                           EDB_DB_PANIC    == pmdGetKRCB()->getDBStatus () )
#define EDB_IS_DB_UP     ( !EDB_IS_DB_DOWN )

#define EDB_SHUTDOWN_DB  { pmdGetKRCB()->setDBStatus(EDB_DB_SHUTDOWN); }

class pmdOptions ;
class EDB_KRCB
{
private :
   // configured options
   char          _dataFilePath [ OSS_MAX_PATHSIZE + 1 ] ;
   char          _logFilePath  [ OSS_MAX_PATHSIZE + 1 ] ;
   int           _maxPool ;
   char          _svcName [ NI_MAXSERV + 1 ] ;
   EDB_DB_STATUS _dbStatus ;
private :
   pmdEDUMgr     _eduMgr ;
   rtn           _rtnMgr ;
public :
   // constructor
   EDB_KRCB ()
   {
      _dbStatus = EDB_DB_NORMAL ;
      memset ( _dataFilePath, 0, sizeof(_dataFilePath) ) ;
      memset ( _logFilePath, 0, sizeof(_logFilePath) ) ;
      _maxPool = 0 ;
      memset ( _svcName, 0, sizeof(_svcName) ) ;
   }
   // destructor
   ~EDB_KRCB () {}

   // inline function
   // get edu mgr
   pmdEDUMgr *getEDUMgr ()
   {
      return &_eduMgr ;
   }
   rtn *getRtnMgr()
   {
      return &_rtnMgr ;
   }
   // get database status
   inline EDB_DB_STATUS getDBStatus ()
   {
      return _dbStatus ;
   }
   // get data file path
   inline const char *getDataFilePath ()
   {
      return _dataFilePath ;
   }
   // get log file path
   inline const char *getLogFilePath ()
   {
      return _logFilePath ;
   }
   // get service name
   inline const char *getSvcName ()
   {
      return _svcName ;
   }
   // get max thread pool
   inline int getMaxPool ()
   {
      return _maxPool ;
   }
   // setup database status
   inline void setDBStatus ( EDB_DB_STATUS status )
   {
      _dbStatus = status ;
   }

   // set data file path
   void setDataFilePath ( const char *pPath )
   {
      strncpy ( _dataFilePath, pPath, sizeof(_dataFilePath) ) ;
   }

   // set log file path
   void setLogFilePath ( const char *pPath )
   {
      strncpy ( _logFilePath, pPath, sizeof(_logFilePath) ) ;
   }

   // set service name
   void setSvcName ( const char *pName )
   {
      strncpy ( _svcName, pName, sizeof(_svcName) ) ;
   }
   // set max pool
   void setMaxPool ( int maxPool )
   {
      _maxPool = maxPool ;
   }

   // setup from pmdOptions
   int init ( pmdOptions *options ) ;
} ;

extern EDB_KRCB pmd_krcb ;

inline EDB_KRCB *pmdGetKRCB()
{
   return &pmd_krcb ;
}

#endif
