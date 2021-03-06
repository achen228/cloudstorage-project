#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>

const char* level[] = 
{
    "INFO",
    "ERROR",
    "FATAL",
    "WARNING",
    "DEBUG"
};

enum LogLevel
{
    INFO = 0,
    ERROR,
    FATAL,
    WARNING,
    DEBUG
};

class LogTime
{
public:
    static void GetTimeStamp(std::string& timestamp)
    {
        time_t systime;
        time(&systime);
        struct tm* ST = localtime(&systime);
        //格式化字符串 [YYYY-MM-DD HH-MM-SS]
        char timenow[23] = {'\0'};
        snprintf(timenow, sizeof(timenow) - 1, "%04d-%02d-%02d %02d:%02d:%02d", ST->tm_year + 1900, ST->tm_mon + 1, ST->tm_mday, ST->tm_hour, ST->tm_min, ST->tm_sec);
        timestamp.assign(timenow, strlen(timenow));
    }
};

inline std::ostream& Log(LogLevel lev, const char* file, int line, const std::string& logmsg)
{
    std::string level_info = level[lev];
    std::string time_stamp;
    LogTime::GetTimeStamp(time_stamp);

    //[时间 info/error/fatal/warning/debug 文件 行号] 具体的错误信息
    std::cout << "[" << time_stamp << " " << level_info << " " << file << ":" << line << "]" << logmsg;
    return std::cout;
}

#define LOG(lev, msg) Log(lev, __FILE__, __LINE__, msg)
