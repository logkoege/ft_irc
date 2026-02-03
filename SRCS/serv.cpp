#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"

serv::serv() : _Port(0), _serverFd(-1) {}

serv::serv(int port, std::string password)
{
    _Port = port;
    _Password = password;
    _serverFd = -1;
}

serv::~serv()
{
    for (size_t i = 0; i < _pfds.size(); i++)
        close(_pfds[i].fd);
}

void serv::initSocket()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
        throw std::runtime_error("Socket prblm");
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Setsocket prblm");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_Port);
    if(bind(_serverFd, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Bind prblm");
    if (listen(_serverFd, SOMAXCONN) < 0)
        throw std::runtime_error("Listen prblm");
    fcntl(_serverFd, F_SETFL, O_NONBLOCK);
    pollfd pfd;
    pfd.fd = _serverFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pfds.push_back(pfd);
    
    std::cout << "Serv up" << std::endl;
}

void serv::acceptNewClient()
{
    int clientFd = accept(_serverFd, NULL, NULL);
    if (clientFd < 0)
        return ;
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pfds.push_back(pfd);
    _client[clientFd] = client(clientFd);

    std::cout << "Client connect -> " << _client[clientFd].getName() << std::endl;
}

void serv::handleClient(size_t i)
{
    int fd = _pfds[i].fd;
    char buffer[512];
    int bytes = recv(_pfds[i].fd, buffer, sizeof(buffer) -1, 0);
    std::string line;

    if (bytes <= 0)
    {
        std::cout << "Client leaved -> " << _client[fd].getName() << std::endl;
        close(_pfds[i].fd);
        _pfds.erase(_pfds.begin() + i);
        _client.erase(fd);
        return;
    }
    buffer[bytes] = '\0';
    _client[fd].addBuffer(buffer);
    while (_client[fd].extractLine(line))
    {
        std::istringstream  iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "NICK")
            handleNick(fd, iss);
        if (cmd == "PASS")
            handlePass(fd, iss);
        if (cmd == "USER")
            handleUser(fd, iss);
        if (cmd == "PRIVMSG")
            handlePrivmsg(fd, line);
        if (cmd == "JOIN")
            handleJoin(fd, iss);
    }
}

void serv::run()
{
    initSocket();
    while (1)
    {
        for (size_t i = 0; i < _pfds.size(); i++)
            _pfds[i].revents = 0;
        if (_pfds.empty())
            continue;
        if (poll(&_pfds[0], _pfds.size(), -1) < 0)
            throw std::runtime_error("Poll prblm");
        for (size_t i = 0; i < _pfds.size(); i++)
        {
            if (_pfds[i].revents & POLLIN)
            {
                if (_pfds[i].fd == _serverFd)
                    acceptNewClient();
                else
                    handleClient(i);
            }
        }
    }
}

void serv::handleNick(int fd, std::istringstream &iss)
{
    std::string newName;
    iss >> newName;

    if (newName.empty())
    {
        sendToClient(fd, "431 : No name given\r\n");
        return;
    }

    if (alreadyUsedName(newName))
    {
        sendToClient(fd, "433 " + newName + " : Name is already use\r\n");
        return;
    }
    _client[fd].setName(newName);
    _client[fd].setregister();
    if (_client[fd].isRegistered())
        sendToClient(fd, "you are registered\nyour nickname : " + _client[fd].getName() + "\nyour username : " + _client[fd].getUser() + "\r\n");}

bool serv::alreadyUsedName(const std::string &nick) const
{

    for (std::map<int, client>::const_iterator it = _client.begin(); it != _client.end(); ++it)
    {
        if (it->second.getName() == nick)
            return (true);
    }
    return (false);
}

void serv::sendToClient(int fd, const std::string &msg)
{
    send(fd, msg.c_str(), msg.size(), 0);
}


void serv::handlePass(int fd, std::istringstream &iss)
{
    std::string password;
    iss >> password;

    if (password != _Password)
        sendToClient(fd, "wrong password\r\n");
    else 
        _client[fd].authenticate();
    _client[fd].setregister();
    if (_client[fd].isRegistered())
        sendToClient(fd, "you are registered\nyour nickname : " + _client[fd].getName() + "\nyour username : " + _client[fd].getUser() + "\r\n");
}

void serv::handleUser(int fd, std::istringstream &iss)
{
    std::string username;
    iss >> username;

    if (username.empty())
    {
        sendToClient(fd, "empty username\r\n");
        return;
    }
    _client[fd].setUser(username);
    _client[fd].setregister();
    if (_client[fd].isRegistered())
        sendToClient(fd, "you are registered\nyour nickname : "
            + _client[fd].getName() + "\nyour username : " + _client[fd].getUser() + "\r\n");
}

void serv::handlePrivmsg(int fd, std::string &line)
{
    if (!(_client[fd].isRegistered()))
    {
        sendToClient(fd,
            "you cannot send private messages if you are not registered\r\n");
        return;
    }
    std::string msg(line);
    while (!msg.empty() &&
            (msg[msg.size() - 1] == '\n' || msg[msg.size() - 1] == '\r'))
        msg.erase(msg.size() - 1);
    size_t start = 0;
    while (start < msg.size() && msg[start] == ' ')
        start++;
    msg.erase(0, start);
    if (msg.compare(0, 8, "PRIVMSG ") == 0)
        msg.erase(0, 8);
    if (msg[0] == '#')
        sendToChannel(fd, msg);
    else
        sendToUser(fd, msg);
}

void serv::sendToChannel(int fd, std::string msg)
{
    size_t pos = msg.find(' ');
    if (pos == std::string::npos)
        return (sendToClient(fd, "No text to send\r\n"));

    std::string chanName = msg.substr(0, pos);
    std::string text = msg.substr(pos + 1);

    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return (sendToClient(fd, "No such channel\r\n"));
    if (!it->second.hasClient(fd))
        return (sendToClient(fd, "You are not in this channel\r\n"));
    std::string out =  _client[fd].getName() + " sended  an msg in -> " + chanName + "  and sayed -> " + text + "\r\n";

    std::set<int> &clients = it->second.getClient();
    for (std::set<int>::iterator its = clients.begin(); its != clients.end(); ++its)
        if (*its != fd)
            send(*its, out.c_str(), out.size(), 0);
}

void    serv::sendToUser(int fd, std::string msg)
{
    std::string user = findUserNick(msg);
    msg.erase(0, user.size() + 1);
    int userfd = findUserFd(user);
    if (userfd == -1)
        sendToClient(fd, "User not found\r\n");
    else
        sendToClientFromClient(fd, userfd, msg);
}

std::string serv::findUserNick(std::string msg)
{
    std::string user(msg);
    int i = 0;

    while (msg[i] != ' ')
    {    
        i++;
    }
    user.erase(i, msg.size());
    return (user);
}

int serv::findUserFd(std::string user)
{
    int userfd = -1;
    std::map<int, client>::iterator it = _client.begin();

    while(it != _client.end())
    {
        if(it->second.getName() == user)
        userfd = it->first;
        it++;
    }
    return(userfd);
}

void serv::sendToClientFromClient(int fd, int userfd, std::string msg)
{
    std::map<int, client>::iterator it = _client.begin();

    while(it != _client.end())
    {
        if(it->second.getFd() == fd)
            msg = "u receved a message from : " + it->second.getName() + " -> " + msg;
        it++;
    }
    send(userfd, msg.c_str(), msg.size(), 0);
}

void serv::handleJoin(int fd, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (chanName.empty() || chanName[0] != '#')
    {
        sendToClient(fd, "Invalid channel name\r\n");
        return;
    }

    if (_channels.find(chanName) == _channels.end())
        _channels[chanName] = channel(chanName);

    _channels[chanName].addClient(fd);

    sendToClient(fd, "Joined " + chanName + "\r\n");
}

