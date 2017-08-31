/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Record Module
 * @n IIS Module for Record sound and Save WAV file in SD card 
 *    Insert sd card 
 *    Call the function by pressing user key to control recorder
 *    This Module will record sound and save it as record1.wav record2.wav by pressing user key
 *    
 * @copyright    [DFRobot](http://www.dfrobot.com), 2017
 * @copyright    GNU Lesser General Public License
 *
 * @author [Zhangjiawei<jiawei.zhang@dfrobot.com>]
 * @version  V1.0
 * @date  2017-8-1
 */
 
#include <Wire.h>
#include <DFRobot_IIS.h>

DFRobot_IIS iis;
const int buttonPin = 16; 
enum Status{
  Ready,
  Recording,
  Stop
}state=Ready;

void setup() {
  Serial.begin(115200);
  iis.init(AUDIO);                             // Init Audio mode and SD card
  iis.initRecorder();                          // Init recorder 
  iis.record("/sdcard/record1.WAV");           // Enter file name to save recording
}

void loop() {
  if((!digitalRead(buttonPin))&&state==Ready){
    while((!digitalRead(buttonPin))){
        delay(10);
    }
    iis.recorderControl(BEGIN);                // Begin recording
    state=Recording;
  }                         
  delay(100);
  if((!digitalRead(buttonPin))&&state==Recording){
    while((!digitalRead(buttonPin))){
        delay(10);
    }
    iis.recorderControl(STOP);                 // Stop recording
    state=Stop;
  }  
  if((!digitalRead(buttonPin))&&state==Stop){
    while((!digitalRead(buttonPin))){
        delay(10);
    }
    iis.record("/sdcard/record2.WAV");         // Change file name
    state=Ready;
  }  
  delay(100);
}