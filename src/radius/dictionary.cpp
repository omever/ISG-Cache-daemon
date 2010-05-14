#include <iostream>
#include <fstream>
#include <libgen.h>
#include <cstring>
#include <cstdlib>

#include "dictionary.h"

using namespace std;

RadiusDictionary::RadiusDictionary()
{
}

void RadiusDictionary::parse(string file)
{
    unsigned short len = file.length()+1;
    char *temp = new char[len];
    
    strncpy(temp, file.c_str(), len-1);
    string dir(dirname(temp));
    
    delete [] temp;
    
    ifstream fin;
    fin.open(file.c_str(), ifstream::in);
    
    if(fin.fail()) {
	cerr << "Error opening file " << file << endl;
	return;
    }
    
    string tmp;
    string currentVendor = "";
    while(! fin.eof()) {
	getline(fin, tmp);
	
	vector<string> fields = split(tmp);
	
	if(! fields.size() ) continue;
	if(fields.at(0)[0] == '#') continue;

	if(fields.at(0) == "$INCLUDE") {
		if(fields.at(1)[0] == '/') {
		    parse(fields.at(1));
		} else {
		    parse(dir+"/"+fields.at(1));
		}
	} else
	if(fields.at(0) == "ATTRIBUTE") {
	    pair<string, attribute> nattr;
	    if( fields.size() < 4 ) continue;
	    nattr.first = fields.at(1);
	    nattr.second.id = atoi(fields.at(2).c_str());
	    nattr.second.type = fields.at(3);
	    nattr.second.vendor = currentVendor;
	    if(fields.size() >= 5)
		nattr.second.vendor = fields.at(4);
	    attrs.insert(nattr);
	} else
	if(fields.at(0) == "VALUE") {
	    if( fields.size() < 4 ) continue;
	    attrs[fields.at(1)].values[fields.at(2)] = atoi(fields.at(3).c_str());
	} else
	if(fields.at(0) == "VENDOR") {
	    if( fields.size() < 3 ) continue;
	    vendors[fields.at(1)] = atoi(fields.at(2).c_str());
	} else
	if(fields.at(0) == "BEGIN-VENDOR") {
	    if( fields.size() < 2 ) continue;
	    currentVendor = fields.at(1);
	} else
	if(fields.at(0) == "END-VENDOR") {
	    if( fields.size() < 2 ) continue;
	    currentVendor = "";
	}
    }
    fin.close();
}

vector<string> RadiusDictionary::split(string instr)
{
    vector<string> ret;
    size_t end = 0, now;
    now = instr.find_first_of("#");
    if(now != string::npos) 
	instr.erase(now);

    while( (now = instr.find_first_not_of("\t ", end)) != string::npos) {
	end = instr.find_first_of("\t ", now);
	ret.push_back(instr.substr(now, end-now));
	if(end == string::npos) break;
    }
    
    return ret;
}

unsigned short RadiusDictionary::getValue(std::string attr, std::string value)
{
    if(!attrs.count(attr)) {
	throw AttributeNotFound();
    }
    if(!attrs[attr].values.count(value)) {
	throw ValueNotFound();
    }
    return attrs[attr].values[value];
}

RadiusDictionary::attribute RadiusDictionary::getAttribute(std::string attr, std::string vendor)
{
    if(!attrs.count(attr)) {
	throw AttributeNotFound();
    }
    return attrs[attr];
}

unsigned int RadiusDictionary::getVendorID(std::string vendor)
{
    if(!vendors.count(vendor)) {
	throw VendorNotFound();
    }
    return vendors[vendor];
}

string RadiusDictionary::getValueString(std::string attr, unsigned int id)
{
    if(!attrs.count(attr)) {
	throw AttributeNotFound();
    }
    std::map<std::string, unsigned int>::iterator i;
    
    for(i=attrs[attr].values.begin(); i!=attrs[attr].values.end(); ++i) {
	if(i->second == id) {
	    return i->first;
	}
    }

    throw ValueNotFound();
}

std::string RadiusDictionary::getAttributeString(unsigned int id, std::string vendor)
{
    std::map<std::string, attribute>::iterator i;
    for(i=attrs.begin(); i!=attrs.end(); ++i) {
	if(vendor == i->second.vendor && id==i->second.id) {
	    return i->first;
	}
    }
    throw AttributeNotFound();
}

std::string RadiusDictionary::getVendorString(unsigned int vendor)
{
    std::map<std::string, unsigned int>::iterator i;
    for(i=vendors.begin(); i!=vendors.end(); ++i) {
	if(vendor == i->second) {
	    return i->first;
	}
    }
    throw VendorNotFound();
}

