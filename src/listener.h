//============================================================================
// Name        : listener.h
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#ifndef LISTENER_H
#define LISTENER_H

#include <string>
#include <list>
#include <libmemcached/memcached.h>
#include <pthread.h>

#include "oracle/billing.h"
#include "radius/dictionary.h"
#include "configuration.h"

class Dispatcher;

class Listener : public Configuration
{
public:
	Listener(std::string config);
	~Listener();
	int start();
	const std::string getValue(const std::string &key);
	void setValue(const std::string &key, const std::string &value, time_t expire);
	static void * thread_handle(void*);
	void lock();
	void unlock();
	int getNextId();
	
	RadiusDictionary dict;
protected:
	void warehouse();
private:
	std::string _path;
	int _sd;
	Billing _bill;
	memcached_st *_mc;
	memcached_server_st *_mc_servers;
	pthread_attr_t attr;
	pthread_t _thread;
	std::list<Dispatcher*> _children;
	pthread_mutex_t _mutex;
	int _sequence;
};

#endif

