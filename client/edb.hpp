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
#ifndef _EDB_HPP_
#define _EDB_HPP_

#include "core.hpp"
#include "ossSocket.hpp"
#include "commandFactory.hpp"
const int CMD_BUFFER_SIZE = 512;

class Edb{
public:
	Edb(){}
	~Edb(){}
	void start(void);
protected:
	void prompt(void);
private:
	void split(const std::string &text, char delim,std::vector<std::string> &result);
	char *readLine(char *p,int length);
	int readInput(const char *pPrompt,int numIndent);
private:
	ossSocket	_sock;//调用的是默认的构造函数
	CommandFactory _cmdFactory;
	char _cmdBuffer[CMD_BUFFER_SIZE];
};

#endif
