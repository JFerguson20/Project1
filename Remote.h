/*
 * server & client
 * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include "Sh.h"
#include <sstream>

int remote(std::vector<std::string> params);
int parseRemoteCommand(std::string command);
