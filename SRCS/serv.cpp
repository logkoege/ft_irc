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
        throw std::runtime_error("socket prblm");
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsocket prblm");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_Port);
    if(bind(_serverFd, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind prblm");
    if (listen(_serverFd, SOMAXCONN) < 0)
        throw std::runtime_error("listen prblm");
    fcntl(_serverFd, F_SETFL, O_NONBLOCK);
    pollfd pfd;
    pfd.fd = _serverFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pfds.push_back(pfd);
    
    std::cout << "serv up" << std::endl;
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

    std::cout << "client connect( " << _client[clientFd].getName() << " )" << std::endl;
}

void serv::handleClient(size_t i)
{
    int fd = _pfds[i].fd;
    char buffer[512];
    int bytes = recv(_pfds[i].fd, buffer, sizeof(buffer) -1, 0);
    std::string line;

    if (bytes <= 0)
    {
        std::cout << _client[fd].getName() << "deco" << std::endl;
        close(_pfds[i].fd);
        _pfds.erase(_pfds.begin() + i);
        _client.erase(fd);
        return;
    }
    buffer[bytes] = '\0';
    _client[fd].addBuffer(buffer);
    while (_client[fd].extractLine(line))
    {
        //std::cout << _client[fd].getName() << " a dis " << line << std::endl;

        std::istringstream  iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "NICK")
            handleNick(fd, iss);
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
            throw std::runtime_error("poll prblm");
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
        sendToClient(fd, "431 :No name given\r\n");
        return;
    }

    if (alreadyUsedName(newName))
    {
        sendToClient(fd, "433 " + newName + ":Name is already use\r\n");
        return;
    }
    _client[fd].setName(newName);
}

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
