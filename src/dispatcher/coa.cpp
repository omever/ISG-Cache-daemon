#include <iostream>

#include "dispatcher.h"
#include "radclient.h"
#include "radpacket.h"
#include "coapacket.h"

DispatcherCOA::DispatcherCOA(Dispatcher *d, Listener &l)
{
    _d = d;
    _listener = &l;
    _dict = &(l.dict);
}

RadPacket DispatcherCOA::genRadiusPacket()
{
	RadPacket packet(_dict);
	packet.setSecret("cisco");
	packet.setIdentifier(_listener->getNextId()%256);
	return packet;
}

int DispatcherCOA::getRadiusReply(RadClient &client, RadPacket &reply)
{
	try {
		client.read(reply);
	}
	catch (std::exception &e) {
		std::cerr << "Radius reply error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}

int DispatcherCOA::sendRadiusPacket(std::string &pbhk, RadPacket &packet, RadPacket &reply)
{
	std::string server;
	size_t pos = pbhk.find(':');

	if(pos == std::string::npos) {
		server = "217.113.112.11";
	} else {
		server = pbhk.substr(0, pos);
	}

	if(server[0] == 'S') {
		server = server.substr(1);
	}

	RadClient client(server, 1813, "cisco");

	try {
	    client.send(packet);
	}
	catch (std::exception &e) {
		std::cerr << "Radius reply error: " << e.what() << std::endl;
		return -1;
	}
	
	return getRadiusReply(client, reply);
}

int DispatcherCOA::wrapRadiusPacket(RadPacket &reply, queryResult &rv)
{
	std::vector<std::string> tmp;
	std::vector<std::string>::iterator i, iend;
	std::multimap<std::string, std::string> m;
	reply.dump();

	
	m.insert(std::pair<std::string, std::string>("Code", reply.getCode()));
	tmp = reply.getVSAttrValues("Cisco", "Cisco-Command-Code");
	
	for(i=tmp.begin(), iend=tmp.end(); i!=iend; ++i) {
		m.insert(std::pair<std::string, std::string>("Cisco-Command-Code", (*i).substr(1)));
	}

	tmp = reply.getVSAttrValues("Cisco", "Cisco-Account-Info");
	for(int i=0; i<tmp.size(); ++i) {
		m.insert(std::pair<std::string, std::string>("Cisco-Account-Info", tmp[i]));
	}

	tmp = reply.getAttrValues("Framed-Address");
	for(int i=0; i<tmp.size(); ++i) {
		m.insert(std::pair<std::string, std::string>("Framed-IP-Address", tmp[i]));
	}

	tmp = reply.getAttrValues("NAS-Port-Id");
	for(int i=0; i<tmp.size(); ++i) {
		m.insert(std::pair<std::string, std::string>("NAS-Port-Id", tmp[i]));
	}

	tmp = reply.getAttrValues("EAP-Type-SIM");
	for(int i=0; i<tmp.size(); ++i) {
		m.insert(std::pair<std::string, std::string>("EAP-Type-SIM", tmp[i]));
	}

	rv.push_back(m);
}

int DispatcherCOA::isgPing(std::string pbhk) 
{
	queryResult rv;
	RadPacketCoA packet(genRadiusPacket());
	RadPacket reply(genRadiusPacket());

	packet.ping(pbhk);
	if(sendRadiusPacket(pbhk, packet, reply) != -1) {
		wrapRadiusPacket(reply, rv);
	}

	_d->sendResult(rv);
}

int DispatcherCOA::isgLoggon(std::string pbhk, std::string login, std::string password)
{
	queryResult rv;
	RadPacketCoA packet(genRadiusPacket());
	RadPacket reply(genRadiusPacket());

	packet.loggon(login, password, pbhk);
	if(sendRadiusPacket(pbhk, packet, reply) != -1) {
		wrapRadiusPacket(reply, rv);
	}

	_d->sendResult(rv);
}

int DispatcherCOA::isgLogout(std::string pbhk)
{
	queryResult rv;
	RadPacketCoA packet(genRadiusPacket());
	RadPacket reply(genRadiusPacket());

	packet.logout(pbhk);

	rv.resize(1);
	rv[0].insert(std::pair<std::string, std::string>("CODE", "CoA-ACK"));

	_d->sendResult(rv);

	int stime = 1000;
	struct timeval tv;
	tv.tv_sec = stime/1000;
	tv.tv_usec = stime%1000;
	    
	select(0, NULL, NULL, NULL, &tv);

	sendRadiusPacket(pbhk, packet, reply);
}

int DispatcherCOA::isgServiceOn(std::string pbhk, std::string service)
{
	queryResult rv;
	RadPacketCoA packet(genRadiusPacket());
	RadPacket reply(genRadiusPacket());

	packet.serviceon(pbhk, service);
	if(sendRadiusPacket(pbhk, packet, reply) != -1) {
		wrapRadiusPacket(reply, rv);
	}

	_d->sendResult(rv);
}

int DispatcherCOA::isgServiceOff(std::string pbhk, std::string service)
{
	queryResult rv;
	RadPacketCoA packet(genRadiusPacket());
	RadPacket reply(genRadiusPacket());

	packet.serviceoff(pbhk, service);
	if(sendRadiusPacket(pbhk, packet, reply) != -1) {
		wrapRadiusPacket(reply, rv);
	}

	_d->sendResult(rv);
}

bool DispatcherCOA::processQuery(std::string &tagname)
{
    std::cerr << "DispatcherCOA" << std::endl;
    if(tagname == "isg_ping") {
		isgPing(_d->namedParam("pbhk"));
    } else
    if(tagname == "isg_logon") {
		isgLoggon(_d->namedParam("pbhk"), _d->namedParam("login"), _d->namedParam("password"));
    } else
    if(tagname == "isg_logout") {
		isgLogout(_d->namedParam("pbhk"));
		return true;
    } else
    if(tagname == "isg_service_on") {
		isgServiceOn(_d->namedParam("pbhk"), _d->namedParam("service"));
    } else
    if(tagname == "isg_service_off") {
		isgServiceOff(_d->namedParam("pbhk"), _d->namedParam("service"));
    } else
    {
		return false;
    }
    return true;
}

void DispatcherCOA::timeout()
{
}
