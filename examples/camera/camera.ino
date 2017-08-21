/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Module
 * @n IIS Module for take photo and save BMP file in SD card 
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

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  iis.init(CAMERA);                     //Init Camera mode and SD card
  delay(100);
}

void loop() {
  if(!digitalRead(buttonPin)){          //Press user key to take photo  
  iis.takephoto("/sdcard/photo.bmp");   //Take photo and save it as photo.bmp in SD card
  }
  delay(500);
}