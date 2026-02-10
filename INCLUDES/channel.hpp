#pragma once

#include <string>
#include <set>

class channel
{
    private :
        std::string         _name;
        std::set<int>       _clients;
        std::set<int>       _operators;
        std::string         _topic;
        std::set<int>       _invites;
        bool                _inviteOnly;
        bool                _topicOnly;
        std::string         _key;
        int                 _limit;   

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
        void                setTopic(const std::string &topic);
        std::string         getTopic() const;
        void                addInvite(int fd);
        bool                isInvited(int fd) const;
        void                removeInvite(int fd);
        void                setInviteOnly(bool b);
        bool                isInviteOnly() const;
        void                setTopicOnly(bool b);
        bool                isTopicOnly() const;
        void                setKey(const std::string &key);
        std::string         getKey() const;
        void                setLimit(int n);
        int                 getLimit() const;

};