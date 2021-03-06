#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <mysql/mysql.h>
#include <json/json.h>
#include <stdio.h>
#include "time.h"
#include "log.hpp"

class TableInfo {
public:
    bool InitTableInfo() {
        //1.初始化
        _mysql = mysql_init(NULL);
        if(_mysql == NULL) {
            LOG(ERROR, "init mysql error") << std::endl;
            return false;
        }
        //2.连接mysql服务器
        if(mysql_real_connect(_mysql, "127.0.0.1", "root", "5201", "db_cloudstorage", 0, NULL, 0) == NULL) {
            LOG(ERROR, "connect mysql server failed") << std::endl;
            return false;
        }   
        //3.设置客户端字符集
        if(mysql_set_character_set(_mysql, "utf8") != 0) {
            LOG(ERROR, "set character failed") << std::endl;
            return false;
        }
        return true;   
    }

    //执行增删查改
    bool DoMysqlQuery(char* sql_str) {
        int ret = mysql_query(_mysql, sql_str);
        if(ret != 0) {
            LOG(ERROR, "query sql failed") << std::endl;
            return false;
        }        
        return true;      
    }

    //查询某一用户信息是否存在，查询条件要具有唯一性
    bool InquireUserInfoExist(char* sql_str) {
        int ret = mysql_query(_mysql, sql_str);
        if(ret != 0) {
            LOG(ERROR, "query sql failed") << std::endl;
            return false;
        }
        //保存查询结果
        MYSQL_RES* res = mysql_store_result(_mysql);
        if(res == NULL) {
            LOG(ERROR, "store result failed") << std::endl;
            return false;
        }
        //获取行数
        int numRow = mysql_num_rows(res);
        if(numRow == 0) {
            //释放结果集,否则会造成资源泄露
            mysql_free_result(res);
            return false;
        }
        //释放结果集,否则会造成资源泄露
        mysql_free_result(res);
        return true;       
    }

    //查询某一用户具体信息，查询条件要具有唯一性
    bool InquireUserInfo(char* sql_str, std::vector<std::string>* vecUserInfo) {
        int ret = mysql_query(_mysql, sql_str);
        if(ret != 0) {
            LOG(ERROR, "query sql failed") << std::endl;
            return false;
        }
        //保存查询结果
        MYSQL_RES* res = mysql_store_result(_mysql);
        if(res == NULL) {
            LOG(ERROR, "store result failed") << std::endl;
            return false;
        }
        //获取行数
        int numRow = mysql_num_rows(res);
        //获取列数
        int numCol = mysql_num_fields(res);
        if(numRow != 1 && numCol != 4) {
            LOG(ERROR, "numRow is not 1 or numCol is not four") << std::endl;
            return false;
        }
        //将查询结果集保存到vector中
        for(int i = 0; i < numRow; i++) {
            //res有读取位置控制，每次获取的都是下一条数据
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string userID = row[0];
            std::string userName = row[1];
            std::string userPassword = row[2];
            std::string userMail = row[3];
            vecUserInfo->push_back(userID);
            vecUserInfo->push_back(userName);
            vecUserInfo->push_back(userPassword);
            vecUserInfo->push_back(userMail);
        }
        //释放结果集,否则会造成资源泄露
        mysql_free_result(res);
        return true;
    }

    bool InquireFile(char* sql_str, std::vector<std::string>* fileVec) {
        int ret = mysql_query(_mysql, sql_str);
        if(ret != 0) {
            LOG(ERROR, "query sql failed") << std::endl;
            return false;
        }
        //保存查询结果
        MYSQL_RES* res = mysql_store_result(_mysql);
        if(res == NULL) {
            LOG(ERROR, "store result failed") << std::endl;
            return false;
        }
        //获取行数
        int numRow = mysql_num_rows(res);
        //获取列数
        int numCol = mysql_num_fields(res);
        if(numRow > 0 && numCol != 4) {
            LOG(ERROR, "numCol is not four") << std::endl;
            return false;
        }
        //将查询结果集保存到vector中
        for(int i = 0; i < numRow; i++) {
            //res有读取位置控制，每次获取的都是下一条数据
            MYSQL_ROW row = mysql_fetch_row(res);
            std::string userID = row[0];
            std::string fileName = row[1];
            std::string auth = row[2];
            //转换时间戳
            int timeInt = atoi(row[3]);
            time_t tick = time_t(timeInt);
            struct tm tm;
            char s[100];
            tm = *localtime(&tick);
            strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
            std::string fileUploadTime = s;
            std::string fileListInfo = userID + "," + fileName + "," + auth + "," + fileUploadTime;
            fileVec->push_back(fileListInfo);
        }
        //释放结果集,否则会造成资源泄露
        mysql_free_result(res);
        return true;
    }

private:
    MYSQL* _mysql;
};
