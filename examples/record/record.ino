/*!
 * @file DFRobot_IIS.h
 * @brief DFRobot's IIS Record Module
 * @n IIS Module for Record sound and Save WAV file in SD card 
 *    Insert sd card 
 *    Call the function by pressing user key to control recorder
 *    This Module will record sound and save it as record1.wav record2.wav by pressing user key
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

void setup(){
    Serial.begin(115200);
    iis.init(AUDIO);                               // Init Audio mode and SD card
    iis.initRecorder();                            // Init recorder 
    iis.record("/sdcard/record1.WAV");             // Enter file name to save recording
    Serial.println("Ready to record");
}

void loop(){
    if((!digitalRead(buttonPin))&&state==Ready){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.recorderControl(BEGIN);                // Begin recording
        Serial.println("Recording");
        state=Recording;
    }
    delay(100);
    if((!digitalRead(buttonPin))&&state==Recording){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.recorderControl(STOP);                 // Stop recording
        Serial.println("Stop and save data");
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