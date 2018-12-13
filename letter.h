/* (c) 2018 ukrkyi */
#ifndef LETTER_H
#define LETTER_H

#include <stdint.h>

struct letter {
	uint8_t w;
	uint8_t letter[7];
} extern const alpha[256];

#endif // LETTER_H
