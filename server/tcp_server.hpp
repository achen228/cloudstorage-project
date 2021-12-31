#pragma once
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "log.hpp"

#define PORT 3389  //端口
#define BACKLOG 20 //已完成连接队列的大小, 即同一时刻，服务端最大的并发连接数

class TcpServer
{
public:
    TcpServer()
    {
        sock_ = -1;
    }

    ~TcpServer()
    {}

    bool InitTcpServer()
    {
        if(!CreateSocket() || !Bind() || !Listen())
        {
            return false;
        }
        return true;
    }

    bool CreateSocket()
    {
        sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sock_ < 0)
        {
            LOG(FATAL, "Create socket failed") << std::endl;
            return false;
        }
        return true;
    }

    bool Bind()
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        int ret = bind(sock_, (struct sockaddr*)&addr, sizeof(addr));
        if(ret < 0)
        {
            LOG(FATAL, "Bind failed") << std::endl;
            return false;
        }
        return true;
    }

    bool Listen(int backlog = BACKLOG)
    {
        int ret = listen(sock_, backlog);
        if(ret < 0)
        {
            LOG(FATAL, "Listen failed") << std::endl;
            return false;
        }
        return true;
    }

    bool Accept(TcpServer& ts, struct sockaddr_in* addr = NULL)
    {
        struct sockaddr_in cliaddr;
        socklen_t cliaddr_len = sizeof(cliaddr);
        int newsock = accept(sock_, (struct sockaddr*)&cliaddr, &cliaddr_len);
        if(newsock < 0)
        {
            LOG(FATAL, "Accept failed") << std::endl;
            return false;
        }
        ts.sock_ = newsock;

        if(addr != NULL)
        {
            memcpy(addr, &cliaddr, cliaddr_len);
        }
        return true;
    }

    //短小数据采用阻塞直接发送
    bool Send(std::string& buf)
    {
        Setblock();
        ssize_t send_size = send(sock_, buf.c_str(), buf.size(), 0);
        if(send_size < 0)
        {
            LOG(FATAL, "Send failed") << std::endl;
            return false;
        }
        return true;
    }

    //大量数据采用非阻塞循环发送
    bool SendNonBlock(std::string& buf)
    {
        SetNonblock();
        ssize_t pos = 0;
        ssize_t last_size = buf.size();

        while(1)
        {
            ssize_t send_size = send(sock_, buf.data() + pos, last_size, 0);
            if(send_size < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    continue;
                }
                LOG(FATAL, "SendNonBlock failed") << std::endl;
                return false;
            }

            //更新发送位置和剩余的数据
            pos += send_size;
            last_size -= send_size;

            if(last_size <= 0)
            {
                break;
            }
        }
        return true;
    }

    //短小数据采用阻塞直接接收
    bool Recv(std::string* buf)
    {
        Setblock();
        char tmpBuf[1024] = {0};
        ssize_t recv_size = recv(sock_, tmpBuf, sizeof(tmpBuf) - 1, 0);
        if(recv_size < 0)
        {
            LOG(FATAL, "RecvNonBlock failed") << std::endl;
            return false;
        }
        else if(recv_size == 0)
        {
            LOG(FATAL, "RecvNonBlock: Peer close connect") << std::endl;
            return false;
        }
        (*buf).assign(tmpBuf, recv_size);
        return true;
    }

    //大量数据采用非阻塞循环接收
    bool RecvNonBlock(std::string* buf)
    {
        SetNonblock();
        while(1)
        {
            char tmp[3] = {0};
            ssize_t recv_size = recv(sock_, tmp, sizeof(tmp) - 1, 0);
            if(recv_size < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    //缓冲区当中没有数据了
                    break;
                }
                LOG(FATAL, "RecvNonBlock failed") << std::endl;
                return false;
            }
            else if(recv_size == 0)
            {
                LOG(FATAL, "RecvNonBlock: Peer close connect") << std::endl;
                return false;
            }

            *buf += tmp;
            if(recv_size < (ssize_t)sizeof(tmp) - 1)
            {
                break;
            }
        }
        return true;
    }

    void Close()
    {
        close(sock_);
        sock_ = -1;
    }

    //设置Socket文件描述符
    void SetSockfd(const int sock)
    {
        sock_ = sock;
    }

    //获取Socket文件描述符
    int GetSockfd()
    {
        return sock_;
    }

private:
    //设置socket非阻塞状态
    void SetNonblock()
    {
        int flags = fcntl(sock_, F_GETFL);
        fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
    }

    //设置socket阻塞状态
    void Setblock()
    {
        int flags = fcntl(sock_, F_GETFL);
        fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
    }

private:
    int sock_;
};
