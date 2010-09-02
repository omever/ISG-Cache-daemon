#include "radclient.h"
#include "radpacket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <string>
#include <iostream>
#include <cstring>

using namespace std;

RadClient::RadClient(std::string server, unsigned short port, std::string secret)
{
    _server = server;
    _port = port;
    _secret = secret;
    _timeout = 2;
    _socket = 0;
        
    inisocket();
}

RadClient::~RadClient()
{
    if(_socket != 0) {
	close(_socket);
    }
}

void RadClient::inisocket()
{
    if(_socket == 0) {
	_socket = socket(PF_INET, SOCK_DGRAM, 0);
	
	struct sockaddr_in sin;
	struct sockaddr_in sout;
	
	memset (&sin, 0, sizeof(sin)); 
	memset (&sout, 0, sizeof(sin)); 
	sin.sin_family = AF_INET; 
	sin.sin_addr.s_addr = inet_addr(_server.c_str()); 
	sin.sin_port = htons(_port); 

	sout.sin_family = AF_INET; 
	sout.sin_addr.s_addr = inet_addr("0.0.0.0"); 
	sout.sin_port = 1813; 
	
	connect(_socket, (struct sockaddr *)&sin, sizeof(sin));
	fcntl(_socket, F_SETFL, fcntl(_socket, F_GETFL) | O_NONBLOCK);
    }
}

void RadClient::send(RadPacket &packet)
{
    string data = packet.pack();
    if(::send(_socket, (void*)data.data(), data.size(), 0) == -1) {
	throw ErrorSendingData();
    }
}

void RadClient::read(RadPacket &packet)
{
    fd_set fdlist;
    struct timeval tv;
    
    FD_ZERO(&fdlist);
    FD_SET(_socket, &fdlist);
    
    tv.tv_sec = _timeout;
    tv.tv_usec = 0;
    
    int res = select(_socket+1, &fdlist, NULL, NULL, &tv);
    
    if(res > 0) {
	string buffer = "";
	char buf[65535];
	
	int len;
	while ((len = ::read(_socket, buf, 65535))>0) {
	    cerr << len << " bytes readed " << endl;
	    buffer.append(buf, len);
	}
	
	packet.unpack(buffer);
    } else if(res < 0) {
	cerr << "Something goes wrong: " << errno << endl;
	throw ErrorReceivingData();
    } else {
	throw TimeoutReading();
    }
}
