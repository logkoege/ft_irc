#include "../INCLUDES/ft_irc.hpp"

serv::serv(){}


serv::serv(int port, std::string password)
{
    _Port = port;
    _Password = password;
}

serv::~serv(){}

void serv::run()
{
    //run the serv
}

