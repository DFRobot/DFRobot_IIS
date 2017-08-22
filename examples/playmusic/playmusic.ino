/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Module
 * @n IIS Module for How to start play WAV file and pause continue stop play  
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
  iis.setHeadphonesVolume(50);                 // Set Headphones Volume from 0 to 99
  iis.setSpeakersVolume(0);                    // Set Speakers   Volume from 0 to 99
  iis.init(AUDIO);                             // Init Audio mode and SD card            
  iis.playMusic("/sdcard/123.WAV");            // Enter file name for play Music       
  delay(500);
}

void loop() {  
  if((!digitalRead(buttonPin))&&(i==0||i==2)){
    while((!digitalRead(buttonPin))){
	     delay(10);
	}
    iis.playerControl(PLAY); 	               // Start or continue playing Music
	i++;
  }                         
  delay(100);
  if((!digitalRead(buttonPin))&&i==1){
    while((!digitalRead(buttonPin))){
	    delay(10);
	}
    iis.playerControl(PAUSE);                  // Pause playing
	i++;
  }
  delay(100);
  if((!digitalRead(buttonPin))&&i==3){
    while((!digitalRead(buttonPin))){
	    delay(10);
	}
    iis.playerControl(STOP);                   // Stop playing
	i=4;
  }  
  if((!digitalRead(buttonPin))&&i==4){
    while((!digitalRead(buttonPin))){
	    delay(10);
	}
    iis.changeMusic("/sdcard/1234.WAV");       // Change music file
	i=0;
  }  
  delay(100);
}