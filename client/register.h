#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

    //获取注册界面对象
    Ui::Register* getRegisterui();

private slots:
    //用户填写注册信息后执行提交按钮的槽函数
    void on_submitRegisterInfoButton_clicked();
    //点击返回登录按钮后执行的槽函数
    void on_submitRegisterInfoButton_2_clicked();

signals:
    //点击提交注册按钮后触发的信号
    void emitsubmitRegisterInfoButtonSignal(QString username, QString passwordFirst,
                                            QString passwordSecond, QString mail);
    //点击返回登录按钮后触发的信号
    void emitsubmitRegisterInfoButton2Signal();

private:
    Ui::Register *ui;
};

#endif // REGISTER_H
