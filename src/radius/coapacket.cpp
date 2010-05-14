#include "coapacket.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <openssl/md5.h>

using namespace std;

RadPacketCoA::RadPacketCoA(RadiusDictionary *dict)
    : RadPacket(dict)
{
    srand(time(NULL));
}

RadPacketCoA::RadPacketCoA(RadPacket &packet)
	: RadPacket(packet)
{
	
}


RadPacketCoA::RadPacketCoA(RadPacket packet)
	: RadPacket(packet)
{
	
}

string RadPacketCoA::_bigmd5(string instr)
{
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)instr.c_str(), instr.size(), md);
    
    string ret = "";
    ret.append((char*)md, MD5_DIGEST_LENGTH);
    return ret;
}

string RadPacketCoA::_bigxor(string in1, string in2)
{
    string result;
    result.resize(in1.size());
    for(int i=0; i<in1.size(); ++i) {
	result[i] = in1[i] ^ in2[i];
    }
    return result;
}

string RadPacketCoA::_passwordpack(string authenticator, string secret, string password)
{
    string passauth = authenticator;
    string pass = authenticator;
    password.insert(0, 1, password.size());
    size_t len = 16-password.size()%16;
    password.append(len, 0);
    for(unsigned int i=0; i<password.size(); i+=16) {
	passauth = _bigxor(password.substr(i,16), _bigmd5(secret+passauth));
	pass += passauth;
    }
    return pass;
}

string RadPacketCoA::_bigrand()
{
    string ret = "";
    for(int i=0; i<4; ++i) {
	unsigned int m = rand();
	ret.append((char*)&m, 4);
    }
    return ret;
}

void RadPacketCoA::loggon(string username, string password, string id)
{
    setCode("CoA-Request");
    insertAttribute("User-Name", username);
    insertVSAttribute("Cisco", "Cisco-Account-Info", id[0] == 'S'?id:string("S")+id);
    insertVSAttribute("Cisco", "Cisco-Command-Code", "\001");

    string pass = _passwordpack(_bigrand(), __secret, password);
    insertVSAttribute("Cisco", "Cisco-Subscriber-Password", pass);
}

string RadPacketCoA::pack(void)
{
    string out = RadPacket::pack();
    
    string prauth;
    prauth.resize(16, 0);
    out.replace(4, 16, _bigmd5(out.substr(0, 4) + prauth + out.substr(20) + __secret));
    return out;
}

void RadPacketCoA::ping(std::string id, std::string service)
{
    setCode("CoA-Request");
    if(id[0] == 'I') {
	insertAttribute("Acct-Session-Id", id.substr(1));
    } else if(id[0] == 'S') {
	insertVSAttribute("Cisco", "Cisco-Account-Info", id);
    } else {
	insertVSAttribute("Cisco", "Cisco-Account-Info", string("S")+id);
    }
    if(service == "") {
	insertVSAttribute("Cisco", "Cisco-Command-Code", "\004 &");
    } else {
	insertVSAttribute("Cisco", "Cisco-Command-Code", "\004"+service);
    }
}

void RadPacketCoA::logout(std::string id)
{
    setCode("CoA-Request");
    insertVSAttribute("Cisco", "Cisco-Account-Info", string("S")+id);
    insertVSAttribute("Cisco", "Cisco-Command-Code", "\002");
}

void RadPacketCoA::serviceon(std::string id, std::string service)
{
    setCode("CoA-Request");
    insertVSAttribute("Cisco", "Cisco-Account-Info", string("S")+id);
    char cmd[2];
    cmd[0] = 0xb;
    cmd[1] = 0;
    insertVSAttribute("Cisco", "Cisco-Command-Code", string(cmd)+service);
}

void RadPacketCoA::serviceoff(std::string id, std::string service)
{
    setCode("CoA-Request");
    insertVSAttribute("Cisco", "Cisco-Account-Info", string("S")+id);
    char cmd[2];
    cmd[0] = 0xc;
    cmd[1] = 0;
    insertVSAttribute("Cisco", "Cisco-Command-Code", string(cmd)+service);
}

void RadPacketCoA::change(std::string id, std::string off, std::string on)
{
    setCode("CoA-Request");
    insertVSAttribute("Cisco", "Cisco-Account-Info", string("S")+id);
    char cmd[2];
    cmd[0] = 0xb;
    cmd[1] = 0;
    insertVSAttribute("Cisco", "Cisco-Command-Code", string(cmd)+on);
    cmd[0] = 0xc;
    insertVSAttribute("Cisco", "Cisco-Command-Code", string(cmd)+off);
}

