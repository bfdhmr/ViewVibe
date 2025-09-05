#include "ckernel.h"
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFileInfo>
#include <QTime>
#include "md5.h"
#include <QBuffer>
#define NetPackMap(a) m_netPackMap[a-_DEF_PACK_BASE]
//添加协议映射
void Ckernel::setNetPackMap()
{
    memset(m_netPackMap,0,sizeof(m_netPackMap));
    //m_netPackMap[_DEF_PACK_LOGIN_RS-_DEF_PACK_BASE]=&Ckernel::slot_dealloginRs;
    NetPackMap(_DEF_PACK_LOGIN_RS)=&Ckernel::slot_dealloginRs;
    NetPackMap(_DEF_PACK_REGISTER_RS)=&Ckernel::slot_dealRegisterRs;
    NetPackMap(DEF_PACK_CREATEROOM_RS)=&Ckernel::slot_dealCreateRoomRs;
    NetPackMap(DEF_PACK_JOINROOM_RS)=&Ckernel::slot_dealJoinRoomRs;
    NetPackMap(DEF_PACK_ROOM_MEMBER)=&Ckernel::slot_dealRoomMemberRq;
    NetPackMap(DEF_PACK_LEAVEROOM_RQ)=&Ckernel::slot_dealLeaveRoomRq;
    NetPackMap(DEF_PACK_AUDIO_FRAME)=&Ckernel::slot_dealAudioFrameRq;
    NetPackMap(DEF_PACK_VIDEO_FRAME)=&Ckernel::slot_dealVideoFrameRq;
    NetPackMap(_DEF_PACK_VIDEO_H264) = &Ckernel::slot_dealVideoH264Rq;
}
//初始化配置
void Ckernel::initConfig()
{
    m_serverIP=_DEF_SERVERIP;
    //路径设置 exe同级目录下->applicationDirPath config.ini
    QString path=QApplication::applicationDirPath()+"/config.ini";
    //判断是否存在
    QFileInfo info(path);
    QSettings settings(path,QSettings::IniFormat,NULL);//有打开，没有创建
    if(info.exists())
    {
        //加载配置文件，ip设置为配置文件里的
        //移动到IP组
        settings.beginGroup("Net");
        //读取ip->addr->赋值
        QVariant ip=settings.value("ip");
        QString strIP=ip.toString();
        if(!strIP.isEmpty())
        {
            m_serverIP=strIP;

        }
        //结束
        settings.endGroup();
    }
    else
    {
        //没有配置文件，写入默认ip
        settings.beginGroup("Net");
        settings.setValue("ip",m_serverIP);
        settings.endGroup();
    }
    qDebug()<<m_serverIP;
}
Ckernel::Ckernel(QObject *parent) : QObject(parent),m_id(0),m_roomid(0)
{
    qDebug()<<"main thread"<<QThread::currentThreadId();
    setNetPackMap();
    initConfig();

    //添加网络
    m_pClient    = new TcpClientMediator;
    m_pClient->OpenNet(m_serverIP.toStdString().c_str());
    connect(m_pClient,SIGNAL(SIG_ReadyData(unsigned int, char*, int)),this,SLOT(slot_dealData( unsigned int, char*, int)));
    //音频和视频的链接
    for(int i=0;i<2;i++)
    {
        m_pAVClient[i]=new TcpClientMediator;
        m_pAVClient[i]->OpenNet(_DEF_SERVERIP,_DEF_PORT);
        connect(m_pAVClient[i],SIGNAL(SIG_ReadyData(unsigned int, char*, int)),this,SLOT(slot_dealData( unsigned int, char*, int)));
    }
    m_pWeChatDlg=new WeChatDialog;
    connect(m_pWeChatDlg,SIGNAL(SIG_close()),this,SLOT(slot_destroy()));
    connect(m_pWeChatDlg,SIGNAL(SIG_createRoom()),this,SLOT(slot_createRoom()));
    connect(m_pWeChatDlg,SIGNAL(SIG_joinRoom()),this,SLOT(slot_joinRoom()));
    //m_pWeChatDlg->show();
    m_pLoginDlg=new LoginDialog;
    connect(m_pLoginDlg,SIGNAL(SIG_loginCommit(QString,QString)
            ),this,SLOT(slot_loginCommit(QString,QString)));
    connect(m_pLoginDlg,SIGNAL(SIG_close()),this,SLOT(slot_destroy()));
    connect(m_pLoginDlg,SIGNAL(SIG_registerCommit(QString ,QString ,QString )),this,SLOT(slot_registerCommit(QString ,QString ,QString)));

    m_pRoomDialog=new RoomDialog;
    connect(m_pRoomDialog,SIGNAL(SIG_VideoPause()),this,SLOT(slot_pauseVideo()));
    connect(m_pRoomDialog,SIGNAL(SIG_VideoStart()),this,SLOT(slot_startVideo()));
    connect(m_pRoomDialog,SIGNAL(SIG_close()),this,SLOT(slot_quitRoom()));
    connect(m_pRoomDialog,SIGNAL(SIG_AudioPause()),this,SLOT(slot_pauseAudio()));
    connect(m_pRoomDialog,SIGNAL(SIG_AudioStart()),this,SLOT(slot_startAudio()));
    connect(m_pRoomDialog,SIGNAL(SIG_ScreenStart()),this,SLOT(slot_openScreen()));
    connect(m_pRoomDialog,SIGNAL(SIG_ScreenPause()),this,SLOT(slot_pauseScreen()));
    //m_pAudioRead=new AudioRead;
    m_pAudioRead=new SDLAudioRead;
     connect(m_pAudioRead,SIGNAL(SIG_sendAudioFrame(QByteArray)),this,SLOT(slot_audioFrame(QByteArray)));
    m_pLoginDlg->show();
    m_pVudioRead=new VideoRead;
    connect(m_pVudioRead,SIGNAL(SIG_sendVideoFrame(QImage)),this,SLOT(slot_sendVideoFrame(QImage)));
    m_screenRead=new ScreenRead;
    connect(m_screenRead,SIGNAL(SIG_getScreenFrame(QImage)),this,SLOT(slot_sendVideoFrame(QImage)));
    m_pSendVideoWorker=QSharedPointer<SendVideoWorker>(new SendVideoWorker);
    connect(this,SIGNAL(SIG_SendVideo(char*,int)),m_pSendVideoWorker.data(),SLOT(slot_sendVideo(char*,int)));
    //设置萌拍效果
    connect(m_pRoomDialog,SIGNAL(SIG_setMoji(int)),m_pVudioRead,SLOT(slot_setMoji(int)));

    //添加编解码
    // 在构造函数中添加
    // 初始化H.264编码器
    m_videoEncoder = new VideoEncoder;
    m_videoEncoder->init(IMAGE_WIDTH, IMAGE_HEIGHT, FRAME_RATE, 400000);
    connect(m_videoEncoder, SIGNAL(SIG_sendVideoPacket(char*,int)),
            this, SLOT(slot_sendEncodedVideo(char*,int)));
    m_videoEncoder->start();

    // 桌面编码器使用实际分辨率
    QScreen* src = QApplication::primaryScreen();
    QPixmap map = src->grabWindow(QApplication::desktop()->winId());
    m_screenEncoder = new VideoEncoder;
    m_screenEncoder->init(map.width(), map.height(), FRAME_RATE, 1000000);
    connect(m_screenEncoder, SIGNAL(SIG_sendVideoPacket(char*,int)),
            this, SLOT(slot_sendEncodedVideo(char*,int)));
    m_screenEncoder->start();
}


//回收
void Ckernel::slot_destroy()
{
    qDebug()<<__func__;
    if(m_pWeChatDlg)
    {
        m_pWeChatDlg->hide();
        delete m_pWeChatDlg;
        m_pWeChatDlg=nullptr;
    }
    if(m_pLoginDlg)
    {
        m_pLoginDlg->hide();
        delete m_pLoginDlg;
        m_pLoginDlg=nullptr;
    }
    if(m_pAudioRead)
    {
        //m_pAudioRead->pause();
        m_pAudioRead->slot_closeAudio();
        delete m_pAudioRead;
        m_pAudioRead=NULL;
    }
    if(m_pRoomDialog)
    {
        m_pRoomDialog->hide();
        delete m_pRoomDialog;
        m_pRoomDialog=NULL;
    }
    if(m_pClient)
    {
        m_pClient->CloseNet();
        delete m_pClient;
        m_pClient=NULL;
    }
    if(m_pVudioRead)
    {
        m_pVudioRead->slot_closeVideo();
        delete m_pVudioRead;
        m_pVudioRead=NULL;
    }
    exit(0);
}
//网络处理
void Ckernel::slot_dealData(uint sock, char *buf, int nlen)
{
    int type=*(int*)buf;
    if(type>=_DEF_PACK_BASE&&type<_DEF_PACK_BASE+_DEF_PACK_COUNT)
    {
        //取得协议头，根据协议映射关系找到函数指针
        PFUN pf=NetPackMap(type);
        if(pf)
        {
            (this->*pf)(sock,buf,nlen);
        }
    }
    delete[] buf;
}

void Ckernel::slot_dealloginRs(uint sock, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_LOGIN_RS* rs=(STRU_LOGIN_RS*)buf;
    //根据不同结果，弹出不同弹窗
    switch(rs->result)
    {
        case user_not_exist:
    {
        QMessageBox::about(m_pLoginDlg,"提示","用户不存在");
        break;
    }
    case password_error:
    {
        QMessageBox::about(m_pLoginDlg,"提示","密码错误");
        break;
    }
    case login_success:
    {
        QString strName=QString("用户[%1]登录成功").arg(rs->m_name);
        QMessageBox::about(m_pLoginDlg,"提示",strName);
        //id记录
        m_name=QString::fromStdString(rs->m_name);
        m_id=rs->userid;
        m_pWeChatDlg->setInfo(m_name);
        //ui跳转
        m_pLoginDlg->hide();
        m_pWeChatDlg->showNormal();

        //注册音频和视频fd
        STRU_AUDIO_REGISTER rq_audio;
        rq_audio.m_userid=m_id;
        STRU_VIDEO_REGISTER rq_video;
        rq_video.m_userid=m_id;

        m_pAVClient[audio_client]->SendData(0,(char*)&rq_audio,sizeof(rq_audio));
        m_pAVClient[video_client]->SendData(0,(char*)&rq_video,sizeof(rq_video));
        break;
    }
    }
}

void Ckernel::slot_dealRegisterRs(uint sock, char *buf, int nlen)
{
    //拆包
    STRU_REGISTER_RS  *rs=(STRU_REGISTER_RS*)buf;
    //根据不同结果，弹出不同提示窗
    switch (rs->result) {
    case tel_is_exist:
    {
        QMessageBox::about(m_pLoginDlg,"提示","电话号码已存在");

    }
    break;
    case name_is_exist:
    {
        QMessageBox::about(m_pLoginDlg,"提示","姓名已存在");
    }
    break;
    case register_success:
    {
        QMessageBox::about(m_pLoginDlg,"提示","注册成功");
    }
    break;
    }
}
//创建房间回复处理
void Ckernel::slot_dealCreateRoomRs(uint sock, char *buf, int nlen)
{
    //拆包
    STRU_CREATEROOM_RS* rs=(STRU_CREATEROOM_RS*)buf;
    //房间号 显示到界面 跳转
    m_pRoomDialog->slot_setInfo(QString::number(rs->m_RoomId));
    //服务器没有发个人信息给你，你作为第一个进入房间的，可以把个人信息放到房间
    UserShow *user=new UserShow;
    connect(user,SIGNAL(SIG_itemClicked(int,QString)),m_pRoomDialog,SLOT(slot_ClickedScreen(int,QString)));
    user->slot_setInfo(m_id,m_name);
    m_pRoomDialog->slot_addUserShow(user);
    m_roomid=rs->m_RoomId;
    m_pRoomDialog->showNormal();
    //音频初始化
    m_pRoomDialog->slot_setAudioCheck(false);

    //视频初始化
    m_pRoomDialog->slot_setVideoCheck(false);
}

void Ckernel::slot_dealJoinRoomRs(uint sock, char *buf, int nlen)
{
    //拆包
    STRU_JOINROOM_RS* rs=(STRU_JOINROOM_RS*)buf;
    //根据结果显示不同
    if(rs->m_lResult==0)
    {
        QMessageBox::about(m_pWeChatDlg,"提示","房间id不存在");
        return;
    }
    //成功
    //房间号 显示到界面 跳转
    m_pRoomDialog->slot_setInfo(QString::number(rs->m_RoomID));
    //跳转 roomid设置
    m_roomid=rs->m_RoomID;
    m_pRoomDialog->showNormal();
    //音频初始化
    m_pRoomDialog->slot_setAudioCheck(false);

    //视频初始化
    m_pRoomDialog->slot_setVideoCheck(false);
}
//房间成员请求处理
void Ckernel::slot_dealRoomMemberRq(uint sock, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_ROOM_MEMBER_RQ* rq=(STRU_ROOM_MEMBER_RQ*)buf;

    //创建用户对应控件
    UserShow *user=new UserShow;
    connect(user,SIGNAL(SIG_itemClicked(int,QString)),m_pRoomDialog,SLOT(slot_ClickedScreen(int,QString)));

    user->slot_setInfo(rq->m_UserID,QString::fromStdString(rq->m_szUser));
    m_pRoomDialog->slot_addUserShow(user);

    //音频的内容
    SDLAudioWrite *aw=NULL;
    //为每个人创建播放音频的对象
    if(m_mapIDToAudioWrite.count(rq->m_UserID)==0)
    {
        aw=new SDLAudioWrite;
        m_mapIDToAudioWrite[rq->m_UserID]=aw;
}

}

void Ckernel::slot_dealLeaveRoomRq(uint sock, char *buf, int nlen)
{
    //拆包
    STRU_LEAVEROOM_RQ *rq=(STRU_LEAVEROOM_RQ*)buf;
    //把这个人从ui上面去掉
    if(rq->m_RoomId==m_roomid)
    {
        m_pRoomDialog->slot_removeUserShow(rq->m_nUserId);
    }
    //去掉对应的音频
    if(m_mapIDToAudioWrite.count(rq->m_nUserId)>0)
    {
        SDLAudioWrite *pAw=m_mapIDToAudioWrite[rq->m_nUserId];
        m_mapIDToAudioWrite.erase(rq->m_nUserId);
        delete pAw;
    }
}
//音频帧处理
void Ckernel::slot_dealAudioFrameRq(uint sock, char *buf, int nlen)
{
    //拆包
    //反序列化
    int userid;
    int roomId;

    char *tmp=buf;
    tmp+=sizeof(int);

    userid=*(int*) tmp;//按照整型取
    tmp+=sizeof(int);

   roomId= *(int*) tmp;
    tmp+=sizeof(int);

    //跳过时间
    tmp+=sizeof(int);


    tmp+=sizeof(int);


    tmp+=sizeof(int);
    int nbufLen=nlen-6*sizeof(int);
    QByteArray ba(tmp,nbufLen);
    if(m_roomid==roomId)
    {
        if(m_mapIDToAudioWrite.count(userid)>0)
        {
            SDLAudioWrite*aw=m_mapIDToAudioWrite[userid];
            aw->slot_playAudioFrame(ba);
        }
    }
}
//视频帧处理
void Ckernel::slot_dealVideoFrameRq(uint sock, char *buf, int nlen)
{
    //拆包
    char *tmp=buf;
    tmp+=sizeof(int);
    int userid=*(int *)tmp;
    tmp+=sizeof(int);
    int roomid=*(int*)tmp;
    tmp+=sizeof(int)*4;
    int datalen=nlen-6*sizeof(int);
    QByteArray bt(tmp,datalen);
    QImage img;
    img.loadFromData(bt);
    if(m_roomid==roomid)
    m_pRoomDialog->slot_refreshUser(userid,img);
}
#define MD5_KEY (1234)
static std::string GetMD5(QString value){
    QString str =QString("%1_%2").arg(value).arg(MD5_KEY);
    std::string strSrc=str.toStdString();
    MD5 md5(strSrc);
    return md5.toString();
}
//提交登录信息
void Ckernel::slot_loginCommit(QString tel, QString pass)
{
    std::string strTel=tel.toStdString();
    //std::string strPass=pass.toStdString();
    STRU_LOGIN_RQ rq;
    strcpy(rq.tel,strTel.c_str());
    std::string strPass=GetMD5(pass);
    qDebug()<<strPass.c_str()<<endl;
    strcpy(rq.password,strPass.c_str());
    //int nlen=strlen(rq.password);
    //MD5 md5(rq.password,nlen);
    qDebug()<<
    m_pClient->SendData(0,(char*)&rq,sizeof(rq));
}
//发送注册消息
void Ckernel::slot_registerCommit(QString tel, QString pass, QString name)
{
    std::string strTel =tel.toStdString();
    //中文
    std::string strName=name.toStdString(); //格式 utf8
    STRU_REGISTER_RQ rq;
    strcpy(rq.tel,strTel.c_str());
    std::string strPass=GetMD5(pass);
    qDebug()<<strPass.c_str()<<endl;
    //兼容utf8 QString->std::string ->char*
    strcpy(rq.name,strName.c_str());
    strcpy(rq.password,strPass.c_str());
    m_pClient->SendData(0,(char*)&rq,sizeof(rq));

}
//创建房间
void Ckernel::slot_createRoom()
{
    //先判断是否在房间内
    if(m_roomid!=0)
    {
        QMessageBox::about(m_pWeChatDlg,"提示","已经在房间内，无法创建,请先退出");
        return;
    }
    //发命令 创建房间
    STRU_CREATEROOM_RQ rq;
    rq.m_UserID=m_id;
    m_pClient->SendData(0,(char*)&rq,sizeof(rq));
}
#include <QInputDialog>
#include <QRegExp>
//加入房间
void Ckernel::slot_joinRoom()
{
    //先判断是否在房间内
    if(m_roomid!=0)
    {
        QMessageBox::about(m_pWeChatDlg,"提示","已经在房间内，无法加入,请先退出");
        return;
    }
    //弹窗口 填房间号
    QString strRoom=QInputDialog::getText(m_pWeChatDlg,"加入房间","请输入房间号");
    //合理化判断
    QRegExp exp("[0-9]\{1,8\}$");
    if(!exp.exactMatch(strRoom))
    {
        QMessageBox::about(m_pWeChatDlg,"提示","房间号输入不合法,长度不超过8的数字");
        return;
    }
    qDebug()<<strRoom;
    //发命令 加入房间
    STRU_JOINROOM_RQ rq;
    rq.m_UserID=m_id;
    rq.m_RoomID=strRoom.toInt();
    m_pClient->SendData(0,(char*)&rq,sizeof(rq));
}

void Ckernel::slot_quitRoom()
{

    //发退出包
    STRU_LEAVEROOM_RQ rq;
    rq.m_nUserId=m_id;
    rq.m_RoomId=m_roomid;
    std::string name=m_name.toStdString();
    strcpy(rq.szUserName,name.c_str());
    m_pClient->SendData(0,(char*)&rq,sizeof(rq));
    //关闭视频 音频
    m_pVudioRead->slot_closeVideo();
    //m_pAudioRead->pause();
    m_pAudioRead->slot_closeAudio();
    m_screenRead->slot_closeVideo();
    m_pRoomDialog->slot_setVideoCheck(false);
    m_pRoomDialog->slot_setAudioCheck(false);
    m_pRoomDialog->slot_setScreenCheck(false);
    //回收所有人的audiowrite
    for(auto ite =m_mapIDToAudioWrite.begin();ite!=m_mapIDToAudioWrite.end();)
    {
        SDLAudioWrite *pWrite=ite->second;
        ite=m_mapIDToAudioWrite.erase(ite);
        delete pWrite;
    }
    //回收资源
    m_pRoomDialog->slot_clearUserShow();
    m_roomid=0;
}
//发送音频帧
void Ckernel::slot_audioFrame(QByteArray ba)
{
    int nPackSize=6*sizeof(int)+ba.size();
    char *buf=new char[nPackSize];
    char *tmp=buf;
    //序列化
    int type=DEF_PACK_AUDIO_FRAME;
    int userid=m_id;
    int roomId=m_roomid;
    QTime tm=QTime::currentTime();
    int min=tm.minute();
    int sec=tm.second();
    int msec=tm.msec();
    *(int*) tmp=type;
    tmp+=sizeof(int);

    *(int*) tmp=userid;//按照整型存
    tmp+=sizeof(int);

    *(int*) tmp=roomId;
    tmp+=sizeof(int);

    *(int*) tmp=min;
    tmp+=sizeof(int);

    *(int*) tmp=sec;
    tmp+=sizeof(int);

    *(int*) tmp=msec;
    tmp+=sizeof(int);

    memcpy(tmp,ba.data(),ba.size());
    //m_pClient->SendData(0,buf,nPackSize);
    m_pAVClient[audio_client]->SendData(0,buf,nPackSize);
    delete []buf;

}

void Ckernel::slot_startAudio()
{
    //m_pAudioRead->start();
    m_pAudioRead->slot_openAudio();
}

void Ckernel::slot_pauseAudio()
{
    //m_pAudioRead->pause();
    m_pAudioRead->slot_closeAudio();
}

void Ckernel::slot_sendVideoFrame(QImage img)
{
    //显示图片
    slot_refreshVideo(m_id,img);
    // 判断是摄像头还是桌面
    VideoEncoder* encoder = nullptr;
    if(img.width() == IMAGE_WIDTH && img.height() == IMAGE_HEIGHT) {
        encoder = m_videoEncoder;
    } else {
        encoder = m_screenEncoder;
    }

    // 添加到编码队列
    if(encoder) {
        encoder->addFrame(m_id, m_roomid, img);
    }
//    //压缩
//    //压缩图片从 RGB24 格式压缩到 JPEG 格式, 发送出去
//    QByteArray ba;
//    QBuffer qbuf(&ba); // QBuffer 与 QByteArray 字节数组联立联系
//    img.save( &qbuf , "JPEG" , 50 ); //将图片的数据写入 ba
//    //使用 ba 对象, 可以获取图片对应的缓冲区
//     // 可以使用 ba.data() , ba.size()将缓冲区发送出
//    //写视频帧，发送
//    int nPackSize=6*sizeof(int)+ba.size();
//    char *buf=new char[nPackSize];
//    char *tmp=buf;
//    *(int*)tmp=DEF_PACK_VIDEO_FRAME;
//    tmp+=sizeof(int);
//    *(int*)tmp=m_id;
//    tmp+=sizeof(int);
//    *(int*)tmp=m_roomid;
//    tmp+=sizeof(int);

//    //延迟过久舍弃一些帧的参考时间
//    QTime tm=QTime::currentTime();
//    *(int*)tmp=tm.minute();
//    tmp+=sizeof(int);
//    *(int*)tmp=tm.second();
//    tmp+=sizeof(int);
//    *(int*)tmp=tm.msec();
//    tmp+=sizeof(int);
//    memcpy(tmp,ba.data(),ba.size());
//    ///发送是一个阻塞函数，如果服务器接收缓冲区由于数据量很大，没有及时处理完缓冲区数据
//    ///滑动窗口会变少，send函数阻塞使得用户界面响应变慢，出现未响应情况
//    // 将视频发送变成一个信号。放到另一个线程执行
//    //m_pClient->SendData(0,buf,nPackSize);
//    //delete []buf;
//    Q_EMIT SIG_SendVideo(buf,nPackSize);
}
//关闭视频
void Ckernel::slot_pauseVideo()
{
    m_pVudioRead->slot_closeVideo();

    if(m_roomid != 0) {
        // 发送一个特殊的包告诉其他人你停止了视频
        STRU_VIDEO_H264 stopPack;
        stopPack.type = _DEF_PACK_VIDEO_H264;
        stopPack.userid = m_id;
        stopPack.roomid = m_roomid;
        stopPack.width = 0;  // width=0 表示停止
        stopPack.height = 0;
        stopPack.pts = QDateTime::currentMSecsSinceEpoch();
        stopPack.dataLen = 0;

        m_pAVClient[video_client]->SendData(0, (char*)&stopPack, sizeof(stopPack));
    }
}
//开启视频
void Ckernel::slot_startVideo()
{
    m_pVudioRead->slot_openVideo();
}

void Ckernel::slot_openScreen()
{
    m_screenRead->slot_openVideo();
}

void Ckernel::slot_pauseScreen()
{
    m_screenRead->slot_closeVideo();
    if(m_roomid != 0) {
        // 发送一个特殊的包告诉其他人你停止了视频
        STRU_VIDEO_H264 stopPack;
        stopPack.type = _DEF_PACK_VIDEO_H264;
        stopPack.userid = m_id;
        stopPack.roomid = m_roomid;
        stopPack.width = 0;  // width=0 表示停止
        stopPack.height = 0;
        stopPack.pts = QDateTime::currentMSecsSinceEpoch();
        stopPack.dataLen = 0;

        m_pAVClient[video_client]->SendData(0, (char*)&stopPack, sizeof(stopPack));
    }
}

void Ckernel::slot_refreshVideo(int id,QImage &img)
{
    m_pRoomDialog->slot_refreshUser(id,img);
}
//多线程发送视频
void Ckernel::slot_sendVideo(char *buf, int nlen)
{
    char *tmp=buf;
    tmp+=sizeof(int);
    tmp+=sizeof(int);
    tmp+=sizeof(int);
    int min,sec,msec;
    min=*(int*)tmp;
    tmp+=sizeof(int);
    sec=*(int*)tmp;
    tmp+=sizeof(int);
    msec=*(int*)tmp;
    tmp+=sizeof(int);
    //当前时间
    QTime ctm=QTime::currentTime();
    //数据包时间
    QTime tm(ctm.hour(),min,sec,msec);

    //发送数据包延迟超过300，舍弃
    if(tm.msecsTo((ctm))>300)
    {
        qDebug()<<"Video send fail";
        delete[] buf;
        return;
    }
    //m_pClient->SendData(0,buf,nlen);
    m_pAVClient[video_client]->SendData(0,buf,nlen);
    delete []buf;
}


void Ckernel::slot_dealVideoH264Rq(uint sock, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_VIDEO_H264* pack = (STRU_VIDEO_H264*)buf;

    if(pack->roomid != m_roomid) return;

    if(pack->width == 0 && pack->height == 0) {
        // 这是一个停止信号，清理该用户的解码器
        if(m_mapIDToDecoder.count(pack->userid) > 0) {
            VideoDecoder* decoder = m_mapIDToDecoder[pack->userid];
            m_mapIDToDecoder.erase(pack->userid);
            decoder->stop();
            delete decoder;
            qDebug() << "清理用户" << pack->userid << "的解码器";
        }
        return;
    }

    // 获取或创建解码器
    VideoDecoder* decoder = nullptr;
    if(m_mapIDToDecoder.count(pack->userid) == 0) {
        decoder = new VideoDecoder(pack->userid);
        decoder->init(pack->width, pack->height);

        int userid = pack->userid;

//        // 如果有同步管理器，连接到同步管理器
//        if(m_mapIDToSyncManager.count(userid) > 0) {
//            qDebug() << "为用户" << userid << "连接解码器到同步管理器";

//            connect(decoder, &VideoDecoder::SIG_frameDecoded,
//                    this, [this, userid](int uid, const QImage& image, qint64 timestamp) {
//                qDebug() << "Lambda被调用 - uid:" << uid << "userid:" << userid << "timestamp:" << timestamp;
//                if(m_mapIDToSyncManager.count(uid) > 0) {
//                    qDebug() << "添加视频帧到同步管理器 - 用户:" << uid;
//                    m_mapIDToSyncManager[uid]->addVideoFrame(static_cast<int64_t>(timestamp), image);
//                } else {
//                    qDebug() << "找不到用户" << uid << "的同步管理器！";
//                }
//            });
//        } else {
//            qDebug() << "用户" << userid << "没有同步管理器，直接显示";
            connect(decoder, &VideoDecoder::SIG_frameDecoded,
                    this, [this](int uid, const QImage& image, int64_t timestamp) {
                        m_pRoomDialog->slot_refreshUser(uid, const_cast<QImage&>(image));
                    });
//        }

        m_mapIDToDecoder[pack->userid] = decoder;
    } else {
        decoder = m_mapIDToDecoder[pack->userid];

        // 检查分辨率是否改变（这是关键修复）
        if(decoder && (pack->width != decoder->getWidth() || pack->height != decoder->getHeight())) {
            qDebug() << "用户" << pack->userid << "的视频分辨率改变，重新初始化解码器";

            // 停止并删除旧解码器
            decoder->stop();
            delete decoder;
            m_mapIDToDecoder.erase(pack->userid);

            // 创建新解码器
            decoder = new VideoDecoder(pack->userid);
            decoder->init(pack->width, pack->height);

            int userid = pack->userid;
//            // 重新连接信号
//            if(m_mapIDToSyncManager.count(userid) > 0) {
//                connect(decoder, &VideoDecoder::SIG_frameDecoded,
//                        this, [this, userid](int uid, const QImage& image, qint64 timestamp) {
//                    if(m_mapIDToSyncManager.count(uid) > 0) {
//                        m_mapIDToSyncManager[uid]->addVideoFrame(static_cast<int64_t>(timestamp), image);
//                    }
//                });
//            } else {
                connect(decoder, &VideoDecoder::SIG_frameDecoded,
                        this, [this](int uid, const QImage& image, int64_t timestamp) {
                            m_pRoomDialog->slot_refreshUser(uid, const_cast<QImage&>(image));
                        });
//            }

            m_mapIDToDecoder[pack->userid] = decoder;
        }
    }

    // 添加数据包到解码队列（传递时间戳）
    if(decoder) {
        decoder->addPacket(pack->data, pack->dataLen, pack->pts);
    }

}

void Ckernel::slot_sendEncodedVideo(char *buf, int len)
{
    //更新时间戳为当前系统时间
    STRU_VIDEO_H264* pack = (STRU_VIDEO_H264*)buf;
    pack->pts = QDateTime::currentMSecsSinceEpoch();

    m_pAVClient[video_client]->SendData(0, buf, len);
    delete[] buf;

}

void Ckernel::slot_showDecodedVideo(int userid, QImage image)
{
     m_pRoomDialog->slot_refreshUser(userid, image);
}

