#include "status.h"

extern "C" {
	#include "GLCD.h"
}

status::status()
{
	x=10;
	y=12;
}

void status::set(statuses _status)
{
	current_status = _status;
	redraw();
}

int status::get()
{
	return current_status;
}

void status::redraw()
{
	LCD_DrawRectangle(0, 0, 320, 12, White);

	switch(current_status)
	{
		case Idle:
			GUI_Text(x,y,(uint8_t*)"Status: Idle",Black,White);
			break;

		case Transmitting:
			GUI_Text(x,y,(uint8_t*)"Status: Transmitting",Black,White);
			break;

		case Waiting:
			GUI_Text(x,y,(uint8_t*)"Status: Waiting",Black,White);
			break;

		case Measuring:
			GUI_Text(x,y,(uint8_t*)"Status: Measuring",Black,White);
			break;

		case Changing_parameters:
			GUI_Text(x,y,(uint8_t*)"Status: Changing parameters",Black,White);
			break;
	}
}