//============================================================================
// Name        : cbilling.h
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#ifndef DIS_CBILLING_H
#define DIS_CBILLING_H

#include <string>

class CachedBilling;
class Listener;
class Dispatcher;

class DispatcherCBilling
{
public:
	DispatcherCBilling(Dispatcher *d, Listener &l);
	~DispatcherCBilling();

	bool processQuery(std::string &);
	void timeout();
private:
    Listener *_listener;
    Dispatcher *_d;
	CachedBilling *_c;
	
protected:
};

#endif
