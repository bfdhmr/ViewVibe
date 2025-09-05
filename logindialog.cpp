#include "logindialog.h"
#include "ui_logindialog.h"
#include <QRegExp>
#include <QMessageBox>
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("注册登录");
    ui->tw_page->setCurrentIndex(0);//默认显示登录界面
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    event->ignore();
    Q_EMIT SIG_close();
}
//清空登录信息
void LoginDialog::on_pb_clear_clicked()
{
   ui->le_passwd->clear();
   ui->le_tel->clear();
}


//清空注册信息
void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le_tel_register->clear();
    ui->le_passwd_register->clear();
    ui->le_confirm_passwd->clear();
    ui->le_name_register->clear();
}

//提交注册信息
void LoginDialog::on_pb_commit_register_clicked()
{
    QString strTel=ui->le_tel_register->text();
    QString strPasswd=ui->le_passwd_register->text();
    QString name=ui->le_name_register->text(); //格式utf-8
    //校验
    //非空校验 清掉空格以后不能是空字符串
    QString tmptel=strTel;
    QString tmppasswd=strPasswd;
    QString tmpname=name;
    if(tmptel.remove(" ").isEmpty()||tmppasswd.remove(" ").isEmpty()||tmpname.remove(" ").isEmpty())
    {
        QMessageBox::about(this,"提示","手机号/密码/昵称不能为空格");
        return;
    }


    //1.校验手机号
    QRegExp reg("^1[3-8][0-9]{6,9}$");
    bool res=reg.exactMatch(tmptel);
    if(!res)
    {
        QMessageBox::about(this,"提示","手机号格式错误，8-11位手机号");
        return;
    }

    //2.校验密码
    if(strPasswd.length()>20)
    {
        QMessageBox::about(this,"提示","密码过长,长度不应超过20");
        return;
    }
    //3.确认密码
    QString str_confirm_passwd= ui->le_confirm_passwd->text();
    if(str_confirm_passwd!=strPasswd)
    {
        QMessageBox::about(this,"提示","两次密码输入不一致");
        return;
    }
    //4.校验name
    if(name.length()>10)
    {
        QMessageBox::about(this,"提示","昵称过长，长度不应超过10");
        return;
    }
    Q_EMIT SIG_registerCommit(strTel,strPasswd,name);
}


void LoginDialog::on_pb_commit_clicked()
{
    QString strTel=ui->le_tel->text();
    QString strPasswd=ui->le_passwd->text();
    //校验
    //非空校验 清掉空格以后不能是空字符串
    QString tmptel=strTel;
    QString tmppasswd=strPasswd;
    if(tmptel.remove(" ").isEmpty()||tmppasswd.remove(" ").isEmpty())
    {
        QMessageBox::about(this,"提示","手机号/密码不能为空格");
        return;
    }

    //1.校验手机号
    QRegExp reg("^1[3-8][0-9]{6,9}$");
    bool res=reg.exactMatch(tmptel);
    if(!res)
    {
        QMessageBox::about(this,"提示","手机号格式错误，8-11位手机号");
        return;
    }

    //2.校验密码
    if(strPasswd.length()>20)
    {
        QMessageBox::about(this,"提示","密码过长,长度不应超过20");
        return;
    }

    Q_EMIT SIG_loginCommit(strTel,strPasswd);
}

