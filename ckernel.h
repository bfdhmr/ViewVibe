#ifndef CKERNEL_H
#define CKERNEL_H
#include <QObject>
#include "wechatdialog.h"
#include "TcpClientMediator.h"
#include "packdef.h"
#include "logindialog.h"
#include <map>
#include "roomdialog.h"
//#include "audioread.h"
//#include "audiowrite.h"
#include "videoread.h"
#include "screenread.h"
#include "threadworker.h"
#include"sdlaudioread.h"
#include"sdlaudiowrite.h"
#include"videodecoder.h"
#include"videoencoder.h"
//协议映射表使用的类型
class Ckernel;
class SendVideoWorker;
typedef void (Ckernel::*PFUN)(uint sock,char*buf,int nlen);
class Ckernel : public QObject
{
    Q_OBJECT
public:
    explicit Ckernel(QObject *parent = nullptr);
    //单例
    static Ckernel * GetInstance()
    {
        static Ckernel kernel;
        return &kernel;
    }
signals:
    void SIG_SendVideo(char *buf,int nlen);
public slots:
    //设置协议映射关系
    void setNetPackMap();
    //初始化配置
    void initConfig();
    void slot_destroy();
    //网络信息处理
    void slot_dealData(uint sock,char* buf,int nlen);
    //登录回复
    void slot_dealloginRs(uint sock,char* buf,int nlen);
    //注册回复
    void slot_dealRegisterRs(uint sock,char* buf,int nlen);
    //创建房间回复
    void slot_dealCreateRoomRs(uint sock,char* buf,int nlen);
    //加入房间回复
    void slot_dealJoinRoomRs(uint sock,char* buf,int nlen);
    //房间成员请求处理
    void slot_dealRoomMemberRq(uint sock,char* buf,int nlen);
    //离开房间的请求处理
    void slot_dealLeaveRoomRq(uint sock,char* buf,int nlen);
    //音频帧处理
    void slot_dealAudioFrameRq(uint sock,char* buf,int nlen);
    //视频帧处理
    void slot_dealVideoFrameRq(uint sock,char* buf,int nlen);
    //发送登录信息
    void slot_loginCommit(QString,QString);
    //发送注册消息
    void slot_registerCommit(QString tel ,QString pass,QString name);
    //创建房间
    void slot_createRoom();
    //加入房间
    void slot_joinRoom();
    //退出房间
    void slot_quitRoom();
    //发送音频帧
    void slot_audioFrame(QByteArray ba);
    void slot_startAudio();
    void slot_pauseAudio();
    //发送视频帧
    void slot_sendVideoFrame(QImage img);
    void slot_pauseVideo();
    void slot_startVideo();
    //桌面
    void slot_openScreen();
    void slot_pauseScreen();
    //刷新图片显示
    void slot_refreshVideo(int id,QImage &img);
    //多线程发送视频
    void slot_sendVideo(char *buf,int nlen);

    //添加编解码
    // H.264视频帧处理
    void slot_dealVideoH264Rq(uint sock, char* buf, int nlen);
    // 编码后视频发送
    void slot_sendEncodedVideo(char* buf, int len);
    // 解码后视频显示
    void slot_showDecodedVideo(int userid, QImage image);
private:
    WeChatDialog* m_pWeChatDlg;
    INetMediator * m_pClient;
    LoginDialog *m_pLoginDlg;
    QString m_serverIP;
    QString m_name;
    RoomDialog* m_pRoomDialog;
    //协议映射表
    PFUN m_netPackMap[_DEF_PACK_COUNT];

    int m_id;
    int m_roomid;
    ///音频 一个采集，多个播放 每一个房间成员 1:1 map映射
//    AudioRead *m_pAudioRead;
//    std::map<int,AudioWrite*> m_mapIDToAudioWrite;
    ///更换为SDL
    SDLAudioRead *m_pAudioRead;
    std::map<int,SDLAudioWrite*> m_mapIDToAudioWrite;
    //视频采集
    VideoRead *m_pVudioRead;
    //桌面采集
    ScreenRead* m_screenRead;

    QSharedPointer<SendVideoWorker> m_pSendVideoWorker;

    enum client_type{
        audio_client=0,video_client=1
    };

    INetMediator* m_pAVClient[2];


    /////////////////////////////////////
    /// H.264编解码
    ///
    VideoEncoder* m_videoEncoder;           // 摄像头编码器
    VideoEncoder* m_screenEncoder;          // 桌面编码器
    std::map<int, VideoDecoder*> m_mapIDToDecoder; // 用户ID到解码器的映射


};
class SendVideoWorker :public ThreadWorker{
    Q_OBJECT
public slots:
    void slot_sendVideo(char *buf,int nlen){
        Ckernel::GetInstance()->slot_sendVideo(buf,nlen);
    }
};
#endif // CKERNEL_H
