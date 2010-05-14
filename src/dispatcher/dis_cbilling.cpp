#include <string>
#include <vector>
#include <iostream>

#include "dis_cbilling.h"
#include "cbilling.h"
#include "billing.h"
#include "utils.h"
#include "src/radius/radpacket.h"

using namespace std;

DispatcherCBilling::DispatcherCBilling(Dispatcher *d, Listener &l)
{
    _d = d;
    _listener = &l;
	_c = &(_listener->cachedBilling());
}

DispatcherCBilling::~DispatcherCBilling()
{
}

bool DispatcherCBilling::processQuery(std::string &tagname)
{
    if(tagname == "radacct") {
		_c->processRequest(_d->namedParams());
	} else if(tagname == "cbilling_dump_online") {
		_c->dumpOnline(_d->namedParams());
	} else {
		return false;
	}
	return true;
}

void DispatcherCBilling::timeout()
{
}
