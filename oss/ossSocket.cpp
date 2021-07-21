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
#include "ossSocket.hpp"
#include "pd.hpp"
//create a listening socket
_ossSocket::_ossSocket(unsigned int port,int timeout){//define construct function of two parameter
	_init = false;
	_fd = 0;
	_timeout = timeout;
	memset(&_sockAddress,0,sizeof(sockaddr_in));
	memset(&_peerAddress,0,sizeof(sockaddr_in));
	_peerAddressLen = sizeof(_peerAddress);
	_addressLen = sizeof(_sockAddress);
	_sockAddress.sin_family = AF_INET;
	_sockAddress.sin_port = htons(port);
	_sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
}
//Create a socket
_ossSocket::_ossSocket(){
	_init = false;
	_fd = 0;
	_timeout = OSS_SOCKET_DEF_TIMEOUT;
	memset(&_sockAddress,0,sizeof(sockaddr_in));
	memset(&_peerAddress,0,sizeof(sockaddr_in));
	_peerAddressLen = sizeof(_peerAddress);
	_addressLen = sizeof(_sockAddress);
}
//Create a connecting socket
_ossSocket::_ossSocket(const char *pHostname,unsigned int port,int timeout){
	struct hostent *hp;
	_init = false;
	_fd = 0;
	_timeout = timeout;
	memset(&_sockAddress,0,sizeof(sockaddr_in));
	memset(&_peerAddress,0,sizeof(sockaddr_in));
	_peerAddressLen = sizeof(_peerAddress);
	_sockAddress.sin_family = AF_INET;
	if((hp = gethostbyname(pHostname)))
		_sockAddress.sin_addr.s_addr = *((int *)hp->h_addr_list[0]);
	else
		_sockAddress.sin_addr.s_addr = inet_addr(pHostname);
	_sockAddress.sin_port = htons(port);
	_addressLen = sizeof(_sockAddress);
}
// Create from existing socket
_ossSocket::_ossSocket(int *sock,int timeout){//第一个参数是个指针
	int rc = EDB_OK;
	_fd = *sock;
	printf("create from existing socket:,_fd=%d\n",_fd);
	_init = true;
	_timeout = timeout;
	_addressLen = sizeof(_sockAddress);
	memset(&_peerAddress,0,sizeof(_peerAddress));
	_peerAddressLen = sizeof(_peerAddress);//这行一定要加上！！！
	rc = getsockname(_fd,(sockaddr *)&_sockAddress,&_addressLen);
	if(rc){
		//printf("Failed to get sock name, error = %d",SOCK_GETLASTERROR);
		PD_LOG(PDERROR,"Failed to get sock name,error = %d",SOCK_GETLASTERROR);
		_init = false;
	}
	else{
		//sleep(0.1);
		rc = getpeername(*sock,(sockaddr *)&_peerAddress,&_peerAddressLen);//_fd是已连接套接字描述符
		PD_RC_CHECK(rc,PDERROR,"Failed to get peer name, error = %d",SOCK_GETLASTERROR);
	}
done:
	return;
error:
	goto done;
}

int ossSocket::getFD(){
	return _fd;
}
bool ossSocket::getInit(){
	return _init;
}

int ossSocket::initSocket(){
	int rc = EDB_OK;
	if(_init){
		//printf("已经初始化了----------------------------------");
		goto done;
	}
	memset(&_peerAddress,0,sizeof(sockaddr_in));
	_peerAddressLen = sizeof(_peerAddress);
	_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//客户端调用了socket函数来创建一个套接字
	printf("init_fd:%d\n",_fd);
	if(-1 == _fd){
		rc = EDB_NETWORK;
		PD_RC_CHECK(rc,PDERROR,"Failed to initialize socket, error = %d",SOCK_GETLASTERROR);
		//printf("Failed to initialize socket,error = %d",SOCK_GETLASTERROR);

		goto error;
	}
	_init = true;
	// set timeout
	setTimeout(_timeout);
done :
	return rc;
error :
	goto done;
}


int ossSocket::setSocketLi(int lOnOff,int linger){
	int rc = EDB_OK;
	struct linger _linger;
	_linger.l_onoff = lOnOff;
	_linger.l_linger = linger;
	rc = setsockopt(_fd,SOL_SOCKET,SO_LINGER,(const char *)&_linger,sizeof(_linger));//-----------------------------------
	return rc;
}
void ossSocket::setAddress(const char *pHostname,unsigned int port){
	struct hostent *hp;
	memset(&_sockAddress,0,sizeof(sockaddr_in));
	memset(&_peerAddress,0,sizeof(sockaddr_in));
	_peerAddressLen = sizeof(_peerAddress);
	_addressLen = sizeof(_sockAddress);
	_sockAddress.sin_family = AF_INET;
	if((hp = gethostbyname(pHostname))){
		_sockAddress.sin_addr.s_addr = *((int *)hp->h_addr_list[0]);//一个ip地址占据4个Byte。（正好是一个unsigned int32整数）
	}else{
		_sockAddress.sin_addr.s_addr = inet_addr(pHostname);
	}
	_sockAddress.sin_port = htons(port);
	//printf("%d\n",_addressLen);
}

int ossSocket::bind_listen(){
	int rc = EDB_OK;
	int temp = 1;
	rc = setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&temp,sizeof(int));
	if(rc){
		PD_LOG(PDWARNING,"Failed to setsockopt SO_REUSEADDR,rc = %d",SOCK_GETLASTERROR);
		printf("Failed to setsockopt SO_REUSEADDR ,rc=%d",SOCK_GETLASTERROR);
	}
	rc = setSocketLi(1,30);
	if(rc){
		PD_LOG(PDWARNING,"failed to setsockopt SO_LINGER, rc=%d",SOCK_GETLASTERROR);
		printf("failed to setsockopt SO_LINGER, rc=%d",SOCK_GETLASTERROR);
	}
	rc = ::bind(_fd,(struct sockaddr *)&_sockAddress,_addressLen);
	if(rc){

		printf("Failed to bind socket,rc=%d",SOCK_GETLASTERROR);
		rc = EDB_NETWORK;
		PD_LOG(PDWARNING,"Failed to bind socket,rc=%d",SOCK_GETLASTERROR);
		goto error;
	}
	rc = listen(_fd,SOMAXCONN);
	if(rc){
		printf("Failed to listen socket, rc=%d",SOCK_GETLASTERROR);
		rc = EDB_NETWORK;
		PD_LOG(PDWARNING,"Failed to listen socket, rc=%d",SOCK_GETLASTERROR);
		goto error;
	}
done :
	return rc;
error :
	close();
	goto done;
}

int ossSocket::send(const char *pMsg, int len,int timeout,int flags){
	int rc = EDB_OK;
	int maxFD = _fd;//向已连接套接字发送信息
	//PD_LOG(PDEVENT,"ossSocket::send函数开始执行，_fd:%d\n",_fd);
	//printf("ossSocket::send函数开始执行，_fd:%d\n",_fd);
	struct timeval maxSelectTime;
	fd_set fds;

	maxSelectTime.tv_sec = timeout / 1000000;
	maxSelectTime.tv_usec = timeout % 1000000;
	if(0 == len){
		return EDB_OK;
	}
	// wait loop until socket is ready
	while(true){
		FD_ZERO(&fds);
		FD_SET(_fd,&fds);
		rc = select(maxFD + 1,NULL,&fds,NULL,timeout>=0?&maxSelectTime:NULL);//select返回成功则表示等待的事件已经发
		if(rc == 0){
			printf("rc等于0\n");
			rc = EDB_TIMEOUT;
			goto done;
		}
		if(0 > rc){
			printf("rc<0\n");
			rc = SOCK_GETLASTERROR;
			// if we failed due to interrupt ,let's continue
			if(EINTR == rc){
				continue;
			}
			rc = EDB_NETWORK;
			PD_RC_CHECK(rc,PDERROR,"failed to select from socket,rc=%d",rc);
			printf("failed to select from socket,rc=%d",rc);

			goto error;
		}
		if(FD_ISSET(_fd,&fds)){
			break;
		}
	}
	while(len > 0){
		// MSG_NOSIGNAL: Requests not to send SIGPIPE on errors on stream
		//oriented sockets when the other end breaks the connection. The EPIPE error is still returned
//		printf("%s\n",pMsg+4);//输出hello world
//		printf("%d\n",len);
		rc = ::send(_fd,pMsg,len,MSG_NOSIGNAL | flags);
		if(rc == -1){
			printf("failed to send,rc=%d",SOCK_GETLASTERROR);
			rc = EDB_NETWORK;
			goto error;
		}
		len -= rc;
		pMsg += rc;
	}
	rc = EDB_OK;
done:
	return rc;
error:
	goto done;
}

bool ossSocket::isConnected(){
	int rc = EDB_OK;
	rc = ::send(_fd,"",0,MSG_NOSIGNAL);
	if(0 > rc)
		return false;
	return true;
}

#define MAX_RECV_RETRIES 5
int ossSocket::recv(char *pMsg,int len,int timeout,int flags){
	int rc = EDB_OK;
	int retries = 0;
	int maxFD = _fd;
	struct timeval maxSelectTime;
	fd_set fds;
	if(0 == len){
		return EDB_OK;
	}
	maxSelectTime.tv_sec = timeout /1000000;
	maxSelectTime.tv_usec = timeout % 1000000;
	while(true){
		FD_ZERO(&fds);
		FD_SET(_fd,&fds);
		rc = select(maxFD + 1, &fds, NULL,NULL,timeout>=0?&maxSelectTime:NULL);
		if(rc == 0){
			rc = EDB_TIMEOUT;
			goto done;
		}
		// if < 0 ,something wrong
		if(rc < 0){
			rc = SOCK_GETLASTERROR;
			if(EINTR == rc){
				continue;
			}
			printf("Failed to select from socket, rc = %d",rc);
			rc = EDB_NETWORK;
			goto error;
		}
		if(FD_ISSET(_fd,&fds)){
			break;
		}
	}
	while(len > 0){
		rc = ::recv(_fd,pMsg,len,MSG_NOSIGNAL|flags);//接收4个字节，给pMsg(第一次则表示总长度)
		if(rc > 0){
			if(flags & MSG_PEEK){
				goto done;
			}
			len -= rc;
			pMsg += rc;
		}else if(rc == 0){
			printf("Peer unexpected shutdown");
			rc = EDB_NETWORK_CLOSE;
			goto error;
		}else{
			rc = SOCK_GETLASTERROR;
			if((EAGAIN == rc || EWOULDBLOCK == rc) && _timeout > 0){
				printf("Recv() timeout: rc=%d",rc);
				rc = EDB_NETWORK;
				goto error;
			}
			if((EINTR == rc) && (retries < MAX_RECV_RETRIES)){
				retries++;
				continue;
			}
			printf("Recv() Failed: rc=%d",rc);
			rc = EDB_NETWORK;
			goto error;
		}
	}
	rc = EDB_OK;
done:
	return rc;
error:
	goto done;
}



int ossSocket::connect(){
	int rc = EDB_OK;
	//SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	//ossSocket::initSocket();
	printf("edb客户端调用了connect，_fd:%d\n",_fd);
	//int ff = 0;
//	printf("------------%d\n",ntohs(_sockAddress.sin_port));
//	printf("%s\n",inet_ntoa(_sockAddress.sin_addr));
	rc = ::connect(_fd,(struct sockaddr *)&_sockAddress,_addressLen);//和服务器建立连接
	printf("connect return rc:%d\n",rc);
	if(rc){
		printf("Failed to connect ,rc = %d",SOCK_GETLASTERROR);
		rc = EDB_NETWORK;
		goto error;
	}
	rc = getsockname(_fd,(sockaddr*)&_sockAddress,&_addressLen);//getsockname会更改_sockAddress的内容！！
	if(rc){
		printf("Failed to get local address,rc = %d",rc);
		rc = EDB_NETWORK;
		goto error;
	}
	// get peer address
	rc = getpeername(_fd,(sockaddr*)&_peerAddress,&_peerAddressLen);
	if(rc){
		printf("Failed to get peer address, rc = %d",rc);
		rc = EDB_NETWORK;
		goto error;
	}
	//printf("_sockAddress.sin_port:%d\n",ntohs(_sockAddress.sin_port)); //_sockAddress.sin_port:32874
	//printf("_peerAddress.sin_port:%d",ntohs(_peerAddress.sin_port));_peerAddress.sin_port:48127
done:
	return rc;
error:
	goto done;
}

void ossSocket::close(){
	if(_init){
		int i = 0;
		i = ::close(_fd);
		if(i < 0){
			i = -1;
		}
		_init = false;
	}
}

int ossSocket::accept(int *sock,struct sockaddr *addr,socklen_t *addrLen,int timeout){
	int rc = EDB_OK;
	int maxFD = _fd;
	struct timeval maxSelectTime;
	fd_set fds;

	maxSelectTime.tv_sec = timeout / 1000000;
	maxSelectTime.tv_usec = timeout % 1000000;//微秒
	// wait loop until socket is ready
	while(true){
		FD_ZERO(&fds);
		FD_SET(_fd,&fds);
		rc = select(maxFD + 1,&fds,NULL,NULL,timeout>=0?&maxSelectTime:NULL);//timeout>=0?&maxSelectTime:NULL
		if(rc == 0){
			*sock = 0;
			rc = EDB_TIMEOUT;//将会总是从这一步开始返回
			goto done;
		}
		if(0 > rc){
			//printf("执行了没有222\n");
			rc = SOCK_GETLASTERROR;
			if(EINTR == rc){//内部系统调用中断所致，不算失败（再来一次）！
				continue;
			}
			PD_LOG(PDEVENT,"failed to select from socket,rc=%d",rc);
			rc = EDB_NETWORK;
			goto error;
		}
		if(FD_ISSET(_fd,&fds)){
			break;
		}
	}
	rc = EDB_OK;
	*sock = ::accept(_fd,addr,addrLen);//返回的是已连接套接字的fd
	PD_LOG(PDEVENT,"连接建立成功，已连接套接字fd:%d",*sock);
	if(-1 == *sock){
		PD_LOG(PDEVENT,"Failed to accept socket, rc=%d",SOCK_GETLASTERROR);
		rc = EDB_NETWORK;
		goto error;
	}
done:
	return rc;
error:
	close();
	goto done;
}

int ossSocket::disableNagle(){
	int rc = EDB_OK;
	int temp = 1;
	rc = setsockopt(_fd,IPPROTO_TCP,TCP_NODELAY,(char *)&temp,sizeof(int));
	if(rc){
		printf("failed to setsockopt, rc = %d",SOCK_GETLASTERROR);
	}
	rc = setsockopt(_fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&temp,sizeof(int));
	if(rc){
		printf("Failed to setsockopt,rc = %d",SOCK_GETLASTERROR);
	}
	return rc;
}

unsigned int ossSocket::_getPort(sockaddr_in *addr){
	return ntohs(addr->sin_port);
}

int ossSocket::_getAddress(sockaddr_in *addr, char *pAddress, unsigned int length){
	int rc = EDB_OK;
	length = length < NI_MAXHOST ? length:NI_MAXHOST;
	rc= getnameinfo((struct sockaddr *)addr,sizeof(sockaddr),pAddress,length,NULL,0,NI_NUMERICHOST);
	if(rc){
		printf("Failed to getnameinfo ,rc = %d",SOCK_GETLASTERROR);
		rc = EDB_NETWORK;
		goto error;
	}
done:
	return rc;
error:
	goto done;
}

unsigned int ossSocket::getLocalPort(){
	return _getPort(&_sockAddress);
}

unsigned int ossSocket::getPeerPort(){
	return _getPort(&_peerAddress);
}

int ossSocket::getLocalAddress(char *pAddress,unsigned int length){
	return _getAddress(&_sockAddress,pAddress,length);
}

int ossSocket::getPeerAddress(char *pAddress,unsigned int length){
	return _getAddress(&_peerAddress,pAddress,length);
}

int ossSocket::setTimeout(int seconds){
	int rc = EDB_OK;
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	//windows take milliseconds as parameter,but linux takes timeval as input
	rc = setsockopt(_fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv));
	if(rc){
		printf("Failed to setsockopt, rc = %d",SOCK_GETLASTERROR);
	}
	rc = setsockopt(_fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv));
	if(rc){
		printf("Failed to setsockopt, rc = %d",SOCK_GETLASTERROR);
	}
	return rc;
}

int _ossSocket::getHostName(char *pName,int nameLen){
	return gethostname(pName,nameLen);
}

int _ossSocket::getPort(const char *pServiceName,unsigned short &port){
	int rc = EDB_OK;
	struct servent *servinfo;
	servinfo = getservbyname(pServiceName,"tcp");
	if(!servinfo){
		port = atoi(pServiceName);//之前传入的字符串就是端口
	}else{
		port = (unsigned short)ntohs(servinfo->s_port);
	}
	return rc;
}
