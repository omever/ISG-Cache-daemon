#ifndef RADPACKET_H
#define RADPACKET_H

#include <string>
#include <vector>
#include <utility>

#include "dictionary.h"

class RadPacket
{
public:
    RadPacket(RadiusDictionary *dict);
    virtual std::string pack();
    void unpack(std::string);
    
    void setSecret(std::string newsecret);
    std::string secret();

    void setCode(std::string newcode);
    void setIdentifier(unsigned char newident);
    void setAuthenticator(unsigned char *newauth);
    void replaceAttribute(std::string name, std::string value);
    void insertAttribute(std::string name, std::string value);
    void replaceVSAttribute(std::string vendor, std::string name, std::string value);
    void insertVSAttribute(std::string vendor, std::string name, std::string value);
    
    void dump();

    std::string getCode();
    unsigned char getIdentifier();
    std::string getAuthenticator();
    
    std::vector<std::string> getAttrValues(std::string name);
    std::vector<std::string> getVSAttrValues(std::string vendor, std::string name);

    class UnknownAttributeType: public std::exception {
	virtual const char* what() const throw() {
	    return "Attribute type not known";
	}
    };
    typedef std::vector<std::pair<std::string, std::string> > AttrList;
private:
    RadiusDictionary *_dict;
    unsigned short _code;
    unsigned short _length;
    unsigned char _identifier;
    unsigned char _authenticator[16];
    
    AttrList _attr;
    std::map<std::string, AttrList> _vsattr;

    void _packHeader(std::string *tmp);
    void _packAttributes(std::string *tmp);
    void _packVSAttributes(std::string *tmp);
    void _unpackHeader(std::string *tmp);
    void _unpackAttributes(std::string *tmp);
    void _unpackVSAttributes(std::string *tmp);

protected:
    std::string __secret;
};

#endif
