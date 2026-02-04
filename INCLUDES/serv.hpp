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
#include <algorithm>
#include <cassert>
#include <complex>
#include <set>
#include "client.hpp"
#include "channel.hpp"


class serv
{

    private:
        std::map<int, client> _client;
        int _Port;
        std::string _Password;
        int _serverFd;
        std::vector<struct pollfd> _pfds;
        std::map<std::string, channel> _channels;

        void        initSocket();
        void        acceptNewClient();
        void        handleClient(size_t i);
    
    public:
        serv();
        serv(int port, std::string password);
        ~serv();

        void        run();
        void        handleNick(int fd, std::istringstream &iss);
        bool        alreadyUsedName(const std::string &nick) const;
        void        sendToClient(int fd, const std::string &msg);
        void        handlePass(int fd, std::istringstream &iss);
        void        handleUser(int fd, std::istringstream &iss);
        void        handlePrivmsg(int fd, std::string &line);
        std::string findUserNick(std::string msg);
        int         findUserFd(std::string user);
        void        handleJoin(int fd, std::istringstream &iss);
        void        sendToUser(int fd, std::string msg);
        void        sendToChannel(int fd, std::string msg);
        void        sendJoinToChannel(std::string msg, std::string chanName);
        void        handlePart(int fd, std::istringstream &iss);
        void        handleKick(int fd, std::istringstream &iss);
        void        sendToChannelExcept(std::string chanName, std::string msg, int exceptFd);
};