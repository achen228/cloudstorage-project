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

#define BUFSIZE 10240   //文件读取缓冲区
#define CURRENTFILEROOT "./userfile/"  //服务器文件存储路径

//解析环境变量 
bool EnvParse(vector<string>& envlist, string* filename, string* userid)
{
    //寻找所需环境变量
    string filenameENV;
    string useridENV;
    for(auto& e : envlist) {
        //FILENAME
        if(e.find("FILENAME") != string::npos) {
            filenameENV = e;
        }
        //USERID
        if(e.find("USERID") != string::npos) {
            useridENV = e;
        }        
    }
    //解析各个字段
    //1.解析filename
    string filenameStr = "=";
    size_t filenamePos = filenameENV.find(filenameStr, 0);
    if(filenamePos == string::npos)
    {
        cerr << "download.cpp/EnvParse(): Cannot find the = " << endl;
        return false;
    }
    *filename = filenameENV.substr(filenamePos + filenameStr.size());

    //2.解析userid
    string useridStr = "=";
    size_t useridPos = useridENV.find(useridStr, 0);
    if(useridPos == string::npos)
    {
        cerr << "download.cpp/EnvParse(): Cannot find the = " << endl;
        return false;
    }  
    *userid = useridENV.substr(useridPos + useridStr.size());

    return true;  
}

//下载文件数据
bool Download(string& path, string* fileBodyStr)
{
    // 打开文件
    int fd = open(path.c_str(), O_RDONLY);
    if(fd < 0)
    {
        cerr << "download.cpp/Download(): open error!" << endl;
        return false;
    }

    lseek(fd, 0, SEEK_SET);

    //循环读取文件
    char buf[BUFSIZE];
    while(1)
    {
        memset(buf, 0, BUFSIZE);
        int ret = read(fd, buf, BUFSIZE);
        if(ret < 0)
        {
            cerr << "download.cpp/Download(): read error!" << endl;
            return false;
        }
        else if(ret == 0)
        {
            //到达文件末尾
            break;
        }
        *fileBodyStr += buf;
    }
    close(fd);
    return true;
}

int main(int argc, char* argv[], char* env[])
{
    // //验证打印环境变量，TODO...测试完成可注释掉
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
    string userid;
    EnvParse(envlist, &filename, &userid);

    //文件路径
    string filePath = CURRENTFILEROOT + userid + "/" + filename;
    //文件内容
    string fileBodyStr;
    //读取文件
    int ret = Download(filePath, &fileBodyStr); 
    if(ret != true)
    {
        cerr << "download.cpp/Download(): Download error" << endl;
        //发送响应信息
        stringstream statusAndFileBody;
        statusAndFileBody << "FAILED" << "\3" << fileBodyStr;
        write(0, statusAndFileBody.str().c_str(), statusAndFileBody.str().size());
        close(0);
        close(1);
        return -1;
    }

    //发送响应信息
    stringstream statusAndFileBody;
    statusAndFileBody << "SUCCESS" << "\3" << fileBodyStr;
    write(0, statusAndFileBody.str().c_str(), statusAndFileBody.str().size());
    close(0);
    close(1);
    return 0;
}
