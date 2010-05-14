#include "listener.h"
#include "dispatcher.h"
#include "balance.h"
#include "service.h"
#include "cbilling/session.h"

#ifndef CBILLING_H
#define CBILLING_H

class CachedBilling
{
public:
	CachedBilling();
	~CachedBilling();
	
	bool processRequest(requestParams &params);
	bool dumpOnline(requestParams &params);
protected:
	bool start(requestParams &params);
	bool stop(requestParams &params);
	bool update(requestParams &params);
private:
	std::list<Session *> __active_sessions;
	std::list<Session *> __inactive_sessions;
};

#endif
