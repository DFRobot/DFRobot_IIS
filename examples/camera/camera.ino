/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Camera Module
 * @n IIS Module for take photo and save BMP file in SD card
 *    Insert sd card
 *    Call the function by pressing user key to take photo
 *    This Module will take photo and save as photo1.bmp,photo2.bmp by pressing user key
 */

#include <Wire.h>
#include "DFRobot_IIS.h"

DFRobot_IIS iis;
const int buttonPin = 16;
int i=0; 

void setup(){
    Serial.begin(115200);
    pinMode(buttonPin, INPUT);
    iis.SDCardInit();                                //SD card init
    iis.connectNet("DFROBOT_AP","12345678");         //Connect wifi
    iis.init(CAMERA);                                //Init Camera mode and SD card
    iis.setPhotoformat(QVGA,GRAY);                   //Set photo size QVGA:320x240 pixelformat GRAYSCALE
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