#pragma once
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#include <string>
#include <cstdlib>
#include <cstring>

class client
{ 
    public:
        int _fd;
        std::string name;

        client();
        ~client();
        client(int fd);
};