#include <vector>
#include <string>
#include <iostream>

#include "balance.h"
#include "utils.h"
#include "listener.h"

using namespace std;

Balance::Balance(Billing *b)
{
	_bill = b;
	user_id = 0;
	balance = 0;
	bytes_in_balance = 0;
	time_balance = 0;
	trust_limit = 0;
}

bool Balance::load(requestParams &params)
{
	vector<string> user_name = params["USER-NAME"];
	string sql;
	string framed_ip_address = ip2num(IFEXIST(params["FRAMED-IP-ADDRESS"]));
	vector<string> query_params;
	cerr << framed_ip_address << endl;
	
	
	if(user_name.empty() || user_name[0] == "static_ip" || user_name[0] == "ethernet") {
		sql = "SELECT a.user_id, c.balance, c.bytes_in_balance, c.time_balance, nvl(c.trast_limit, 0) trust_limit \
						FROM bill_user a, account c, ip2user d, server s \
						WHERE c.contract_id = a.contract_id \
						  AND a.user_type IN ('homenet', 'isg') \
						  AND a.user_id = d.user_id \
						  AND :1 BETWEEN d.ip_num_min AND d.ip_num_max \
						  AND d.server_name = s.server_name \
						  AND s.snmp_name = :2";
		
		query_params.push_back(framed_ip_address);
		query_params.push_back(IFEXIST(params["NAS-IP-ADDRESS"]));
		
	} else {
		sql = "SELECT a.user_id, c.balance, c.bytes_in_balance, c.time_balance, nvl(c.trast_limit, 0) trust_limit \
						FROM bill_user a, account c \
						WHERE c.contract_id = a.contract_id \
						  AND a.user_type IN ('homenet', 'isg') \
						  AND a.login = :1";
		query_params.push_back(user_name[0]);
	}
	
	
	queryResult r;
		
	_bi = new BillingInstance(*_bill);
	_bi->SQL(sql, query_params, r);
	delete _bi;
	_bi = NULL;

	if(r.empty()) {
		return false;
	}
	
	map<string,string>::iterator i;
	
	if((i = r[0].find("USER_ID")) != r[0].end()) {
		from_string<typeof(user_id)>(user_id, i->second);
	} else {
		user_id = -1;
	}
		
	if((i = r[0].find("BALANCE")) != r[0].end()) {
		from_string<double>(balance, i->second);
	} else {
		balance = 0;
	}
		
	if((i = r[0].find("BYTES_IN_BALANCE")) != r[0].end()) {
		from_string<int>(bytes_in_balance, i->second);
	} else {
		bytes_in_balance = 0;
	}
		
	if((i = r[0].find("TIME_BALANCE")) != r[0].end()) {
		from_string<int>(time_balance, i->second);
	} else {
		time_balance = 0;
	}
		
	if((i = r[0].find("TRUST_LIMIT")) != r[0].end()) {
		from_string<double>(trust_limit, i->second);
	} else {
		trust_limit = 0;
	}
	
	return true;
}
