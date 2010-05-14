#include <string>
#include <iostream>

#include "session.h"
#include "utils.h"
#include "balance.h"
#include "service.h"

using namespace std;

Session::Session()
{
	_active = false;
	__services.clear();
	__inactive_services.clear();
	_balance = NULL;
}

Session::~Session()
{
	if(_balance != NULL)
		delete _balance;
}

bool Session::is_active()
{
	return _active;
}

bool Session::match(requestParams &params)
{
	if(IFEXIST(params["CISCO-SERVICE-INFO"]).length() > 0) {
		string ps_id = params.avpair("parent-session-id");
		if(ps_id.length() && ps_id == _acct_session_id
			&& _nas_ip_address != IFEXIST(params["NAS-IP-ADDRESS"])
			&& _ip_address == IFEXIST(params["FRAMED-IP-ADDRESS"])
			&& _nas_port == IFEXIST(params["NAS-PORT-ID"])) {
			return true;
		} else {
			return false;
		}
	} else {
		if(_acct_unique_session_id != IFEXIST(params["ACCT-UNIQUE-SESSION-ID"])
			|| _nas_ip_address != IFEXIST(params["NAS-IP-ADDRESS"]))
			return false;
	}
	return true;
}

bool Session::submatch(requestParams &params)
{
	if(_ip_address == IFEXIST(params["FRAMED-IP-ADDRESS"])
		&& _login == IFEXIST(params["USER-NAME"])
		&& _pbhk == IFEXIST(params["CISCO-ACCOUNT-INFO"])
		&& _nas_port == IFEXIST(params["NAS-PORT-ID"])
		&& abs(_start_time - string_cast<int>(IFEXIST(params["TIMESTAMP"]))) <= 60
	) {
		return true;
	} else {
		return false;
	}
}

bool Session::_start_service(requestParams &params)
{
	cerr << "Starting service" << endl;
	std::string svc = params["CISCO-SERVICE-INFO"][0];
	if(__services.count(svc)) {
		// Сервис уже есть, проверить оно ли это
		if(__services[svc]->match(params)) { // Это тот-же сервис, что у нас есть, просто пропускаем запрос
			return true;
		} else { // фигня какая-то, завершаем старый сервис
			__services[svc]->finish();
			__inactive_services.push_back(__services[svc]);
			__services.erase(svc);
		}
	}
	Service *s = new Service();
	s->start(params);
	__services[svc] = s;
	return true;
}

bool Session::_start_session(requestParams &params)
{
	cerr << "Starting session" << endl;
	_active = true;
	if(_balance != NULL)
		_balance->load(params);
	_acct_session_id = IFEXIST(params["ACCT-SESSION-ID"]);
	_acct_unique_session_id = IFEXIST(params["ACCT-UNIQUE-SESSION-ID"]);
	_ip_address = IFEXIST(params["FRAMED-IP-ADDRESS"]);
	_nas_ip_address = IFEXIST(params["NAS-IP-ADDRESS"]);
	_login = IFEXIST(params["USER-NAME"]);
	
	for(int i=0; i<params["CISCO-ACCOUNT-INFO"].size(); ++i) {
		if(params["CISCO-ACCOUNT-INFO"][i][0] == 'S' || params["CISCO-ACCOUNT-INFO"][i][0] == 'I') {
			_pbhk = params["CISCO-ACCOUNT-INFO"][i];
		}
	}
	
	if(_pbhk.empty()) {
		_pbhk = std::string("S") + _ip_address;
	}
	_nas_port = IFEXIST(params["NAS-PORT-ID"]);
	_update_activity(params);
	
	return true;
}

bool Session::start(requestParams &params)
{
	if(IFEXIST(params["CISCO-SERVICE-INFO"]) != "") {
		return _start_service(params);
	} else {
		return _start_session(params);
	}
}

bool Session::_stop_service(requestParams &params)
{
	std::string svc = params["CISCO-SERVICE-INFO"][0];
	if(__services.count(svc)) {
		// Сервис уже есть, проверить оно ли это
		if(__services[svc]->match(params)) { // Это тот-же сервис
			return __services[svc]->stop(params);
		} else { // фигня какая-то
			cerr << "Session's service does not match" << endl;
		}
	}
	return false;
}

bool Session::_stop_session(requestParams &params)
{
	_active = false;
	_update_activity(params);
	return true;
}

bool Session::stop(requestParams &params)
{
	if(IFEXIST(params["CISCO-SERVICE-INFO"]) != "") {
		return _stop_service(params);
	} else {
		return _stop_session(params);
	}
}

bool Session::_update_service(requestParams &params)
{
	std::string svc = params["CISCO-SERVICE-INFO"][0];
	if(__services.count(svc)) {
		// Сервис уже есть, проверить оно ли это
		if(__services[svc]->match(params)) { // Это тот-же сервис
			return __services[svc]->update(params);
		} else { // фигня какая-то
			cerr << "Session's service does not match" << endl;
		}
	}
	return false;
}

bool Session::_update_session(requestParams &params)
{
	_update_activity(params);
	return true;
}

void Session::_update_activity(requestParams &params)
{
	_last_activity = time(NULL);
}

bool Session::update(requestParams &params)
{
	if(IFEXIST(params["CISCO-SERVICE-INFO"]) != "") {
		return _update_service(params);
	} else {
		return _update_session(params);
	}
}

time_t Session::last_activity()
{
	return _last_activity;
}

bool Session::forcedStop()
{
	std::map<std::string, Service*>::iterator i, iend = __services.end();
	for(i=__services.begin(); i != iend; ++i)
		i->second->dump();
}

bool Session::dump()
{
	cout << "SID: " << _acct_session_id
		 << "; USID: " << _acct_unique_session_id
		 << "; IP: " << _ip_address
		 << "; NAS: " << _nas_ip_address
		 << "; LOGIN: " << _login
		 << endl;

}
