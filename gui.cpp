#include "gui.h"

#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <memory>

#include "button.h"
#include "status.h"

extern "C" {
	#include "delay.h"
	#include "GLCD.h"
	#include "TouchPanel.h"
	#include "tdc.h"
	void touch(Coordinate* coord);
	void init();
	void set_status_idle();
	void set_status_waiting();
	void set_status_transmitting();
	void drawControls();
	void encoder(int8_t);
}

long x, y;   	//for touch
//uint16_t DAC_value = 0;

button* pObject;
void (button::*pMember)(int8_t);

std::unique_ptr<button> DACbutton;
std::unique_ptr<button> calButton;
std::unique_ptr<button> pulseWidthButton;
std::unique_ptr<button> pulseCountButton;
std::unique_ptr<button> FireButton;
status main_status;

void set_status_idle()
{
	main_status.set(status::Idle);
}

void set_status_transmitting()
{
	main_status.set(status::Transmitting);
}

void set_status_waiting()
{
	main_status.set(status::Waiting);
}

void fire()
{
	printf("in fire()\n");
	printf("calling tdc_measure()\n");
	uint32_t time_measured =  tdc_measure();

	char buffer[20];
    sprintf(buffer,"%10d ps", time_measured);
    GUI_Text(200, 200, (uint8_t*)buffer, Black, BG);
}

void DAC_set(uint16_t value)
{
	LPC_DAC->DACR = (value & 0b0000001111111111)<<6;	//get rid of upper 6bits and shift 
	//DAC_value = (value & 0b0000001111111111);
}

uint16_t DAC_get()
{
	return ((LPC_DAC->DACR >> 6) & 0b0000001111111111);
}

void checkFreeRam()
{
	uint32_t stack_pointer = __get_MSP();
	void *heap_pointer = malloc(1);
    char buffer1[20];
    sprintf(buffer1,"stack: %x", stack_pointer);
    char buffer2[20];
    sprintf(buffer2,"heap: %p", heap_pointer);
    GUI_Text(10, 50, (uint8_t*)buffer1, Black, BG);
    GUI_Text(10, 30, (uint8_t*)buffer2, Black, BG);
    free(heap_pointer);
}


void blink()
{
	LPC_GPIO3->FIOCLR = (1<<25);	//for debug blink LED
	_delay_ms(100);
	LPC_GPIO3->FIOSET = (1<<25);
	_delay_ms(100);
}

	
void drawControls()
{
	LCD_Clear(BG);

	main_status.redraw();
	
	calButton->show();
	DACbutton->show();
	FireButton->show();

}

void init()
{
	LCD_Initializtion();
	TP_Init(); 
	_delay_ms(100);
	tdc_init();

	calButton.reset(new button("Cal.", 90, 180, &TouchPanel_Calibrate));
	calButton->sizeX = 65;
	calButton->YlabelOffset = 16;

	DACbutton.reset(new button("DAC", 10, 180));
	DACbutton->sizeX = 65;
	DACbutton->YlabelOffset = 16;
	DACbutton->value = DAC_get();
	DACbutton->roll_factor = 10;

	FireButton.reset(new button("Fire", 90, 30, &fire));
	FireButton->sizeX = 65;
	FireButton->YlabelOffset = 16;


	drawControls();
}

void encoder(int8_t _encoder_change)
{
	if(pObject != nullptr && pMember != nullptr && main_status.get() == status::Changing_parameters)
		(pObject->*pMember)(_encoder_change);	
}

void setPtrToCurrentValue(button* _pObject, void (button::*_pMember)(int8_t))
{
	pObject = _pObject;
	pMember = _pMember;
	main_status.set(status::Changing_parameters);
}

void touch(Coordinate* coord)
{  
	if(main_status.get() == status::Changing_parameters)
	{
		DAC_set(DACbutton->value);
		main_status.set(status::Idle);
	}
	else
	{
		getDisplayPoint(&display, coord, &matrix);
		y = display.x;
		x = display.y;			
		button::processTouch(x,y);
	}	
	
	_delay_ms(50); 
	while(!(LPC_GPIO2->FIOPIN & (1<<13)));         //wait for release
	_delay_ms(100);                                //eliminate ringing
	
}
