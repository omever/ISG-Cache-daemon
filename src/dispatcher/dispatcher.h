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
#include "dis_coa.h"
#include "dis_oracle.h"
#include "dis_cbilling.h"
#include "request_params.h"

enum stateEnum
{
	    WAIT_ROOT_EL,
	    READING_DATA,
		READING_ATTRIBS,
	    FINISHING
};

//typedef std::map<std::string, std::vector<std::string> > requestParams;

class Dispatcher : protected DispatcherCOA, protected DispatcherOracle, protected DispatcherCBilling
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
	std::string namedParam(std::string paramName);
	requestParams &namedParams();
	
	void lock();
	void unlock();
	
	std::string getCache(std::string key);
	void setCache(std::string key, std::string value, int timeout = 600);
protected:
	void mainloop();

	static void parserStartElement(void *ctx, const xmlChar * fullname, const xmlChar ** attr);
	static void parserEndElement(void *ctx, const xmlChar * fullname);
	static void parserCharacters(void *ctx, const xmlChar * ch, int len);
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
	requestParams _named_params;
	Listener * _listener;
	pthread_t _thread;
	pthread_mutex_t _mutex;
	std::string _tmpname;
	std::string _tmpvalue;
};
#endif

