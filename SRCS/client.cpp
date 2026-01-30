#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"

client::client() : _authenticated(false), _fd(-1), _name("clanker_"){}
client::~client(){}
client::client(int fd) :  _authenticated(false), _fd(fd)
{
    _name = "levai_";

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
    if (!line.empty() && line[line.size() -1 == '\r'])
        line.erase(line.size() - 1);
    _buffer.erase(0, pos + 1);
    return (true);
    
}

void    client::setName(const std::string &name)
{
    this->_name = name;
}

void    client::authenticate()
{
    this->_authenticated = true;
}