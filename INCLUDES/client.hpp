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

class client
{
    private:
        bool        _user;
        bool        _nick;
        bool        _registered;
        bool        _authenticated;
        int         _fd;
        std::string _name;
        std::string _username;
        std::string _buffer;
        
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
        void        setUser(const std::string &username);
        bool        isRegistered() const;
        const       std::string &getUser()const;
        void        setregister();


};