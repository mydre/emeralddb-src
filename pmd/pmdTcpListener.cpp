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
#include "ossSocket.hpp"

#include "pmdEDU.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "pd.hpp"

#define PMD_TCPLISTENER_RETRY 5
#define OSS_MAX_SERVICENAME NI_MAXSERV

int pmdTcpListenerEntryPoint ( pmdEDUCB *cb, void *arg )
{
   int          rc        = EDB_OK ;
   pmdEDUMgr *  eduMgr    = cb->getEDUMgr() ;
   EDUID        myEDUID   = cb->getID() ;
   unsigned int retry     = 0 ;
   EDUID        agentEDU  = PMD_INVALID_EDUID ;
   char         svcName[OSS_MAX_SERVICENAME+1] ;
   while ( retry <= PMD_TCPLISTENER_RETRY && !EDB_IS_DB_DOWN )//重试循环
   {
      retry ++ ;

      strcpy( svcName, pmdGetKRCB()->getSvcName() ) ;//从krcb中拿到服务名
      PD_LOG ( PDEVENT, "Listening on port_test %s\n", svcName ) ;

      int port = 0 ;
      int len = strlen ( svcName ) ;
      for ( int i = 0; i<len; ++i )
      {
         if ( svcName[i] >= '0' && svcName[i] <= '9' )
         {
            port = port * 10 ;
            port += int( svcName[i] - '0' ) ;
         }
         else
         {
            PD_LOG ( PDERROR, "service name error!\n" ) ;
         }
      }

      ossSocket sock ( port ) ;//创建socket_addr的地址和端口
      rc = sock.initSocket () ;
      EDB_VALIDATE_GOTOERROR ( (EDB_OK==rc), rc, "Failed initialize socket" )
      rc = sock.bind_listen () ;
      EDB_VALIDATE_GOTOERROR ( (EDB_OK==rc), rc, "Failed to bind/listen socket");
      // once bind is successful, let's set the state of EDU to RUNNING
      if ( EDB_OK != ( rc = eduMgr->activateEDU ( myEDUID )) )
      {
         goto error ;
      }
      // master loop for tcp listener
      while ( !EDB_IS_DB_DOWN )//监听循环
      {
         int s ;
         rc = sock.accept (&s, NULL, NULL ) ;//每次来监听请求（每次等待10毫秒，如果没有人来，就重新跳到顶部）
         // if we don't get anything for a period of time, let's loop
         if ( EDB_TIMEOUT == rc )
         {
            rc = EDB_OK ;
            continue ;
         }
         // if we receive error due to database down, we finish
         if ( rc && EDB_IS_DB_DOWN )
         {
            rc = EDB_OK ;
            goto done ;
         }
         else if ( rc )
         {
            // if we fail due to error, let's restart socket
            PD_LOG ( PDERROR, "Failed to accept socket in TcpListener" ) ;
            PD_LOG ( PDEVENT, "Restarting socket to listen" ) ;
            break ;
         }

         // assign the socket to the arg
         void *pData = NULL ;//把void * 理解为一个4B的数据
         *((int *) &pData) = s ;//先变为（int *）类型的指针，然后再对指向的内容赋值

         rc = eduMgr->startEDU ( EDU_TYPE_AGENT, pData, &agentEDU ) ;//为新连接的客户端开启一个EDU
         if ( rc )
         {
            if ( rc == EDB_QUIESCED )
            {
               // we cannot start EDU due to quiesced
               PD_LOG ( PDWARNING, "Reject new connection due to quiesced database" ) ;
            }
            else
            {
               PD_LOG ( PDERROR, "Failed to start EDU agent" ) ;
            }
            // close remote connection if we can't create new thread
            ossSocket newsock(&s ) ;
            newsock.close () ;//关闭这个已连接套接字
            PD_LOG(PDEVENT,"the server closed one connection");
            continue ;//开始下一轮的监听！！！
         }
      }
      if ( EDB_OK != ( rc = eduMgr->waitEDU ( myEDUID )) )
      {//一直在运行队列当中
         goto error ;
      }
   } // while ( retry <= PMD_TCPLISTENER_RETRY )
done :
   return rc;
error :
   switch ( rc )
   {
   case EDB_SYS :
      PD_LOG ( PDSEVERE, "System error occured" ) ;
      break ;
   default :
      PD_LOG ( PDSEVERE, "Internal error" ) ;
   }
   goto done ;
}
