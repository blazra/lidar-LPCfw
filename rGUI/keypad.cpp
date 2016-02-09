#include "keypad.h"

extern "C" {
  #include "GLCD.h"
}

keypad::keypad()
{

	keypad::buttons = {		button(const_cast<char*>("0"), x+0*60, y+0*60),
							button(const_cast<char*>("OK"), x+2*60, y+0*60),
							button(const_cast<char*>("1"), x+0*60, y+1*60),
							button(const_cast<char*>("2"), x+1*60, y+1*60),
							button(const_cast<char*>("3"), x+2*60, y+1*60),
							button(const_cast<char*>("4"), x+0*60, y+2*60),
							button(const_cast<char*>("5"), x+1*60, y+2*60),
							button(const_cast<char*>("6"), x+2*60, y+2*60),
							button(const_cast<char*>("7"), x+0*60, y+3*60),
							button(const_cast<char*>("8"), x+1*60, y+3*60),
							button(const_cast<char*>("9"), x+2*60, y+3*60)
						};

	for (int i = 0; i < 11; ++i)
	{
		buttons[i].show();
	}
}

keypad::~keypad()
{
	LCD_DrawRectangle(x, y, x+170, y+230, BG);
}
