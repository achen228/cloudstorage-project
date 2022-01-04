#include "clientwidget.h"
#include "ui_clientwidget.h"

clientWidget::clientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::clientWidget)
{
    ui->setupUi(this);
    //1.设置服务器IP地址和端口
    serverIP.setAddress("101.42.229.145");
    serverPort = 3389;
    //2.连接到服务器
    tcpSocket = new QTcpSocket();
    tcpSocket->connectToHost(serverIP, serverPort);
    //3.进入登录窗口
    loginWidget.show();
    //4.点击登录按钮后发送的信号
    connect(&loginWidget, SIGNAL(emitloginButtonSignal(QString, QString)),
             this, SLOT(acceptLoginSignal(QString, QString)));
    //5.点击注册按钮后发送的信号
    connect(&loginWidget, SIGNAL(emitregisterButtonSignal()), this, SLOT(acceptRegisterSignal()));
    //6.点击提交注册信息按钮后发送的信号
    connect(&registerWidget, SIGNAL(emitsubmitRegisterInfoButtonSignal(QString, QString, QString, QString)),
            this, SLOT(acceptsubmitRegisterInfoSignal(QString, QString, QString, QString)));
    //7.点击返回登录按钮后发送的信号
    connect(&registerWidget, SIGNAL(emitsubmitRegisterInfoButton2Signal()),
            this, SLOT(acceptsubmitRegisterInfo2Signal()));
    //8.当服务器有响应消息到来时，接收消息的槽函数
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    //9.登录时，当账号和密码填充后即文本改变时，发送信号textChanged()，执行登录按钮可点击的槽函数
    //获取登录界面对象
    Ui::loginRegister* loginui = loginWidget.getLoginui();
    connect(loginui->useridEdit, SIGNAL(textChanged(QString)), this, SLOT(enableLoginButton(void)));
    connect(loginui->passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(enableLoginButton(void)));
    //10.注册时，当昵称、密码、邮箱填充后即文本改变时，发送信号textChanged()，执行注册按钮可点击的槽函数
    //获取注册界面对象
    Ui::Register* registerui = registerWidget.getRegisterui();
    connect(registerui->usernameEdit, SIGNAL(textChanged(QString)), this, SLOT(enableRegisterButton(void)));
    connect(registerui->passwordFirstEdit, SIGNAL(textChanged(QString)), this, SLOT(enableRegisterButton(void)));
    connect(registerui->passwordSecondEdit, SIGNAL(textChanged(QString)), this, SLOT(enableRegisterButton(void)));
    connect(registerui->mailEdit, SIGNAL(textChanged(QString)), this, SLOT(enableRegisterButton(void)));
    //11.我的文件和共享文件窗口使能待增加右键菜单功能的控件，否则在控件上无法弹出右键菜单；
    ui->tableWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget_3->setContextMenuPolicy(Qt::CustomContextMenu);
    //12.上传文件窗口时，当选择本地文件后，执行使能上传文件按钮的槽函数
    connect(ui->showFileNameEdit, SIGNAL(textChanged(QString)), this, SLOT(enableUploadFileButton(void)));
    //13.文件搜索窗口使能待增加右键菜单功能的控件，否则在控件上无法弹出右键菜单；
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    //14.点击修改个人信息按钮时，当信息都填充时，发送信号textChanged()，执行修改信息可点击的槽函数
    connect(ui->personNameEdit, SIGNAL(textChanged(QString)), this, SLOT(enableModInfoButton(void)));
    connect(ui->personPasswordEdit, SIGNAL(textChanged(QString)), this, SLOT(enableModInfoButton(void)));
    connect(ui->personMailEdit, SIGNAL(textChanged(QString)), this, SLOT(enableModInfoButton(void)));
}

clientWidget::~clientWidget()
{
    delete ui;
    tcpSocket->close();
}

//使能修改信息按钮对应的槽函数
void clientWidget::enableModInfoButton(void)
{
    if(ui->personNameEdit->text().isEmpty() || ui->personPasswordEdit->text().isEmpty() ||
            ui->personMailEdit->text().isEmpty()) {
        ui->modInfoButton->setEnabled(false);
    } else {
        ui->modInfoButton->setEnabled(true);
    }
}

//执行登录按钮可点击的槽函数
void clientWidget::enableLoginButton(void)
{
    bool useridOk, passwordOk;
    //text()：获取输入文本(QString)
    //toDouble()：QString转换为double，参数保存转换是否成功的结果
    Ui::loginRegister* loginui = loginWidget.getLoginui();
    loginui->useridEdit->text().toDouble(&useridOk);
    loginui->passwordEdit->text().toDouble(&passwordOk);
    //当userid和password操作数都输入了有效数据，则使能登录按钮，否则设置禁用
    loginui->loginButton->setEnabled(useridOk && passwordOk);
}

//执行注册按钮可点击的槽函数
void clientWidget::enableRegisterButton(void)
{
    Ui::Register* registerui = registerWidget.getRegisterui();
    if(registerui->usernameEdit->text().isEmpty() || registerui->passwordFirstEdit->text().isEmpty() ||
            registerui->passwordSecondEdit->text().isEmpty() || registerui->mailEdit->text().isEmpty()) {
        registerui->submitRegisterInfoButton->setEnabled(false);
    } else {
        registerui->submitRegisterInfoButton->setEnabled(true);
    }
}

//接收登录信号执行的槽函数
void clientWidget::acceptLoginSignal(QString userid, QString password)
{
//    qDebug() << "userid:" << userid << ", password:" << password;
    //向服务器发送登录请求验证用户id和密码是否正确
    QJsonObject json;
    json.insert("reqType", USER_LOGIN);
    json.insert("userid", userid.toInt());
    json.insert("password", password);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//接收注册信号执行的槽函数
void clientWidget::acceptRegisterSignal()
{
    //进入用户注册界面
    registerWidget.show();
}

//接收提交注册信息信号执行的槽函数
void clientWidget::acceptsubmitRegisterInfoSignal(QString username, QString passwordFirst,
                                                  QString passwordSecond, QString mail)
{
//    qDebug() << "username:" << username << ", passwordFirst" << passwordFirst <<
//                ", passwordSecond" << passwordSecond << ", mail" << mail;
    if(passwordFirst != passwordSecond) {
        //创建消息提示框
        QMessageBox msgBox(QMessageBox::NoIcon, "提示", "两次输入的密码不同，请重新输入");
        //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
        msgBox.exec();
    } else {
        QJsonObject json;
        json.insert("reqType", USER_REGISTER);
        json.insert("username", username);
        json.insert("password", passwordFirst);
        json.insert("mail", mail);
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
    }
    //获取注册界面对象
    Ui::Register* registerui = registerWidget.getRegisterui();
    //清空已输入的注册信息
    registerui->usernameEdit->clear();
    registerui->passwordFirstEdit->clear();
    registerui->passwordSecondEdit->clear();
    registerui->mailEdit->clear();
}

//接收返回登录信号执行的槽函数
void clientWidget::acceptsubmitRegisterInfo2Signal()
{
    //进入用户登录界面
    loginWidget.show();
}

//接收服务器响应消息的槽函数
void clientWidget::onReadyRead()
{
    //获取等待读取消息字节数
    QByteArray buf;
    if(tcpSocket->bytesAvailable()) {
        //读取消息并保存
        buf = tcpSocket->readAll();
    }
    QString msg = buf;
    //收到新数据
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(msg.toUtf8().data(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!"<<json_error.errorString();
        return;
    }
    QJsonObject rootObj = jsonDoc.object();
    //解析响应状态
    QJsonValue statusValue =  rootObj.value("status");
    QString status = statusValue.toString();
    //解析响应类型
    QJsonValue resTypeValue =  rootObj.value("resType");
    int resType = -1;
    resType = resTypeValue.toInt();
    switch(resType) {
    case RES_USER_LOGIN:{
        //服务器响应登录请求
        //获取登录界面对象
        Ui::loginRegister* loginui = loginWidget.getLoginui();
        if(status == RES_STATUS_SUCCESS) {
            //登录成功后保存记录用户ID
            _userId = loginui->useridEdit->text();
            //进入文件列表界面
            loginWidget.hide();
            this->show();
            //发送个人信息请求
            //向服务器发送展示个人信息窗口请求
            QJsonObject json;
            json.insert("reqType", USER_INFO);
            json.insert("userid", _userId.toInt());
            //直接使用构造函数
            QJsonDocument doc(json);
            QByteArray buf = doc.toJson(QJsonDocument::Indented);
            tcpSocket->write(buf);
        } else if(status == RES_STATUS_FAILED) {
            //提示用户登录失败，返回登录界面
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "账号或密码错误，请重新输入");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
            //清空已输入的登录信息
            loginui->useridEdit->clear();
            loginui->passwordEdit->clear();
        }
        break;
    }
    case RES_USER_REGISTER:{
        //服务器响应注册请求
        if(status == RES_STATUS_SUCCESS) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonValue useridValue =  rootObj.value("userid");
            int userid = useridValue.toInt();
            QString info = "注册成功，账号：" + QString::number(userid, 10);
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", info);
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "注册失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_FILE_CHMOD:{
        //服务器响应文件权限更改请求
        if(status == RES_STATUS_SUCCESS) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件权限修改成功");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件权限修改失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        //文件权限修改成功或失败都去刷新当前文件列表窗口
        QJsonObject json;
        json.insert("reqType", FILE_LIST);
        int userId = _userId.toInt();
        json.insert("userid", userId);
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
        break;
    }
    case RES_FILE_LIST:{
        //服务器响应文件列表请求
        if(status == RES_STATUS_SUCCESS) {
            //每次刷新前删除清空
            for(int row = ui->tableWidget_2->rowCount() - 1; row >= 0; row--)
            {
                ui->tableWidget_2->removeRow(row);
            }
            //每次刷新前删除清空
            for(int row = ui->tableWidget_3->rowCount() - 1; row >= 0; row--)
            {
                ui->tableWidget_3->removeRow(row);
            }
            //单击单元格时选中一整行
            ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tableWidget_2->setSelectionMode(QAbstractItemView::SingleSelection);
            ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tableWidget_3->setSelectionMode(QAbstractItemView::SingleSelection);
            //设置单元格禁止编辑
            ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
            ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers);
            //解析服务器响应的数据
            QJsonObject rootObj = jsonDoc.object();
            QJsonValue fileListValue =  rootObj.value("fileList");
            QJsonArray arrayJson = fileListValue.toArray();
            //使用map保存用户和对应的文件信息
            QMap<int, QVector<QString>> _qmap;
            for(int i = 0; i < arrayJson.size(); i++)
            {
                QJsonValue fileInfoValue = arrayJson.at(i);
                QString fileInfoStr = fileInfoValue.toString();
                QStringList list = fileInfoStr.split(",");
                QString userIDStr = list[0];
                int userID = userIDStr.toInt();
                QString fileNameStr = list[1];
                QString authStr = list[2];
                QString timeStr = list[3];
                QString fileListInfoStr = userIDStr + "," + fileNameStr + "," + authStr + "," + timeStr;
                //对用户ID做聚合
                QMap<int, QVector<QString>>::Iterator it = _qmap.find(userID);
                if(it == _qmap.end()) {
                    QVector<QString> _qvec;
                    _qvec.push_back(fileListInfoStr);
                    _qmap.insert(userID, _qvec);
                } else {
                    //不知道如何对it.value()直接执行插入数据,那就先从map中删除再重新插入
                    QVector<QString> _qvec = it.value();
                    _qvec.push_back(fileListInfoStr);
                    _qmap.erase(it);
                    _qmap.insert(userID, _qvec);
                }
            }
            //遍历展示到界面
            QMap<int, QVector<QString>>::Iterator it = _qmap.begin();
            while(it != _qmap.end()) {
                QVector<QString> _qvec = it.value();
                for(int i = 0; i < _qvec.size(); i++) {
                    QStringList list = _qvec[i].split(",");
                    QString userIDStr = list[0];
                    QString fileNameStr = list[1];
                    QString authStr = list[2];
                    QString timeStr = list[3];
                    //我的文件列表
                    QStringList lineListSelf;
                    lineListSelf.append(fileNameStr);
                    lineListSelf.append(authStr);
                    lineListSelf.append(timeStr);
                    //共享文件列表
                    QStringList lineListShare;
                    lineListShare.append(userIDStr);
                    lineListShare.append(fileNameStr);
                    lineListShare.append(authStr);
                    lineListShare.append(timeStr);

                    if(_userId == userIDStr) {
                        //将文件信息展示到我的文件
//                        qDebug() << "我的文件:" << userIDStr << "," << fileNameStr << "," <<
//                                    authStr << "," << timeStr;
                        //设置所有单元格的字体
                        ui->tableWidget_2->setFont(QFont("song", 18));
                        //设置表格的列数，必须要先设置，不然表格显示不出来
                        ui->tableWidget_2->setColumnCount(3);
                        //建立表头
                        QStringList header;
                        header << "文件名称" << "文件权限" << "上传时间";
                        ui->tableWidget_2->setHorizontalHeaderLabels(header);
                        //设置表头显示模式,这里设置的是拉伸模式
                        ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                        //向表里面动态添加参数
                        int rowCount = ui->tableWidget_2->rowCount();
                        ui->tableWidget_2->insertRow(rowCount);
                        for(int i = 0; i < 3; i++) {
                            ui->tableWidget_2->setItem(rowCount, i, new QTableWidgetItem(lineListSelf[i]));
                        }
                    } else {
                        //将文件信息展示到共享文件
//                        qDebug() << "共享文件:" << userIDStr << "," << fileNameStr << "," <<
//                                    authStr << "," << timeStr;
                        //设置所有单元格的字体
                        ui->tableWidget_3->setFont(QFont("song", 16));
                        //设置表格的列数，必须要先设置，不然表格显示不出来
                        ui->tableWidget_3->setColumnCount(4);
                        //建立表头
                        QStringList header;
                        header << "用户ID" << "文件名称" << "文件权限" << "上传时间";
                        ui->tableWidget_3->setHorizontalHeaderLabels(header);
                        //设置表头显示模式,这里设置的是拉伸模式
                        ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                        //向表里面动态添加参数
                        int rowCount = ui->tableWidget_3->rowCount();
                        ui->tableWidget_3->insertRow(rowCount);
                        for(int i = 0; i < 4; i++) {
                            ui->tableWidget_3->setItem(rowCount, i, new QTableWidgetItem(lineListShare[i]));
                        }
                    }
                }
                it++;
            }
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "刷新文件列表失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_FILE_DOWNLOAD:{
        //服务器响应文件下载请求
        if(status == RES_STATUS_SUCCESS) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonValue filebodyValue =  rootObj.value("filebody");
            QString fileBody = filebodyValue.toString();
            qDebug() << "文件下载请求响应成功";
            //弹框选择本地保存路径
            QString filePath = QFileDialog::getSaveFileName(this, "保存文件", "./",
                                                            "txt(*.txt);;ALL(*.*)");
            if(filePath.isEmpty())
            {
                qDebug() << "选择的文件路径为空";
                return;
            }
            QFile fileInfo(filePath);
            if(!fileInfo.open(QIODevice::WriteOnly)) {
                qDebug() << "文件打开失败";
                //创建消息提示框
                QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件打开失败");
                //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
                msgBox.exec();
                return;
            } else {
                //写入文件内容
                fileInfo.write(fileBody.toUtf8());
                fileInfo.close();
            }
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件下载成功");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        } else if(status == RES_STATUS_FAILED) {
            qDebug() << "文件下载请求响应失败";
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件下载失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_FILE_UPLOAD:{
        //服务器响应文件上传请求
        if(status == RES_STATUS_SUCCESS) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "上传文件成功");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
            //清空展示文件名的lineEdit框
            ui->showFileNameEdit->clear();
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "上传文件失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_FILE_SEARCH:{
        //服务器响应文件搜索请求
        if(status == RES_STATUS_SUCCESS) {
            //每次刷新前删除清空
            for(int row = ui->tableWidget->rowCount() - 1; row >= 0; row--)
            {
                ui->tableWidget->removeRow(row);
            }
            //单击单元格时选中一整行
            ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
            //设置单元格禁止编辑
            ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
            //解析服务器响应的数据
            QJsonObject rootObj = jsonDoc.object();
            QJsonValue resQueryValue =  rootObj.value("resQuery");
            QJsonArray arrayJson = resQueryValue.toArray();
            //遍历展示到界面
            for(int i = 0; i < arrayJson.size(); i++) {
                QJsonValue queryInfoValue = arrayJson.at(i);
                QString queryInfoStr = queryInfoValue.toString();
                QStringList list = queryInfoStr.split(",");

                //设置所有单元格的字体
                ui->tableWidget->setFont(QFont("song", 16));
                //设置表格的列数，必须要先设置，不然表格显示不出来
                ui->tableWidget->setColumnCount(4);
                //建立表头
                QStringList header;
                header << "用户ID" << "文件名称" << "文件权限" << "上传时间";
                ui->tableWidget->setHorizontalHeaderLabels(header);
                //设置表头显示模式,这里设置的是拉伸模式
                ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
                //向表里面动态添加参数
                int rowCount = ui->tableWidget->rowCount();
                ui->tableWidget->insertRow(rowCount);
                for(int i = 0; i < 4; i++) {
                    ui->tableWidget->setItem(rowCount, i, new QTableWidgetItem(list[i]));
                }
            }
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "文件搜索失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_USER_INFO:{
        //服务器响应展示个人信息窗口请求
        if(status == RES_STATUS_SUCCESS) {
            //解析服务器响应的数据
            QJsonObject rootObj = jsonDoc.object();
            QJsonValue personNameValue =  rootObj.value("personName");
            QString personNameStr = personNameValue.toString();
            QJsonValue personPasswordValue =  rootObj.value("personPassword");
            QString personPasswordStr = personPasswordValue.toString();
            QJsonValue personMailValue =  rootObj.value("personMail");
            QString personMailStr = personMailValue.toString();
            //打印日志
            qDebug() << "查询的用户信息:" << personNameStr << "," << personPasswordStr << "," << personMailStr;
            //展示到个人信息界面
            ui->personNameEdit->setText(personNameStr);
            ui->personPasswordEdit->setText(personPasswordStr);
            ui->personMailEdit->setText(personMailStr);
            ui->personIDEdit->setText(_userId);
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "刷新个人信息失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_MOD_INFO:{
        //服务器响应修改信息请求
        if(status == RES_STATUS_SUCCESS) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "修改信息成功");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "修改信息失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    case RES_USER_LOGOUT:{
        //服务器响应用户注销账号请求
        if(status == RES_STATUS_SUCCESS) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "注销账号成功");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
            close();
        } else if(status == RES_STATUS_FAILED) {
            //创建消息提示框
            QMessageBox msgBox(QMessageBox::NoIcon, "提示", "注销账号失败");
            //显示消息提示框，并进入事件循环，点击Ok的时候会退出事件循环
            msgBox.exec();
        }
        break;
    }
    default:
        //未知的服务器响应类型
        break;
    }
}

//当切换选项卡的时候执行的槽函数
void clientWidget::on_tabWidget_tabBarClicked(int index)
{
    switch(index){
    case 0:{
        qDebug() << "个人信息请求";
        //向服务器发送展示个人信息窗口请求
        QJsonObject json;
        json.insert("reqType", USER_INFO);
        json.insert("userid", _userId.toInt());
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
        break;
    }
    case 1:{
        qDebug() << "我的文件请求";
        QJsonObject json;
        json.insert("reqType", FILE_LIST);
        int userId = _userId.toInt();
        json.insert("userid", userId);
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
        break;
    }
    case 2:{
        qDebug() << "共享文件请求";
        QJsonObject json;
        json.insert("reqType", FILE_LIST);
        int userId = _userId.toInt();
        json.insert("userid", userId);
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
        break;
    }
    case 3:{
        qDebug() << "文件上传请求";
        //点击按钮后直接通过槽函数已实现，暂时这里不用
        break;
    }
    case 4:{
        qDebug() << "文件搜索请求";
        //通过槽函数单独实现
        break;
    }
    case 5:{
        qDebug() << "传输列表请求";
        //暂时不实现
        break;
    }
    default:
        break;
    }
}

//共享文件窗口点击鼠标右键菜单槽函数
void clientWidget::on_tableWidget_3_customContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* downloadFile = menu.addAction(tr("下载文件"));
    menu.addSeparator();
    //连接共享文件窗口菜单中下载文件的槽函数
    connect(downloadFile, SIGNAL(triggered()), this, SLOT(slotdownloadtableWidget_3Menu()));
    menu.exec(QCursor::pos());
}

//执行共享文件窗口菜单中下载文件的槽函数
void clientWidget::slotdownloadtableWidget_3Menu()
{
    QStringList list;
    QList<QTableWidgetItem*> items = ui->tableWidget_3->selectedItems();
    int count = items.count();
    for(int i = 0; i < count; i++)
    {
        QTableWidgetItem *item = items.at(i);
        QString text = item->text(); //获取内容
        list.append(text);
    }
    QString userID = list[0];
    QString fileName = list[1];
    //向服务器发送下载文件请求
    QJsonObject json;
    json.insert("reqType", FILE_DOWNLOAD);
    json.insert("userid", userID.toInt());
    json.insert("filename", fileName);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//我的文件窗口点击鼠标右键菜单槽函数
void clientWidget::on_tableWidget_2_customContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* downloadFile = menu.addAction(tr("下载文件"));
    menu.addSeparator();
    QAction* chmodFile = menu.addAction(tr("更改文件权限"));
    menu.addSeparator();
    //连接我的文件窗口菜单中下载文件的槽函数
    connect(downloadFile, SIGNAL(triggered()), this, SLOT(slotdownloadtableWidget_2Menu()));
    //连接我的文件窗口菜单中更改文件权限的槽函数
    connect(chmodFile, SIGNAL(triggered()), this, SLOT(slotchmodtableWidget_2Menu()));
    menu.exec(QCursor::pos());
}

//执行我的文件窗口菜单中下载文件的槽函数
void clientWidget::slotdownloadtableWidget_2Menu()
{
    QStringList list;
    QList<QTableWidgetItem*> items = ui->tableWidget_2->selectedItems();
    int count = items.count();
    for(int i = 0; i < count; i++)
    {
        QTableWidgetItem *item = items.at(i);
        QString text = item->text(); //获取内容
        list.append(text);
    }
    QString fileName = list[0];
    //向服务器发送下载文件请求
    QJsonObject json;
    json.insert("reqType", FILE_DOWNLOAD);
    json.insert("userid", _userId.toInt());
    json.insert("filename", fileName);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//执行我的文件窗口菜单中更改文件权限的槽函数
void clientWidget::slotchmodtableWidget_2Menu()
{
    QStringList list;
    QList<QTableWidgetItem*> items = ui->tableWidget_2->selectedItems();
    int count = items.count();
    for(int i = 0; i < count; i++)
    {
        QTableWidgetItem *item = items.at(i);
        QString text = item->text(); //获取内容
        list.append(text);
    }
    QString fileName = list[0];
    QString authStr = list[1];
    int auth = authStr.toInt();
    //设置当前文件的相反权限
    if(auth == 0) {
        auth = 1;
    } else if(auth == 1)
    {
        auth = 0;
    }
    //向服务器发送文件权限修改请求
    QJsonObject json;
    json.insert("reqType", FILE_CHMOD);
    json.insert("userid", _userId.toInt());
    json.insert("filechmod", auth);
    json.insert("filename", fileName);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//上传文件窗口时：选择本地文件按钮执行的槽函数,作用是得到文件，并显示文件信息
void clientWidget::on_selectFileButton_clicked()
{
    //获得文件路径，并且判断是否为空
    _filePath = QFileDialog::getOpenFileName(this, "open", "./", "ALL(*.*)");
    if(_filePath.isEmpty())
    {
        return;
    }
    //接下来得到文件信息，使用QFileInfo
    QFileInfo info(_filePath);
    _fileName = info.fileName();
    _fileSize = info.size();
    ui->showFileNameEdit->setText(_fileName);
    qDebug() << _fileName << "," << _fileSize;
}

//上传文件按钮对应的槽函数
void clientWidget::on_uploadFileButton_clicked()
{
    //向服务器发送文件上传请求
    QJsonObject json;
    json.insert("reqType", FILE_UPLOAD);
    json.insert("userid", _userId.toInt());
    int auth;
    if(ui->setauthcomboBox->currentIndex() == 0) {
        //文件权限私有
        auth = 0;
    } else if(ui->setauthcomboBox->currentIndex() == 1) {
        //文件权限共享
        auth = 1;
    }
    json.insert("filechmod", auth);
    json.insert("filename", _fileName);
    //获取文件内容
    QFile file(_filePath);
    bool ok = file.open(QIODevice::ReadOnly);
    QString fileBody;
    if(true == ok)
     {
        QByteArray readFileBody = file.readAll();
        fileBody = readFileBody;
        file.close();
    }
    json.insert("filebody", fileBody);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//上传文件窗口时，当选择本地文件后，执行使能上传文件按钮的槽函数
void clientWidget::enableUploadFileButton(void)
{
    QString isEmpty = ui->showFileNameEdit->text();
    if(isEmpty.isEmpty()) {
        ui->uploadFileButton->setEnabled(false);
    } else {
        ui->uploadFileButton->setEnabled(true);
    }
}

//文件搜索按钮对应的槽函数
void clientWidget::on_searchButton_clicked()
{
    QString queryWord = ui->searchEdit->text();
    //向服务器发送文件搜索请求
    QJsonObject json;
    json.insert("reqType", FILE_SEARCH);
    json.insert("userid", _userId.toInt());
    json.insert("query", queryWord);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//文件搜索窗口点击鼠标右键菜单槽函数
void clientWidget::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QAction* downloadFile = menu.addAction(tr("下载文件"));
    menu.addSeparator();
    //连接共享文件窗口菜单中下载文件的槽函数
    connect(downloadFile, SIGNAL(triggered()), this, SLOT(slotdownloadtableWidgetMenu()));
    menu.exec(QCursor::pos());
}

//执行文件搜索窗口菜单中下载文件的槽函数
void clientWidget::slotdownloadtableWidgetMenu()
{
    QStringList list;
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    int count = items.count();
    for(int i = 0; i < count; i++)
    {
        QTableWidgetItem *item = items.at(i);
        QString text = item->text(); //获取内容
        list.append(text);
    }
    QString userID = list[0];
    QString fileName = list[1];
    //向服务器发送下载文件请求
    QJsonObject json;
    json.insert("reqType", FILE_DOWNLOAD);
    json.insert("userid", userID.toInt());
    json.insert("filename", fileName);
    //直接使用构造函数
    QJsonDocument doc(json);
    QByteArray buf = doc.toJson(QJsonDocument::Indented);
    tcpSocket->write(buf);
}

//个人信息界面修改信息按钮对应的槽函数
void clientWidget::on_modInfoButton_clicked()
{
    //弹框提示用户确定是否要修改信息
    QMessageBox msgBox(
        QMessageBox::Question, //图标
        "修改信息", //标题
        "是否确定要修改信息？",  //提示消息
        QMessageBox::Yes | QMessageBox::No, //按钮
        this); //父窗口
    if(msgBox.exec() == QMessageBox::Yes) {
        //修改信息
        QString personName = ui->personNameEdit->text();
        QString personPassword = ui->personPasswordEdit->text();
        QString personMail = ui->personMailEdit->text();
        //向服务器发送修改信息请求
        QJsonObject json;
        json.insert("reqType", MOD_INFO);
        json.insert("userid", _userId.toInt());
        json.insert("personName", personName);
        json.insert("personPassword", personPassword);
        json.insert("personMail", personMail);
        //直接使用构造函数
        QJsonDocument doc(json);
        QByteArray buf = doc.toJson(QJsonDocument::Indented);
        tcpSocket->write(buf);
    } else if(msgBox.exec() == QMessageBox::No) {
        //取消修改信息
        return;
    }
}

//用户注销账号对应的槽函数
void clientWidget::on_logoutButton_clicked()
{
    //弹框提示用户确定是否要注销账号
    QMessageBox msgBox(
        QMessageBox::Question, //图标
        "注销账号", //标题
        "是否确定要注销账号？",  //提示消息
        QMessageBox::Yes | QMessageBox::No, //按钮
        this); //父窗口
   if(msgBox.exec() == QMessageBox::Yes) {
       //向服务器注销账号请求
       QJsonObject json;
       json.insert("reqType", USER_LOGOUT);
       json.insert("userid", _userId.toInt());
       //直接使用构造函数
       QJsonDocument doc(json);
       QByteArray buf = doc.toJson(QJsonDocument::Indented);
       tcpSocket->write(buf);
   } else if(msgBox.exec() == QMessageBox::No) {
       //取消注销账号
       return;
   }
}
