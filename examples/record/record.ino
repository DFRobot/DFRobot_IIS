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
    ePrepare,
    eRecording,
    eStop
}eState=ePrepare;

void setup(){
    Serial.begin(115200);
    iis.SDCardInit();                              // SD card init
    iis.init(AUDIO);                               // Init Audio mode
    iis.initRecorder();                            // Init recorder 
    iis.record("/record1.WAV");                    // Enter file name to save recording
    Serial.println("Ready to record");
}

void loop(){
    if((!digitalRead(buttonPin))&&eState==ePrepare){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.recorderControl(BEGIN);                // Begin recording
        Serial.println("Recording");
        eState=eRecording;
    }
    delay(100);
    if((!digitalRead(buttonPin))&&eState==eRecording){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.recorderControl(STOP);                 // eStop recording
        Serial.println("Stop and save data");
        eState=eStop;
    }  
    if((!digitalRead(buttonPin))&&eState==eStop){
        while((!digitalRead(buttonPin))){
            delay(10);
        }
        iis.record("/record2.WAV");                // Change file name
        eState=ePrepare;
    }
    delay(100);
}