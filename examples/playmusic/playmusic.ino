/*!
 * @file playmusic.ino
 * @brief Example of playing music
 * @details  IIS Module for how to begin to play a WAV file,how to excute orders pause,continue,stop and play the next 
 * @n Insert sd card with test1.wav and test2.wav.
 * @n Call the function by pressing user key to control music player
 * @n The Module would operate as follows when pressed: play 
 * @n test1.wav>>pause>>continue>>mute>>Volume:50>>stop>>play test2.wav photo1.bmp,photo2.bmp by pressing user key
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT license (MIT)
 * @author [fengli](li.feng@dfrobot.com)
 * @version  V1.0.0
 * @date  2022-03-18
 * @https://github.com/DFRobot/DFRobot_IIS
*/

#include <Wire.h>
#include "DFRobot_IIS.h"

DFRobot_IIS iis;
const int buttonPin = 16; 
int i=0;

void setup(){
    Serial.begin(115200);
    iis.SDCardInit();                              // SD card init
    iis.init(AUDIO);                               // Init Audio mode
    iis.setHeadphonesVolume(50);                   // Set Headphones Volume from 0 to 99
    iis.setSpeakersVolume(50);                     // Set Speakers   Volume from 0 to 99
    iis.initPlayer();                              // Init Music player
    Serial.println("Init player");
    iis.playMusic("/test1.WAV");                   // Choose music file
    Serial.println("Ready to play");
    delay(500);
}

void loop(){  
    if((!digitalRead(buttonPin))&&(i==0||i==2)){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.playerControl(PLAY);                   // Start or continue playing Music
        Serial.println("play");
        i++;
    }
    delay(100);
    if((!digitalRead(buttonPin))&&i==1){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.playerControl(PAUSE);                  // Pause playing
        Serial.println("pause");
        i++;
    }
    delay(100);
    if((!digitalRead(buttonPin))&&i==3){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.muteSpeakers();                        // Mute mode
        iis.muteHeadphones();
        Serial.println("mute mode");
        i++;
    }
    if((!digitalRead(buttonPin))&&i==4){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.setHeadphonesVolume(50);
        iis.setSpeakersVolume(50);
        Serial.println("Volume=50");
        i++;
    }
    if((!digitalRead(buttonPin))&&i==5){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.playerControl(STOP);                   // Stop playing
        Serial.println("Stop");
        i++;
    }
    if((!digitalRead(buttonPin))&&i==6){
    while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.playMusic("/test2.WAV");               // Change music file
        iis.playerControl(PLAY);                   // Play test2.wav
        i=1;
    }
    delay(100);
}
