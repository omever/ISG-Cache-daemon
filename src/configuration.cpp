/*
 * configuration.cpp
 *
 *  Created on: 19.07.2010
 *      Author: om
 */

#include "configuration.h"

#include <fstream>
#include <iostream>

using namespace std;

Configuration::Configuration() {
	__file.clear();
}

Configuration::Configuration(const std::string & file)
{
	Configuration();
	__file = file;
}

Configuration::~Configuration() {
}

void Configuration::load()
{
	if(__file.empty())
		throw NoFileException();

	__data.clear();

	ifstream src(__file.c_str(), ios::in);
	if(src.bad())
		throw DictionaryLoadException();

	string buffer;
	string section;
	while(getline(src, buffer).good()) {
		size_t beg = buffer.find_first_not_of("\t ");
		if(beg == string::npos || buffer.at(beg) == '#')
			continue;

		if(buffer.at(beg) == '[') {
			size_t end = buffer.find(']');
			if(end == string::npos)
				continue;

			section = buffer.substr(beg + 1, end-beg-1);
			continue;
		}

		string key;
		string value;

		size_t key_end = buffer.find_first_of("\t =", beg+1);
		if(key_end == string::npos) {
			key = buffer.substr(beg);
			value = "true";
		} else {
			size_t value_beg = buffer.find_first_not_of("\t =", key_end + 1);
			if(value_beg == string::npos) {
				key = buffer.substr(beg, key_end - beg);
				value = "true";
			} else {
				size_t value_end = buffer.find_last_not_of("\t ");
				if(value_end == string::npos || value_end <= value_beg) {
					key = buffer.substr(beg, key_end - beg);
					value = buffer.substr(value_beg);
				} else {
					key = buffer.substr(beg, key_end - beg);
					value = buffer.substr(value_beg, value_end - value_beg + 1);
				}
			}
		}

		if(section != "") {
			key = section + ":" + key;
		}
		__data[key] = value;
	}
}


void Configuration::open(const std::string & file)
{
	__file = file;
}


void Configuration::save()
{
}


std::string Configuration::get(const std::string & key)
{
	return __data[key];
}

void Configuration::set(const std::string & key, const std::string & value)
{
	throw NotImplementedException();
}

void Configuration::dump()
{
	map<string, string>::iterator i, iend = __data.end();
	for(i = __data.begin(); i != iend; ++i)
		cout << "KEY:" << i->first << " VALUE:\"" << i->second << "\"" << endl;
}
