#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

#define BUFSIZE 10240   //文件接受缓冲区
#define CURRENTFILEROOT "./userfile/"  //将文件存储在当前路径下
#define FILEROOT "/home/burnzhang/code/cpp_code/cloudstorage/server/userfile/" //根据用户ID创建目录

uint64_t stringtouint64(string strnum)
{
    uint64_t num = 0;
    uint64_t lastnum = 0;
    for(int i = strnum.size() - 1, j = 0; i >= 0; --i, ++j)
    {
        num = (strnum[i] - '0') * pow(10, j) + lastnum; 
        lastnum = num;
    }
    return num;
}

//解析环境变量 
bool EnvParse(vector<string>& envlist, string* filename, string* bodyLength, string* userid)
{
    //寻找所需环境变量
    string filenameENV;
    string bodyLengthENV;
    string useridENV;
    for(auto& e : envlist) {
        //FILENAME
        if(e.find("FILENAME") != string::npos) {
            filenameENV = e;
        }
        //BODYLENGTH
        if(e.find("BODYLENGTH") != string::npos) {
            bodyLengthENV = e;
        }
        //USERID
        if(e.find("USERID") != string::npos) {
            useridENV = e;
        }        
    }
    //解析各个字段
    //解析filename
    string filenameStr = "=";
    size_t filenamePos = filenameENV.find(filenameStr, 0);
    if(filenamePos == string::npos)
    {
        cerr << "upload.cpp/EnvParse(): Cannot find the = " << endl;
        return false;
    }
    *filename = filenameENV.substr(filenamePos + filenameStr.size());

    //解析bodyLength
    string bodyLengthStr = "=";
    size_t bodyLengthPos = bodyLengthENV.find(bodyLengthStr, 0);
    if(bodyLengthPos == string::npos)
    {
        cerr << "upload.cpp/EnvParse(): Cannot find the = " << endl;
        return false;
    }
    *bodyLength = bodyLengthENV.substr(bodyLengthPos + bodyLengthStr.size());

    //解析userid
    string useridStr = "=";
    size_t useridPos = useridENV.find(useridStr, 0);
    if(useridPos == string::npos)
    {
        cerr << "upload.cpp/EnvParse(): Cannot find the = " << endl;
        return false;
    }  
    *userid = useridENV.substr(useridPos + useridStr.size());

    return true;  
}

//写文件
bool SaveFile(const string& userid, const string& filename, const string& body)
{
    //根据用户ID创建目录
    string userPath = FILEROOT + userid;
    int doesItExist = access(userPath.c_str(), 0);
    if(doesItExist == -1) {
        //该目录不存在则创建,若存在则跳过直接打开
        cerr << "upload.cpp/SaveFile(): Directory does not exist, create!" << endl;
        int status = mkdir(userPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(status == -1) {
            cerr << "upload.cpp/SaveFile(): Failed to create user ID directory!" << endl;
            return false;
        }
    }
    //当前文件路径
    string filepath = CURRENTFILEROOT + userid + "/" + filename;
    //创建文件
    int filefd = open(filepath.c_str(), O_RDWR | O_CREAT | O_APPEND, 0664);
    if(filefd < 0)
    {
        cerr << "upload.cpp/SaveFile(): open error!" << endl;
        return false;
    }

    //设置非阻塞模式
    int flag = fcntl(filefd, F_GETFL);
    fcntl(filefd, F_SETFL, flag | O_NONBLOCK);

    //写入数据
    size_t bodylen = body.size();
    size_t curwrite = 0;
    while(curwrite < bodylen)
    {
        int ret = write(filefd, &body[0] + curwrite, bodylen - curwrite);
        if(ret < 0)
        {
            if(ret == EAGAIN)
                continue;
            else 
            {
                cerr << "upload.cpp/SaveFile(): write error!" << endl;
                return false;
            }
        }
        curwrite += ret;
    }

    // 关闭文件
    close(filefd);
    return true;
}

int main(int argc, char* argv[], char* env[])
{
    //验证打印环境变量，TODO...测试完成可注释掉
    // for(int i = 0; env[i] != NULL; i++) {
    //     cerr << "env[" << i << "]====[" << env[i] << "]\n";
    // }

    //接受环境变量 (main函数参数 / getenv())
    vector<string> envlist;
    for(int i = 0; env[i] != NULL; ++i)
    {
        string temp;
        temp.assign(env[i]);
        envlist.push_back(temp);
    }

    //解析环境变量
    string filename;
    string bodyLengthStr;
    string userid;
    EnvParse(envlist, &filename, &bodyLengthStr, &userid);

    // 接受正文数据 (边接收边存储) 
    uint64_t bodylength = stringtouint64(bodyLengthStr);
    char buf[BUFSIZE];
    uint64_t rlen = 0;
    while(rlen < bodylength) {
        memset(buf, 0, BUFSIZE);
        int ret = read(1, buf, BUFSIZE);
        if(ret < 0)
        {
            cerr << "upload.cpp: read error" << endl;
            //发送响应信息
            string status = "FAILED";
            write(0, status.c_str(), status.size());
            close(0);
            close(1);
            return -1;
        }

        string filebody;
        filebody.assign(buf, ret);
        rlen += ret;

        //将数据存储到文件
        cerr << "文件名称:" << filename << endl;
        ret = SaveFile(userid, filename, filebody);
        if(ret == false) {
            //发送响应信息
            string status = "FAILED";
            write(0, status.c_str(), status.size());
            close(0);
            close(1);
            return -1;
        }
    }

    //发送响应信息
    string status = "SUCCESS";
    write(0, status.c_str(), status.size());
    close(0);
    close(1);

    return 0;
}
