#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <vector>
#include <map>
#include <exception>

struct attribute_s
{
    unsigned char id;
    std::string type;
    std::string vendor;
    std::map<std::string, unsigned int> values;
};


class RadiusDictionary
{
public:
	typedef struct attribute_s attribute;

	RadiusDictionary();
    void parse(std::string file);
    unsigned short getValue(std::string attr, std::string value);
    attribute getAttribute(std::string attr, std::string vendor="");
    unsigned int getVendorID(std::string vendor);

    std::string getValueString(std::string attr, unsigned int id);
    std::string getAttributeString(unsigned int id, std::string vendor="");
    std::string getVendorString(unsigned int vendor);
        
    std::vector<std::string> split(std::string instr);


    class AttributeNotFound: public std::exception {
	virtual const char* what() const throw() {
	    return "Specified attribute not found in dictionary";
	}
    };

    class ValueNotFound: public std::exception {
	virtual const char* what() const throw() {
	    return "Specified attribute value not found in dictionary";
	}
    };
    class VendorNotFound: public std::exception {
	virtual const char* what() const throw() {
	    return "Specified vendor not found in dictionary";
	}
    };

private:
    std::map<std::string, attribute> attrs;
    std::map<std::string, unsigned int> vendors;
};

#endif
