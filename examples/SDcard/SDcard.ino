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
  iis.SDcard_Init();                              //Init SD card
  iis.SDcard_Open("/sdcard/test.txt",SD_Write);   //Open file in SD card
  iis.SDcard_Write("Hello World!");               //Write data to file
  iis.SDcard_Close();                             //Close and save data
  iis.SDcard_Open("/sdcard/test.txt",SD_Read);
  iis.SDcard_Read();                              //Read data from file
  iis.SDcard_Close();
  }

void loop(){
  delay(100);
}