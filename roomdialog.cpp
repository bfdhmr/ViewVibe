#include "roomdialog.h"
#include "ui_roomdialog.h"
#include <QDebug>
#include <QMessageBox>
RoomDialog::RoomDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoomDialog)
{
    ui->setupUi(this);
    m_mainLayout=new QVBoxLayout;
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(5);
    is_Max=false;
    //设置一个竖直布局的画布  ，可以向这里添加控件
    ui->wdg_list->setLayout(m_mainLayout);
   /* for(int i=1;i<=6;i++)
    {
        UserShow* user=new UserShow;
        user->slot_setInfo(i,QString("测试%1").arg(i));
        slot_addUserShow(user);
    }*/
}

RoomDialog::~RoomDialog()
{
    delete ui;
}

void RoomDialog::slot_setInfo(QString roomid)
{
    QString title=QString("房间号:%1").arg(roomid);
    setWindowTitle(title);
    ui->lb_title->setText(title);
}

void RoomDialog::slot_addUserShow(UserShow *user)
{
    m_mainLayout->addWidget(user);
    m_mapIDToUserShow[user->m_id]=user;
}

void RoomDialog::slot_refreshUser(int id, QImage &img)
{
    //预览图的id与要刷新的id一致,我们刷新预览图
    if(ui->wdg_userShow->m_id==id)
    {
        ui->wdg_userShow->slot_setImage(img);
    }
    qDebug()<<id;
    if(m_mapIDToUserShow.count(id)>0)
    {
        UserShow * user=m_mapIDToUserShow[id];

        user->slot_setImage(img);
    }
}

void RoomDialog::slot_removeUserShow(UserShow *user)
{
    user->hide();
    m_mainLayout->removeWidget(user);

}

void RoomDialog::slot_removeUserShow(int id)
{
    if(m_mapIDToUserShow.count(id)>0)
    {
        UserShow *user=m_mapIDToUserShow[id];
        slot_removeUserShow(user);
    }
}

void RoomDialog::slot_setAudioCheck(bool check)
{
    ui->cb_audio->setChecked(check);
}
void RoomDialog::slot_setVideoCheck(bool check)
{
    ui->cb_video->setChecked(check);
}

void RoomDialog::slot_ClickedScreen(int id, QString name)
{
    ui->wdg_userShow->slot_setInfo(id,name);
}
void RoomDialog::slot_clearUserShow()
{
    for(auto ite=m_mapIDToUserShow.begin();ite!=m_mapIDToUserShow.end();ite++)
    {
        slot_removeUserShow(ite->second);
    }
}
//退出房间
void RoomDialog::on_pb_close_clicked()
{
    this->close();
}

//退出房间
void RoomDialog::on_pb_quit_clicked()
{
    this->close();
}
//选择要不要退出来
void RoomDialog::closeEvent(QCloseEvent *event)
{
    if(QMessageBox::question(this,"退出提示","是否退出会议?")==QMessageBox::Yes)
    {
        //发送退出房间信号
        Q_EMIT SIG_close();
        event->accept();
        return;
    }
    event->ignore();
}

//开启或关闭 音频
void RoomDialog::on_cb_audio_clicked()
{
    if(ui->cb_audio->isChecked())
    {
        //ui->cb_audio->setCheckable(false);
        Q_EMIT SIG_AudioStart();

    }
    else{
        //ui->cb_audio->setCheckable(true);
        Q_EMIT SIG_AudioPause();
    }
}

//开启或关闭 视频
void RoomDialog::on_cb_video_clicked()
{
    if(ui->cb_video->isChecked())
    {
        ui->cb_desk->setChecked(false);
        Q_EMIT SIG_ScreenPause();
        Q_EMIT SIG_VideoStart();

    }
    else{

        Q_EMIT SIG_VideoPause();
    }
}

void RoomDialog::slot_setScreenCheck(bool check)
{
    ui->cb_desk->setChecked(check);
}
void RoomDialog::on_cb_desk_clicked()
{
    if(ui->cb_desk->isChecked())
    {
        ui->cb_video->setChecked(false);
        Q_EMIT SIG_VideoPause();
        Q_EMIT SIG_ScreenStart();
    }
    else{
        Q_EMIT SIG_ScreenPause();
    }
}


void RoomDialog::on_pb_max_clicked()
{
    if(!is_Max)
    {
        this->showFullScreen();
        is_Max=true;
    }
    else{
        this->showNormal();
        is_Max=false;
    }
}

void RoomDialog::on_pb_min_clicked()
{
    this->showMinimized();
}


void RoomDialog::on_cb_moji_currentIndexChanged(int index)
{

    Q_EMIT SIG_setMoji(index);
}

