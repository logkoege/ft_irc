#include "../INCLUDES/channel.hpp"
#include "../INCLUDES/serv.hpp"

channel::channel() {}

channel::channel(const std::string &name)
{
    this->_name = name;
}

channel::~channel()
{}

const std::string &channel::getName()
{
    return (this->_name);
}

void channel::removeClient(int fd)
{
    _clients.erase(fd);
}

bool channel::hasClient(int fd)
{
    return (_clients.count(fd));
}
std::set<int> &channel::getClient()
{
    return(_clients);
}

bool    channel::isClient(int fd)
{
    return (_clients.count(fd));
}

bool    channel::isOp(int fd)
{
    return _operators.count(fd);
}

void    channel::addClient(int fd)
{
    _clients.insert(fd);
}

void    channel::addOp(int fd)
{
    if (isClient(fd))
        _operators.insert(fd);
}

void    channel::deOp(int fd)
{
    _operators.erase(fd);
}
