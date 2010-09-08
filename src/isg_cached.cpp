//============================================================================
// Name        : isg_cached.cpp
// Author      : Grigory Holomiev <omever@gmail.com>
// Version     :
// Copyright   : Property of JV InfoLada
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

#include "oracle/billing.h"
#include "listener.h"

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

static Listener *__global_l;

void logrotate()
{
    string logfile("/var/log/isg/cached.log");
    if(__global_l != NULL)
	logfile = __global_l->get("MAIN:logfile");
	
    int fd = open(logfile.c_str(), O_CREAT | O_APPEND | O_LARGEFILE | O_WRONLY);
	close(2);
    dup2(fd, 2);
	close(1);
    dup2(fd, 1);
    close(fd);
    setlinebuf(stdout);
}

void sig(int signum)
{
	std::cerr << "Signal " << signum << " received" << std::endl;
	if(signum == SIGUSR1) {
		std::cerr << "Rotating logs" << std::endl;
	    logrotate();
	}
}

int main(int argc, char ** argv) {

	__global_l = NULL;
	
	string file = "/etc/cached.ini";

	if(argc > 1)
		file = argv[1];

	try {
		Listener l(file);
		
		__global_l = &l;
		
		cerr << "Debug: " << l.get("MAIN:debug") << endl;
		signal(SIGPIPE, sig);
		signal(SIGTTOU, sig);
		signal(SIGKILL, sig);
		signal(SIGSTOP, sig);
		signal(SIGUSR1, sig);
		if(l.get("MAIN:debug") != "true") {

			string user = l.get("MAIN:userid");
			if(user.length()) {
				struct passwd *r = getpwnam(user.c_str());
				if(r == NULL) {
					cerr << "Error looking for a user wwwrun" << endl;
					return -1;
				}
				setuid(r->pw_uid);
				setgid(r->pw_gid);
			}
			logrotate();
		}

		l.start();
	}
	catch (exception &e) {
		cerr << "Exception caught: " << e.what() << endl;
		cerr << "While loading configuration " << file << endl;
		return -1;
	}
    return EXIT_SUCCESS;
}
