#ifndef SERVICE_H
#define SERVICE_H

#include "utils.h"
#include "listener.h"
#include "dispatcher.h"
#include "request_params.h"

class Service
{
	public:
		Service();
		~Service();
		bool start(requestParams &params);
		bool stop(requestParams &params);
		bool update(requestParams &params);
		bool match(requestParams &params);
		bool finish();
		bool dump();
	protected:
	private:
		std::string __service_name;
		std::string _acct_session_id;
		std::string _acct_unique_session_id;
		
		size64 input_octets;
		size64 output_octets;
		size64 input_packets;
		size64 output_packets;
		time_t session_time;
};

#endif
