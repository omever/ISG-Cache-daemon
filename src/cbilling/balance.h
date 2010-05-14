#ifndef BALANCE_H
#define BALANCE_H

#include "dispatcher.h"
#include "request_params.h"

class Balance
{
public:
	Balance(Billing *b);
	bool load(requestParams &params);
	
	int user_id;
	double balance;
	int bytes_in_balance;
	int time_balance;
	double trust_limit;
private:
	Billing *_bill;
	BillingInstance *_bi;
};

#endif
