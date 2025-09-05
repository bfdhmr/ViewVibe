#ifndef ROOMDIALOG_H
#define ROOMDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include "usershow.h"
#include "screenread.h"
namespace Ui {
class RoomDialog;
}

class RoomDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    void SIG_AudioPause();
    void SIG_AudioStart();
    void SIG_VideoStart();
    void SIG_VideoPause();
    void SIG_ScreenStart();
    void SIG_ScreenPause();
    void SIG_setMoji(int Moji);
public:
    explicit RoomDialog(QWidget *parent = nullptr);
    ~RoomDialog();

public slots:
    void slot_setInfo(QString roomid);
    void slot_addUserShow(UserShow *user );
    void slot_refreshUser(int id,QImage&img);
    void slot_removeUserShow(UserShow *user);
    void slot_removeUserShow(int id);
    void slot_setAudioCheck(bool check);
    void slot_clearUserShow();
    void slot_setVideoCheck(bool check);
    void slot_ClickedScreen(int id,QString name);
    void slot_setScreenCheck(bool check);
private slots:
    void on_pb_close_clicked();

    void on_pb_quit_clicked();

     void closeEvent(QCloseEvent *event);
     void on_cb_audio_clicked();

     void on_cb_video_clicked();

     void on_cb_desk_clicked();

     void on_pb_max_clicked();

     void on_pb_min_clicked();

     void on_cb_moji_currentIndexChanged(int index);

private:
    Ui::RoomDialog *ui;
    QVBoxLayout *m_mainLayout;
    std::map<int,UserShow*> m_mapIDToUserShow;
    bool is_Max;
};

#endif // ROOMDIALOG_H
