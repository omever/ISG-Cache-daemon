#include <iostream>

#include "dispatcher.h"

DispatcherOracle::DispatcherOracle(Dispatcher *d, Billing &b)
{
    _d = d;
    _bill = &b;
}

bool DispatcherOracle::processQuery(std::string &tagname)
{
    std::cerr << "DispatcherOracle" << std::endl;
    if(tagname == "status") {
	getOnlineData(_d->namedParam("pbhk"));
    } else
    if(tagname == "get_isg_services") {
	getIsgServices(_d->namedParam("pbhk"));
    } else
    if(tagname == "get_static_ip") {
	getStaticIP(_d->namedParam("ip"), _d->namedParam("server"));
    } else {
	return false;
    }
    return true;
}


int DispatcherOracle::getOnlineData(std::string pbhk)
{
    if(pbhk[0] != 'S' && pbhk[0] != 'I')
		pbhk = 'S' + pbhk;
	
	std::cerr << "quering online: " << pbhk << std::endl;
	std::string rv = _d->getCache(std::string("STATUS:") + pbhk);
	if(!rv.size()) {
		std::cerr << "Reading from source " << std::endl;
		queryResult r;

		_bi = new BillingInstance(*_bill);
		_bi->queryOnline(pbhk, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);
		_d->setCache(std::string("STATUS:") + pbhk, rv, 60);
	} else {
		std::cerr << "Cache hit" << std::endl;
	}

	_d->sendString(rv);
}

int DispatcherOracle::getIsgServices(std::string pbhk)
{
    if(pbhk[0] != 'S' && pbhk[0] != 'I')
		pbhk = 'S' + pbhk;
	
	std::cerr << "quering services: " << pbhk << std::endl;
	std::string rv = _d->getCache(std::string("ISG_SERVICES:") + pbhk);
	if(!rv.size()) {
		std::cerr << "Reading from source " << std::endl;
		queryResult r;
		
		_bi = new BillingInstance(*_bill);
		_bi->queryIsgServices(pbhk, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);
		_d->setCache(std::string("ISG_SERVICES:") + pbhk, rv, 60);
	} else {
		std::cerr << "Cache hit" << std::endl;
	}

	_d->sendString(rv);
}

int DispatcherOracle::getStaticIP(std::string ip, std::string server)
{
	std::cerr << "quering services: " << ip << " at " << server << std::endl;
	std::string rv = _d->getCache(std::string("STATIC_IP:") + server + ":" + ip);
	if(!rv.size()) {
		std::cerr << "Reading from source " << std::endl;
		queryResult r;

		_bi = new BillingInstance(*_bill);

		_bi->queryStaticIP(ip, server, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);
		_d->setCache(std::string("STATIC_IP:") + server + ":" + ip, rv, 600);
	} else {
		std::cerr << "Cache hit" << std::endl;
	}

	_d->sendString(rv);
}

void DispatcherOracle::timeout()
{
    if(_bi != NULL)
	_bi->cancelRequest();
}
