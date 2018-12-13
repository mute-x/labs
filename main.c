//coding: KOI8-U
#include <stm32f4xx.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <locale.h>

#include "system.h"
#include "display.h"
#include "ultrasonic.h"

#define TEMP 22 // TODO get data from temperature sensor

static char error[] = "----------------";
static char letters[20] = "----------------";

static bool update;

void SysTick_Handler()
{
	HAL_IncTick();
}

void setup(void)
{
	set_system_clock();
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_InitTick(0);

	display_init();
	ultrasonic_init();

	setlocale(LC_NUMERIC, "uk_UA");
}

void ultrasonic_error()
{
	memcpy(letters, error, sizeof(error));
}

void ultrasonic_completed(uint16_t time)
{
	if (time >= 37000)
		ultrasonic_error();
	else {
		snprintf(letters, sizeof(letters), "%06.3f м", (331.5 + 0.6 * 22) * time / 2000000.);
		update = true;
	}
}

int main(void)
{
	setup();
	int x;
	while (1) {
		x = 0;

		ultrasonic_start();
		while (!update)
			__WFI();

		display_set_write_position(3, 25);

		for (int i = 0; i < strlen(letters); i++) {
			x += display_putletter(letters[i]);
		}

		while (HAL_GetTick() % 1000)
			__WFI();
		display_clear();
	}
	return 0;
}
