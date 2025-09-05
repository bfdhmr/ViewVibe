# ViewVibe
Overview
ViewVibe is an audio and video chat software that enables users to quickly organize voice and video calls via room numbers. It supports one-click room creation, real-time video calls, screen sharing, and more. Designed for easy use, it lowers the barrier for friends to play games or watch competitions together with voice chat.
ViewVibe 是一款音视频聊天软件，用户可通过房间号快速组织语音、视频通话。支持一键创建房间、实时视频通话、屏幕共享等功能，旨在让好友们更便捷地连麦游戏或观看比赛，操作门槛低，打开即可使用。
Tech Stack
Client: Developed with QT on Windows. User login is supported, and personal information is encrypted via MD5 with salt during transmission.
Server: Implemented in C/C++ (code in myserver file), running on Ubuntu 16. It uses epoll and thread pool for efficient processing, and stores user information in MySQL database.
Video: Captures video data via OpenCV, integrates libfacedetect for face detection (to enable cute selfie feature), and transmits after encoding/decoding with ffmpeg.
Audio: Uses SDL for capture and playback, implements voice activity detection via WebRTC VAD, and uses Opus codec for network transmission adaptation.
客户端：基于 Windows 平台的 QT 开发。支持用户登录，个人信息通过 MD5 加盐方式加密传输。
服务端：基于 C/C++ 开发（代码位于 myserver 文件），运行于 Ubuntu 16 环境。通过 epoll 和线程池实现高效处理，用户信息存储于 MySQL 数据库。
视频：借助 OpenCV 采集视频数据，结合 libfacedetect 进行人脸检测（实现萌拍功能），经 ffmpeg 编解码后传输。
音频：采用 SDL 进行采集与播放，通过 WebRTC VAD 实现静音检测，最终使用 Opus 编码解码以适配网络传输需求。
Quick Start
Client (Windows)
Set up QT development environment on Windows.
Clone the repository and open the client project in QT Creator.
Build and run the client application.
客户端（Windows）
在 Windows 上搭建 QT 开发环境。
克隆代码仓库，在 QT Creator 中打开客户端项目。
构建并运行客户端应用程序。
Server (Ubuntu 16)
Locate the server code in the myserver file.
Install dependencies including MySQL, OpenCV, ffmpeg, SDL, etc.
Compile the server code and start the service.
服务端（Ubuntu 16）
在 myserver 文件中找到服务端代码。
安装 MySQL、OpenCV、ffmpeg、SDL 等依赖。
编译服务端代码并启动服务。
Features
One-click room creation and room number generation.
Real-time video call and screen sharing.
User login with MD5 salted encryption for personal information.
Efficient server processing with epoll and thread pool.
Video processing with OpenCV, libfacedetect (cute selfie), and ffmpeg.
Audio processing with SDL, WebRTC VAD (silence detection), and Opus codec.
一键创建房间并生成房间号。
实时视频通话、屏幕共享。
用户登录，个人信息采用 MD5 加盐加密。
服务端通过 epoll 和线程池实现高效处理。
视频处理集成 OpenCV、libfacedetect（萌拍）和 ffmpeg。
音频处理采用 SDL、WebRTC VAD（静音检测）和 Opus 编解码。
