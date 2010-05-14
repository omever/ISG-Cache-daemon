#ifndef UTILS_H
#define UTILS_H

#include <sstream>
#include <string>

#define IFEXIST(x) (x.size()?x.at(0):"")

typedef unsigned long long size64;

template <class T>
bool from_string(T& t, 
                 const std::string& s)
{
  std::istringstream iss(s);
  return !(iss >> t).fail();
}

template <class T>
T string_cast(const std::string& s)
{
	T t;
	std::istringstream iss(s);
	iss >> t;
	return t;
}

std::string ip2num(const std::string &src);

#endif
