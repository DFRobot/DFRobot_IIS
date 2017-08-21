#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "SDcard.h"
#include "esp_err.h"
#include "esp_log.h"

typedef struct BMP_HEADER 
{  
uint16_t BfType;    
unsigned int BfSize; 
uint16_t BfReserved1; 
uint16_t BfReserved2; 
unsigned int BfOffBits;
unsigned int BitsSize;
unsigned int BiWidth;
unsigned int BiHighth;
uint16_t BiPlanes;
uint16_t BitCount; 
unsigned int BiCompression;
unsigned int BiSizeTmage;
unsigned int Bixpels;
unsigned int Biypels;
unsigned int BiClrUsed;
unsigned int BiClrImportant;
}BMP_HEADER;

struct BMP
{
  BMP_HEADER header;
  FILE       *fp;
};

typedef struct BMP *HANDLE_BMP;

//void Cam_init();







#endif
