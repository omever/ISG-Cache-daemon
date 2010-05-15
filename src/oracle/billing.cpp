//============================================================================
// Name        : billing.cpp
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : ISG Cache Daemon
//============================================================================

#include <occi.h>
#include <oci.h>
#include <oratypes.h>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "billing.h"

using namespace oracle::occi;

#define QUERY_ISG_ONLINE "SELECT io.user_id, \
								 to_char(io.start_time, 'HH24:MI:SS DD.MM') start_time, \
								 to_char(io.last_activ_time, 'HH24:MI:SS DD.MM') last_time, \
								 round(io.balance, 2) mb, \
								 round(io.money_val, 2) mv, \
								 round((io.bytes_in_balance/(1024*1024)), 2) bb, \
								 round((io.bytes_in_val/(1024*1024)), 2) bv, \
								 io.time_balance tb, \
								 io.time_val tv, \
								 p.plan_name_print plan \
						FROM isg_online io LEFT JOIN plan4user p4u ON (io.user_id = p4u.user_id) LEFT JOIN plan p ON (p4u.plan_id = p.plan_id) \
						WHERE isg_pbhk = :1 and acct_status=1"

#define QUERY_LOG_SRC "SELECT ipaddress,bytes_in \
  FROM ip_log_src_isg a full outer join ip2user b on (a.ipaddress = b.ip_num_min) \
	  WHERE bytes_in >1024 \
	    ORDER BY bytes_in"

#define QUERY_ISG_SERVICES "SELECT s.isg_service_id \
    FROM isg_online o, plan4user p, isg_plan2service s \
    WHERE o.user_id = p.user_id AND p.plan_id = s.plan_id AND s.auto = 0 AND o.acct_status = 1 AND o.isg_pbhk = :1"

#define QUERY_STATIC_IP "SELECT 'staticip' value \
    FROM bill_user a, server b, ip2user c \
    WHERE a.user_id = c.user_id AND c.server_name = b.server_name \
      AND c.ip_num_min = ip2num(:1) \
      AND b.snmp_name = :2 \
      AND a.state = 'on'"
      
Billing::Billing()
{
	pthread_mutex_init(&__mutex, NULL);
	__env = Environment::createEnvironment("UTF8", "UTF8", Environment::THREADED_MUTEXED);
	__pool = NULL;
}

Billing::Billing(std::string connstr, std::string username, std::string password)
{
	Billing();
	connect(connstr, username, password);
}

Environment* Billing::getEnv()
{
	return __env;
}

Billing::~Billing()
{
	pthread_mutex_destroy(&__mutex);
	__env->terminateStatelessConnectionPool(__pool);
	Environment::terminateEnvironment(__env);
}

void Billing::connect(std::string connstr, std::string username, std::string password)
{
	__pool = __env->createStatelessConnectionPool(username, password, connstr, 15, 1, StatelessConnectionPool::HOMOGENEOUS);
	__pool->setStmtCacheSize(50);
	__pool->setBusyOption(StatelessConnectionPool::NOWAIT);
}

Connection * Billing::yieldConnection()
{
	int rv = pthread_mutex_trylock(&__mutex);
	if(rv)
		return NULL;

	Connection *conn = NULL;
	try {
		conn = __pool->getConnection();
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   conn = NULL;
	}

	pthread_mutex_unlock(&__mutex);
	return conn;
}

void Billing::releaseConnection(Connection *conn)
{
	pthread_mutex_lock(&__mutex);
	try {
		__pool->releaseConnection(conn);
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	}

	pthread_mutex_unlock(&__mutex);
}

BillingInstance::BillingInstance(Billing &bill)
{
	_conn = bill.yieldConnection();
	_bill = &bill;
}

BillingInstance::~BillingInstance()
{
	if(_conn != NULL)
		_bill->releaseConnection(_conn);
}


int BillingInstance::queryOnline(std::string pbhk, queryResult &_rv)
{
	if(_conn == NULL) {
		return 0;
	}
	int retval = 0;
	Statement *sth = NULL;
	try {
		sth = _conn->createStatement(QUERY_ISG_ONLINE);

		sth->setAutoCommit(true);
		sth->setString(1, pbhk);

		std::cerr << "Query execute" << std::endl;
		ResultSet *rs = sth->executeQuery();
		const std::vector<MetaData> md = rs->getColumnListMetaData();

		if(rs) {
			while( rs->next() ) {
				std::multimap<std::string, std::string> tmp;
				tmp.clear();
				for(int i=0; i < md.size(); ++i) {
					tmp.insert(std::pair<std::string, std::string>(md.at(i).getString(MetaData::ATTR_NAME), rs->getString(i+1)));
				}
				_rv.push_back(tmp);
			}
		}
		sth->closeResultSet(rs);
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   retval = sqlExcp.getErrorCode();
	}

	if(sth != NULL) {
		try {
			_conn->terminateStatement(sth);
		}
		catch (SQLException &sqlExcp)
		{
		   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
		   retval = sqlExcp.getErrorCode();
		}
	}

	return retval;
}

int BillingInstance::queryIsgServices(std::string pbhk, queryResult &_rv)
{
	if(_conn == NULL) {
		return 0;
	}
	int retval = 0;
	Statement *sth = NULL;
	try {
		sth = _conn->createStatement(QUERY_ISG_SERVICES);

		sth->setAutoCommit(true);
		sth->setString(1, pbhk);

		std::cerr << "Query execute" << std::endl;
		ResultSet *rs = sth->executeQuery();
		const std::vector<MetaData> md = rs->getColumnListMetaData();

		if(rs) {
			while( rs->next() ) {
				std::multimap<std::string, std::string> tmp;
				tmp.clear();
				for(int i=0; i < md.size(); ++i) {
					tmp.insert(std::pair<std::string, std::string>(md.at(i).getString(MetaData::ATTR_NAME), rs->getString(i+1)));
				}
				_rv.push_back(tmp);
			}
		}
		sth->closeResultSet(rs);
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   retval = sqlExcp.getErrorCode();
	}

	if(sth != NULL) {
		try {
			_conn->terminateStatement(sth);
		}
		catch (SQLException &sqlExcp)
		{
		   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
		   retval = sqlExcp.getErrorCode();
		}
	}

	return retval;
}

int BillingInstance::queryStaticIP(std::string ip, std::string server, queryResult &_rv)
{
	if(_conn == NULL) {
		return 0;
	}
	int retval = 0;
	Statement *sth = NULL;
	try {
		sth = _conn->createStatement(QUERY_STATIC_IP);

		sth->setAutoCommit(true);
		sth->setString(1, ip);
		sth->setString(2, server);

		std::cerr << "Query execute" << std::endl;
		ResultSet *rs = sth->executeQuery();
		const std::vector<MetaData> md = rs->getColumnListMetaData();

		if(rs) {
			while( rs->next() ) {
				std::multimap<std::string, std::string> tmp;
				tmp.clear();
				for(int i=0; i < md.size(); ++i) {
					tmp.insert(std::pair<std::string, std::string>(md.at(i).getString(MetaData::ATTR_NAME), rs->getString(i+1)));
				}
				_rv.push_back(tmp);
			}
		}
		sth->closeResultSet(rs);
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   retval = sqlExcp.getErrorCode();
	}

	if(sth != NULL) {
		try {
			_conn->terminateStatement(sth);
		}
		catch (SQLException &sqlExcp)
		{
		   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
		   retval = sqlExcp.getErrorCode();
		}
	}

	return retval;
}

int BillingInstance::SQL(std::string query, std::vector<std::string> params, queryResult &_rv)
{
	if(_conn == NULL) {
		return 0;
	}
	int retval = 0;
	Statement *sth = NULL;

	try {
		sth = _conn->createStatement(query);

		sth->setAutoCommit(true);

		for(int i=0; i<params.size(); ++i) {
			sth->setString(i+1, params.at(i));
		}

		std::cerr << "Query execute" << std::endl;
		ResultSet *rs = NULL;
		int count = 0;

		switch(sth->execute()) {
			case Statement::RESULT_SET_AVAILABLE:
				rs = sth->getResultSet();
				break;
			case Statement::UPDATE_COUNT_AVAILABLE:
				count = sth->getUpdateCount();
				break;
			case Statement::NEEDS_STREAM_DATA:
			case Statement::PREPARED:
			case Statement::STREAM_DATA_AVAILABLE:
			case Statement::UNPREPARED:
			default:
				std::cerr << "Unable status type of the execute result method! Within query: " << query << std::endl;
				break;
		}
		if(rs != NULL) {
			const std::vector<MetaData> md = rs->getColumnListMetaData();

			if(rs) {
				while( rs->next() ) {
					std::multimap<std::string, std::string> tmp;
					tmp.clear();
					for(int i=0; i < md.size(); ++i) {
						tmp.insert(std::pair<std::string, std::string>(md.at(i).getString(MetaData::ATTR_NAME), rs->getString(i+1)));
					}
					_rv.push_back(tmp);
				}
			}
			sth->closeResultSet(rs);
		}
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   retval = sqlExcp.getErrorCode();
	}

	if(sth != NULL) {
		_conn->terminateStatement(sth);
	}

	return retval;
}

int BillingInstance::queryInitialSubscriber(std::string login, std::string password, queryResult &_rv) {
	std::vector<std::string> __params;
	__params.push_back(login);
	__params.push_back(password);

	return SQL("SELECT rc, user_id, pred_summ, balance, trast_limit \
		FROM TABLE(PKG_ACC.FIRSTLY_ABON_CHECK_ISG(:1, :2))", __params, _rv);
}

void BillingInstance::cancelRequest(void)
{
	if(_conn == NULL) {
		return;
	}

	std::cerr << "Cancelling request ";
	OCIError *errhp;
	
	OCIHandleAlloc(_bill->getEnv()->getOCIEnvironment() , (dvoid **)&errhp, (ub4)OCI_HTYPE_ERROR, (size_t)0, (dvoid **)0);

	OCISvcCtx *s = _conn->getOCIServiceContext();
	std::cerr << "(0x" << std::hex << (void*)s << ")";

	sword r = OCIBreak(s, errhp);
	std::cerr << "; OCIBreak=" << r;
	r = OCIReset(s, errhp);
	std::cerr << "; OCIReset=" << r << std::endl;

	OCIHandleFree((dvoid *)errhp, (ub4)OCI_HTYPE_ERROR);
}

int BillingInstance::querySQL(std::string query, const std::map<std::string, std::vector<std::string> > &params, queryResult &_rv)
{
	if(_conn == NULL) {
		return 0;
	}
	int retval = 0;
	Statement *sth = NULL;

	try {
		sth = _conn->createStatement(query);

		sth->setAutoCommit(true);

		OCIStmt * sth_oci = sth->getOCIStatement();
		OCIError * error;
		OCIHandleAlloc(_bill->getEnv()->getOCIEnvironment() , (dvoid **)&error, (ub4)OCI_HTYPE_ERROR, (size_t)0, (dvoid **)0);
		OCIBind * hndl[256];
		sb4 found;
		OraText *bnvp[256], *invp[256];
		ub1 bnvl[256], inpl[256], dupl[256];
		
		int retval = OCIStmtGetBindInfo(sth_oci, error, (ub4)256, (ub4)1, &found, bnvp, bnvl, invp, inpl, dupl, hndl);
		
		std::cerr << "Total " << found << " bound variables found! Error code = " << retval << std::endl;
		for(int i = 0; i < found; ++i)
		{
		    std::cerr << "Bound variable " << (char*)bnvp[i] << std::endl;
		    std::map<std::string, std::vector<std::string> >::const_iterator k;
		    std::string par((char*)bnvp[i]);
		    std::transform(par.begin(), par.end(), par.begin(), ::tolower);
		    if((k = params.find(par)) != params.end() && k->second.size() > 0) {
			std::cerr << "Bounding: " << OCIBindByName(
			    sth_oci, 
			    &hndl[i], 
			    error, 
			    bnvp[i], 
			    bnvl[i], 
			    (text*)k->second.at(0).c_str(), 
			    k->second.at(0).length()+1, 
			    SQLT_STR, (dvoid*)0, (ub2*)0, (ub2*)0, (ub4)0, (ub4*)0, (ub4)0) << std::endl;
		    } else {
			std::cerr << "Param " << (char*)bnvp[i] << " not found!" << std::endl;
		    }
		}

		OCIHandleFree((dvoid *)error, (ub4)OCI_HTYPE_ERROR);
		
		std::cerr << "Query execute" << std::endl;
		ResultSet *rs = NULL;
		int count = NULL;

		switch(sth->execute()) {
			case Statement::RESULT_SET_AVAILABLE:
				rs = sth->getResultSet();
				break;
			case Statement::UPDATE_COUNT_AVAILABLE:
				count = sth->getUpdateCount();
				break;
			case Statement::NEEDS_STREAM_DATA:
			case Statement::PREPARED:
			case Statement::STREAM_DATA_AVAILABLE:
			case Statement::UNPREPARED:
			default:
				std::cerr << "Unable status type of the execute result method! Within query: " << query << std::endl;
				break;
		}
		if(rs != NULL) {
			const std::vector<MetaData> md = rs->getColumnListMetaData();

			if(rs) {
				while( rs->next() ) {
					std::multimap<std::string, std::string> tmp;
					tmp.clear();
					for(int i=0; i < md.size(); ++i) {
						tmp.insert(std::pair<std::string, std::string>(md.at(i).getString(MetaData::ATTR_NAME), rs->getString(i+1)));
					}
					_rv.push_back(tmp);
				}
			}
			sth->closeResultSet(rs);
		}
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
	   std::cerr <<sqlExcp.getErrorCode() << " at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage() << std::endl;
	   retval = sqlExcp.getErrorCode();
	}

	if(sth != NULL) {
		_conn->terminateStatement(sth);
	}

	return retval;
}

