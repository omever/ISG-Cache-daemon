//============================================================================
// Name        : listener.cpp
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : ISG Cache Daemon
//============================================================================

#include <string>
#include <iostream>
#include <cassert>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>

#include "listener.h"
#include "dispatcher/dispatcher.h"

using namespace std;

#define UNIX_PATH_MAX	108
Listener::Listener(string config)
	: Configuration(config)
{

#ifdef ENABLE_LIBMEMCACHE
	_mc = memcached_create(NULL);
	memcached_return rc;

	_mc_servers = memcached_server_list_append(NULL, "127.0.0.1", 0, &rc);
	assert(rc == MEMCACHED_SUCCESS);

	rc = memcached_server_push(_mc, _mc_servers);
	assert(rc == MEMCACHED_SUCCESS);
#endif

	pthread_mutex_init(&_mutex, NULL);

	_children.clear();

	dict.parse(DATADIR "/isg_cached/radius/dictionary");

	_sequence = 0;

	Configuration::load();
	Configuration::dump();
}

Listener::~Listener()
{
	close(_sd);

	pthread_mutex_destroy(&_mutex);
}

int Listener::start()
{
	try {
		_bill.connect(get("ORACLE:SID"), get("ORACLE:USER"), get("ORACLE:PASSWORD"));
	}
	catch (exception &ex) {
		cerr << "Exception caught while executing query: " << ex.what() << endl;
		return -1;
	}

	unlink(get("MAIN:socket").c_str());

	_sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(_sd == -1) {
		cerr << "Error opening listener socket" << endl;
		return -1;
	}

	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	snprintf(addr.sun_path, UNIX_PATH_MAX, "%s", get("MAIN:socket").c_str());
	addr.sun_family = AF_UNIX;

	if(bind(_sd, (struct sockaddr *)&addr, len) != 0) {
		cerr << "Error binding to listener socket: " << strerror(errno) << endl;
		return -1;
	}

	if(get("MAIN:mode").size()) {
		stringstream tmp(get("MAIN:mode"));
		mode_t mode;
		tmp >> oct >> mode;
		mode &= 0777;
		cerr << "Setting listener socket to mode " << mode << endl;
		chmod(addr.sun_path, mode);
	}

	if(listen(_sd, SOMAXCONN) != 0) {
		cerr << "Error listening socket" << endl;
		return -1;
	}

//Starting new thread for warehouse operational

	attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int td = pthread_create(&_thread, &attr, Listener::thread_handle, this);
	if(td) {
		cerr << "Error creating warehouse thread" << endl;
		return -1;
	}

	struct sockaddr_un clientaddr;
	int newsd;
	while(1) {
		while((newsd = accept(_sd, (struct sockaddr *)&clientaddr, &len)) != -1) {
			cout << "new socket established " << newsd << ". Closing" << endl;

			Dispatcher *d = new Dispatcher(newsd, _bill, *this);

			lock();
			if(_children.size() > 2046) {
				cerr << "To many processes working, resetting" << endl;
				close(newsd);
			} else {
				int rv = d->start();
				cerr << "Dispatcher started: " << rv << endl;
				if(rv == 0) {
					_children.push_back(d);
				} else {
					delete d;
				}
			}
			unlock();

			cerr << "Closed..." << endl;
		}
		cerr << "Done..." << errno << ": " << strerror(errno) << endl;
		if(errno == EMFILE || errno == ENFILE) {
			sleep(1);
			continue;
		}
		if(errno == EINTR || errno == EAGAIN)
			continue;
		if(errno == ENETDOWN || errno == EHOSTDOWN || errno == ENONET || errno == ENETUNREACH) {
			sleep(5);
			continue;
		}
		break;
	}
	cerr << "Socket disconnected" << endl;
}

void * Listener::thread_handle(void* arg)
{
	Listener *l = (Listener*) arg;
	if(l) {
		l->warehouse();
	}
	delete l;
	pthread_exit(NULL);
}

void Listener::warehouse()
{
	cerr << "Endless warehouse cycle start" << endl;
	while(1) {
		Cache::warehouse();
		lock();
		list<Dispatcher*>::iterator i, tmp, iend = _children.end();

		for(i=_children.begin(); i != _children.end(); i = tmp) {
			tmp = i;
			tmp ++;
			if((*i)->is_done()) {
				cerr << "Cleaning up Dispatcher at 0x" << hex << (void*)(*i) << endl;
				_children.erase(i);
				delete(*i);
			} else if(time(NULL) - (*i)->last_activity() > 10) {
				cerr << "Long running thread should be killed at 0x" << hex << (void*)(*i) << endl;
				(*i)->timeout();
			}
		}

		unlock();
		sleep(1);
	}
}

void Listener::remove_child(Dispatcher *d)
{
	if(pthread_mutex_trylock(&_mutex) == 0) {
		cerr << "Removing child upon finish of the dispatcher" << endl;
		_children.remove(d);
		delete(d);
		unlock();
	}
}

void Listener::lock()
{
	pthread_mutex_lock(&_mutex);
}

void Listener::unlock()
{
	pthread_mutex_unlock(&_mutex);
}

int Listener::getNextId()
{
	int temp;
	lock();
	temp = _sequence++;
	unlock();
	return temp;
}

