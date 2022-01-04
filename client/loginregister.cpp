#include "loginregister.h"
#include "ui_loginregister.h"

loginRegister::loginRegister(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::loginRegister)
{
    ui->setupUi(this);
}

loginRegister::~loginRegister()
{
    delete ui;
}

//用户登录界面点击注册按钮后执行的槽函数
void loginRegister::on_registerButton_clicked()
{
    //点击注册按钮后，将当前的登录页面隐藏起来
    this->hide();
    //发射注册信号
    emit emitregisterButtonSignal();
}

//用户登录界面点击登录按钮后执行的槽函数
void loginRegister::on_loginButton_clicked()
{
    //发射登录信号
    emit emitloginButtonSignal(ui->useridEdit->text(), ui->passwordEdit->text());
}

//获取登录界面对象
Ui::loginRegister* loginRegister::getLoginui()
{
    return ui;
}
