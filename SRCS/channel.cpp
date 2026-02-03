#include "../INCLUDES/channel.hpp"

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

void channel::addClient(int fd)
{
    _clients.insert(fd);
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