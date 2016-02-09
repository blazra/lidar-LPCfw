#ifndef KEYPAD_H
#define KEYPAD_H

#include <vector>
#include "button.h"

class keypad
{
public:
	const int x = 140;
	const int y = 5;
	std::vector<button> buttons;

	keypad();
	~keypad();
	
};

#endif