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
uint16_t pulseFreq = 1000;

button* pObject;
void (button::*pMember)(int8_t);

std::unique_ptr<button> DACbutton;
std::unique_ptr<button> calButton;
std::unique_ptr<button> pulseWidthButton;	//can be implemented? 10-80ns
std::unique_ptr<button> pulseCountButton;
std::unique_ptr<button> measureButton;
std::unique_ptr<button> fireButton;
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

void measure()
{
	printf("in measure()\n");
	printf("calling tdc_measure()\n");
	uint32_t time_measured =  tdc_measure();

	char buffer[20];
    sprintf(buffer,"%10d ps", time_measured);
    GUI_Text(200, 200, (uint8_t*)buffer, Black, BG);
}

// void __attribute__( ( noinline, long_call ) ) virtualFire()
// {
// 	LPC_GPIO0->FIOSET = 1<<5;
// 		__ASM("nop");
// 		__ASM("nop");
// 		__ASM("nop");
// 	LPC_GPIO0->FIOCLR = 1<<5;
// }
// 00002578 <virtualFire()>:
//     2578:	4b03      	ldr	r3, [pc, #12]	; (2588 <virtualFire()+0x10>)
//     257a:	2220      	movs	r2, #32
//     257c:	619a      	str	r2, [r3, #24]
//     257e:	bf00      	nop
//     2580:	bf00      	nop
//     2582:	bf00      	nop
//     2584:	61da      	str	r2, [r3, #28]
//     2586:	4770      	bx	lr
//     2588:	2009c000 	.word	0x2009c000	//lower first OMG!

uint16_t virtualFire[] = { 0x4B03, 0x2220, 0x619A, 0xBF00, 0xBF00, 0xBF00, 0x61DA, 0x4770, 0xC000, 0x2009 };

typedef void (*funcPtr)(void);

void fire()
{
	funcPtr virtualFirePtr =  (funcPtr) (((uint32_t) virtualFire)+1);
	
	set_status_transmitting();
	asm volatile("cpsid i");	//disable all interrupts
	for(int i=0; i<pulseCountButton->value; i++)
	{
		virtualFirePtr();
	}
	asm volatile("cpsie i");	//re-enable interrupts
	set_status_idle();
}

void DAC_set(uint16_t value)
{
	LPC_DAC->DACR = (value & 0b0000001111111111)<<6;	//get rid of upper 6bits and shift 
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
	pulseCountButton->show();
	fireButton->show();
	measureButton->show();

}

void init()
{
	LCD_Initializtion();
	TP_Init(); 
	_delay_ms(100);
	tdc_init();

	DACbutton.reset(new button("DAC", 10, 180));
	DACbutton->sizeX = 65;
	DACbutton->YlabelOffset = 16;
	DACbutton->value = DAC_get();
	DACbutton->roll_factor = 10;

	calButton.reset(new button("Cal.", 90, 180, &TouchPanel_Calibrate));
	calButton->sizeX = 65;
	calButton->YlabelOffset = 16;

	pulseCountButton.reset(new button("Pulses", 10, 105));
	pulseCountButton->sizeX = 65;
	pulseCountButton->YlabelOffset = 16;
	pulseCountButton->value = 1;

	fireButton.reset(new button("Fire", 90, 105, &fire));
	fireButton->sizeX = 65;
	fireButton->YlabelOffset = 16;

	measureButton.reset(new button("Measure", 90, 30, &measure));
	measureButton->sizeX = 65;
	measureButton->YlabelOffset = 16;


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
