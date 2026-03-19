#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"
#include <csignal>

serv *g_server = NULL;

void handle_sigint(int sig)
{
    (void)sig;
    if (g_server)
        g_server->stop();
}

int main(int argc, char **argv)
{
    try
    {
        if (argc != 3)
            throw std::logic_error("Error : ./ircserv <port> <password>");
        int port = atoi(argv[1]);
        if (port <= 0 || port > 65535)
            throw std::logic_error("Error : incorrect port");
        int i = 0;
        while (argv[2][i])
        {
            if (!isalnum(argv[2][i]))
                throw std::logic_error("Error : password must contain alphanumeric charatere");
            i++;
        }
        if (strlen(argv[2]) == 0)
            throw std::logic_error("Error : password must contain alphanumeric charatere");
        std::string password = argv[2];
        serv server(port, password);
        g_server = &server;
        signal(SIGINT, handle_sigint);
        server.run();
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}