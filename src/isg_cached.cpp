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
//#define DEBUG

void logrotate()
{
    int fd = open("/var/log/isg/cached.log", O_CREAT | O_APPEND | O_LARGEFILE | O_WRONLY);
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

int main(void) {
#ifndef DEBUG    
    signal(SIGPIPE, sig);
    signal(SIGTTOU, sig);
    signal(SIGKILL, sig);
    signal(SIGSTOP, sig);
    signal(SIGUSR1, sig);

	struct passwd *r = getpwnam("wwwrun");
	if(r == NULL) {
		cerr << "Error looking for a user wwwrun" << endl;
		return -1;
	}
	setuid(r->pw_uid);
    setgid(r->pw_gid);

    logrotate();
#endif

    Listener l("/tmp/test.sock");
    l.start();
    return EXIT_SUCCESS;
}
