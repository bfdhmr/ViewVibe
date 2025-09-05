#ifndef WORLD_H
#define WORLD_H
#include <QAudioInput>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include "speex/include/speex.h"
#define SPEEX_QUALITY (8)
#define USE_SPEEX (1)
//webrtc vad静音检测
#define USE_VAD (1)
#include "WebRtc_Vad/webrtc_vad.h"
enum ENUM_PLAY_STATE {stopped,playing,pausing};
#endif // WORLD_H
