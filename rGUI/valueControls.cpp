#include "valueControls.h"


extern "C" {
  #include "GLCD.h"
}


valueControls::valueControls(int _x, int _y, uint16_t currentValue, void (*_set)(uint16_t) )
{
	this->set=_set;
	x=_x;
	y=_y;
	value=currentValue;
	color=BG;
	text_color=Black;
	plusIntVoltButton = button();
	minusIntVoltButton = button();
	//plusIntVoltButton.assignHandler(std::bind(&valueControls::plusVolt, this));
	//minusIntVoltButton.assignHandler(std::bind(&valueControls::minusVolt, this));

	plusIntVoltButton.x = x;                          
	plusIntVoltButton.y = y+100;
	plusIntVoltButton.label = "+";
	plusIntVoltButton.show();

	minusIntVoltButton.x = x;
	minusIntVoltButton.y = y;
	minusIntVoltButton.label = "-";
	minusIntVoltButton.show();

	redrawValue();
}

valueControls::~valueControls()
{
	LCD_DrawRectangle(x, y, x+50, y+150, BG);
}

void valueControls::plusVolt()
{
	value++;
	checkRange();
	redrawValue();
	this->set(value);
}

void valueControls::minusVolt()
{
	value--;
	checkRange();
	redrawValue();
	this->set(value);
}

void valueControls::checkRange()
{
	if(value==21)                                       
		value=0;
	else if(value==255)
		value=20; 
}

void valueControls::redrawValue()
{
	sprintf(str, "%3d", value);
	GUI_Text(x+10, y+80, (uint8_t*)str, text_color, color);
}

void valueControls::addToValue(int8_t additive)
{
	value += additive;
	checkRange();
	redrawValue();
	this->set(value);
}