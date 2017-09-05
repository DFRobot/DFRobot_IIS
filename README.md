# DFRobot_IIS Library for Arduino
Provides an Arduino library for FireBeetle Covers-Camera&Audio Media Board 
## Table of Contents

* [Summary](#summary)
* [Methods](#methods)
* [History](#history)
* [Credits](#credits)
<snippet>
<content>

## Summary

The library is used to Play music file from SD card,Record sound and save in SD card,take photo and save in SD card

## Methods

### Choose audio mode or camera mode

    void init(uint8_t mode);
    mode:AUDIO CAMERA

### Enter the volume(from 0 to 99) to set volume or choose mute mode when playMusic 

    void setSpeakersVolume(uint8_t volume);
    void setHeadphonesVolume(uint8_t volume);
    void muteSpeakers(void);
    void muteHeadphones(void);

### Init Music Player

    void initPlayer();

### Enter the WAV file name in the SD card you want to play

    int playMusic(const char *filename);

### Control the music player

    void playerControl(uint8_t cmd);
    cmd:PLAY PAUSE STOP

### Init Recorder

    void initRecorder();

### Enter the WAV file name you want to store in the SD card 

    int recordSound(const char *outputFilename);

### Control the recorder

    void recorderControl(uint8_t cmd);
    cmd:BEGIN STOP 

### Enter the BMP file name you want to store in the SD card

    void takePhoto(const char *pictureFilename);

## History

- data 2017-8-1
- version V1.0

## Credits

- author [Zhangjiawei<jiawei.zhang@dfrobot.com>]