#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <vector>

typedef struct area area;

class button
{
public:

  static std::vector<area> touchAreas;
  static int count;

  char mode = 'R'; //R for Roller or F for function
  int x,y,sizeX,sizeY, YlabelOffset;
  uint16_t color;
  uint16_t text_color;
  char* label;
  uint16_t value;
  uint8_t roll_factor;
  void (button::*touch_handler_roller)();
  void (*touch_handler_function)();

  button();
  button(char* label);
  button(char* _label, int _x, int _y);
  button(char* _label, int _x, int _y, void(*handler)());

  ~button();

  void show();
  bool touched(long _x, long _y);
  void touchDiscriminator(long _x, long _y);
  void handleTouch();
  void assignHandler(void (button::*handler)());
  void defaultHandler();
  void addToValue(int8_t);
  void redrawValue();
  static void processTouch(int x, int y);
};

struct area {
    int x1;
    int y1;
    int x2;
    int y2;
    button* objPtr;
  };


#endif