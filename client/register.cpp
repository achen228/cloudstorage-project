#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
}

Register::~Register()
{
    delete ui;
}

//用户填写注册信息后执行提交按钮的槽函数
void Register::on_submitRegisterInfoButton_clicked()
{
    //发射提交注册按钮信号
    emit emitsubmitRegisterInfoButtonSignal(ui->usernameEdit->text(), ui->passwordFirstEdit->text(),
                                            ui->passwordSecondEdit->text(), ui->mailEdit->text());
}

//点击返回登录按钮后执行的槽函数
void Register::on_submitRegisterInfoButton_2_clicked()
{
    //点击返回登录按钮后，将当前的登录页面隐藏起来
    this->hide();
    //发射返回登录按钮信号
    emit emitsubmitRegisterInfoButton2Signal();
}

//获取注册界面对象
Ui::Register* Register::getRegisterui()
{
    return ui;
}
