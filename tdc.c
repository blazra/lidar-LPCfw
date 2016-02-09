#include "tdc.h"
#include "stdint.h"
#include "LPC17xx.h"
#include "delay.h"
#include "GLCD.h"
void set_status_waiting();
void set_status_idle();

#define CONFIG1			0x00
#define CONFIG2			0x01
#define INT_STATUS		0x02
#define INT_MASK		0x03
#define COARSE_CNTR_OVF_H 		0x04
#define COARSE_CNTR_OVF_L 		0x05
#define CLOCK_CNTR_OVF_H 		0x06
#define CLOCK_CNTR_OVF_L 		0x07
#define CLOCK_CNTR_STOP_MASK_H 	0x08
#define CLOCK_CNTR_STOP_MASK_L 	0x09
#define TIME1			0x10
#define CLOCK_COUNT1	0x11
#define TIME2			0x12
#define CLOCK_COUNT2 	0x13
#define TIME3		 	0x14
#define CLOCK_COUNT3 	0x15
#define TIME4		 	0x16
#define CLOCK_COUNT4 	0x17
#define TIME5		 	0x18
#define CLOCK_COUNT5 	0x19
#define TIME6			0x1A
#define CALIBRATION1	0x1B
#define CALIBRATION2	0x1C

uint8_t config1_data = 0;

void tdc_send_command(uint8_t auto_increment, uint8_t write_flag, uint8_t address)
{
	uint8_t command = 0;
	
	if(auto_increment)
		command = 0x80;
	if(write_flag)
		command |= 0x40;
	
	command |= address;

	tdc_write_to_spi_FIFO(command);
}

void tdc_write_to_spi_FIFO(uint8_t data)
{
	while( !(LPC_SSP0->SR & 0x02) );		//wait if TX FIFO not full is 0

	LPC_SSP0->DR = data;
}

uint8_t tdc_read_from_spi_FIFO()
{
	while( !(LPC_SSP0->SR & 0x04) );		//wait if RX FIFO not empty is 0

	return LPC_SSP0->DR;
}

void tdc_set_register(uint8_t address, uint8_t data)
{
	LPC_GPIO2->FIOCLR =	(1<<7);				//CS low
	tdc_send_command(0, 1, address);
	tdc_write_to_spi_FIFO(data);
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	LPC_GPIO2->FIOSET =	(1<<7);				//CS high
}

uint8_t tdc_get_register_8b(uint8_t address)
{
	LPC_GPIO2->FIOCLR =	(1<<7);				//CS low
	tdc_send_command(0, 0, address);		//send address with read command
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy

	while( LPC_SSP0->SR & 1<<2 )			//if receive FIFO not empty
		tdc_read_from_spi_FIFO();			//empty the receive FIFO

	tdc_write_to_spi_FIFO(0);				//writes zeroes and receives data

	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	
	uint8_t result = tdc_read_from_spi_FIFO();
	LPC_GPIO2->FIOSET =	(1<<7);				//CS high

	return result;
}

uint32_t tdc_get_register_24b(uint8_t address)
{
	LPC_GPIO2->FIOCLR =	(1<<7);				//CS low
	tdc_send_command(0, 0, address);
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	uint32_t temp;

	while( LPC_SSP0->SR & 1<<2 )			//if receive FIFO not empty
		tdc_read_from_spi_FIFO();			//empty the receive FIFO

	tdc_write_to_spi_FIFO(0);				//writes zeroes and receives data
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	temp  = tdc_read_from_spi_FIFO();
	temp  =  temp<<8;

	tdc_write_to_spi_FIFO(0);				//writes zeroes and receives data
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	temp |= tdc_read_from_spi_FIFO();
	temp  =  temp<<8;

	tdc_write_to_spi_FIFO(0);				//writes zeroes and receives data
	while( LPC_SSP0->SR & 1<<4 );			//wait if SSP is busy
	temp |= tdc_read_from_spi_FIFO();

	LPC_GPIO2->FIOSET =	(1<<7);				//CS high
	return temp;
}

void tdc_spi_start()
{
	while( LPC_SSP0->SR & 1<<4 );		//wait if SSP is busy
	printf("reconfiguring SSP\n");
	LPC_SSP0->CR1 = 0;				//SSP disable
	LPC_SSP0->CR0 &= ~(0x3<<6);		//SSP reconfig
	LPC_SSP0->CR0 |=  (100<<8);		
	LPC_SSP0->CR1 = 2;				//SSP enable
}

void tdc_spi_stop()
{
	while( LPC_SSP0->SR & 1<<4 );		//wait if SSP is busy
	printf("reconfiguring SSP\n");
	LPC_GPIO2->FIOSET =	(1<<7);	//CS high
	LPC_SSP0->CR1 = 0;			//SSP disable
	LPC_SSP0->CR0 |=  (0x3<<6);	//SSP reconfig
	LPC_SSP0->CR0 &= ~(100<<8);
	LPC_SSP0->CR1 = 2;			//SSP enable
}

void tdc_init()
{
	LPC_GPIO0->FIOCLR = (1<<0);				//enable low
	_delay_ms(10);				
	LPC_GPIO0->FIOSET = (1<<0);				//enable high
	_delay_ms(10);
	LPC_GPIO2->FIOSET =	(1<<7);				//CS high
	_delay_ms(10);
	
	config1_data = 0x02;					//measurement mode 2

	tdc_spi_start();
	tdc_set_register(CONFIG1, config1_data);	

	_delay_ms(1);
	
	tdc_set_register(CONFIG2, 0xC0);		//Calibration 2 - measuring 40CLOCK periods
	tdc_spi_stop();
}

#define CALIBRATION2_PERIODS 40
#define CLOCK_PERIOD 100000		//10MHz period = 100 ns = 100 000 ps

uint32_t tdc_measure()
{
	printf("in tdc_measure\n");
	tdc_spi_start();

	uint8_t config2 = tdc_get_register_8b(CONFIG2);
	uint8_t int_mask = tdc_get_register_8b(INT_MASK);

	printf("config2: %x\n", config2);
	printf("int_mask: %x\n", int_mask);

	printf("starting measurement\n");
	tdc_set_register(CONFIG1, config1_data | 0x01); //start measurement

	tdc_spi_stop();

	set_status_waiting();
	while(LPC_GPIO0->FIOPIN & (1<<1));			//wait for interrupt from TDC (P0.1 active low)
	set_status_idle();

	tdc_spi_start();

	printf("getting results\n");
	uint8_t status = tdc_get_register_8b(INT_STATUS);
	tdc_set_register(INT_STATUS, 0b11111);				//clear all interrupts
	uint32_t time1 = tdc_get_register_24b(TIME1);
	uint32_t clock_count1 = tdc_get_register_24b(CLOCK_COUNT1);
	uint32_t time2 = tdc_get_register_24b(TIME2);
	uint32_t clock_count2 = tdc_get_register_24b(CLOCK_COUNT2);
	uint32_t calibration1 = tdc_get_register_24b(CALIBRATION1);
	uint32_t calibration2 = tdc_get_register_24b(CALIBRATION2);

	printf("calculating time\n");
	uint32_t calCount = (calibration2 - calibration1)/(CALIBRATION2_PERIODS - 1);
	uint32_t normLSB	= CLOCK_PERIOD/calCount;
	uint32_t TOF1 = time1*normLSB + clock_count1*CLOCK_PERIOD - time2*normLSB;

	tdc_spi_stop();

	return TOF1;
}