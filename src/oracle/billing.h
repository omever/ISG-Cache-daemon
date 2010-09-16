#ifndef BILLING_H
#define BILLING_H

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <pthread.h>
#include <oci.h>
#include <occi.h>
#include <oratypes.h>

#include "queryresult.h"

namespace oracle {
	namespace occi {
		class Environment;
		class Connection;
		class Statement;
		class StatelessConnectionPool;
	};
};

/**
 * Oracle exceptions
 */
class OracleException : public std::exception
{
public:
	OracleException(const std::string &description){
		__description = description;
	}

	OracleException(const char * description){
		__description = description;
	}

	virtual ~OracleException() throw () {

	}

	virtual const char* what() const throw()
	{
		return __description.c_str();
	}
private:
	std::string __description;
};


class Billing
{
public:
    Billing();
    Billing(std::string connstr, std::string username, std::string password);
    ~Billing();
	oracle::occi::Environment* getEnv();

    void connect(std::string connstr, std::string username, std::string password);
	oracle::occi::Connection *yieldConnection();
	void releaseConnection(oracle::occi::Connection *);

	pthread_mutex_t __mutex;
private:
	oracle::occi::Environment *__env;
	oracle::occi::StatelessConnectionPool *__pool;
};


class BillingInstance
{
public:
	BillingInstance(Billing &bill);
	~BillingInstance();

	int SQL(std::string query, std::vector<std::string> params, queryResult &_rv);

	int queryOnline(std::string pbhk, queryResult &_rv);
	int queryIsgServices(std::string pbhk, queryResult &_rv);
	int queryStaticIP(std::string ip, std::string server, queryResult &_rv);
	int queryInitialSubscriber(std::string login, std::string password, queryResult &_rv);
	int querySQL(std::string query, const std::map<std::string, std::vector<std::string> > &params, queryResult &_rv);
	void cancelRequest(void);
protected:
	bool checkerr(OCIError *errhp, sword status);
	std::string _info;
	sb4 _errcode;
private:
	oracle::occi::Connection *_conn;
	Billing *_bill;
};
#endif
