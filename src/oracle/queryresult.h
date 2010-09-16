#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include <vector>
#include <map>
#include <string>

class queryResult
    : public std::vector<std::multimap<std::string, std::string> >
{
public:
	queryResult();
	void add_bind(const std::string &name, const std::string &value, bool is_null = false);
	void add_bind(const std::string &name, const char *value, bool is_null = false);
	void add_bind(const char *name, const std::string &value, bool is_null = false);
	void add_bind(const char *name, const char *value, bool is_null = false);
	const std::string & get_bind_value(std::string &name);
	const bool & bind_isnull(std::string &name);
	const std::string to_xml();
protected:
private:
	std::map<std::string, std::pair<std::string, bool> > __bindouts;
	std::string __info;
	long long __code;
	static const std::string __emptystring;
	static const bool __true;
};

#endif
