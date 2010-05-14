//============================================================================
// Name        : coa.h
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#ifndef DISPATCHER_COA_H
#define DISPATCHER_COA_H

class RadClient;
class RadPacket;

class DispatcherCOA
{
public:
    DispatcherCOA(Dispatcher *d, Listener &l);
    void timeout();
protected:
	int isgPing(std::string pbhk);
	int isgLoggon(std::string pbhk, std::string login, std::string password);
	int isgLogout(std::string pbhk);
	int isgServiceOn(std::string pbhk, std::string service);
	int isgServiceOff(std::string pbhk, std::string service);

	int getRadiusReply(RadClient&, RadPacket &);
	int sendRadiusPacket(std::string&, RadPacket&, RadPacket&);
	RadPacket genRadiusPacket();
	int wrapRadiusPacket(RadPacket&, queryResult&);
	
	bool processQuery(std::string &);
private:
    RadiusDictionary *_dict;
    Listener *_listener;
    Dispatcher *_d;
};

#endif
