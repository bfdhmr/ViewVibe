#include "audioread.h"
//数据量 40ms 640字节
//1s的数据量 640*25=16k
AudioRead::AudioRead(QObject *parent) : QObject(parent)
{
    //speex初始化
    speex_bits_init(&bits_enc);
    Enc_State=speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
    //设置压缩质量

    //设置质量为8(15kbps)
    int tmp=SPEEX_QUALITY;
    speex_encoder_ctl(Enc_State,SPEEX_SET_QUALITY,&tmp);
    //声卡采样格式 采样率8kHz，位宽16位，单声道

    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
       format.setSampleType(QAudioFormat::UnSignedInt);
       QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
       if (!info.isFormatSupported(format)) {
           QMessageBox::information(NULL , "提示", "打开音频设备失败");
           format = info.nearestFormat(format);
       }
       m_audio_in=NULL;
       m_buffer_in=NULL;
       m_timer=new QTimer;
       connect(m_timer,SIGNAL(timeout()),this,SLOT(ReadMore()));
       m_audioState=stopped;
#ifdef USE_VAD
      //vad创建
       if(0!=WebRtcVad_Create(&handle)){
           qDebug()<<"vad create failed";
       }
       //vad初始化
       if(0!=WebRtcVad_Init(handle))
       {
           qDebug()<<"vad init failed";
       }
#endif
}

AudioRead::~AudioRead()
{
    //delete Enc_State
    if(m_timer)
    {
        m_timer->stop();
        delete m_timer;
        m_timer=NULL;
    }
    if(m_audio_in)
    {
        m_audio_in->stop();
        delete m_audio_in;
        m_audio_in=NULL;
    }
#ifdef USE_VAD
    WebRtcVad_Free(handle);

#endif
}
//压缩率 640->76
void AudioRead::ReadMore()
{
#ifdef USE_SPEEX
    if (!m_audio_in)
            return;

        QByteArray m_buffer(2048,0);
        qint64 len = m_audio_in->bytesReady();
        if (len < 640)
        {
            return;
        }
        //从音频设备对应的缓冲区中读取 640字节
        qint64 l = m_buffer_in->read(m_buffer.data(), 640);
        qDebug() << "l sizes:"<< l ;
        QByteArray frame;
        //静音检测之后再编码
#ifdef USE_VAD
        char *bufferdata=(char*)m_buffer.data();
        //每次操作320个字节
        for(int i=0;i<640;i+=320)
        {
            char *tmp=bufferdata+i;
            //vad 返回值 -1错误 0静音 1有声
            if(0==WebRtcVad_Process(handle,8000,(int16_t*)tmp,160))
            {
                    //静音，清零空间
                    memset(tmp,0,320);
            }

        }

     #endif
        //speex编码
        char bytes[800] = {0};
            int i = 0;
            float input_frame1[320];
            char* pInData = (char*)m_buffer.data() ;
            char buf[800] = {0};
            memcpy( buf , pInData , 640);

            //speex 窄带模式 8kHz 每次编码 320字节 大端存储 数据
            //PCM 采集的是小端的
            int nbytes = 0;
            {
                //小端转化为大端->float
                short num = 0;
                for (i = 0;i < 160;i++)
                {
                    num = (uint8_t)buf[2 * i] | (((uint8_t)buf[2 * i + 1]) << 8);
                    input_frame1[i] = num;
                    //num = m_buffer[2 * i] | ((short)(m_buffer[2 * i + 1]) << 8);
                    //qDebug() << "float in" << num << input_frame1[i];
                }
                //压缩数据
                speex_bits_reset(&bits_enc);
                speex_encode(Enc_State,input_frame1,&bits_enc);
                nbytes = speex_bits_write(&bits_enc,bytes,800);
                frame.append(bytes,nbytes);
                //小端转化为大端
                for (i = 0;i < 160;i++)
                {
                    num = (uint8_t)buf[2 * i + 320] | (((uint8_t)buf[2 * i + 1 + 320]) << 8);
                    input_frame1[i] = num;
                }
                //压缩数据
                speex_bits_reset(&bits_enc);
                speex_encode(Enc_State,input_frame1,&bits_enc);
                nbytes = speex_bits_write(&bits_enc,bytes,800);
                frame.append(bytes,nbytes);
                qDebug() << "nbytes = " << frame.size();
            }

            //以信号的形式发送
            Q_EMIT SIG_audioFrame( frame );

//        frame.append(m_buffer.data(),640);

//        //以信号的形式发送
//        Q_EMIT SIG_audioFrame( frame );
#else
    if (!m_audio_in)
            return;

        QByteArray m_buffer(2048,0);
        qint64 len = m_audio_in->bytesReady();
        if (len < 640)
        {
            return;
        }
        //从音频设备对应的缓冲区中读取 640字节
        qint64 l = m_buffer_in->read(m_buffer.data(), 640);
        qDebug() << "l sizes:"<< l ;
        QByteArray frame;
        frame.append(m_buffer.data(),640);

        //以信号的形式发送
        Q_EMIT SIG_audioFrame( frame );
#endif
}

void AudioRead::start()
{
    if(m_audioState==stopped||m_audioState==pausing)
    {
        m_audio_in=new QAudioInput(format,this);
        //返回一个缓冲区地址给成员
        m_buffer_in=m_audio_in->start();//声音采集开始
        m_timer->start(1000/40);//不能超过40
        m_audioState=playing;
    }
}

void AudioRead::pause()
{
    if(m_audioState==playing)
    {
        m_timer->stop();
        if(m_audio_in)
        {
            m_audio_in->stop();
            delete m_audio_in;
            m_audio_in=NULL;
        }
        m_audioState=pausing;
    }
}
//Webrtc 致力于实现推流、开会等的开源项目
