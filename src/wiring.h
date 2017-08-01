#ifndef __WIRING_H__
#define __WIRING_H__

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


// Some functions from Arduino/Wiring
void pinmode(int pin, int mode);
void digitalwrite(int pin, int value);
void Delay(int millis);

#define OUTPUT 0
#define INPUT 1
#define INPUT_PULLUP 2

// Some functions to make OpenMV happy
#define systick_sleep(t) Delay(t)
#define __disable_irq()
#define __enable_irq()



#endif //__OMV_PORT_H__
