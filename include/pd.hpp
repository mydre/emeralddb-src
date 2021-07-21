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

#ifndef PD_HPP__
#define PD_HPP__
#include<string>
#define PD_LOG_STRINGMAX 4096
#define PD_LOG(level,fmt,...)												\
	do{																		\
		if(_curPDLevel >= level)											\
		{																	\
			pdLog(level,__func__,__FILE__,__LINE__,fmt, ##__VA_ARGS__);		\
		}																	\
	}while(0)																\

#define PD_CHECK(cond,retCode,gotoLabel,level,fmt,...)						\
	do{																		\
		if(!(cond))															\
		{																	\
			rc = (retCode);													\
			PD_LOG((level),fmt,##__VA_ARGS__);								\
			goto gotoLabel;													\
		}																	\
	}while(0)																\

#define PD_RC_CHECK(rc,level,fmt,...)										\
	do{																		\
		PD_CHECK((EDB_OK==(rc)),(rc),error,(level),fmt,##__VA_ARGS__);		\
	}while(0)																\

#define EDB_VALIDATE_GOTOERROR(cond,ret,str)								\
	{if(!cond){pdLog(PDERROR,__func__,__FILE__,__LINE__,str);rc=ret;goto error;}}

#ifdef _DEBUG
#define EDB_ASSERT(cond,str)													\
	{if(!(cond)){pdassert(str,__func__,__FILE__,__LINE__);}}					\

#define EDB_CHECK(cond,str)														\
		{if(!(cond)){pdcheck(str,__func__,__FILE__,__LINE__);}}					\

#else
#define EDB_ASSERT(cond,str) {if(cond){}}
#define EDB_CHECK(cond,str) {if(cond){}}
#endif

enum PDLEVEL
{
   PDSEVERE = 0,
   PDERROR,
   PDEVENT,
   PDWARNING,
   PDINFO,
   PDDEBUG
} ;
extern PDLEVEL _curPDLevel;
const char *getPDLevelDesp(PDLEVEL level);

#define PD_DFT_DIAGLEVEL PDWARNING
void pdLog(PDLEVEL level,const char *func,const char *file,unsigned int line,const char *format,...);
void pdLog ( PDLEVEL level, const char *func, const char *file,unsigned int line, std::string message ) ;
#endif
