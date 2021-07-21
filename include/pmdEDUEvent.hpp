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

#ifndef PMDEDUEVENT_HPP__
#define PMDEDUEVENT_HPP__

#include "core.hpp"
#define pmdEDUEventType int
enum pmdEDUEventTypes{
	PMD_EDU_EVENT_NONE = 0,
	PMD_EDU_EVENT_TERM,
	PMD_EDU_EVENT_RESUME,
	PMD_EDU_EVENT_ACTIVE,
	PMD_EDU_EVENT_DEACTIVE,
	PMD_EDU_EVENT_MSG,
	PMD_EDU_EVENT_TIMEOUT,
	PMD_EDU_EVENT_LOCKWAKEUP
};

class pmdEDUEvent{

public:
	pmdEDUEvent():_eventType(PMD_EDU_EVENT_NONE),
	_release(false),
	_Data(NULL){}

	pmdEDUEvent(pmdEDUEventTypes type):_eventType(type),_release(false),_Data(NULL){}

	pmdEDUEvent(pmdEDUEventTypes type,bool release,void *data)://这一行要是data
			_eventType(type),
			_release(release),
			_Data(data){}

	void reset(){
		_eventType = PMD_EDU_EVENT_NONE;
		_release = false;
		_Data = NULL;
	}
//private:,人家没说这是私有的成员变量
	pmdEDUEventType _eventType;
	bool _release;
	void *_Data;

};

#endif
