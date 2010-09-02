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
#include <pthread.h>

#include "oracle/billing.h"
#include "radius/dictionary.h"
#include "configuration.h"
#include "Cache.h"

class Dispatcher;

class Listener : public Configuration, public Cache
{
public:
	Listener(std::string config);
	~Listener();
	int start();
	static void * thread_handle(void*);
	void lock();
	void unlock();
	int getNextId();
	void remove_child(Dispatcher *d);
	RadiusDictionary dict;
protected:
	void warehouse();
private:
	std::string _path;
	int _sd;
	Billing _bill;
	pthread_attr_t attr;
	pthread_t _thread;
	std::list<Dispatcher*> _children;
	pthread_mutex_t _mutex;
	int _sequence;
};

#endif

