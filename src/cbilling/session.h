#ifndef SESSION_H
#define SESSION_H

#include <list>
#include <map>
#include "request_params.h"

class Listener;
class Dispatcher;
class Balance;
class Service;

class Session
{
	public:
		Session();
		~Session();
		bool start(requestParams &params);
		bool stop(requestParams &params);
		bool update(requestParams &params);
		bool match(requestParams &params);
		bool submatch(requestParams &params);
		bool is_active();
		bool forcedStop();
		bool dump();
		time_t last_activity();
	protected:
		bool _active;
		Balance *_balance;
		std::string _ip_address;
		std::string _nas_ip_address;
		std::string _login;
		std::string _pbhk;
		std::string _nas_port;
		std::string _acct_session_id;
		std::string _acct_unique_session_id;
		time_t _start_time;
		time_t _last_activity;
		
	private:
		std::map<std::string, Service*> __services;
		std::list<Service*> __inactive_services;
		
		bool _start_service(requestParams &params);
		bool _start_session(requestParams &params);
		bool _stop_service(requestParams &params);
		bool _stop_session(requestParams &params);
		bool _update_service(requestParams &params);
		bool _update_session(requestParams &params);
		
		void _update_activity(requestParams &params);
};

#endif
