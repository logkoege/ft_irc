#include "../INCLUDES/channel.hpp"
#include "../INCLUDES/serv.hpp"

channel::channel() : _inviteOnly(false), _topicOnly(false), _limit(-1) 
{}

channel::channel(const std::string &name) : _name(name), _inviteOnly(false), _topicOnly(false), _limit(-1) 
{}

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

void    channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

std::string channel::getTopic() const
{
    return _topic;
}

void    channel::addInvite(int fd)
{
    _invites.insert(fd);
}

bool    channel::isInvited(int fd) const
{
    if (_invites.count(fd) > 0)
        return (true);
    return (false);
}

void    channel::removeInvite(int fd)
{
    _invites.erase(fd);
}

void channel::setInviteOnly(bool b)
{
    _inviteOnly = b;
}

bool channel::isInviteOnly() const
{ 
    return _inviteOnly;
}

void channel::setTopicOnly(bool b)
{
    _topicOnly = b;
}

bool channel::isTopicOnly() const
{ 
    return _topicOnly;
}

void channel::setKey(const std::string &key)
{ 
    _key = key;
}

std::string channel::getKey() const
{
    return _key;
}

void channel::setLimit(int n)
{
    _limit = n;
}

int channel::getLimit() const
{
    return _limit;
}
