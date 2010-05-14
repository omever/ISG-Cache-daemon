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

#include "oracle/billing.h"
#include "listener.h"

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

void sig(int signum)
{
	std::cerr << "Signal " << signum << " received" << std::endl;
}

int main(void) {
/*    if(fork()) {
		std::cout << "ISG Cache daemon started" << std::endl;
		exit(0);
    }
*/
/*    
    close(0);
    close(1);
    close(2);
    int fd = open("/var/log/isg_cached_test.log", O_CREAT | O_APPEND | O_LARGEFILE | O_WRONLY);
    dup2(fd, 2);
    dup2(fd, 1);
    setlinebuf(stdout);

    setuid(30);
    setgid(8);
*/    

    signal(SIGPIPE, sig);
    signal(SIGTTOU, sig);
    signal(SIGKILL, sig);
    signal(SIGSTOP, sig);
    Listener l("/tmp/test.sock");
    l.start();

    return EXIT_SUCCESS;
}
