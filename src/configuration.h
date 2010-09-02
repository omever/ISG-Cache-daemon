/*
 * configuration.h
 *
 *  Created on: 19.07.2010
 *      Author: om
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <exception>
#include <map>

class NoFileException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "No configuration file specified";
  }
};

class DictionaryLoadException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Error loading configuration";
  }
};

class NotImplementedException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "This feature is not implemented yet";
  }
};

class Configuration {
public:
	Configuration();
	Configuration(const std::string &file);
	virtual ~Configuration();

	void open(const std::string &file);
	void load();
	void save();

	std::string get(const std::string &key);
	void set(const std::string &key, const std::string &value);

	void dump();
private:
	std::string __file;
	std::map<std::string, std::string> __data;
};

#endif /* CONFIGURATION_H_ */
