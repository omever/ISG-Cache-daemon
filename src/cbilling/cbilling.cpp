#include <iostream>

#include "cbilling.h"

using namespace std;

CachedBilling::CachedBilling()
{
	__active_sessions.clear();
	__inactive_sessions.clear();
}

CachedBilling::~CachedBilling()
{
}

bool CachedBilling::dumpOnline(requestParams &params)
{
	cout << "Active sessions dump: " << endl;
	list<Session *>::iterator i, iend = __active_sessions.end();
	for(i = __active_sessions.begin(); i != iend ; ++i) {
		(*i)->dump();
	}
}

bool CachedBilling::processRequest(requestParams &params)
{
	string status = IFEXIST(params["ACCT-STATUS-TYPE"]);
	bool res = false;
	if(status == "Start") {
		res = start(params);
	} else
	if(status == "Stop") {
		res = stop(params);
	} else
	if(status == "Interim-Update") {
		res = update(params);
	} else {
		cerr << "Unknown acct-status-type " << status << endl;
	}
	return res;
}

bool CachedBilling::start(requestParams &params)
{
	list<Session *>::iterator i, iend = __active_sessions.end();
	// перебираем активные сессии
	cerr << "start processing" << endl;
	for(i = __active_sessions.begin(); i != iend ; ++i) {
		cerr << "iterating over active sessions..." << endl;
		if(! (*i)->match(params) ) // Не наша сессия, пропускаем
			continue;
		if(params.count("CISCO-SERVICE-INFO") == 0 || params["CISCO-SERVICE-INFO"][0].length() == 0) { // Запрос на сессию, проверяем на дубликат
			if((*i)->submatch(params)) { // Дублирующий запрос на сессию, игнорируем
				return true;
			} else { // Конфликтующий запрос, закрываем сессию
				(*i)->forcedStop();
				__inactive_sessions.push_back(*i);
				__active_sessions.erase(i);
			}
		} else { // Запрос на сервис, передаём текущей сессии
			return (*i)->start(params);
		}
	}
	
	// Не нашли ничего подходящего в сети, смотрим на историю происходящего
	iend = __inactive_sessions.end();
	for(i = __inactive_sessions.begin(); i != iend ; ++i) {
		cerr << "iterating over inactive sessions..." << endl;
		if((*i)->match(params) && time(NULL) - (*i)->last_activity() < 300) // Устаревшие данные, неважно что там
			return true;
	}
	
	cerr << "New session!" << endl;
	// Новая сессия!
	Session * s = new Session();
	__active_sessions.push_back(s);
	return s->start(params);
}

bool CachedBilling::stop(requestParams &params)
{
	list<Session *>::iterator i, iend = __active_sessions.end();
	for(i = __active_sessions.begin(); i != iend ; ++i) {
		if(! (*i)->match(params) ) // Не наша сессия, пропускаем
			continue;
		
		return (*i)->stop(params);
	}

	cerr << "Session not found " << endl;
	return start(params);
}

bool CachedBilling::update(requestParams &params)
{
	list<Session *>::iterator i, iend = __active_sessions.end();
	for(i = __active_sessions.begin(); i != iend ; ++i) {
		if(! (*i)->match(params) ) // Не наша сессия, пропускаем
			continue;
		
		return (*i)->update(params);
	}
	
	cerr << "Session not found " << endl;
	return start(params);
}

