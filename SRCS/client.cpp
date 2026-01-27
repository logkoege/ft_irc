#include "../INCLUDES/serv.hpp"
#include "../INCLUDES/client.hpp"

client::client() : _fd(-1), name("clanker_"){}
client::~client(){}
client::client(int fd) : _fd(fd)
{
    name = "levai_";

    if (fd == 0)
        name += '0';
    while (fd > 0)
    {
        name += char(fd % 10 + '0');
        fd = fd / 10;
    }
}
