#ifndef LOGINREGISTER_H
#define LOGINREGISTER_H

#include <QWidget>

namespace Ui {
class loginRegister;
}

class loginRegister : public QWidget
{
    Q_OBJECT

public:
    explicit loginRegister(QWidget *parent = 0);
    ~loginRegister();

    //获取登录界面对象
    Ui::loginRegister* getLoginui();

private slots:
    //用户登录界面点击注册按钮后执行的槽函数
    void on_registerButton_clicked();
    //用户登录界面点击登录按钮后执行的槽函数
    void on_loginButton_clicked();

signals:
    //点击登录按钮后触发的信号
    void emitloginButtonSignal(QString userid, QString password);
    //点击注册按钮后触发的信号
    void emitregisterButtonSignal();
private:
    Ui::loginRegister *ui;
};

#endif // LOGINREGISTER_H
