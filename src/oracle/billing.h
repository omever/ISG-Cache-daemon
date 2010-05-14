#ifndef BILLING_H
#define BILLING_H

#include <string>
#include <vector>
#include <map>
#include <pthread.h>

namespace oracle {
	namespace occi {
		class Environment;
		class Connection;
		class Statement;
		class StatelessConnectionPool;
	};
};

typedef std::vector<std::multimap<std::string, std::string> > queryResult;

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
	void cancelRequest(void);
protected:
private:
	oracle::occi::Connection *_conn;
	Billing *_bill;
};
#endif
