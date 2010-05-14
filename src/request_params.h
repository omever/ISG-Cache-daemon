#ifndef REQUEST_PARAMS_H
#define REQUEST_PARAMS_H

#include <string>
#include <map>
#include <vector>

class requestParams : public std::map<std::string, std::vector<std::string> >
{
	public:
		std::string avpair(std::string name);
		std::vector<std::string> & operator[](const std::string &);
};

#endif
