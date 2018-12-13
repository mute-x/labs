/* (c) 2018 ukrkyi */

/*
 * Pins used:
 *	E10 ---> GPIO ---> RST
 *	E11 ---> GPIO ---> CE
 *	E12 ---> SCK4 ---> CLK
 *	E13 ---> GPIO ---> DC
 *	E14 ---> MOSI4 --> DIN
 *
 * Peripherals used:
 *	SPI1 @ 4 MHz
 *
 */

// Incluudes

#include "display.h"
#include "spi.h"
#include "letter.h"

#include <stm32f4xx.h>

// Types and variables

enum qtype_t {COMMAND, DATA, DUMMY};

static struct __attribute__((packed)) command{
	uint8_t type;
	uint8_t data;
} queue[256];
static volatile uint8_t head = 0, tail = 0;

// Function declaration

static inline void set_cmd_mode();
static inline void set_data_mode();

static inline void cs_enable();
static inline void cs_disable();

static inline bool queue_put(enum qtype_t type, uint8_t data);
static inline void queue_start_send(void);

static inline int max(int a, int b);
static inline int min(int a, int b);

// Defines

#define LCDWIDTH 84
#define LCDHEIGHT 48

#define LINENUMBER 6

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

// Functions

void display_init()
{
	// Port E setup
	__HAL_RCC_GPIOE_CLK_ENABLE();

	GPIO_InitTypeDef conf = {
		.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_13,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH,
		.Alternate = GPIO_AF2_TIM4,
	};
	HAL_GPIO_Init(GPIOE, &conf);

	spi_init(1, SystemCoreClock / 4000000);

	GPIOE->BSRR = GPIO_BSRR_BR10;
	for (volatile int i = 0; i < 100; i++);
	GPIOE->BSRR = GPIO_BSRR_BS10;

	// Flush queue
	head = tail = 0;
	while (spi_is_tx()); // Wait for all previous commands to complete

	// get into the EXTENDED mode!
	queue_put(COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);

	// LCD bias select (4 is optimal?)
	queue_put(COMMAND, PCD8544_SETBIAS | 4);

	// set VOP
	queue_put(COMMAND, PCD8544_SETVOP | 0x3F);

	// normal mode
	queue_put(COMMAND, PCD8544_FUNCTIONSET);

	// Set display to Normal
	queue_put(COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

	// Start sending commands
	queue_start_send();

	display_clear();
}

void display_clear()
{
	// This should start sending data
	display_set_write_position(0, 0);

	for (int i = 0; i < 504; i++)
		while (!queue_put(DATA, 0x00))
			queue_start_send(); // Maybe we are not transmitting? hmm....
}

void display_set_write_position(unsigned line, unsigned offset)
{
	if (line >= LINENUMBER || offset >= LCDWIDTH)
		return;

	while (!queue_put(COMMAND, PCD8544_SETYADDR | line));
	while (!queue_put(COMMAND, PCD8544_SETXADDR | offset));

	// Try transmit if nothing is transmitted
	queue_start_send();
}

int display_putletter(unsigned char letter)
{
	int i;

	for (i = 0; i < alpha[letter].w; i++)
		while (!queue_put(DATA, alpha[letter].letter[i]));

	// Try transmit if nothing is transmitted
	queue_start_send();

	return alpha[letter].w;
}

bool queue_put(enum qtype_t type, uint8_t data)
{
	if (tail == head - 1)
		return false;
	struct command cmd = {.type = type, .data = data};
	bool irq = __get_PRIMASK();
	__disable_irq();
	queue[tail++] = cmd;
	if (!irq)
		__enable_irq();
	return true;
}

static inline void queue_start_send(void)
{
	if (spi_can_send()) {
		head++;
		cs_enable();
		if (queue[head - 1].type == COMMAND)
			set_cmd_mode();
		else if (queue[head - 1].type == DATA)
			set_data_mode();
		spi_send(queue[head - 1].data);
	}
}

void spi_tx_cplt()
{
	static const struct command dummy = {.type = DUMMY};
	// We are always transmitting something
	// Configure pins for previous byte
	switch (queue[head - 1].type) {
	case COMMAND:
		cs_enable();
		set_cmd_mode();
		break;
	case DATA:
		cs_enable();
		set_data_mode();
		break;
	case DUMMY:
		if (head == tail) {
			cs_disable();
			return;
		}
		break;
	}
	if (head != tail) {
		spi_send(queue[head++].data);
	} else {
		queue[tail++] = dummy;
		head = tail;
		spi_send(0);
	}
}

void set_cmd_mode()
{
	GPIOE->BSRR = GPIO_BSRR_BR13;
}
void set_data_mode()
{
	GPIOE->BSRR = GPIO_BSRR_BS13;
}

void cs_enable()
{
	GPIOE->BSRR = GPIO_BSRR_BR11;
}
void cs_disable()
{
	GPIOE->BSRR = GPIO_BSRR_BS11;
}

static inline int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}

static inline int min(int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}
