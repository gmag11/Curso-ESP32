#ifndef TIMESYNC_H
#define TIMESYNC_H


#include <Arduino.h>


void initTimeSync ();
void time_sync_cb (timeval* ntptime);

#endif // TIMESYNC_H