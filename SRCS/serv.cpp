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

void    serv::initSocket()
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

void    serv::acceptNewClient()
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
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    std::string line;

    if (bytes <= 0)
    {
        while (_client[fd].extractLine(line))
        {
            std::istringstream iss(line);
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
            if (cmd == "KICK")
                handleKick(fd, iss);
            if (cmd == "MODE")
                handleMode(fd, iss);
            if (cmd == "QUIT")
                handleQuit(fd);
            if (cmd == "NAMES")
                handleNames(fd, iss);
            if (cmd == "PART")
                handlePart(fd, iss);
            if (cmd == "LIST")
                handleList(fd);
            if (cmd == "INVITE")
                handleInvite(fd, iss);
            if (cmd == "WHO")
                handleWho(fd, iss);
            if (cmd == "TOPIC")
                handleTopic(fd, iss);
        }
        std::cout << "Client leaved -> " << _client[fd].getName() << std::endl;
        close(fd);
        _pfds.erase(_pfds.begin() + i);
        _client.erase(fd);
        return;
    }
    buffer[bytes] = '\0';
    _client[fd].addBuffer(buffer);
    while (_client[fd].extractLine(line))
    {
        std::istringstream iss(line);
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
        if (cmd == "KICK")
            handleKick(fd, iss);
        if (cmd == "MODE")
            handleMode(fd, iss);
        if (cmd == "QUIT")
            handleQuit(fd);
        if (cmd == "NAMES")
            handleNames(fd, iss);
        if (cmd == "PART")
            handlePart(fd, iss);
        if (cmd == "LIST")
            handleList(fd);
        if (cmd == "INVITE")
            handleInvite(fd, iss);
        if (cmd == "WHO")
            handleWho(fd, iss);
        if (cmd == "TOPIC")
            handleTopic(fd, iss);
    }
}


void    serv::run()
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

void    serv::handleNick(int fd, std::istringstream &iss)
{
    std::string newName;
    iss >> newName;

    if (newName.empty())
        return(sendReply(fd, "431", "No name given"));
    if (alreadyUsedName(newName))
        return(sendReply(fd, "433", "Name is already use"));

    _client[fd].setName(newName);
    _client[fd].setRegister();
    if (_client[fd].isRegistered())
        sendReply(fd, "001", "Welcome to the IRC server");
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

void    serv::sendToClient(int fd, const std::string &msg)
{
    send(fd, msg.c_str(), msg.size(), 0);
}

void    serv::handlePass(int fd, std::istringstream &iss)
{
    if (_client[fd].isAuthenticated())
        return sendReply(fd, "462", "You may not reregister");

    std::string password;
    iss >> password;

    if (password.empty())
        return sendReply(fd, "461", "PASS :Not enough parameters");
    if (password != _Password)
        return sendReply(fd, "464", "Password incorrect");
    else 
        _client[fd].authenticate();
    _client[fd].setRegister();
    if (_client[fd].isRegistered())
         sendReply(fd, "001", "Welcome to the IRC server");
}

void    serv::handleUser(int fd, std::istringstream &iss)
{
    if (_client[fd].isRegistered())
        return sendReply(fd, "462", "You may not reregister");
    std::string username;
    std::string mode;
    std::string unused;
    iss >> username;
    iss >> mode;
    iss >> unused;

    if (username.empty())
        return sendReply(fd, "461", "USER :Not enough parameters");
    _client[fd].setUser(username);
    _client[fd].setRegister();
    if (_client[fd].isRegistered())
        sendReply(fd, "001", "Welcome to the IRC server");
}

void    serv::handlePrivmsg(int fd, std::string &line)
{
    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");

    size_t pos = line.find(" :");
    if (pos == std::string::npos)
        return sendReply(fd, "412", "No text to send");

    std::string before = line.substr(0, pos);
    std::string text = line.substr(pos + 2);
    std::istringstream iss(before);
    std::string cmd;
    std::string target;

    iss >> cmd >> target;
    if (target.empty())
        return sendReply(fd, "411", "No recipient given (PRIVMSG)");

    if (target[0] == '#') 
    {
        std::string out = ":" + _client[fd].getName() + "!" + _client[fd].getUser() +
                          "@localhost PRIVMSG " + target + " :" + text + "\r\n";
        sendToChannel(target, out);
    }
    else
    {
        sendToUser(fd, target + " " + text);
    }
}

void    serv::sendToChannel(const std::string &chanName, const std::string &msg)
{
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end()) return;

    std::set<int> &clients = it->second.getClient();
    for (std::set<int>::iterator its = clients.begin(); its != clients.end(); ++its)
        send(*its, msg.c_str(), msg.size(), 0);
}


void    serv::sendToUser(int fd, std::string msg)
{
    std::string target = findUserNick(msg);
    msg.erase(0, target.size() + 1);
    int targetFd = findUserFd(target);

    if (targetFd == -1)
    {
        sendReply(fd, "401", target + " :No such nick");
        return;
    }
    std::string out = ":" + _client[fd].getName() + "!" + _client[fd].getUser() +
                      "@localhost PRIVMSG " + target + " :" + msg + "\r\n";
    send(targetFd, out.c_str(), out.size(), 0);
}

std::string serv::findUserNick(std::string msg)
{
    std::string user(msg);
    int i = 0;
    while (msg[i] != ' ')
        i++;
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

void    serv::handleJoin(int fd, std::istringstream &iss)
{
    std::string chanName;
    std::string key;
    iss >> chanName;
    iss >> key;

    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (chanName.empty() || chanName[0] != '#')
        return sendReply(fd, "476", "Invalid channel name");
    if (_channels.find(chanName) == _channels.end())
        _channels[chanName] = channel(chanName);
    channel &chan = _channels[chanName];
    if (chan.hasClient(fd))
        return sendReply(fd, "443", chanName + " :You're already on that channel");
    if (chan.isInviteOnly() && !chan.isInvited(fd) && !chan.isOp(fd))
        return sendReply(fd, "473", chanName + " :Cannot join channel (+i)");
    if (!chan.getKey().empty() && chan.getKey() != key)
        return sendReply(fd, "475", chanName + " :Cannot join channel (+k)");
    if (chan.getLimit() != -1 && (int)chan.getClient().size() >= chan.getLimit())
        return sendReply(fd, "471", chanName + " :Cannot join channel (+l)");
   chan.addClient(fd);
    if (chan.getClient().size() == 1)
        chan.addOp(fd);
    std::string out = ":" + _client[fd].getName() + " JOIN " + chanName + "\r\n";
    sendToClient(fd, out);
    sendJoinToChannel(out, chanName, fd);

}

void    serv::sendJoinToChannel(std::string msg, std::string chanName, int exceptFd)
{
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end()) return;

    std::set<int> &clients = it->second.getClient();
    for (std::set<int>::iterator its = clients.begin(); its != clients.end(); ++its)
    {
        if (*its != exceptFd)
            send(*its, msg.c_str(), msg.size(), 0);
    }
}


void    serv::handlePart(int fd, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (!_client[fd].isRegistered())
        return (sendReply(fd, "451", "You have not registered"));
    if (chanName.empty() || chanName[0] != '#')
        return (sendReply(fd, "461", "PART :Not enough parameters"));
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return (sendReply(fd, "403", chanName + " :No such channel"));
    if (!it->second.hasClient(fd))
        return (sendReply(fd, "442", chanName + " :You're not on that channel"));
    std::string msg = ":" + _client[fd].getName() + " PART " + chanName + "\r\n";
    sendToChannelExcept(chanName, msg, fd);
    it->second.removeClient(fd);
    it->second.deOp(fd);
    if (it->second.getClient().empty())
        _channels.erase(it);
    else
        ensureOp(it->second);
}

void    serv::handleKick(int fd, std::istringstream &iss)
{
    std::string chanName;
    std::string targetUser;
    iss >> chanName;
    iss >> targetUser;

    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (chanName.empty() || targetUser.empty())
        return sendReply(fd, "461", "KICK :Not enough parameters");
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendReply(fd, "403", chanName + " :No such channel");
    if (!it->second.isOp(fd))
        return sendReply(fd, "482", chanName + " :You're not channel operator");
    int targetFd = findUserFd(targetUser);
    if (targetFd == -1 || !it->second.hasClient(targetFd))
        return sendReply(fd, "441", targetUser + " " + chanName + " :They aren't on that channel");
    std::string msg = ":" + _client[fd].getName() + " KICK " + chanName + " " + targetUser + " :Kicked by " + _client[fd].getName() + "\r\n";
    sendToClient(targetFd, msg);
    sendToChannelExcept(chanName, msg, targetFd);
    it->second.removeClient(targetFd);
    it->second.deOp(targetFd);

    if (it->second.getClient().empty())
        _channels.erase(it);
    else
        ensureOp(it->second);
}

void    serv::sendToChannelExcept(std::string chanName, std::string msg, int exceptFd)
{
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end()) return;

    std::set<int> &clients = it->second.getClient();
    for (std::set<int>::iterator its = clients.begin(); its != clients.end(); ++its)
    {
        if (*its != exceptFd)
            send(*its, msg.c_str(), msg.size(), 0);
    }
}

void    serv::handleQuit(int fd)
{
    std::string nick = _client[fd].getName();
    std::string user = _client[fd].getUser();
    std::string msg = ":" + nick + "!"+ user +"@localhost QUIT :Client Quit\r\n";
    for (std::map<std::string, channel>::iterator it = _channels.begin();
         it != _channels.end(); )
    {
        if (it->second.hasClient(fd))
        {
            sendToChannelExcept(it->first, msg, fd);
            it->second.removeClient(fd);
            it->second.deOp(fd);
            if (it->second.getClient().empty())
            {
                std::map<std::string, channel>::iterator erase = it++;
                _channels.erase(erase);
                continue;
            }
            else
                ensureOp(it->second);
        }
        ++it;
    }
    close(fd);
    _client.erase(fd);
    for (size_t i = 0; i < _pfds.size(); i++)
    {
        if (_pfds[i].fd == fd)
        {
            _pfds.erase(_pfds.begin() + i);
            break;
        }
    }
}

void    serv::handleNames(int fd, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (chanName.empty())
        return sendToClient(fd, "Usage: NAMES #channel\r\n");

    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendToClient(fd, "No such channel\r\n");
    if (!it->second.hasClient(fd))
        return sendToClient(fd, "You are not in this channel\r\n");

    std::string msg = "Users in " + chanName + " : ";
    std::set<int> &clients = it->second.getClient();

    for (std::set<int>::iterator c = clients.begin(); c != clients.end(); ++c)
    {
        if (it->second.isOp(*c))
            msg += "@";
        msg += _client[*c].getName() + " ";
    }
    sendToClient(fd, msg + "\r\n");
}

void    serv::ensureOp(channel &chan)
{
    std::set<int> &clients = chan.getClient();
    if (clients.empty())
        return;
    for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (chan.isOp(*it))
            return;
    }
    int newOp = *clients.begin();
    chan.addOp(newOp);
    std::string msg = ":" + _client[newOp].getName() + "!"+ _client[newOp].getUser() +"@localhost MODE " + chan.getName() + " +o " + _client[newOp].getName() + "\r\n";
    sendToChannelExcept(chan.getName(), msg, -1);
}


void    serv::sendReply(int fd, const std::string &code, const std::string &msg)
{
    std::string out = ":ircserv " + code + " " + _client[fd].getName()
        + " :" + msg + "\r\n";
    send(fd, out.c_str(), out.size(), 0);
}

void    serv::handleTopic(int fd, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;
    std::string newTopic;
    getline(iss, newTopic);
    if (!newTopic.empty() && newTopic[0] == ' ')
        newTopic.erase(0, 1);
    if (!newTopic.empty() && newTopic[0] == ':')
        newTopic.erase(0, 1);
    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (chanName.empty())
        return sendReply(fd, "461", "TOPIC :Not enough parameters");
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendReply(fd, "403", chanName + " :No such channel");
    if (!it->second.hasClient(fd))
        return sendReply(fd, "442", chanName + " :You're not on that channel");
    if (!newTopic.empty())
    {
        if (!it->second.isOp(fd))
            return sendReply(fd, "482", chanName + " :You're not channel operator");
        it->second.setTopic(newTopic);
        std::string msg = ":" + _client[fd].getName() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
        sendToChannel(chanName, msg);
    }
    else
    {
        std::string topic = it->second.getTopic();
        sendReply(fd, "332", chanName + " :" + topic);
    }
}

void    serv::handleList(int fd)
{
    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");

    for (std::map<std::string, channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        std::string msg = "322 " + _client[fd].getName() + " " + it->first + " :" + it->second.getTopic() + "\r\n";
        sendToClient(fd, msg);
    }
    sendReply(fd, "323", "End of LIST");
}

void    serv::handleInvite(int fd, std::istringstream &iss)
{
    std::string targetNick;
    std::string chanName;
    iss >> targetNick >> chanName;

    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (targetNick.empty() || chanName.empty())
        return sendReply(fd, "461", "INVITE :Not enough parameters");
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendReply(fd, "403", chanName + " :No such channel");
    if (!it->second.hasClient(fd))
        return sendReply(fd, "442", chanName + " :You're not on that channel");
    int targetFd = findUserFd(targetNick);
    if (targetFd == -1)
        return sendReply(fd, "401", targetNick + " :No such nick");
    it->second.addInvite(targetFd);
    std::string msg = ":" + _client[fd].getName() + " INVITE " + targetNick + " " + chanName + "\r\n";
    sendToClient(targetFd, msg);
    sendReply(fd, "341", targetNick + " " + chanName);
}

void    serv::handleWho(int fd, std::istringstream &iss)
{
    std::string chanName;
    iss >> chanName;

    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (chanName.empty())
        return sendReply(fd, "461", "WHO :Not enough parameters");
    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendReply(fd, "403", chanName + " :No such channel");
    std::set<int> &clients = it->second.getClient();
    for (std::set<int>::iterator c = clients.begin(); c != clients.end(); ++c)
    {
        std::string flags = it->second.isOp(*c) ? "H@" : "H";
        std::string msg = "352 " + _client[fd].getName() + " " + chanName + " "
                        + _client[*c].getUser() + " " + _client[*c].getName()
                        + " :0 " + flags + " " + _client[*c].getName() + "\r\n";
        sendToClient(fd, msg);
    }
    sendReply(fd, "315", chanName + " :End of WHO list");
}

void    serv::handleMode(int fd, std::istringstream &iss)
{
    std::string chanName;
    std::string mode;
    std::string param;
    iss >> chanName;
    iss >> mode;
    iss >> param;

    if (!_client[fd].isRegistered())
        return sendReply(fd, "451", "You have not registered");
    if (chanName.empty() || mode.empty())
        return sendReply(fd, "461", "MODE :Not enough parameters");

    std::map<std::string, channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
        return sendReply(fd, "403", chanName + " :No such channel");

    channel &chan = it->second;
    if (!chan.isOp(fd))
        return sendReply(fd, "482", chanName + " :You're not channel operator");

    std::string msg;
    if (mode == "+o" || mode == "-o") {
        int targetFd = findUserFd(param);
        if (targetFd == -1 || !chan.hasClient(targetFd))
            return sendReply(fd, "441", param + " " + chanName + " :They aren't on that channel");
        if (mode == "+o")
            chan.addOp(targetFd);
        else
            chan.deOp(targetFd);
    }
    else if (mode == "+i")
        chan.setInviteOnly(true);
    else if (mode == "-i")
        chan.setInviteOnly(false);
    else if (mode == "+t")
        chan.setTopicOnly(true);
    else if (mode == "-t")
        chan.setTopicOnly(false);
    else if (mode == "+k")
    {
        if (param.empty())
            return sendReply(fd, "461", "MODE :Not enough parameters");
        chan.setKey(param);
    }
    else if (mode == "-k")
        chan.setKey("");
    else if (mode == "+l")
    {
        if (param.empty())
            return sendReply(fd, "461", "MODE :Not enough parameters");
        chan.setLimit(atoi(param.c_str()));
    }
    else if (mode == "-l")
        chan.setLimit(0);
    else
        return sendReply(fd, "472", mode + " :Unknown mode");
    msg = ":" + _client[fd].getName() + " MODE " + chanName + " " + mode;
    if (!param.empty()) msg += " " + param;
    msg += "\r\n";
    sendToChannelExcept(chanName, msg, -1);
}

