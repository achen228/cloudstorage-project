#pragma once 
#include <sys/epoll.h>
#include <vector>
#include "tcp_server.hpp"

#define EVENT_AMOUNT 1024 //最大监控事件数量
#define EVENT_ARRAY_SIZE 20 //事件数组大小

class Epoll
{
public:
    Epoll()
    {
        _epollfd = -1;
    }

    ~Epoll()
    {}

    bool EpollInit()
    {
        _epollfd = epoll_create(EVENT_AMOUNT);
        if(_epollfd < 0)
        {
            LOG(FATAL, "create error") << std::endl;
            return false;
        }
        return true;
    }

    bool EpollAdd(int fd)
    {
        struct epoll_event ep;
        //开启ET模式：一次性获取完整数据
        ep.events = EPOLLIN | EPOLLET;
        ep.data.fd = fd;
        int ret = epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ep);
        if(ret < 0)
        {
            LOG(FATAL, "ctl error") << std::endl;
            return false;
        }
        return true;
    }

    bool EpollDel(int fd)
    {
        int ret = epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
        if(ret < 0)
        {
            LOG(FATAL, "ctl error") << std::endl;
            return false;
        }
        return true;
    }

    bool EpollWait(std::vector<TcpServer>& socklist)
    {
        struct epoll_event fd_arr[EVENT_ARRAY_SIZE];
        int ret = epoll_wait(_epollfd, fd_arr, sizeof(fd_arr) / sizeof(fd_arr[0]), -1);
        if(ret < 0)
        {
            LOG(FATAL, "epollwait error") << std::endl;
            return false;
        }
        else if(ret == 0)
        {
            LOG(FATAL, "Waiting timeout") << std::endl;
            return false;
        }

        //防止数组越界
        if(ret > int(sizeof(fd_arr) / sizeof(fd_arr[0])))
        {
            ret = sizeof(fd_arr) / sizeof(fd_arr[0]);
        }

        for(int i = 0; i < ret; i++)
        {
            TcpServer ts;
            ts.SetSockfd(fd_arr[i].data.fd);
            socklist.push_back(ts);
        }
        return true;
    }
private:
    int _epollfd;
};
