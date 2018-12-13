/* (c) 2018 ukrkyi */
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include <stdbool.h>

extern void ultrasonic_init();

extern void ultrasonic_start();

extern void ultrasonic_error();
extern void ultrasonic_completed(uint16_t time);

#endif // ULTRASONIC_H
