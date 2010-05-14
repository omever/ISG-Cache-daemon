#ifndef COA_PACKET
#define COA_PACKET

#include <string>

#include "dictionary.h"
#include "radclient.h"
#include "radpacket.h"

class RadPacketCoA : public RadPacket
{
public:
    RadPacketCoA(RadiusDictionary *dict);
	RadPacketCoA(RadPacket &);
	RadPacketCoA(RadPacket);
    std::string pack();
    void ping(std::string id, std::string service="");
    void loggon(std::string username, std::string password, std::string id);
    void logout(std::string id);
    void serviceon(std::string id, std::string service);
    void serviceoff(std::string id, std::string service);
    void change(std::string id, std::string off, std::string on);
private:
    std::string _bigmd5(std::string instr);
    std::string _bigxor(std::string in1, std::string in2);
    std::string _passwordpack(std::string authenticator, std::string secret, std::string password);
    std::string _bigrand();
};

#endif
