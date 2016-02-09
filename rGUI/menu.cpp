#include "menu.h"

#include <functional>

menu::menu(int _x, int _y, char label[], uint16_t (*_get)(), void (*_set)(uint16_t))
{
	this->x = _x;
	this->y = _y;

	this->get = _get;
	this->set = _set;

	menuButton.reset(new button());
	menuButton->x = x;
	menuButton->y = y;
	menuButton->sizeX = 65;
	menuButton->YlabelOffset = 16;
	menuButton->label = label;
	//menuButton->assignHandler(std::bind(&menu::handleTouch, this));

	showMenuButton();
}
menu::~menu()
{

}

void menu::showMenu()
{
	//blink();
	
}

void menu::setLabel(char label[])
{
	this->menuButton->label = label;
}

void menu::showMenuButton()
{
  	menuButton->show();
}

void menu::handleTouch()
{
	if(myValControls)
	{
		myValControls.reset(nullptr);
	}
	else
		myValControls.reset(new valueControls(250, 40, get(), this->set));

}