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
Listener::Listener(std::string config)
	: Configuration(config)
{
	_mc = memcached_create(NULL);
	memcached_return rc;

	_mc_servers = memcached_server_list_append(NULL, "127.0.0.1", 0, &rc);
	assert(rc == MEMCACHED_SUCCESS);

	rc = memcached_server_push(_mc, _mc_servers);
	assert(rc == MEMCACHED_SUCCESS);

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

	memcached_free(_mc);
	memcached_server_free(_mc_servers);

	pthread_mutex_destroy(&_mutex);
}

int Listener::start()
{
	try {
		_bill.connect(get("ORACLE:SID"), get("ORACLE:USER"), get("ORACLE:PASSWORD"));
	}
	catch (std::exception &ex) {
		std::cerr << "Exception caught while executing query: " << ex.what() << std::endl;
		return -1;
	}

	unlink(get("MAIN:socket").c_str());

	_sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(_sd == -1) {
		std::cerr << "Error opening listener socket" << std::endl;
		return -1;
	}

	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	snprintf(addr.sun_path, UNIX_PATH_MAX, "%s", get("MAIN:socket").c_str());
	addr.sun_family = AF_UNIX;

	if(bind(_sd, (struct sockaddr *)&addr, len) != 0) {
		std::cerr << "Error binding to listener socket: " << strerror(errno) << std::endl;
		return -1;
	}

	if(get("MAIN:mode").size()) {
		stringstream tmp(get("MAIN:mode"));
		mode_t mode;
		tmp >> oct >> mode;
		mode &= 0777;
		std::cerr << "Setting listener socket to mode " << mode << std::endl;
		chmod(addr.sun_path, mode);
	}

	if(listen(_sd, SOMAXCONN) != 0) {
		std::cerr << "Error listening socket" << std::endl;
		return -1;
	}

//Starting new thread for warehouse operational

	attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int td = pthread_create(&_thread, &attr, Listener::thread_handle, this);
	if(td) {
		std::cerr << "Error creating warehouse thread" << std::endl;
		return -1;
	}

	struct sockaddr_un clientaddr;
	int newsd;
	while(1) {
		while((newsd = accept(_sd, (struct sockaddr *)&clientaddr, &len)) != -1) {
			std::cout << "new socket established " << newsd << ". Closing" << std::endl;

			Dispatcher *d = new Dispatcher(newsd, _bill, *this);

			pthread_mutex_lock(&_mutex);
			int rv = d->start();
			std::cerr << "Dispatcher started: " << rv << std::endl;
			if(rv == 0) {
				_children.push_back(d);
			} else {
				delete d;
			}
			pthread_mutex_unlock(&_mutex);

			std::cerr << "Closed..." << std::endl;
		}
		std::cerr << "Done..." << errno << ": " << strerror(errno) << std::endl;
		if(errno == EINTR || errno == EAGAIN)
			continue;
		if(errno == ENETDOWN || errno == EHOSTDOWN || errno == ENONET || errno == ENETUNREACH) {
			sleep(5);
			continue;
		}
		break;
	}
	std::cerr << "Socket disconnected" << std::endl;
}

const std::string Listener::getValue(const std::string &key)
{
	std::string retval;
	size_t val_len;
	uint32_t flags;
	memcached_return rc;

	pthread_mutex_lock(&_mutex);
	char * result = memcached_get(_mc, key.c_str(), key.length(), &val_len, &flags, &rc);
	pthread_mutex_unlock(&_mutex);

	if(result != NULL) {
		retval = result;
		free(result);
		return retval;
	} else {
		return std::string();
	}
}

void Listener::setValue(const std::string &key, const std::string &value, time_t expire)
{
	memcached_return rc;
	pthread_mutex_lock(&_mutex);
	rc = memcached_set(_mc, key.c_str(), key.length(), value.c_str(), value.length(), expire, 0);
	pthread_mutex_unlock(&_mutex);
	if(rc != MEMCACHED_SUCCESS) {
		std::cerr << "Error storing data to memcached" << std::endl;
	}
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
	std::cerr << "Endless warehouse cycle start" << std::endl;
	while(1) {
		pthread_mutex_lock(&_mutex);
		std::list<Dispatcher*>::iterator i, tmp, iend = _children.end();

		for(i=_children.begin(); i != _children.end(); i = tmp) {
			tmp = i;
			tmp ++;
			if((*i)->is_done()) {
				std::cerr << "Cleaning up Dispatcher at 0x" << std::hex << (void*)(*i) << std::endl;
				_children.erase(i);
				delete(*i);
			} else if(time(NULL) - (*i)->last_activity() > 10) {
				std::cerr << "Long running thread should be killed at 0x" << std::hex << (void*)(*i) << std::endl;
				(*i)->timeout();
			}
		}

		pthread_mutex_unlock(&_mutex);
		sleep(1);
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

