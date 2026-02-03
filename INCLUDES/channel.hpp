#pragma once

#include <string>
#include <set>

class channel
{
    private :
        std::string _name;
        std::set<int> _clients;
    public :
        channel();
        channel(const std::string &name);
        ~channel();

        const std::string  &getName();

        void                addClient(int fd);
        void                removeClient(int fd);
        bool                hasClient(int fd);

        std::set<int>       &getClient();

    };