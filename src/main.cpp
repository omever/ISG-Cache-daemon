#include "dictionary.h"
#include "radclient.h"
#include "radpacket.h"
#include "coapacket.h"

#include <string>
#include <iostream>
#include <fstream>
#include <openssl/md5.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <arpa/inet.h>

using namespace std;

    RadiusDictionary dict;
    

int getreply(void *ptr)
{
    RadClient *client = (RadClient*)ptr;
    RadPacket got(&dict);
    try {
	client->read(got);
    }
    catch (exception &e) {
	cerr << "Error occured: " << e.what() << endl;
	return -1;
    }
    got.dump();
    cout << got.getCode() << endl;
    vector<string> tmp;
    tmp = got.getVSAttrValues("Cisco", "Cisco-Command-Code");
    if(tmp.size()) {
        cout << endl;
	cout << "Cisco:Cisco-Command-Code"<<endl;
	for(int i=0; i<tmp.size(); ++i) {
	    cout << tmp.at(i) << endl;
	}
    }
    tmp = got.getVSAttrValues("Cisco", "Cisco-Account-Info");
    if(tmp.size()) {
	cout << endl;
	cout << "Cisco:Cisco-Account-Info"<<endl;
	for(int i=0; i<tmp.size(); ++i) {
	    cout << tmp.at(i) << endl;
	}
    }
    tmp = got.getAttrValues("Framed-Address");
    if(tmp.size()) {
	cout << endl;
	cout << "Radius:Framed-Address"<<endl;
	for(int i=0; i<tmp.size(); ++i) {
	    cout << tmp.at(i) << endl;
	}
    }

    tmp = got.getAttrValues("NAS-Port-Id");
    if(tmp.size()) {
	cout << endl;
	cout << "Radius:NAS-Port-Id"<<endl;
	for(int i=0; i<tmp.size(); ++i) {
	    cout << tmp.at(i) << endl;
	}
    }
    
    return 0;
}

RadPacketCoA getpacket(char id)
{
    RadPacketCoA packet(&dict);
    packet.setSecret("cisco");
    packet.setIdentifier(id);
    return packet;
}

int cmain(int argc, char **argv)
{
    char pid = time(NULL)%256;
    std::vector<std::string> _params;
    _params.clear();
    
    std::string mybuf;

    dict.parse("/usr/share/freeradius/dictionary");
    dict.parse("/u01/app/billing/isg/coa-client/dictionary");

    
    cerr << "Reading params" << endl;
    while(! cin.eof() )
    {
	getline(cin, mybuf);
	cerr << mybuf << endl;
	_params.push_back(mybuf);
    }
    
    if(_params.size() < 2) {
	cerr << "Not enough params" << endl;
	return -1;
    }

    if(argc > 1) {
	int stime = atoi(argv[1]);
	if(stime) { 
// Fork the process and close main application, after that sleep for stime (in ms) period and to all the dirt job
	    if(fork()) {
		close(0);
		close(1);
		close(2);
		return(0);
	    }
	    
// Do the selected sleep :)
	    struct timeval tv;
	    tv.tv_sec = stime/1000;
	    tv.tv_usec = stime%1000;
	    
	    select(0, NULL, NULL, NULL, &tv);
	}
    }

    
    std::string command = _params[0];
    std::string id = _params[1];
    
    vector<RadPacketCoA> plist;
    plist.clear();
    
    if(command == "loggon") {
	if(_params.size() < 4) {
	    cerr << "Not enough params" << endl;
	    return -1;
	}
	std::string user = _params[2];
	std::string passwd = _params[3];
	RadPacketCoA packet = getpacket(pid++);
	packet.loggon(user, passwd, id);
	plist.push_back(packet);
    } else
    if(command == "logout") {
	RadPacketCoA packet = getpacket(pid++);
	packet.logout(id);
	plist.push_back(packet);
    } else
    if(command == "ping") {
	RadPacketCoA packet = getpacket(pid++);
	packet.ping(id, _params[2]);
	plist.push_back(packet);
    } else
    if(command == "on") {
	if(_params.size() < 3) {
	    cerr << "Not enough params" << endl;
	    return -1;
	}
	std::string service = _params[2];
	RadPacketCoA packet = getpacket(pid++);
	packet.serviceon(id, service);
	plist.push_back(packet);
    } else
    if(command == "off") {
	if(_params.size() < 3) {
	    cerr << "Not enough params" << endl;
	    return -1;
	}
	std::string service = _params[2];
	RadPacketCoA packet = getpacket(pid++);
	packet.serviceoff(id, service);
	plist.push_back(packet);
    } else
    if(command == "change") {
	if(_params.size() < 4) {
	    cerr << "Not enough params" << endl;
	    return -1;
	}
	std::string off = _params[2];
	std::string on = _params[3];
	RadPacketCoA packet = getpacket(pid++);
	packet.serviceoff(id, off);
	plist.push_back(packet);

	packet = getpacket(pid++);
	packet.serviceon(id, on);
	plist.push_back(packet);
    } else {
	cerr << "Unknown command" << endl;
	return -1;
    }
        
    try {
//	packet.dump();
    }
    catch (exception &e)
    {
	cerr << "Error occured: " <<e.what() << endl;
    close(0);
    close(1);
    close(2);
	return -1;
    }
    
    
    string server;
    
    size_t pos = id.find(':');
    
    if(pos == string::npos) {
	server = "217.113.112.11";
    } else {
	server = id.substr(0, pos);
    }

//    server = "217.113.112.9";
    if(server[0] == 'S') {
	server = server.substr(1);
    }
    RadClient client(server, 1813, "cisco");
    
    cerr << "Sending data... " << endl;
    try {
	for(int i=0; i<plist.size(); i++) {
//	    plist.at(i).dump();
	    client.send(plist.at(i));
	    int res = getreply(&client);
	    if(res) {
		close(0);
    		close(1);
		close(2);
		return -1;
	    }
	    
	    
	}
    }
    catch (exception &e)
    {
	cerr << "Error occured: " << e.what() << endl;
    close(0);
    close(1);
    close(2);
	return -1;
    }
    
    cerr << "Receiving data... " << endl;
//    cerr << dec << "Program terminated with [" << res << "] status code" << endl;

    close(0);
    close(1);
    close(2);
    
    return 0;
}
