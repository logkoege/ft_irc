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
#include <sstream>
#include <map>
#include "client.hpp"

class serv
{

    private:
        std::map<int, client> _client;

        int _Port;
        std::string _Password;

        int _serverFd;
        std::vector<struct pollfd> _pfds;

        void initSocket();
        void acceptNewClient();
        void handleClient(size_t i);
    
    public:
        serv();
        serv(int port, std::string password);
        ~serv();

        void run();

        void    handleNick(int fd, std::istringstream &iss);
        bool    alreadyUsedName(const std::string &nick) const;
        void    sendToClient(int fd, const std::string &msg);


};