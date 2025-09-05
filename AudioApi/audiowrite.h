#ifndef AUDIOWRITE_H
#define AUDIOWRITE_H

#include <QObject>
#include "world.h"
class AudioWrite : public QObject
{
    Q_OBJECT
public:
    explicit AudioWrite(QObject *parent = nullptr);
    ~AudioWrite();
public slots:
    void slot_playAudio(QByteArray bt);
signals:

private:
    QAudioOutput *m_audio_out;
    QIODevice *m_buffer_out;
    QAudioFormat format;

    //speex 相关全局变量
    SpeexBits bits_dec;
    void *Dec_State;
};

#endif // AUDIOWRITE_H
