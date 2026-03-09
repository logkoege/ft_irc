#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"

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
            if(!isalnum(argv[2][i]))
            {
                throw std::logic_error("Error : password must contain alphanumeric charatere");
                return 1;
            }    
            i++;
        }
        if (strlen(argv[2]) == 0)
        {
            throw std::logic_error("Error : password must contain alphanumeric charatere");
            return 1;
        }
        std::string password = argv[2];
        serv server(port, password);
        server.run();
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}