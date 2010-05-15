//============================================================================
// Name        : dispatcher.cpp
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#include <ctime>
#include <unistd.h>
#include <pthread.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
#include <errno.h>

#include "dispatcher.h"
#include "billing.h"
#include "listener.h"

#define IFEXIST(x) x.size()?x.at(0):""

Dispatcher::Dispatcher(int socket, Billing &bill, Listener &l) : 
    DispatcherCOA(this, l), 
    DispatcherOracle(this, bill)
{
	_sd = socket;
	_ts = time(NULL);

	_xmlHandler = new xmlSAXHandler;
	memset(_xmlHandler, 0, sizeof(xmlSAXHandler));

	_xmlHandler->initialized = XML_SAX2_MAGIC;
	_xmlHandler->startElement = &Dispatcher::parserStartElement;
	_xmlHandler->endElement = &Dispatcher::parserEndElement;
	_xmlHandler->warning = &Dispatcher::parserWarning;
	_xmlHandler->error = &Dispatcher::parserError;
	_xmlHandler->endDocument = &Dispatcher::parserEndDocument;

	_state = WAIT_ROOT_EL;
	_depth = 0;
	_params.clear();
	_named_params.clear();
	_empty_string.clear();
	_empty_stringarray.clear();

	_listener = &l;
	_is_done = false;
	_is_processing = false;

	pthread_mutex_init(&_mutex, NULL);
}

Dispatcher::~Dispatcher()
{
	_params.clear();
	std::cerr << "Dispatcher is closing" << std::endl;

	pthread_mutex_destroy(&_mutex);

	if(_sd != -1)
		close(_sd);
	delete _xmlHandler;
}

void Dispatcher::timeout()
{
    lock();
	if(_is_processing == true) {
	    DispatcherCOA::timeout();
	    DispatcherOracle::timeout();
	}
    unlock();
}


int Dispatcher::start()
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int td = pthread_create(&_thread, &attr, Dispatcher::thread_handle, this);
	if(td) {
		return -1;
	}

	return 0;
}

unsigned int Dispatcher::last_activity()
{
	return _ts;
}

bool Dispatcher::is_done()
{
	return _is_done;
}

void * Dispatcher::thread_handle(void * arg)
{
	Dispatcher *d = (Dispatcher*) arg;
	if(d) {
		d->mainloop();
	}
	pthread_exit(NULL);
}

void Dispatcher::mainloop()
{
	std::cerr << "Main loop start" << std::endl;
	char buff[256];
	std::string data("");
	size_t sz;
	xmlParserCtxtPtr ctx = NULL;
	while(_sd != -1 && ((sz = recv(_sd, buff, 255, 0)) > 0)) {
		buff[sz] = 0;
		if(ctx == NULL) {
			ctx = xmlCreatePushParserCtxt(_xmlHandler, this, NULL, 0, NULL);
		}
		std::cout << "Parse!" << std::endl;
		xmlParseChunk(ctx, buff, sz, 0);
	}
	xmlParseChunk(ctx, buff, 0, 1);
	xmlFreeParserCtxt(ctx);
	std::cerr << "Done..." << std::endl;
	_is_done = true;
}

bool Dispatcher::processQuery(std::string fullname)
{
    bool res;
    _is_processing = true;
    std::cerr << "Dispatcher" << std::endl;
    if(res = DispatcherCOA::processQuery(fullname) || DispatcherOracle::processQuery(fullname)) {
	std::cerr << "Processed " << std::endl;
    }
    _is_processing = false;
    return res;	
}

void Dispatcher::parserStartElement(void *ctx, const xmlChar * fullname, const xmlChar ** attr)
{
	Dispatcher *d = NULL;
	if(ctx) {
		d = ((Dispatcher*)ctx);
	}

	switch(d->_state) {
		case WAIT_ROOT_EL:
			if(!xmlStrcasecmp(fullname, BAD_CAST "query")) {
				d->_state = READING_DATA;
			} else {
				std::cerr << "Error: root element is not query" << std::endl;
				return;
			}
			break;
		case READING_DATA:
			while(*attr != NULL) {
			    d->_named_params[(const char*)*(attr)].push_back((const char*)*(attr+1));
			    attr += 2;
			}
			
			d->_state = WAIT_PARAMS;
			break;
		case WAIT_PARAMS:
			std::cerr << "waiting for params with level " << d->_depth << std::endl;
			if(!xmlStrcasecmp(fullname, BAD_CAST "param")) {
				std::cerr << "Reading params" << std::endl;
				const char *name = NULL;
				const char *value = NULL;
				if(attr != NULL)
				while(*attr != NULL) {
					if(!xmlStrcasecmp(*attr, BAD_CAST "name")) {
						name = (const char*)*(attr+1);
					} else if(!xmlStrcasecmp(*attr, BAD_CAST "value")) {
						value = (const char*)*(attr+1);
					}

					if(name != NULL && value != NULL)
						break;
					attr += 2;
				}

				if(name != NULL && value != NULL) {
					std::cerr << "Adding attribute " << name << " with value " << value << std::endl;
					d->_named_params[name].push_back(value);
				}
			}

			break;
		case FINISHING:
			break;
	};
	d->_depth++;
}

void Dispatcher::parserEndElement(void *ctx, const xmlChar * fullname)
{
	Dispatcher *d = NULL;
	if(ctx) {
		d = ((Dispatcher*)ctx);
	} else {
		std::cerr << "Error - no dispatcher pointer" << std::endl;
		return;
	}

	if(d->_depth == 2)
	switch(d->_state) {
		case WAIT_PARAMS:
			if(d->processQuery((char*)fullname)) {
				d->_state = FINISHING;
			} else {
				std::cerr << "Error: action element is not correct" << std::endl;
				return;
			}
			break;
		default:
			break;
	}
	d->_depth--;
}

const std::string & Dispatcher::namedParam(const char *paramName) const
{
	std::string _temp(paramName);
	return namedParam(_temp);
}

const std::string & Dispatcher::namedParam(const std::string &paramName) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator i;
	if((i = _named_params.find(paramName)) != _named_params.end() && i->second.size() > 0) {
		return i->second.at(0); 
	} else {
		return _empty_string;
	}
}

const std::vector<std::string> & Dispatcher::namedParams(const std::string &paramName) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator i;
	if((i = _named_params.find(paramName)) != _named_params.end()) {
		return i->second; 
	} else {
		return _empty_stringarray;
	}
}

const std::map<std::string, std::vector<std::string> > & Dispatcher::allNamedParams() const
{
    return _named_params;
}

void Dispatcher::parserWarning(void *ctx, const char *msg, ...)
{
	std::cerr << "Warning issued: " << msg << std::endl;
}

void Dispatcher::parserError(void *ctx, const char *msg, ...)
{
	std::cerr << "Error issued: " << msg << std::endl;
	va_list args;
	va_start(args, msg);
	vprintf( msg, args );
	va_end(args);

	Dispatcher *d = NULL;
	if(ctx) {
		d = ((Dispatcher*)ctx);
	} else {
		std::cerr << "Error - no dispatcher pointer" << std::endl;
		return;
	}
	
	d->error("Source XML is not well formed");
}

void Dispatcher::error(std::string errorstring)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	int rc;
	std::string returnValue;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "error");

	xmlDocSetRootElement(doc, root);


	xmlNodePtr branch = xmlNewChild(root, NULL, BAD_CAST "errorstring", BAD_CAST errorstring.c_str());

	xmlBufferPtr buf = xmlBufferCreate();
	xmlSaveCtxtPtr sav = xmlSaveToBuffer(buf, "UTF-8", XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY);

	xmlSaveDoc(sav, doc);
	xmlSaveClose(sav);

	send(_sd, (char*)buf->content, buf->use, 0);
	close(_sd);
	_sd = -1;
	
	xmlBufferFree(buf);
	xmlFreeDoc(doc);
	
	_is_done = true;
}

void Dispatcher::parserEndDocument(void *ctx)
{
}

const std::string Dispatcher::wrapResult(queryResult &r)
{
	xmlDocPtr doc;
	xmlNodePtr root;
	int rc;
	std::string returnValue;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "result");

	std::stringstream temp;
	temp << r.size();
	xmlNewProp(root, BAD_CAST "count", BAD_CAST temp.str().c_str()); 
	xmlDocSetRootElement(doc, root);


	for(int i=0; i<r.size(); ++i) {
		xmlNodePtr branch = xmlNewChild(root, NULL, BAD_CAST "branch", NULL);

		std::multimap<std::string, std::string>::iterator it, iend = r.at(i).end();
		for(it=r.at(i).begin(); it!=iend; ++it) {
			std::cerr << "Add data: " << it->first << " = " << it->second << std::endl;
			xmlNodePtr child = xmlNewChild(branch, NULL, BAD_CAST it->first.c_str(), BAD_CAST it->second.c_str());
		}
	}

	xmlBufferPtr buf = xmlBufferCreate();
	xmlSaveCtxtPtr sav = xmlSaveToBuffer(buf, "UTF-8", XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY);

	xmlSaveDoc(sav, doc);
	xmlSaveClose(sav);

	std::cerr << "Data: " << buf->use << "\n" << buf->content << std::endl;

	returnValue = (char*)buf->content;

	xmlBufferFree(buf);
	xmlFreeDoc(doc);
	//xmlCleanupParser();

	return returnValue;
}

int Dispatcher::resultHandler(void *ctx, const char* buffer, int len)
{
	std::cerr << "Result handler " << std::endl << buffer << std::endl;
	Dispatcher *d = NULL;
	if(ctx) {
		d = ((Dispatcher*)ctx);
	} else {
		std::cerr << "Error - no dispatcher pointer" << std::endl;
		return -1;
	}

	send(d->_sd, buffer, len, 0);
	return 0;
}

int Dispatcher::closeHandler(void *ctx)
{
	std::cerr << "Close handler " << std::endl;
	Dispatcher *d = NULL;
	if(ctx) {
		d = ((Dispatcher*)ctx);
	} else {
		std::cerr << "Error - no dispatcher pointer" << std::endl;
		return -1;
	}

	close(d->_sd);
	d->_sd = -1;
	return 0;
}

void Dispatcher::sendString(std::string &data)
{
	send(_sd, data.c_str(), data.length(), 0);
	shutdown(_sd, 2);
}

void Dispatcher::sendResult(queryResult &rv)
{
	std::string res = wrapResult(rv);
	sendString(res);
}

void Dispatcher::lock()
{
    pthread_mutex_lock(&_mutex);
}

void Dispatcher::unlock()
{
    pthread_mutex_unlock(&_mutex);
}

std::string Dispatcher::getCache(std::string key)
{
    return _listener->getValue(key);
}

void Dispatcher::setCache(std::string key, std::string value, int timeout)
{
    _listener->setValue(key, value, timeout);
}
