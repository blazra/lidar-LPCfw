#include "LPC17xx.h"
#include "delay.h"
#include <stdio.h>
#include "TouchPanel.h"
#include "tdc.h"

//declarations from gui.cpp
void set_status_idle();
void set_status_transmitting();
void touch(Coordinate* coord);
void init();
void encoder(int8_t);

char touch_flag = 0;
int8_t encoder_change = 0;

void blink()
{
	LPC_GPIO3->FIOCLR = (1<<25);	//for debug blink LED
	_delay_ms(100);
	LPC_GPIO3->FIOSET = (1<<25);
	_delay_ms(100);
}

void pllfeed()
{
__disable_irq();
LPC_SC->PLL0FEED=0x000000aa;
LPC_SC->PLL0FEED=0x00000055;
__enable_irq();    
}

void setpll()
{
// the crystal oscillator is 12 MHz
// main oscillator frequency 300 MHz: M = (300 x N) / (2 x 12)
uint8_t n=2;
uint8_t m=25;
// // processor clock 100 MHz = 300 MHz / D
uint8_t d=3;
// // disconnect
 LPC_SC->PLL0CON=0x00000001; pllfeed();
 // disable
 LPC_SC->PLL0CON=0x00000000; pllfeed();
 // set new PLL values
 LPC_SC->PLL0CFG=((n-1)<<16)|(m-1); pllfeed();
 // enable
 LPC_SC->PLL0CON=0x00000001; pllfeed();
 // set cpu clock divider
 LPC_SC->CCLKCFG=d-1;
 // wait for lock
 while (LPC_SC->PLL0STAT&0x04000000==0);
 // connect
 LPC_SC->PLL0CON=0x00000003; pllfeed();
}

// P3.25 connected to LED1
// P3.26 connected to LED2
// P2.11 connected to KEY1
void dbg_init()
{
	init();
}

int main()
{	
	setpll();	// 100MHz
	

	LPC_SC->PCONP 		|= 	(1<<15);	//power up GPIO
	LPC_GPIO3->FIODIR 	|= 	(1<<25);	//set P3.25 to output mode - LED1
	LPC_GPIO0->FIODIR 	|= 	(1<<0);		//set P0.0 to output mode - TDC enable pin
	LPC_GPIO0->FIODIR 	|= 	(1<<5);
	LPC_GPIO2->FIODIR 	|= 	(1<<7);		//set P2.7 to output mode - TDC CS pin
	LPC_GPIO3->FIOSET 	 = 	(1<<25);
	LPC_GPIO2->FIOSET 	 = 	(1<<7);		//TDC CS high
	LPC_GPIO0->FIOCLR 	 = 	(1<<5);
	LPC_GPIO0->FIOCLR 	 = 	(1<<0);		//disable TDC

	//DAC
	LPC_SC->PCLKSEL0 		|= 1<<22;		// CCLK clock to DAC 
	LPC_PINCON->PINSEL1 	|= 1<<21;		// set DAC on P0.26
	LPC_PINCON->PINMODE1 	|= 1<<21;		// no pull-up nor pull-down
	LPC_DAC->DACR 			 = 1020<<6;

	
	LPC_GPIOINT->IO2IntEnF |= 1<<13;
	NVIC_EnableIRQ(EINT3_IRQn);

	//QEI
	LPC_SC->PCONP 			|= 	1<<18;		//power up QEI
	LPC_SC->PCLKSEL0		|=	1<<0;		//select clock to CCLK
	LPC_PINCON->PINSEL3		|=	( (1<<8) | (1<<14) );
	LPC_QEI->QEICONF		|=	1<<2;		// count 4 edges instead of default 2
	LPC_QEI->FILTER			 =	10000;		// 1/100MHz = 10ns
	LPC_QEI->QEIIES			 =  1<<5;
	NVIC_EnableIRQ(QEI_IRQn);

	// LPC_SC->PCONP |= 1 << 1; 	//Power up Timer 0
	// LPC_SC->PCLKSEL0 |= 1 << 2; // Clock for timer = CCLK
	// LPC_TIM0->MR0 = 1 << 26; 	// Give a value suitable for the LED blinking frequency based on the clock frequency
	// LPC_TIM0->MCR |= 1 << 0; 	// Interrupt on Match0 compare
	// LPC_TIM0->MCR |= 1 << 1; 	// Reset timer on Match 0.
	// LPC_TIM0->TCR |= 1 << 1; 	// Reset Timer0
	// LPC_TIM0->TCR &= ~(1 << 1); // stop resetting the timer.
	// NVIC_EnableIRQ(TIMER0_IRQn); // Enable timer interrupt
	//LPC_TIM0->TCR |= 1 << 0; 	// Start timer

	init();

	Coordinate* coord;   

	while(1)
	{
		if(!(LPC_GPIO2->FIOPIN1 & 0b00001000))
		{
			set_status_transmitting();
			asm volatile("cpsid i");	//disable all interrupts
			for(int i=0; i<100; i++)
			{
				LPC_GPIO0->FIOSET = 1<<5;
				__ASM("nop");
				__ASM("nop");
				__ASM("nop");
				LPC_GPIO0->FIOCLR = 1<<5;
				_delay_ms(10);			
			}
			asm volatile("cpsie i");	//re-enable interrupts
			set_status_idle();

			LPC_GPIO3->FIOCLR = (1<<25);	//blink with LED once
			_delay_ms(200);
			LPC_GPIO3->FIOSET = (1<<25);
			_delay_ms(200);

			while(!(LPC_GPIO2->FIOPIN1 & 0b00001000));
		}

		if(touch_flag)
		{
			//touch();
			touch_flag=0;
		}

		if(coord=Read_Ads7846())
		{
			printf("before touch() \n");
			touch(coord);
			printf("after touch() \n");
		}

		if(abs(encoder_change)>=4)
		{
			encoder(encoder_change/4);
			encoder_change = encoder_change%4;
		}
		
		
	}
	return 0;
}

uint32_t dacvalue=1;

void QEI_IRQHandler(void)
{
	if(LPC_QEI->QEISTAT)
	{
		encoder_change++;
	}
	else
	{
		encoder_change--;
	}
	LPC_QEI->QEICLR	= 1<<5;
}

void EINT3_IRQHandler(void)
{		
	touch_flag = 1;
	LPC_GPIOINT->IO2IntClr |= 1<<13;	//clear interrupt flag
}

void TIMER0_IRQHandler(void)
{
    if((LPC_TIM0->IR & 0x01) == 0x01) // if MR0 interrupt
    {
        LPC_TIM0->IR |= 1 << 0; // Clear MR0 interrupt flag
        LPC_GPIO3->FIOPIN ^= 1 << 25; // Toggle the LED
        dacvalue = (dacvalue<<1);
        if(dacvalue == (1<<9))
        	dacvalue = (dacvalue>>9);
        LPC_DAC->DACR = (dacvalue<<6);
        //sprintf(str, "%i", dacvalue);
        //GUI_Text(52,176,str,White,Black);
    }

}