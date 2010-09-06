/*
 * Cache.cpp
 *
 *  Created on: 27.08.2010
 *      Author: om
 */

#include "Cache.h"
#include <iostream>
using namespace std;

Cache::Cache() {
	pthread_mutex_init(&_mutex, NULL);
}

Cache::~Cache() {
#ifdef ENABLE_LIBMEMCACHE
	memcached_free(_mc);
	memcached_server_free(_mc_servers);
#endif
	pthread_mutex_destroy(&_mutex);
}

const std::string Cache::getValue(const std::string &key)
{
std::string retval;

#ifdef ENABLE_LIBMEMCACHE
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
#else
	pthread_mutex_lock(&_mutex);
	if(__minicache.count(key) > 0) {
		retval = __minicache[key].second;
	} else {
		retval = std::string();
	}
	pthread_mutex_unlock(&_mutex);

	return retval;
#endif
}

void Cache::setValue(const std::string &key, const std::string &value, time_t expire)
{
#ifdef ENABLE_LIBMEMCACHE
	memcached_return rc;
	pthread_mutex_lock(&_mutex);
	rc = memcached_set(_mc, key.c_str(), key.length(), value.c_str(), value.length(), expire, 0);
	pthread_mutex_unlock(&_mutex);
	if(rc != MEMCACHED_SUCCESS) {
		std::cerr << "Error storing data to memcached" << std::endl;
	}
#else
	pthread_mutex_lock(&_mutex);
	__minicache[key].first = time(NULL)+expire;
	__minicache[key].second = value;
	pthread_mutex_unlock(&_mutex);
#endif
}

void Cache::warehouse()
{
#ifndef ENABLE_LIBMEMCACHE
	pthread_mutex_lock(&_mutex);
	time_t cur_time = time(NULL);
	std::map<std::string, std::pair<int, std::string> >::iterator i, iend = __minicache.end();
	for(i = __minicache.begin(); i != iend;) {
		if(cur_time >= i->second.first) {
			cerr << "Cleaning up element with key " << i->first << endl;
			__minicache.erase(i++);
		} else {
			++i;
		}
	}
	pthread_mutex_unlock(&_mutex);
#endif
}
