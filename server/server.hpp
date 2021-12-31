#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "tcp_server.hpp"
#include "epoll.hpp"
#include "threadpool.hpp"
#include "log.hpp"
#include "json/json.h"
#include "db.hpp"
using namespace std;

#define CGIUPLOAD "./upload"    //外部处理程序(文件上传)
#define BUFSIZE 10240   //文件接受缓冲区

//客户端请求类型
enum ReqType
{
    USER_LOGIN = 0,
    USER_REGISTER,
    FILE_CHMOD,
    FILE_LIST,
    FILE_DOWNLOAD,
    FILE_UPLOAD
};

//用户注册信息
struct UserRegister 
{
    string username;
    string password;
    string mail;
};

//用户登录信息
struct UserLogin 
{
    int userid;
    string password;
};

//更改文件权限
struct FileChmod 
{
    int userid;
    int fileChmodValue;
    string filename;
};

//定时文件列表请求
struct FileList
{
    int userid;
};

struct FileUpload
{
    int userid;
    string filename;
    int fileChmodValue;
    string filebody;
};

class BaseServer
{
public:
    bool ServiceStart()
    {
        //初始化 TcpServer
        int ret = _listenSock.InitTcpServer();
        if (ret == false)
        {
            LOG(FATAL, "Tcpsocket error!") << std::endl;
            return false;
        }

        //初始化 Epoll
        ret = _ep.EpollInit();
        if (ret == false)
        {
            LOG(FATAL, "EpollInit error!") << std::endl;
            return false;
        }

        // 获取监听套接字描述符
        int fd = _listenSock.GetSockfd();
        _ep.EpollAdd(fd);
        while (1)
        {
            //epoll 监控
            vector<TcpServer> listsock;
            ret = _ep.EpollWait(listsock);
            if (ret == false)
            {
                continue;
            }

            //分类处理就绪事件
            for (auto &sock : listsock)
            {
                if (sock.GetSockfd() == _listenSock.GetSockfd())
                {
                    //监听套接字
                    TcpServer clisock;
                    ret = sock.Accept(clisock);
                    if (ret == false)
                    {
                        continue;
                    }

                    _ep.EpollAdd(clisock.GetSockfd());

                    //TODO...
                    //自定义客户端socket对SIGPIPE信号的处理
                }
                else
                {
                    //客户端套接字
                    ThreadTask tt;
                    tt.SetTask(sock.GetSockfd(), ThreadPoolHandle);
                    _thpool.PushTask(tt);
                    _ep.EpollDel(sock.GetSockfd());
                }
            }
        }
        return true;
    }

    //定义线程处理函数
    static bool ThreadPoolHandle(int data) {
        LOG(INFO, "Start processing client requests") << endl;
        //初始化mysql
        TableInfo tableInfo;
        int ret = tableInfo.InitTableInfo();
        if (ret == false)
        {
            return false;
        }
        //设置有请求到来的客户端socket
        TcpServer clientSocketfd;
        clientSocketfd.SetSockfd(data);
        //接收客户端数据
        string reqMsg;
        ret = clientSocketfd.RecvNonBlock(&reqMsg);
        if(ret == false) 
        {
            LOG(ERROR, "recv reqMsg error") << endl;
            return false;
        }
        //解析客户端请求类型，json反序列化
        int reqType = -1;
        Json::Reader read;
        Json::Value reqJson;
        read.parse(reqMsg, reqJson, false);
        reqType = reqJson["reqType"].asInt();
        //响应客户端的请求，json序列化
        string status = "FAILED";
        Json::Value resJson;

        switch (reqType)
        {
        case USER_LOGIN:{
            UserLogin userLogin;
            userLogin.userid = reqJson["userid"].asInt();
            userLogin.password = reqJson["password"].asString();
            string logInfo = "reqType:USER_LOGIN, userid:"  + reqJson["userid"].asString() 
                             + ", password:" + reqJson["password"].asString();
            LOG(INFO, logInfo) << endl;
            //处理用户登录请求
            doUserLoginToDB(userLogin, &status, tableInfo);
            resJson["status"] = status;
            break;
        }
        case USER_REGISTER:{
            UserRegister userRegister;
            userRegister.username = reqJson["username"].asString();
            userRegister.password = reqJson["password"].asString();
            userRegister.mail = reqJson["mail"].asString();
            string logInfo = "reqType:USER_REGISTER, username:" + reqJson["username"].asString()
                             + ", password:" + reqJson["password"].asString()
                             + ", mail:" + reqJson["mail"].asString();
            LOG(INFO, logInfo) << endl;
            //处理用户注册请求
            int userId;
            doUserRegisterToDB(userRegister, &userId, &status, tableInfo);
            resJson["status"] = status;
            resJson["userid"] = userId;
            break;
        }
        case FILE_CHMOD:{
            FileChmod fileChmod;
            fileChmod.userid = reqJson["userid"].asInt();
            fileChmod.fileChmodValue = reqJson["filechmod"].asInt();
            fileChmod.filename = reqJson["filename"].asString();
            string logInfo = "reqType:FILE_CHMOD, userid:"  + reqJson["userid"].asString() 
                             + ", filechmod:" + reqJson["filechmod"].asString()
                             + ", filename:" + reqJson["filename"].asString();
            LOG(INFO, logInfo) << endl;
            //处理用户更改文件权限请求
            doFileChmodToDB(fileChmod, &status, tableInfo);
            resJson["status"] = status;
            break;
        }
        case FILE_LIST:{
            FileList fileList;
            fileList.userid = reqJson["userid"].asInt();            
            string logInfo = "reqType:FILE_LIST, userid:"  + reqJson["userid"].asString();
            LOG(INFO, logInfo) << endl;
            //处理定时文件列表请求
            vector<string> fileVec;
            doFileListToDB(fileList, &status, &fileVec, tableInfo);
            Json::Value fileListJson;
            int fileAmount  = fileVec.size();
            for(int i = 0; i < fileAmount; i++) {
                fileListJson[i] = fileVec[i];
            }
            //文件列表数量、文件列表
            resJson["status"] = status;
            resJson["fileAmount"] = fileAmount;
            resJson["fileList"] = fileListJson;
            break;
        }
        case FILE_DOWNLOAD:{
            //处理文件下载请求
            break;
        }
        case FILE_UPLOAD:{
            FileUpload fileUpload;
            fileUpload.userid = reqJson["userid"].asInt();
            fileUpload.fileChmodValue = reqJson["filechmod"].asInt();   
            fileUpload.filename = reqJson["filename"].asString();
            fileUpload.filebody = reqJson["filebody"].asString();
            string logInfo = "reqType:FILE_UPLOAD, userid:"  + reqJson["userid"].asString() 
                             + ", filename:" + reqJson["filename"].asString()
                             + ", filechmod:" + reqJson["filechmod"].asString()
                             + ", filebody:" + reqJson["filebody"].asString();
            LOG(INFO, logInfo) << endl;                 
            //处理文件上传请求
            doFileUploadToDB(fileUpload, &status, tableInfo);
            resJson["status"] = status;
            break;
        }
        default:
            //未知的请求类型
            LOG(ERROR, "unknown request type") << endl;
            resJson["status"] = status;
            break;
        }

        //服务器发送数据给客户端
        Json::FastWriter writer;
        string resMsg = writer.write(resJson);
        ret = clientSocketfd.SendNonBlock(resMsg);
        if(ret < 0)
        {
            LOG(ERROR, "recv msg error") << endl;
            return false;
        }
        LOG(INFO, "Finish processing client requests") << endl;
        return true;
    }

private:
    static void doUserLoginToDB(UserLogin& userLogin, string* status, TableInfo& tableInfo)
    {
        //判断用户ID和密码是否正确
        char inquireIDPasswordStr[1024];
        snprintf(inquireIDPasswordStr, sizeof(inquireIDPasswordStr), "select id from user_register_info where userid = %d and password = '%s';", 
                 userLogin.userid, userLogin.password.c_str());
        int ret = tableInfo.InquireUserInfoExist(inquireIDPasswordStr);
        if(ret == false) {
            //未查询到
            *status = "FAILED";
            LOG(ERROR, "NO Query the user information") << endl;
            return;
        }
        //查询到该用户信息
        *status = "SUCCESS";
        LOG(INFO, "Query the user information") << endl;
    }

    static void doUserRegisterToDB(UserRegister& userRegister, int* userId, string* status, TableInfo& tableInfo)
    {
        //TODO...如果生成的随机数一直重复就会死循环(概率几乎0)
        int ret;
        int randNumber;
        while (1) {
            //添加随机数种子 作用：利用系统时间生成随机数 防止每次随机数都一样
            srand((unsigned int)time(NULL));
            //生成一个六位的随机数作为用户ID
            randNumber = rand() % 1000000;
            //判断生成的用户ID是否重复，若重复则重新生成
            char inquireIDStr[1024];
            snprintf(inquireIDStr, sizeof(inquireIDStr), "select id from user_register_info where userid = %d;", randNumber);
            ret = tableInfo.InquireUserInfoExist(inquireIDStr);
            if(ret == false) {
                LOG(ERROR, "userid generated failed") << endl;
                break;
            }
        }
        //获取当前系统时间戳
        time_t timestamp;
        //向mysql中插入用户注册信息
        char sqlStr[1024];
        snprintf(sqlStr, sizeof(sqlStr), "insert into user_register_info(userid, name, password, mail, register_time) values(%d, '%s', '%s', '%s', %d);", 
                 randNumber, userRegister.username.c_str(), userRegister.password.c_str(), userRegister.mail.c_str(), time(&timestamp));
        ret = tableInfo.DoMysqlQuery(sqlStr);
        if(ret == false) {
            //插入失败
            LOG(ERROR, "Failed to insert user registration information") << endl;
            *status = "FAILED";
            *userId = -1;
            return;
        }
        //插入成功
        *status = "SUCCESS";
        *userId = randNumber;
        LOG(INFO, "Success to insert user registration information") << endl;
    }

    static void doFileChmodToDB(FileChmod& fileChmod, string* status, TableInfo& tableInfo)
    {
        char sqlStr[1024];
        snprintf(sqlStr, sizeof(sqlStr), "update file_info set auth = %d where userid = %d and filename = '%s';", 
                 fileChmod.fileChmodValue, fileChmod.userid, fileChmod.filename.c_str());
        int ret = tableInfo.DoMysqlQuery(sqlStr);
        if(ret == false) {
            //修改文件权限失败
            *status = "FAILED";
            LOG(ERROR, "Failed to modify file permissions") << endl;
            return;
        } 
        //修改文件权限成功
        *status = "SUCCESS";   
        LOG(INFO, "Success to modify file permissions") << endl;    
    }

    static void doFileListToDB(FileList& fileList, string* status, vector<string>* fileVec, TableInfo& tableInfo)
    {
        //查询用户自己的文件或者其它用户共享的文件
        char sqlStr[1024];
        snprintf(sqlStr, sizeof(sqlStr), "select userid, filename from file_info where userid = %d or auth = 1;", fileList.userid);
        int ret = tableInfo.InquireFile(sqlStr, fileVec);
        if(ret == false) {
            //文件列表查询失败
            *status = "FAILED";
            LOG(ERROR, "File list query failed") << endl;
            return;
        }
        //文件列表查询成功
        *status = "SUCCESS"; 
        LOG(INFO, "File list query success") << endl;
    }

    static void doFileUploadToDB(FileUpload& fileUpload, string* status, TableInfo& tableInfo)
    {
        //检验该文件是否已经存在
        char inquireIDStr[1024];
        snprintf(inquireIDStr, sizeof(inquireIDStr), "select id from file_info where userid = %d and filename = '%s';", 
                 fileUpload.userid, fileUpload.filename.c_str());
        int ret = tableInfo.InquireUserInfoExist(inquireIDStr);
        if(ret == true) {
            LOG(ERROR, "The file already exists") << endl;
            *status = "FAILED";
            return;
        }
        //保存文件内容到磁盘中
        doFileUploadToDisk(fileUpload, status);
        if(*status == "FAILED") {
            LOG(ERROR, "Failed to upload file to disk") << endl;
            return;
        }
        //获取当前系统时间戳
        time_t timestamp;
        //将用户ID和文件名插入DB中记录
        char sqlStr[1024]; 
        snprintf(sqlStr, sizeof(sqlStr), "insert into file_info(userid, filename, auth, upload_time) values(%d, '%s', %d, %d);", 
                 fileUpload.userid, fileUpload.filename.c_str(), fileUpload.fileChmodValue, time(&timestamp));
        ret = tableInfo.DoMysqlQuery(sqlStr);
        if(ret == false) {
            LOG(ERROR, "File name insertion failed during upload") << endl;
            *status = "FAILED";
            return;
        }
        *status = "SUCCESS"; 
        LOG(INFO, "The upload information is inserted into the DB successfully") << endl;
    }

private:
    static void doFileUploadToDisk(FileUpload& fileUpload, string* status)
    {
        //父子进程管道通信
        //父进程向子进程传输请求信息
        int pipefdin[2] = {-1};
        //子进程向父进程传输响应信息
        int pipefdout[2] = {-1};
        int ret = pipe(pipefdin);
        if(ret < 0) {
            cerr << "server.hpp/doFileUploadToDisk(): pipe create error!! " << endl;
            *status = "FAILED";
            return;
        }
        ret = pipe(pipefdout);
        if(ret < 0) {
            cerr << "server.hpp/doFileUploadToDisk(): pipe create error!! " << endl;
            *status = "FAILED";
            return;
        }

        //创建子进程 
        pid_t pid = fork();
        if(pid < 0) {
            cerr << "server.hpp/doFileUploadToDisk(): fork error! " << endl;
            *status = "FAILED";
            return;
        } else if(pid == 0) {
            //设置环境变量TODO
            //添加文件名
            setenv("FILENAME", fileUpload.filename.c_str(), 1);
            //添加用户ID
            string userIDStr = to_string(fileUpload.userid);
            setenv("USERID", userIDStr.c_str(), 1);
            //添加文件内容长度
            uint64_t bodyLength = fileUpload.filebody.size();
            string bodyLengthStr = to_string(bodyLength);
            setenv("BODYLENGTH", bodyLengthStr.c_str(), 1);

            //子进程关闭管道不用的一端
            close(pipefdin[1]);
            close(pipefdout[0]);

            //由于要程序替换，故重定向文件描述符进行通信
            //将管道的读取端与标准输出关联起来
            dup2(pipefdin[0], 1);
            //将管道的写入端与标准输入关联起来                                 
            dup2(pipefdout[1], 0);

            //子进程程序替换：main函数参数来接受传递的参数
            execlp(CGIUPLOAD, CGIUPLOAD, NULL);

            //关闭重定向后的文件描述符
            close(pipefdin[0]);
            close(pipefdout[1]);
        }

        //父进程关闭管道不用的一端
        close(pipefdin[0]);
        close(pipefdout[1]);

        //忽略SIGPIPE信号
        signal(SIGPIPE, SIG_IGN);

        //向子进程发送正文数据
        size_t tmpLen = 0;
        while(tmpLen < fileUpload.filebody.size())
        {
            int ret = write(pipefdin[1], &fileUpload.filebody[0] + tmpLen, fileUpload.filebody.size() - tmpLen);
            if(ret < 0)
            {
                cerr << "server.hpp/doFileUploadToDisk(): write error" << endl;
                *status = "FAILED";
                return;
            }
            tmpLen += ret;
        }

        //接受子进程发送的响应信息     
        stringstream resChildProcess;
        char buf[BUFSIZE];
        while(1)
        {
            memset(buf, 0, BUFSIZE);
            int ret = read(pipefdout[0], buf, BUFSIZE);
            if(ret < 0)
            {
                cerr << "server.hpp/doFileUploadToDisk(): read error" << endl;
                *status = "FAILED";
                return;
            }

            if(ret == 0) {
                break;
            }
            string tmp;
            tmp.assign(buf, ret);
            resChildProcess << tmp;
        }
        close(pipefdin[1]);
        close(pipefdout[0]);
        *status = resChildProcess.str();
        string logInfo = "File upload completed, resChildProcess status:" + *status;
        LOG(INFO, logInfo) << endl; 
    }

private:
    //监听套接字
    TcpServer _listenSock;
    //epoll模型
    Epoll _ep;
    //线程池
    ThreadPool _thpool;
};
