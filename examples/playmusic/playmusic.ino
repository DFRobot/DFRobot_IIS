/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Module
 * @n IIS Module for playMusic 
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2017
 * @copyright	GNU Lesser General Public License
 *
 * @author [Zhangjiawei]
 * @version  V1.0
 * @date  2017-8-1
 */

#include <Wire.h>
#include "DFRobot_IIS.h"



DFRobot_IIS iis;


void setup() {
  Serial.begin(115200);
  iis.init(Audioplay);
  iis.playMusic("/sdcard/123.WAV");
  delay(500);
}

void loop() {
  delay(500);
}
