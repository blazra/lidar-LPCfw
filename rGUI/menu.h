#ifndef MENU_H
#define MENU_H

#include <memory>
#include "keypad.h"
#include "valueControls.h"
#include "button.h"
#include "gui.h"
extern "C" {
	#include "GLCD.h"
}

class menu
{
public:
	std::unique_ptr<button> menuButton;
	std::unique_ptr<keypad> myKeypad;
	std::unique_ptr<valueControls> myValControls;

	uint16_t (*get)();
	void (*set)(uint16_t);

	int x;
	int y;

	menu(int _x, int _y, char label[], uint16_t (*get)(), void (*set)(uint16_t));
	~menu();

	void handleTouch();
	void showMenu();
	void showMenuButton();
	void setLabel(char label[]);
};


#endif