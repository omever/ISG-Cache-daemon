/*
 * Cache.h
 *
 *  Created on: 27.08.2010
 *      Author: om
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <string>
#include <map>
#include <pthread.h>

class Cache {
public:
	Cache();
	virtual ~Cache();
	const std::string getValue(const std::string &key);
	void setValue(const std::string &key, const std::string &value, time_t expire);
protected:
	void warehouse();
private:
	pthread_mutex_t _mutex;

#ifdef ENABLE_LIBMEMCACHE
	memcached_st *_mc;
	memcached_server_st *_mc_servers;
#else
	std::map<std::string, std::pair<int, std::string> > __minicache;
#endif
};

#endif /* CACHE_H_ */
