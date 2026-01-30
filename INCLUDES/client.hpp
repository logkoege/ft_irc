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
    private:
        bool        _authenticated;
        int         _fd;
        std::string _buffer;
        std::string _name;
        
        public:
        
        client();
        ~client();
        client(int fd);

        int         getFd() const;
        bool        isAuthenticated() const;
        const       std::string &getName() const;

        void        addBuffer(const std::string &data);
        bool        extractLine(std::string &line);

        void        setName(const std::string &name);
        void        authenticate();
        
};