#ifndef VALUECONTROLS_H
#define VALUECONTROLS_H

#include <stdint.h>
#include <stdio.h>

extern "C" {
  #include "GLCD.h"
}

#include "button.h"

class valueControls
{
public:
    void (*set)(uint16_t);
	  int x,y;
  	unsigned char value;
  	uint16_t color;
  	uint16_t text_color;
  	char str[5];
  	button plusIntVoltButton;
  	button minusIntVoltButton;

    valueControls(int _x, int _y, uint16_t currentValue, void (*_set)(uint16_t));
  	~valueControls();

  	void show(int _x,int _y);               
  	void redrawValue();
    void plusVolt();
    void minusVolt();
    void checkRange();
    void addToValue(int8_t);
};


#endif