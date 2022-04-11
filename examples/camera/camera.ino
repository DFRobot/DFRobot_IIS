/*!
 * @file camera.ino
 * @brief Example of taking photo using camera
 * @details IIS Module for take photo and save BMP file in SD card Insert sd card
 * @n  Call the function by pressing user key to take photo
 * @n  This Module will take photo and save as photo1.bmp,photo2.bmp by pressing user key
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
const char* SSID    ="yourSSID";  //wifi name
const char* PASSWORD="SSID password"; //wifi password
void setup(){
    Serial.begin(115200);
    pinMode(buttonPin, INPUT);
    iis.SDCardInit();                                //SD card init
    iis.init(CAMERA);                                //Init Camera mode
    iis.connectNet(SSID,PASSWORD);                   //Connect wifi
    iis.setFreamsize(QVGA);                          //Set photo size QVGA:320x240
    iis.setPixformat(GRAYSCALE);                     //Set pixelformat GRAYSCALE
    iis.sendPhoto();                                 //Send photo to net
    delay(100);
    Serial.println("Ready to take photo");
}

void loop(){
    if(!digitalRead(buttonPin)&&i==0){               //Press user key to take photo
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.snapshot("/photo1.bmp");                 //Take photo and save it as photo1.bmp in SD card
        Serial.println("take photo1");
        i=1;
    }
    delay(500);
    if(!digitalRead(buttonPin)&&i==1){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.snapshot("/photo2.bmp");
        Serial.println("take photo2");  
        i=0;
    }
}
