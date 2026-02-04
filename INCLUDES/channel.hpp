#pragma once

#include <string>
#include <set>

class channel
{
    private :
        std::string         _name;
        std::set<int>       _clients;
        std::set<int>       _operators;

        public :
        channel();
        channel(const std::string &name);
        ~channel();

        const std::string   &getName();

        void                removeClient(int fd);
        bool                hasClient(int fd);
        std::set<int>       &getClient();
        bool                isClient(int fd);
        bool                isOp(int fd);
        void                addClient(int fd);
        void                addOp(int fd);
        void                deOp(int fd);
};