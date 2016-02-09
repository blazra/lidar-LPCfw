#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>


class status
{
public:
  status();
  //~status();

  int x;
  int y;
  enum statuses
  {
  	Idle,
  	Transmitting,
    Waiting,
  	Measuring,
    Changing_parameters
  };
  statuses current_status = Idle;

  void set(statuses status);
  int get();
  void redraw(); 
  
};


#endif