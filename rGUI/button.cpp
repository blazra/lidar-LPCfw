#include "button.h"

#include <string.h>
extern "C" {
  #include "GLCD.h"
}
#include <vector>
#include <stdio.h>
#include "gui.h" //for blink()


std::vector<area> button::touchAreas;
int button::count = 0;


button::button()
:button(const_cast<char*>("label"))
{
  
}

button::button(char* _label)
:button(_label, 0, 0)
{

}

button::button(char* _label, int _x, int _y)
{
  value = 0;
  x = _x;
  y = _y;
  YlabelOffset = 0;
  sizeX = 50;
  sizeY = 50;
  color = RGB565CONVERT(70, 130, 180);
  text_color = RGB565CONVERT(39, 67, 82);
  label = _label;
  roll_factor = 1;

  area newArea;
  newArea.x1=_x;
  newArea.y1=_y;
  newArea.x2=_x+sizeX;
  newArea.y2=_y+sizeY;
  newArea.objPtr=this;
  button::touchAreas.push_back(newArea);
  printf("adding newArea for button\n");

  touch_handler_roller = &button::defaultHandler;
}

button::button(char* _label, int _x, int _y, void(*handler)())
:button(_label, _x, _y)
{
  touch_handler_function = handler;
  mode = 'F';
}

button::~button()
{
  printf("destructing button\n");
  
}

void button::show()
{
  LCD_DrawRectangle(x, y, x+sizeX, y+sizeY, color);
  GUI_Text(x-(strlen(label)*4)+(sizeX/2), y+8+(sizeY/2)+YlabelOffset, (uint8_t*)label, text_color, color);
  if(mode == 'R')
    redrawValue();
}

void button::redrawValue()
{
  char buffer[10];
  sprintf(buffer,"%d", value);
  LCD_DrawRectangle(x, y, x+sizeX, y+16, color);
  GUI_Text(x-(strlen(label)*4)+(sizeX/2), y+8+(sizeY/2)-YlabelOffset, (uint8_t*)buffer, text_color, color);
}

bool button::touched(long _x, long _y)
{
  return (_x>=x && _y>=y && _x<=x+sizeX && _y<=y+sizeY);
}

void button::touchDiscriminator(long _x, long _y)
{
  printf("in touchDiscriminator\n");
  if(_x>=x && _y>=y && _x<=x+sizeX && _y<=y+sizeY)
  {
    if(mode == 'R')
    {
      printf("calling touch_handler_roller\n");
      (this->*touch_handler_roller)();
    }
    else if(mode == 'F')
    {
      printf("calling touch_handler_function\n");
      this->touch_handler_function();
    }
  }
}

void button::assignHandler(void (button::*handler)())
{
  //touch_handler = handler;
}

void button::defaultHandler()
{
  setPtrToCurrentValue(this, &button::addToValue);
}

void button::addToValue(int8_t change)
{
  change = change*roll_factor;
  value += change;
  redrawValue();
}

void button::processTouch(int x, int y)
{
  printf("in processTouch\n");
  for(uint8_t i=0; i < touchAreas.size(); i++) {
    printf("size of touchAreas: %d\n", touchAreas.size());
    printf("calling touchDiscriminator number %d\n", i);
    touchAreas[i].objPtr->touchDiscriminator(x,y);
  }
}