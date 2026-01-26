#pragma once

#include "ft_irc.hpp"

class serv
{

    private:
        int _Port;
        std::string _Password;

    public:
        serv();
        ~serv();
        serv(int port, std::string password);


        void socket();
        void run();
};