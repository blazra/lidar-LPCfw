#ifndef GUI_H
#define GUI_H

#include "button.h"

void blink();
void checkFreeRam();
void setPtrToCurrentValue(button*, void (button::*)(int8_t));
uint16_t DAC_get();
void DAC_set(uint16_t value);



#endif