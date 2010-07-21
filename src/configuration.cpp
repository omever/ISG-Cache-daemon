/*
 * configuration.cpp
 *
 *  Created on: 19.07.2010
 *      Author: om
 */

#include "configuration.h"

using namespace std;

Configuration::Configuration() {
	__file.clear();
	__d = NULL;
}

Configuration::Configuration(const std::string & file)
{
	Configuration();
	__file = file;
}

Configuration::~Configuration() {
	if(__d != NULL)
		iniparser_freedict(__d);
}

void Configuration::load()
{
	if(__file.empty())
		throw NoFileException();

	__d = iniparser_load((char*)__file.c_str());
	if(__d == NULL)
		throw DictionaryLoadException();
}


void Configuration::open(const std::string & file)
{
	if(__d != NULL)
		iniparser_freedict(__d);

	__file = file;
}


void Configuration::save()
{
}


std::string Configuration::get(const std::string & key)
{
	char * tmp = iniparser_getstr(__d, (char*)key.c_str());
	if(tmp == NULL)
		return string();
	return string(tmp);
}

void Configuration::set(const std::string & key, const std::string & value)
{
	throw NotImplementedException();
}


