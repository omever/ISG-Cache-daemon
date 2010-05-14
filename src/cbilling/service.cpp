#include "service.h"
#include <iostream>

using namespace std;

Service::Service()
{
	input_octets = 0;
	output_octets = 0;
	input_packets = 0;
	output_packets = 0;
	
	__service_name = "";
	_acct_session_id = "";
	_acct_unique_session_id = "";
}

Service::~Service()
{
}

bool Service::dump()
{
	cout << "\t" << __service_name
		 << "; IN: " << input_octets << "b/" << input_packets 
		 << "p; OUT: " << output_octets << "b/" << output_packets << "p" << endl;
}
bool Service::start(requestParams &params)
{
	__service_name = IFEXIST(params["CISCO-SERVICE-INFO"]);
	_acct_session_id = IFEXIST(params["ACCT-SESSION-ID"]);
	_acct_unique_session_id = IFEXIST(params["ACCT-UNIQUE-SESSION-ID"]);
	
	input_octets = 0;
	output_octets = 0;
	input_packets = 0;
	output_packets = 0;
}

bool Service::update(requestParams &params)
{
	size64 tmp;

	tmp = string_cast<size64>(IFEXIST(params["ACCT-INPUT-OCTETS"]));
	input_octets = tmp + (tmp < input_octets?4294967296LL:0);

	tmp = string_cast<size64>(IFEXIST(params["ACCT-OUTPUT-OCTETS"]));
	output_octets = tmp + (tmp < output_octets?4294967296LL:0);

	tmp = string_cast<size64>(IFEXIST(params["ACCT-INPUT-PACKETS"]));
	input_packets = tmp + (tmp < input_packets?4294967296LL:0);

	tmp = string_cast<size64>(IFEXIST(params["ACCT-OUTPUT-PACKETS"]));
	output_packets = tmp + (tmp < output_packets?4294967296LL:0);
}

bool Service::stop(requestParams &params)
{
	update(params);
}

bool Service::match(requestParams &params)
{
	if(__service_name == IFEXIST(params["CISCO-SERVICE-INFO"]) &&
		_acct_session_id == IFEXIST(params["ACCT-SESSION-ID"]) &&
		_acct_unique_session_id == IFEXIST(params["ACCT-UNIQUE-SESSION-ID"]))
		return true;
	
	return false;
}

bool Service::finish()
{
}

