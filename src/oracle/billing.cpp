//============================================================================
// Name        : billing.cpp
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : ISG Cache Daemon
//============================================================================

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <exception>
#include <sstream>

#include "billing.h"

using namespace oracle::occi;
using namespace std;

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

#define BINDABLE_BUFFER_SIZE 65536

Billing::Billing()
{
	pthread_mutex_init(&__mutex, NULL);
	__pool = NULL;
	__env = Environment::createEnvironment("UTF8", "UTF8", Environment::THREADED_MUTEXED);
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
	if(__env != NULL) {
		if(__pool != NULL)
			__env->terminateStatelessConnectionPool(__pool);
		Environment::terminateEnvironment(__env);
	}
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

	std::map<std::string, char*> _buffer;
	std::map<std::string, sb2> indp;
	std::map<std::string, ub2> alenp;
	vector<text*> defs;
	vector<string> colnames;
	vector<sb2*> indps;

	_buffer.clear();
	defs.clear();
	indps.clear();

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
		    if(_buffer.count((char*)bnvp[i]) > 0) {
			    std::cerr << "Double binded variable skipping doubles" << std::endl;
			    continue;
		    }
		    char * ptr = new char[BINDABLE_BUFFER_SIZE];
		    _buffer[(char*)bnvp[i]] = ptr;
		    memset(ptr, 0, BINDABLE_BUFFER_SIZE);

		    if((k = params.find(par)) != params.end() && k->second.size() >= 0) {
				strncpy(ptr, k->second.at(0).c_str(), BINDABLE_BUFFER_SIZE-1);
				alenp[(char*)bnvp[i]] = k->second.at(0).length()+1;
				indp[(char*)bnvp[i]] = 0;
			} else {
				alenp[(char*)bnvp[i]] = 0;
				indp[(char*)bnvp[i]] = -1;
			}
			std::cerr << "Bounding: " << OCIBindByName(
			    sth_oci, 
			    &hndl[i], 
			    error, 
			    bnvp[i], 
			    bnvl[i], 
			    ptr, 
			    BINDABLE_BUFFER_SIZE,
			    SQLT_STR, (void*)&(indp[(char*)(bnvp[i])]), &(alenp[(char*)(bnvp[i])]), (ub2*)0, (ub4)0, (ub4*)0, (ub4)0) << std::endl;
		}

	// Checking type of the query
		ub2 oci_stmt_type;

		checkerr(error, OCIAttrGet(sth_oci,
				OCI_HTYPE_STMT,
				(void*)&oci_stmt_type,
				0,
				OCI_ATTR_STMT_TYPE,
				error));

		ub4 iters;
		switch(oci_stmt_type) {
		case OCI_STMT_SELECT:
			iters = 0;
			break;
		default:
			iters = 1;
			break;
		}

		checkerr(error, OCIStmtExecute(
				_conn->getOCIServiceContext(),
				sth_oci,
				error,
				iters,
				0,
				(OCISnapshot*)0,
				(OCISnapshot*)0,
				OCI_DEFAULT
		));


		OCIParam *paramdp = NULL;
		ub4 counter = 1;
		sb4 parm_status;
		parm_status = OCIParamGet(sth_oci, OCI_HTYPE_STMT, error, (void**)&paramdp, counter);

		while (parm_status==OCI_SUCCESS) {
			ub2 dtype;
			text *col_name;
			ub4 col_name_len;
			/* Retrieve the data type attribute */
			checkerr(error, OCIAttrGet((dvoid*) paramdp, (ub4) OCI_DTYPE_PARAM,
					(dvoid*) &dtype, (ub4 *)0, (ub4) OCI_ATTR_DATA_TYPE, error));
			/* Retrieve the column name attribute */
			checkerr(error, OCIAttrGet((dvoid*) paramdp, (ub4) OCI_DTYPE_PARAM, (dvoid**) &col_name, (ub4 *)&col_name_len, (ub4) OCI_ATTR_NAME, error));

			ub2 deptlen;
			checkerr(error, OCIAttrGet(paramdp, OCI_DTYPE_PARAM, (dvoid*) &deptlen, (ub4 *) 0, (ub4) OCI_ATTR_DATA_SIZE, (OCIError *) error));

			// Some DB masters very like 8-bit encodings for no real reason.
			// Therefore we need to preserve up to 6 bytes per character to insure
			// buffer capacity (UTF8 uses up to 6 bytes per character)
			ub2 datalen = deptlen * 6 + 1;
			text *dept = new text[datalen];
			OCIDefine *defnp;

			defs.push_back(dept);
			colnames.push_back((const char*)col_name);
			indps.push_back(new sb2);

			if (parm_status = OCIDefineByPos(sth_oci, &defnp, error, counter, (ub1 *) dept, datalen, SQLT_STR, (dvoid *) indps.back(), (ub2 *) 0, (ub2 *)0, OCI_DEFAULT))
			{
				checkerr(error, parm_status);
				return(-1);
			}

			/* increment counter and get next descriptor, if there is one */
			counter++;
			parm_status = OCIParamGet(sth_oci, OCI_HTYPE_STMT, error, (void**)&paramdp, counter);
		}

		if(defs.size() > 0) {
			while(OCIStmtFetch(sth_oci, error, 1, OCI_FETCH_NEXT, OCI_DEFAULT) == OCI_SUCCESS) {
				std::multimap<std::string, std::string> tmp;
				tmp.clear();
				cerr << "Adding data defs sz=" << defs.size() << ";VALUE=";
				for(int i=0; i < defs.size(); ++i) {
					if(*indps.at(i) == -1) {
						tmp.insert(std::pair<std::string, std::string>(colnames.at(i), ""));
						cerr << "(NULL)";
					} else {
						tmp.insert(std::pair<std::string, std::string>(colnames.at(i), (const char*)defs.at(i)));
						cerr << (const char*)defs.at(i);
					}
					cerr << endl;
				}
				_rv.push_back(tmp);
			}
		}

		OCIHandleFree((dvoid *)error, (ub4)OCI_HTYPE_ERROR);

		if(_buffer.size()) {
			std::map<std::string, char *>::iterator i, iend = _buffer.end();
			std::map<std::string, std::vector<std::string> >::const_iterator k;
			for(i = _buffer.begin() ; i != iend ; ++i) {
				std::string par(i->first);
				std::transform(par.begin(), par.end(), par.begin(), ::tolower);
				if(
						(k = params.find(par)) != params.end()
						&& k->second.at(0).compare(i->second) == 0
						&& indp[i->first] != -1)
					continue;

				_rv.add_bind(i->first, i->second, indp[i->first] == -1);
			}
		}
		retval = 0;
	}
	catch (SQLException &sqlExcp)
	{
		stringstream temp;
		temp << "Oracle error at " << __FILE__ << "/" << __LINE__ << ": " << sqlExcp.getMessage();
		throw OracleException(temp.str());
	}

	if(sth != NULL) {
		_conn->terminateStatement(sth);
	}

	std::map<std::string, char *>::iterator i, iend = _buffer.end();
	cerr << "Cleaning buffers:";
	cerr << "_buffer sz=" << _buffer.size() << ";";
	for(i = _buffer.begin() ; i != iend ; ++i) 
		delete [] (i->second);

	cerr << "defs sz=" << defs.size() << endl;
	for(int i=0; i<defs.size(); ++i) {
		delete [] defs[i];
	}
	for(int i=0; i<indps.size(); ++i)
		delete indps[i];

	return retval;
}

bool BillingInstance::checkerr(OCIError *errhp, sword status)
{
	text errbuf[512];
	ub4 buflen;
	if (status == OCI_SUCCESS) return true;

	switch (status)
	{
		case OCI_SUCCESS_WITH_INFO:
			OCIErrorGet ((void *) errhp, (ub4) 1, (text *) NULL, &_errcode,
					errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
			_info = (const char*)errbuf;
			return true;
			break;
		case OCI_NEED_DATA:
			throw OracleException("OCI_NEED_DATA");
			break;
		case OCI_NO_DATA:
			throw OracleException("OCI_NO_DATA");
			break;
		case OCI_ERROR:
			OCIErrorGet ((void *) errhp, (ub4) 1, (text *) NULL, &_errcode,
					errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
			throw OracleException((const char*)errbuf);
			break;
		case OCI_INVALID_HANDLE:
			throw OracleException("OCI_INVALID_HANDLE");
			break;
		case OCI_STILL_EXECUTING:
			throw OracleException("OCI_STILL_EXECUTING");
			break;
		case OCI_CONTINUE:
			throw OracleException("OCI_CONTINUE");
			break;
		default:
			throw OracleException("OCI_ERROR");
			break;
	}
	return true;
}
