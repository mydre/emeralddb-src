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
#ifndef CORE_HPP__
#define CORE_HPP__

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<linux/limits.h>
#include<sys/time.h>
#include<time.h>
#include<stdarg.h>
#include<unistd.h>
#include<syscall.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/tcp.h>
#include<sys/mman.h>

#include<string>
#include<map> //this use red-black-tree
#include<set>
#include<vector>
#include<iostream>

#define OSS_MAX_PATHSIZE PATH_MAX
#define OSS_FILE_SEP_STR "/"
#define OSS_FILE_SEP_CHAR *((const char *)OSS_FILE_SEP_STR)[0]
#define OSS_NEWLINE "\n" // "\r\n" correspond in windows
#define OSS_SOCKET_DEF_TIMEOUT 500000

// error code list
#define EDB_OK 0
#define EDB_IO -1
#define EDB_INVALIDARG 				-2
#define EDB_PERM 					-3
#define EDB_OOM 					-4
#define EDB_SYS 					-5
#define EDB_PMD_HELP_ONLY 			-6
#define EDB_PMD_FORCE_SYSTEM_EDU 	-7
#define EDB_TIMEOUT 				-8
#define EDB_QUIESCED 				-9
#define EDB_EDU_INVAL_STATUS 		-10
#define EDB_NETWORK 				-11
#define EDB_NETWORK_CLOSE 			-12
#define EDB_APP_FORCED 				-13
#define EDB_IXM_ID_EXIST 			-14
#define EDB_HEADER_INVALID 			-15
#define EDB_IXM_ID_NOT_EXIST 		-16
#define EDB_NO_ID 					-17

inline void myP(const std::string &ss){
	std::cout<<ss<<std::endl;
}

#endif
