#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QMessageBox> //消息提示框头文件
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include "loginregister.h"
#include "register.h"
#include "protobuf.h"
#include "ui_register.h"
#include "ui_loginregister.h"
#include <QHeaderView>
#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QFile>


namespace Ui {
class clientWidget;
}

class clientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit clientWidget(QWidget *parent = 0);
    ~clientWidget();

public slots:
    //接收登录信号执行的槽函数
    void acceptLoginSignal(QString userid, QString password);
    //接收注册信号执行的槽函数
    void acceptRegisterSignal();
    //接收提交注册信息信号执行的槽函数
    void acceptsubmitRegisterInfoSignal(QString, QString, QString, QString);
    //接收返回登录信号执行的槽函数
    void acceptsubmitRegisterInfo2Signal();
    //接收服务器响应的槽函数
    void onReadyRead();
    //执行登录按钮可点击的槽函数
    void enableLoginButton(void);
    //执行注册按钮可点击的槽函数
    void enableRegisterButton(void);
    //上传文件窗口时，当选择本地文件后，执行使能上传文件按钮的槽函数
    void enableUploadFileButton(void);
    //执行文件搜索窗口菜单中下载文件的槽函数
    void slotdownloadtableWidgetMenu();

private slots:
    //当切换选项卡的时候执行的槽函数
    void on_tabWidget_tabBarClicked(int index);
    //共享文件窗口点击鼠标右键菜单槽函数
    void on_tableWidget_3_customContextMenuRequested(const QPoint &pos);
    //我的文件窗口点击鼠标右键菜单槽函数
    void on_tableWidget_2_customContextMenuRequested(const QPoint &pos);
    //执行共享文件窗口菜单中下载文件的槽函数
    void slotdownloadtableWidget_3Menu();
    //执行我的文件窗口菜单中下载文件的槽函数
    void slotdownloadtableWidget_2Menu();
    //执行我的文件窗口菜单中更改文件权限的槽函数
    void slotchmodtableWidget_2Menu();
    //上传文件窗口时：选择本地文件按钮执行的槽函数,作用是得到文件，并显示文件信息
    void on_selectFileButton_clicked();
    //上传文件按钮对应的槽函数
    void on_uploadFileButton_clicked();
    //文件搜索按钮对应的槽函数
    void on_searchButton_clicked();
    //文件搜索窗口点击鼠标右键菜单槽函数
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    //个人信息界面修改信息按钮对应的槽函数
    void on_modInfoButton_clicked();
    //使能修改信息按钮对应的槽函数
    void enableModInfoButton(void);
    //用户注销账号对应的槽函数
    void on_logoutButton_clicked();

private:
    Ui::clientWidget *ui;
    //登录界面窗口对象
    loginRegister loginWidget;
    //注册界面窗口对象
    Register registerWidget;
    //和服务器通信的套接字
    QTcpSocket* tcpSocket;
    //服务器地址
    QHostAddress serverIP;
    //服务器端口
    quint16 serverPort;
    //保存记录用户ID
    QString _userId;
    //上传文件时选择获取的本地文件路径,文件名文件大小
    QString _filePath;
    QString _fileName;
    int _fileSize;
};

#endif // CLIENTWIDGET_H
