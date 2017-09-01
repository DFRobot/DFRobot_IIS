/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Camera Module
 * @n IIS Module for take photo and save BMP file in SD card
 *    Insert sd card
 *    Call the function by pressing user key to take photo
 *    This Module will take photo and save as photo1.bmp,photo2.bmp by pressing user key
 *     
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [Zhangjiawei<jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-1
 */

#include <Wire.h>
#include "DFRobot_IIS.h"

DFRobot_IIS iis;
const int buttonPin = 16;
int i=0; 

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  iis.init(CAMERA);                            //Init Camera mode and SD card
  delay(100);
}

void loop() {
  if(!digitalRead(buttonPin)&&i==0){           //Press user key to take photo
    while((!digitalRead(buttonPin))){
        delay(10);
    }
  iis.takePhoto("/sdcard/photo1.bmp");         //Take photo and save it as photo1.bmp in SD card
  i=1;
  }
  delay(500);
  if(!digitalRead(buttonPin)&&i==1){   
    while((!digitalRead(buttonPin))){
        delay(10);
    }  
  iis.takePhoto("/sdcard/photo2.bmp");       
  i=0;
  }
}