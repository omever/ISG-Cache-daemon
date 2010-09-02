//============================================================================
// Name        : dispatcher.h
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <libxml/parser.h>
#include <vector>
#include <string>
#include "billing.h"
#include "listener.h"
#include "coa.h"
#include "oracle.h"

enum stateEnum
{
	    WAIT_ROOT_EL,
	    READING_DATA,
		WAIT_PARAMS,
	    FINISHING
};

class Dispatcher : protected DispatcherCOA, protected DispatcherOracle
{
public:
	Dispatcher(int socket, Billing &bill, Listener &l);
	~Dispatcher();

	int start();
	unsigned int last_activity();
	static void * thread_handle(void*);
	bool is_done();
	void timeout();

	const std::string wrapResult(queryResult &r);
	void error(std::string errorstring);

	void sendResult(queryResult &r);
	void sendString(std::string &data);
	const std::string & namedParam(const std::string &paramName) const;
	const std::string & namedParam(const char *paramName) const;
	const std::vector<std::string> & namedParams(const std::string &paramName) const;
	const std::map<std::string, std::vector<std::string> > & allNamedParams() const;
	
	void lock();
	void unlock();
	
	std::string getCache(std::string key);
	void setCache(std::string key, std::string value, int timeout = 600);
protected:
	void mainloop();

	static void parserStartElement(void *ctx, const xmlChar * fullname, const xmlChar ** attr);
	static void parserEndElement(void *ctx, const xmlChar * fullname);
	static void parserWarning(void *ctx, const char *msg, ...);
	static void parserError(void *ctx, const char *msg, ...);
	static void parserEndDocument(void *ctx);
	static int resultHandler(void *ctx, const char *data, int len);
	static int closeHandler(void *ctx);

	bool processQuery(std::string fullname);
private:
	int _sd;
	int _ts;
	bool _is_done;
	bool _is_processing;
	xmlSAXHandler *_xmlHandler;
	stateEnum _state;
	int _depth;
	std::vector<std::string> _params;
	std::map<std::string, std::vector<std::string> > _named_params;
	Listener * _listener;
	pthread_t _thread;
	pthread_mutex_t _mutex;
	std::string _empty_string;
	std::vector<std::string> _empty_stringarray;
};
#endif

