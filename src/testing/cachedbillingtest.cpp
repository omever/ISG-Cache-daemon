/*
 *      cachedbillingtest.cpp
 *      
 *      Copyright 2009 Grigory Holomiev <omever@gmail.com>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */


#include <iostream>
#include "../dispatcher/cbilling.h"
#include "../dispatcher/dispatcher.h"
#include "../oracle/billing.h"
#include "../radius/dictionary.h"
#include "../radius/radpacket.h"

using namespace std;

int main(int argc, char** argv)
{
	Billing _bill;

	try {
		_bill.connect("(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=217.113.112.22)(PORT=1521)))(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=BILL)))", "BILL_DBA", "DjkuJLjycrbQ");
	}
	catch (std::exception &ex) {
		std::cerr << "Exception caught while executing query: " << ex.what() << std::endl;
		return -1;
	}
	
	requestParams query;
	query["USER-NAME"].push_back("ethernet");
	query["FRAMED-IP-ADDRESS"].push_back("217.113.123.55");
	query["NAS-IP-ADDRESS"].push_back("217.113.112.11");
	
	CachedBilling::Balance balance(&_bill);
	if(balance.load(query)) {
	    cout << "Got data" << endl;
	} else {
	    cout << "Empty data" << endl;
	}
	
	cout << "Results are: " << endl
	     << "\tUSER_ID: " << balance.user_id << endl
	     << "\tBALANCE: " << balance.balance << endl
	     << "\tBYTES: " << balance.bytes_in_balance << endl
	     << "\tTIME: " << balance.time_balance << endl
	     << "\tTRUST_LIMIT: " << balance.trust_limit << endl;
	cout << "All ok" << endl;
	return 0;
}
