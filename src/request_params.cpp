#include <string>
#include <vector>
#include <ctype.h>

#include "request_params.h"

using namespace std;

string requestParams::avpair(string name)
{
	vector<string>::iterator i, iend;
	for(i = (*this)["CISCO-AVPAIR"].begin(); i != (*this)["CISCO-AVPAIR"].end() ; ++i) {
		size_t pos = i->find("=");
		if(pos == string::npos)
			continue;
		
		if(i->substr(0, pos-1) == name)
			return i->substr(pos+1);
	}
}

std::vector<std::string> & requestParams::operator[](const std::string &Key)
{
	string tmp = "";
	for(int i=0; i<Key.length(); ++i)
		tmp += toupper(Key[i]);
	return ((map<string, vector<string> >*)this)->operator[](tmp);
}
