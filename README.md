# ViewVibe
# ViewVibe - Audio and Video Chat Software

## Overview
ViewVibe is an audio and video chat software that allows users to quickly organize voice and video calls through room numbers. It supports one-click room creation, real-time video calls, screen sharing, and other features. It aims to make it more convenient for multiple friends to play games or watch competitions together with voice chat, and it is easy to use with a low operation threshold.

## Tech Stack
- **Client**: Developed with QT on Windows. It supports login, and personal information is encrypted and transmitted via MD5 with salt.
- **Server**: Developed with C/C++ (code in `myserver` file) and runs on Ubuntu 16. It uses `epoll` and thread pool for efficient processing, and user information is stored in MySQL database.
- **Video**: Video data is captured via OpenCV, combined with `libfacedetect` for face detection to realize the cute selfie feature, and transmitted after encoding and decoding by `ffmpeg`.
- **Audio**: SDL is used for collection and playback, WebRTC VAD is used for silence detection, and finally Opus encoding and decoding is used to meet network transmission requirements.

## development environment
### Client (Windows)
1. Set up the QT development environment on Windows.
2. Clone the repository and open the client project in QT Creator.
3. Build and run the client application.

### Server (Ubuntu 16)
1. Locate the server code in the `myserver` file.
2. Install dependencies such as MySQL, OpenCV, ffmpeg, and SDL.
3. Compile the server code and start the service.

## Features
- One-click room creation and room number generation.
- Real-time video calls and screen sharing.
- User login with MD5 salted encryption for personal information transmission.
- Efficient server processing with `epoll` and thread pool.
- Video processing with OpenCV, `libfacedetect` (cute selfie), and `ffmpeg`.
- Audio processing with SDL, WebRTC VAD (silence detection), and Opus codec.


---

# ViewVibe - 音视频聊天软件

## 概述
ViewVibe 是一款音视频聊天软件，用户可通过房间号快速组织语音、视频通话。支持一键创建房间、实时视频通话、屏幕共享等功能，旨在让多个好友更便捷地连麦游戏或观看比赛，操作门槛低，打开即可使用。

## 技术栈
- **客户端**：基于 Windows 平台的 QT 开发，支持用户登录，个人信息通过 MD5 加盐方式加密传输。
- **服务端**：基于 C/C++ 开发（代码位于 `myserver` 文件），运行于 Ubuntu 16 环境，通过 `epoll` 和线程池实现高效处理，用户信息存储于 MySQL 数据库。
- **视频**：通过 OpenCV 采集视频数据，结合 `libfacedetect` 进行人脸检测以实现萌拍功能，经 `ffmpeg` 编解码后传输。
- **音频**：采用 SDL 进行采集与播放，通过 WebRTC VAD 实现静音检测，最终使用 Opus 编码解码以适配网络传输需求。

## 开发环境
### 客户端（Windows）
1. 在 Windows 上搭建 QT 开发环境。
2. 克隆代码仓库，在 QT Creator 中打开客户端项目。
3. 构建并运行客户端应用程序。

### 服务端（Ubuntu 16）
1. 在 `myserver` 文件中找到服务端代码。
2. 安装 MySQL、OpenCV、ffmpeg、SDL 等依赖。
3. 编译服务端代码并启动服务。

## 功能
- 一键创建房间并生成房间号。
- 实时视频通话、屏幕共享。
- 用户登录，个人信息采用 MD5 加盐加密传输。
- 服务端通过 `epoll` 和线程池实现高效处理。
- 视频处理集成 OpenCV、`libfacedetect`（萌拍）和 `ffmpeg`。
- 音频处理采用 SDL、WebRTC VAD（静音检测）和 Opus 编解码。
