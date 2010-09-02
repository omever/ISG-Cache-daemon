#ifndef RADCLIENT_H
#define RADCLIENT_H

#include <string>

class RadPacket;

class RadClient
{
public:
    RadClient(std::string server, unsigned short port, std::string secret);
    ~RadClient();
    void send(RadPacket &packet);
    void read(RadPacket &packet);
    class ErrorSendingData: public std::exception {
	virtual const char* what() const throw() {
	    return "Can not send data over the network";
	}
    };
    class ErrorReceivingData: public std::exception {
	virtual const char* what() const throw() {
	    return "Can not read data over the network";
	}
    };
    class TimeoutReading: public std::exception {
	virtual const char* what() const throw() {
	    return "Timeout while reading data from network";
	}
    };
private:
    std::string _server, _secret;
    unsigned short _port;
    unsigned short _timeout;
    void inisocket();
    int _socket;
};

#endif
