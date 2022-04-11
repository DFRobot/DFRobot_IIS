# DFRobot_IIS
- [English Version](./README.md)

DFRobot FireBeetle萤火虫系列是专为物联网设计的低功耗开发组件。此款FireBeetle Covers扩展板，集成NAU8822高品质IIS解码芯片、OV7725高清摄像头接口、SD（SDIO接口）插槽、以及耳机和话筒接口，并且还板载了一个小型咪头输入接口。NAU8822集成12@8Ω的BTL扬声器驱动，以及40mW@
16Ω的耳机驱动接口，可以直接插接8Ω的扬声器和耳机。同时，NAU8822还支持DAC语音信号采集，集成可编程麦克风放大器，可以通过板载的咪头或者话筒接口，输入语音，并将文件保存在SD卡中，实现录音功能。其次，模块提供的OV7725接口，可以直接连接OV7725摄像头，图片刷新频率最高支持60fps@VGA，实现640×480的简单拍照功能

![产品效果图](./resources/images/DFR0498.jpg)

## 产品链接 (https://www.dfrobot.com.cn/goods-1667.html)
    DFR0498:Fermion: FireBeetle 摄像头及音频扩展板

## 目录

  * [概述](#概述)
  * [库安装](#库安装)
  * [方法](#方法)
  * [兼容性](#兼容性)
  * [历史](#历史)
  * [创作者](#创作者)

## 概述

     1.驱动DFR0498上面的音频设备
   
## 库安装
要使用此库，请先下载库文件，并将其粘贴到\Arduino\libraries目录中，然后打开示例文件夹并在该文件夹中运行demo。

## 方法

```C++
   /**
   * @fn init
   * @brief 初始化函数
   * @details 初始化I2S的模式,在esp32中I2S可用做音频控制器和摄像头控制器
   * @param mode IIS模式,AUDIO或者CAMERA 
   * @return bool类型的数据
   * @retval true 初始化成功
   * @retval false 初始化失败
   */
  bool init(uint8_t mode);

  /**
   * @fn SDCardInit
   * @brief 挂载sd卡
   * @return bool类型的数据
   * @retval true 初始化成功
   * @retval false 初始化失败
   */
  bool SDCardInit(void);

  /**
   * @fn sendPhoto
   * @brief 将从摄像头获取的照片通过网络发出去
   */
  void sendPhoto(void);

  /**
   * @fn setSpeakersVolume
   * @brief 设置扬声器的音量大小
   * @param volume 音量,范围是 0~99
   */
  void setSpeakersVolume(uint8_t volume);
  
  /**
   * @fn muteSpeakers
   * @brief 关掉播放器
   */
  void muteSpeakers(void);

  /**
   * @fn setHeadphonesVolume
   * @brief 设置耳机音量
   * @param volume 音量,范围是 0~99
   */
  void setHeadphonesVolume(uint8_t volume);
  
  /**
   * @fn muteHeadphones
   * @brief 关掉播放器
   */
  void muteHeadphones(void);
  
  /**
   * @fn setFreamsize
   * @brief 设置摄像头的像素
   * @param photoSize  可选择(QQVGA,QQVGA2,QICF,HQVGA,QVGA,RGB555,GRAYSCALE)
   * @return 返回设置的像素
   */
  uint8_t setFreamsize(uint8_t photoSize);

  /**
   * @fn setPixformat
   * @brief 设置色彩的颜色深度
   * @param pixelFormat  RGB555(RGB565真彩色),GRAYSCALE(灰度图)
   * @return 返回设置的颜色深度
   */
  uint8_t setPixformat(uint8_t pixelFormat);

  /**
   * @fn snapshot
   * @brief Take photo
   * @param pictureFilename 要保存图片的名字
   */ 
  void snapshot(const char *pictureFilename);
  
  /**
   * @fn connectNet
   * @brief 连接无线网
   * @param ssid     WIFI的名字
   * @param password WIFI的密码
   */ 
  void connectNet(const char* ssid,const char* password);

  /**
   * @fn initPlayer
   * @brief 初始化音乐播放器
   */
  void initPlayer();
  
  /**
   * @fn initRecorder
   * @brief 初始化录音设备
   */
  void initRecorder();

  /**
   * @fn playMusic
   * @brief 播放音乐
   * @param filename 要播放的音乐的文件名
   */
  void playMusic(const char *Filename);

  /**
   * @fn record
   * @brief record sound
   * @param filename 要保存录音的文件名
   */
  void record(const char *Filename);

  /**
   * @fn playerControl
   * @brief 控制音乐播放器
   * @param cmd 控制音乐播放器的命令
   * @n    PLAY   开始或继续播放
   * @n    PAUSE  暂停播放音乐
   * @n    STOP   停止播放音乐
   */
  void playerControl(uint8_t cmd);

  /**
   * @fn recorderControl
   * @brief 控制录音机
   * @param cmd 控制录音机的命令
   * @n   BEGIN  开始记录
   * @n   STOP   停止记录
   */
  void recorderControl(uint8_t cmd);
  
  /**
   * @fn SDcard_Init
   * @brief 初始化sd卡
   * @param mountpoint SD卡名字
   */
  bool SDcard_Init(const char* mountpoint="/sdcard");

```

## 兼容性

主板               | 通过  | 未通过   | 未测试   | 备注
------------------ | :----------: | :----------: | :---------: | -----
FireBeetle-ESP32  |      √       |             |            | 
FireBeetle-ESP8266  |             |      √       |            | 
FireBeetle-BLE4.1 |             |       √      |            | 


## 兼容性


- 2017/9/29 - Version 1.0.0 released.
- 2022/3/21 - Version 1.0.1 released.

## 创作者


Written by fengli(li.feng@dfrobot.com), 2022.3.21 (Welcome to our [website](https://www.dfrobot.com/))



