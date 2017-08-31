/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS SD card Module
 * @n SD card initialization, read and write 
 *    Create a text file in SD card to write "Hello World!" and read it
 * @copyright    [DFRobot](http://www.dfrobot.com), 2017
 * @copyright    GNU Lesser General Public License
 *
 * @author [Zhangjiawei <jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-30
 */

#include <Wire.h>
#include "DFRobot_IIS.h"

DFRobot_IIS iis;

void setup(){
  Serial.begin(115200);
  iis.SDcard_Init();
  iis.SDcard_Write("/sdcard/test.txt","Hello World!");
  iis.SDcard_Read("/sdcard/test.txt",12);
}

void loop(){
  delay(100);
}