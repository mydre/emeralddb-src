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
#ifndef OSSSOCKET_HPP__
#define OSSSOCKET_HPP__
#include "core.hpp"
#define SOCK_GETLASTERROR errno

// by default 10ms timeout

// max hostname
#define OSS_MAX_HOSTNAME NI_MAXHOST
#define OSS_MAX_SERVICENAME NI_MAXSERV

class _ossSocket{
private:
	int _fd;
	socklen_t _addressLen;
	socklen_t _peerAddressLen;
	struct sockaddr_in _sockAddress;
	struct sockaddr_in _peerAddress;
	bool _init;
	int _timeout;
protected:
	unsigned int _getPort(sockaddr_in *addr);
	int _getAddress(sockaddr_in *addr,char *pAddress, unsigned int length);
public:
	int setSocketLi(int lOnOff,int linger);
	void setAddress(const char *pHostName, unsigned int port);
	// create a listening socket
	_ossSocket();//the declare of function
	_ossSocket(unsigned int port,int timeout = 0);
	// create a connect socket
	_ossSocket(const char *pHostname,unsigned int port,int timeout);
	// create from a existing socket
	_ossSocket(int *sock,int timeout = 0);
	~_ossSocket(){
		close();
	}
	int initSocket();
	int bind_listen();
	int send(const char *pMsg,int len,int timeout = OSS_SOCKET_DEF_TIMEOUT,
			 int flags = 0);
	int recv(char *pMsg,int len,
			 int timeout = OSS_SOCKET_DEF_TIMEOUT,
			 int flags = 0);
	int recvNF(char *pMsg,int len,int timeout = OSS_SOCKET_DEF_TIMEOUT);
	int connect();
	void close();
	int accept(int *sock,struct sockaddr *addr,socklen_t *addrlen,
			  int timeout = OSS_SOCKET_DEF_TIMEOUT);
	int disableNagle();//Nagle represent a special operation.(compact several packets)
	unsigned int getPeerPort();
	int getPeerAddress(char *pAddress,unsigned int length);
	unsigned int getLocalPort();
	int getLocalAddress(char *pAddress,unsigned int length);
	int setTimeout(int seconds);
	static int getHostName(char *pName,int nameLen);
	static int getPort(const char *pServiceName,unsigned short &port);
	bool isConnected();
	int getFD();
	bool getInit();
};
typedef class _ossSocket ossSocket;

#endif
