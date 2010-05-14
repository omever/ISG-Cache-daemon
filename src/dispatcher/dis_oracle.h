//============================================================================
// Name        : oracle.h
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#ifndef DISPATCHER_ORACLE_H
#define DISPATCHER_ORACLE_H

class DispatcherOracle
{
public:
    DispatcherOracle(Dispatcher *d, Billing &bill);
    void timeout();
protected:
	int getOnlineData(std::string pbhk);
	int getIsgServices(std::string pbhk);
	int getStaticIP(std::string ip, std::string server);
	
	bool processQuery(std::string &);
private:
    Billing * _bill;
    BillingInstance * _bi;
    Dispatcher *_d;
};

#endif
