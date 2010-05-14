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
		getOnlineData(_d->namedParam("pbhk"), _d->namedParam("force"));
    } else
    if(tagname == "get_isg_services") {
		getIsgServices(_d->namedParam("pbhk"));
    } else
    if(tagname == "get_static_ip") {
		getStaticIP(_d->namedParam("ip"), _d->namedParam("server"));
    } else
    if(tagname == "check_initial_subscriber") {
		checkInitialSubscriber(_d->namedParam("login"), _d->namedParam("password"));
    } else
    if(tagname == "raw_sql") {
		querySQL(_d->namedParam("sql"), _d->namedParams("arg"));
    } else {
		return false;
    }
    return true;
}


int DispatcherOracle::getOnlineData(std::string pbhk, std::string force)
{
    if(pbhk[0] != 'S' && pbhk[0] != 'I')
		pbhk = 'S' + pbhk;
	
	std::cerr << "quering online: " << pbhk << std::endl;
	std::string rv;
	if(!force.size()) {
	    rv = _d->getCache(std::string("STATUS:") + pbhk);
	}
	if(!rv.size()) {
		std::cerr << "Reading from source " << std::endl;
		queryResult r;

		_bi = new BillingInstance(*_bill);
		_bi->queryOnline(pbhk, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);
		_d->setCache(std::string("STATUS:") + pbhk, rv, 20);
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

int DispatcherOracle::checkInitialSubscriber(std::string login, std::string password) {
	std::cerr << "quering initial subscriber: " << login << "\t" << password << std::endl;
	std::string rv = _d->getCache(std::string("INITIAL_SUBSCRIBER:") + login + ":" + password);
	if(!rv.size()) {
		std::cerr << "Reading from source" << std::endl;
		queryResult r;

		_bi = new BillingInstance(*_bill);

		_bi->queryInitialSubscriber(login, password, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);
		_d->setCache(std::string("INITIAL_SUBSCRIBER:") + login + ":" + password, rv, 3600);
	} else {
		std::cerr << "Cache hit" << std::endl;
	}

	_d->sendString(rv);
}

int DispatcherOracle::querySQL(std::string sql, std::vector<std::string> params, std::string key, int timeout)
{
	std::cerr << "quering raw SQL: " << sql << std::endl;
	std::string rv;
	if(timeout) {
		rv = _d->getCache(std::string("RAW_SQL:") + key);
	}
	if(!rv.size()) {
		std::cerr << "Reading from source" << std::endl;
		queryResult r;

		_bi = new BillingInstance(*_bill);

		_bi->querySQL(sql, params, r);

		delete _bi;
		_bi = NULL;

		rv = _d->wrapResult(r);

		if(timeout) {
			_d->setCache(std::string("RAW_SQL:") + key, rv, timeout);
		}
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
