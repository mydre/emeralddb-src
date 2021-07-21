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
#include<iostream>
#include<sstream>
#include "core.hpp"
#include "edb.hpp"
#include "command.hpp"

const char SPACE = ' ';
const char TAB = '\t';
const char BACK_SLANT = '\\';
const char NEW_LINE = '\n';

int gQuit = 0;

void Edb::start(void){
	std::cout<<"Welcome to EmeraldDB Shell!"<<endl;
	std::cout<<"edb help for help, Ctrl+c or quit to exit"<<std::endl;
	while(gQuit == 0){
		prompt();//每次提示
	}
}

void Edb::prompt(void){
	int ret = EDB_OK;
	ret = readInput("edb",0);
	if(ret){
		return;
	}
	// Input string
	std::string textInput = _cmdBuffer;
	// Split the inputing sentence.
	std::vector<std::string> textVec;
	split(textInput,SPACE,textVec);
	int count = 0;
	std::string cmd = "";
	std::vector<std::string> optionVec;

	std::vector<std::string>::iterator iter = textVec.begin();
	// handle different command here.
	ICommand *pCmd = NULL;
	for(;iter != textVec.end();iter++){
		std::string str = *iter;
		if(count == 0){
			cmd = str;
			count++;
		}else{
			optionVec.push_back(str);
		}
	}
	pCmd = _cmdFactory.getCommandProcesser(cmd.c_str());
	if(NULL != pCmd){
		pCmd->execute(_sock,optionVec);
	}

}

int Edb::readInput(const char *pPrompt,int numIndent){
	memset(_cmdBuffer,0,CMD_BUFFER_SIZE);
	// print tab
	for(int i = 0;i<numIndent;i++){
		std::cout<<TAB;
	}
	// print "edb> "
	std::cout<<pPrompt<<"> ";
	// read a line from cmd;
	readLine(_cmdBuffer,CMD_BUFFER_SIZE-1);
	int curBufLen = strlen(_cmdBuffer);
	while(_cmdBuffer[curBufLen-1] == BACK_SLANT && (CMD_BUFFER_SIZE-curBufLen) > 0){
		for(int i = 0;i<numIndent;i++){
			std::cout<<TAB;
		}
		std::cout<<"> ";
		readLine(&_cmdBuffer[curBufLen-1],CMD_BUFFER_SIZE-curBufLen);
	}
	curBufLen = strlen(_cmdBuffer);
	for(int i = 0;i<curBufLen;i++){
		if(_cmdBuffer[i] == TAB){
			_cmdBuffer[i] = SPACE;
		}
	}
	return EDB_OK;

}

char *Edb::readLine(char *p,int length){
	int len = 0;
	int ch;
	while((ch=getchar()) != NEW_LINE){
		switch(ch){
		case BACK_SLANT:
			break;
		default:
			p[len] = ch;
			len++;
		}
		continue;
	}
	len = strlen(p);
	p[len] = 0;
	return p;
}

void Edb::split(const std::string &text,char delim,std::vector<std::string> &result){
	size_t strLen = text.length();
	size_t first = 0;
	size_t pos = 0;
	for(first = 0;first <strLen;first=pos+1){//下一个的first从pos+1开始
		pos = first;//first相当于是一个新的字符串
		while(text[pos] != delim && pos < strLen){
			pos++;
		}
		std::string str = text.substr(first,pos-first);
		result.push_back(str);
	}
	return;
}

int main(int argc,char **argv){
	Edb edb;
	edb.start();
	return 0;
}
