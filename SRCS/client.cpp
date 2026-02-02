#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"

client::client() : _user(false), _nick(false), _registered(false), _authenticated(false), _fd(-1), _name("Clanker_"){}
client::~client(){}
client::client(int fd) :  _user(false), _nick(false), _registered(false), _authenticated(false), _fd(fd)
{
    _name = "Levai_";

    if (fd == 0)
        _name += '0';
    while (fd > 0)
    {
        _name += char(fd % 10 + '0');
        fd = fd / 10;
    }
}

int client::getFd() const
{
    return (this->_fd);
}

bool    client::isAuthenticated() const
{
    return(_authenticated);
}

const   std::string &client::getName() const
{
    return(this->_name);
}

void    client::addBuffer(const std::string &data)
{
    this->_buffer += data;
}

bool    client::extractLine(std::string &line)
{
    size_t pos = _buffer.find("\n");
    if (pos == std::string::npos)
        return (false);
    line = _buffer.substr(0, pos);
    _buffer.erase(0, pos + 1);
    return (true);
}

void    client::setName(const std::string &name)
{
    this->_name = name;
    this->_nick = true;
}

void    client::authenticate()
{
    this->_authenticated = true;
}

void    client::setUser(const std::string &username)
{
    this->_username = username;
    this->_user = true;
}

const std::string &client::getUser()const
{
    return(this->_username);
}

bool    client::isRegistered() const
{
    return(_registered);
}

void    client::setregister()
{
    if (this->_nick && this->_user && this->_authenticated)
        this->_registered = true;
}