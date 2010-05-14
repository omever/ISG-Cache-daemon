#include "utils.h"

#include <sstream>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

string ip2num(const string &src)
{
	struct in_addr in;
	int ip = inet_aton(src.c_str(), &in);
	ostringstream oss;
	oss << htonl(in.s_addr);
	return oss.str();
}

