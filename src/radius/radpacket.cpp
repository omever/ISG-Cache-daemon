#include "radpacket.h"

#include <exception>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>

using namespace std;

RadPacket::RadPacket(RadiusDictionary *dict)
{
    _code = 0;
    _identifier = 0;
    memset(_authenticator, 0, 16);
    _dict = dict;
}

string RadPacket::pack()
{
    string tmp = "";
    
    _packHeader(&tmp);
    _packAttributes(&tmp);
    _packVSAttributes(&tmp);
    
    unsigned short sz = htons(tmp.size());
    
    tmp.replace(2, 2, (const char*)(&sz), 2);
    return tmp;
}

void RadPacket::unpack(std::string data)
{
    string tmp = data;
    _unpackHeader(&tmp);
    _unpackAttributes(&tmp);
    _unpackVSAttributes(&tmp);
}

void RadPacket::setCode(string newcode)
{
    unsigned short ucode;
    try {
	ucode = _dict->getValue("Packet-Type", newcode);
    }
    catch (exception& e) {
	cerr << e.what() << endl;
	return;
    }
    _code = ucode;
}

void RadPacket::setSecret(string newsecret)
{
    __secret = newsecret;
}

string RadPacket::secret()
{
    return __secret;
}

void RadPacket::setIdentifier(unsigned char newident)
{
    _identifier = newident;
}

void RadPacket::setAuthenticator(unsigned char *newauth)
{
    memcpy(_authenticator, newauth, 16);
}

void RadPacket::_packHeader(string *tmp)
{
    char buffer[20];
    buffer[0] = _code;
    buffer[1] = _identifier;
    buffer[2] = buffer[3] = 0;
    memcpy((char*)buffer+4, _authenticator, 16);
    tmp->replace(0, 20, buffer, 20);
}

void RadPacket::_unpackHeader(string *tmp)
{
    _code = tmp->at(0);
    _identifier = tmp->at(1);
    _length = tmp->at(2)+tmp->at(3)*256;
    
    memcpy(_authenticator, (char*)(tmp->c_str())+4, 16);
    tmp->erase(0, 20);
}

void RadPacket::_packAttributes(string *tmp)
{
    string s_attr;
    AttrList::iterator i;
    for(i = _attr.begin(); i!= _attr.end(); ++i) {
	s_attr = "";
	RadiusDictionary::attribute attrib;
        try {
    	    attrib = _dict->getAttribute(i->first);
	}
	catch (exception& e) {
	    cerr << i->first << endl;
	    cerr << e.what() << endl;
	    cerr << "While packing attribute " << i->first << endl;
	    continue;
	}
	s_attr.resize(2);
        s_attr[0] = attrib.id;
        s_attr[1] = 0;
	if(attrib.type == "integer") {
	    unsigned int tmp = htonl(atoi(i->second.c_str()));
	    s_attr.append((char*)&tmp, 4);
	} else
	if(attrib.type == "string") {
	    s_attr.append(i->second);
	} else
	if(attrib.type == "date") {
	} else
        if(attrib.type == "octets") {
	    s_attr.append(i->second);
        } else {
	    throw UnknownAttributeType();
	}
	s_attr[1] = (unsigned char)s_attr.size();
    }
    tmp->append(s_attr);    
}

void RadPacket::_unpackAttributes(string *tmp)
{
    _attr.clear();
    string s_attr;
    int i=0;
    while(tmp->size() && i<tmp->size()) {
	unsigned char id = tmp->at(i);
	cerr << "ID: " << hex << (int)id << endl;
	unsigned char len = tmp->at(i+1);
	if(id == 26) {
	    i+=len;
	    continue;
	}
	string data = tmp->substr(i+2, len-2);
	tmp->erase(i, len);
	
	string name, value;
	RadiusDictionary::attribute attr;
	try {
	    name = _dict->getAttributeString(id);
	    attr = _dict->getAttribute(name);
	}
	catch (exception& e) {
	    cerr << "Unknown attribute " << id << endl;
	    continue;
	}
	try {
	    if(attr.type == "ipaddr") {
		in_addr ip;
		ip.s_addr = *(unsigned int*)(data.c_str());
		value = inet_ntoa(ip);
	    } else if(attr.type == "integer") {
		unsigned int in = *(unsigned int*)(data.c_str());
		char tmp[32];
		snprintf(tmp,32, "%d", in);
		value = tmp;
//		value = _dict->getValueString(name, atoi(data.c_str()));
	    } else if(attr.type == "octets") {
		value = "0x";
		char tmp[3];
		for(int j=0; j<data.length(); ++j) {
		    snprintf(tmp, 3, "%02X", data[j]);
		    value += tmp;
		}
		value = data;
	    } else if(attr.type == "string") {
		value = data;
	    } else {
		value = _dict->getValueString(name, atoi(data.c_str()));
	    }
	}
	catch (exception& e) {
	    cerr << "Unknown value " << data << " for attribute " << name << "(" << attr.type << ")"<< endl;
	    value = data;
	}
	_attr.push_back(make_pair<string, string>(name, value));
    }
}

void RadPacket::_unpackVSAttributes(string *tmp)
{
    _vsattr.clear();
    string s_attr;
    int i=0;
    while(tmp->size() && i<tmp->size()) {
	unsigned char id = tmp->at(i);
	unsigned char len = tmp->at(i+1);
	if(id != 26) {
	    i+=len;
	    continue;
	}
	unsigned int vtmp = (unsigned int&)(*((tmp->substr(i+2, 4)).data()));
	unsigned int vid = ntohl(vtmp);
	unsigned char vtype = tmp->at(i+6);
	unsigned char vlen = tmp->at(i+7);
	string data = tmp->substr(i+8, len-8);

	tmp->erase(i, len);
	
	string name, value, vendor;
	try {
	    vendor = _dict->getVendorString(vid);
	}
	catch (exception& e) {
	    cerr << "Unknwon vendor " << vid << endl;
	    continue;
	}
	RadiusDictionary::attribute attr;
	try {
	    name = _dict->getAttributeString(vtype, vendor);
	    attr = _dict->getAttribute(name, vendor);
	}
	catch (exception& e) {
	    cerr << "Unknown attribute " << vtype << endl;
	    continue;
	}
	
	value = data;
	try {
	    if(attr.type == "ipaddr") {
		in_addr ip;
		ip.s_addr = *(unsigned int*)(data.c_str());
		value = inet_ntoa(ip);
	    } else if(attr.type == "octets") {
		value = "0x";
		char tmp[3];
		for(int j=0; j<data.length(); ++j) {
		    snprintf(tmp, 3, "%02X", data[j]);
		    value += tmp;
		}
		value = data;
	    } else if(attr.type == "string") {
		value = data;
	    } else {
		value = _dict->getValueString(name, atoi(data.c_str()));
	    }
	}
	catch (exception& e) {
	    cerr << "Unknown value " << data << " for attribute " << name << "(" << attr.type << ")" << endl;
	    // Cound not found value
	}
	_vsattr[vendor].push_back(make_pair<string, string>(name, value));
    }
}

void RadPacket::_packVSAttributes(string *tmp)
{
    map<string, AttrList>::iterator j;
    for(j=_vsattr.begin(); j!=_vsattr.end(); ++j) {
	unsigned int vendor_id;
	try {
	    vendor_id = htonl(_dict->getVendorID(j->first));
	}
	catch (exception& e) {
	    cerr << e.what() << endl;
	    continue;
	}
        string s_attr;
        AttrList::iterator i;
		for(i = j->second.begin(); i!= j->second.end(); ++i) {
			s_attr = "";
			RadiusDictionary::attribute attrib;
			try {
				attrib = _dict->getAttribute(i->first, j->first);
			}
			catch (exception& e) {
				cerr << i->first << "\t" << j->first << endl;
				cerr << e.what() << endl;
				continue;
			}
			s_attr.resize(8);
			s_attr[0] = 26;
			s_attr[1] = 0;
			s_attr.replace(2, 4, (const char*)&vendor_id, 4);
			s_attr[6] = attrib.id;
			s_attr[7] = 0;
			if(attrib.type == "integer") {
				unsigned int tmp = htonl(atoi(i->second.c_str()));
				s_attr.append((char*)&tmp, 4);
			} else
			if(attrib.type == "string") {
				s_attr.append(i->second);
			} else
			if(attrib.type == "date") {
			} else
			if(attrib.type == "octets") {
				s_attr.append(i->second);
			} else {
				throw UnknownAttributeType();
			}
			s_attr[1] = (unsigned char)s_attr.size();
			s_attr[7] = s_attr[1]-6;
			tmp->append(s_attr);    
		}
    }
}

void RadPacket::replaceAttribute(std::string name, std::string value)
{
}

void RadPacket::insertAttribute(std::string name, std::string value)
{
    _attr.push_back(make_pair(name, value));
}

void RadPacket::insertVSAttribute(std::string vendor, std::string name, std::string value)
{
    _vsattr[vendor].push_back(make_pair(name,value));
}

void RadPacket::dump()
{
    char tmp[4];
    cerr << "Radius packet DUMP: " << endl
         << "           Code:" << _dict->getValueString("Packet-Type", _code) << endl
         << "     Identifier:" << "0x" ;
    snprintf(tmp, 4, "%02X", _identifier);
    cerr  << tmp << endl
         << "  Authenticator:" << "0x" ;
    for(int i=0; i<16; ++i) {
	snprintf(tmp, 4, "%02X ", _authenticator[i]);
	cerr << tmp;
    }
    cerr << endl;
    
    cerr << "  --- Attributes --- " << endl;
    for(int i=0; i<_attr.size(); ++i) {
	cerr << "        " << _attr[i].first << " = " << _attr[i].second << endl;
    }
    cerr << endl;
    
    std::map<std::string, AttrList>::iterator j;
    for(j=_vsattr.begin(); j!=_vsattr.end(); ++j) {
	cerr << "  --- VSAttributes " << j->first << " --- " << endl;
	for(int i=0; i<j->second.size(); ++i) {
	    cerr << "        " << j->second[i].first << " = " << j->second[i].second << endl;	
	}
    }
}

std::string RadPacket::getCode()
{
    return _dict->getValueString("Packet-Type", _code);
}

vector<string> RadPacket::getAttrValues(std::string name)
{
    vector<string> ret;
    for(int i=0; i<_attr.size(); ++i) {
	if(_attr[i].first == name) {
	    ret.push_back(_attr[i].second);
	}
    }
    return ret;
}

vector<string> RadPacket::getVSAttrValues(std::string vendor, std::string name)
{
    vector<string> ret;
    std::map<std::string, AttrList>::iterator j;
    for(j=_vsattr.begin(); j!=_vsattr.end(); ++j) {
	if(j->first != vendor) continue;
	for(int i=0; i<j->second.size(); ++i) {
	    if(j->second[i].first == name) ret.push_back(j->second[i].second);
	}
    }
    return ret;
}

