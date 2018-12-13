/* (c) 2018 ukrkyi */

/*
 * Driver for SPI transmission.
 *
 * Pins used:
 *	E12 ---> SCK4
 *	E14 ---> MOSI4
 *
 * Peripherals used:
 *	SPI4
 *
 */
#include "spi.h"
#include <stm32f4xx.h>

#define SPI SPI4

void spi_init(int count, int div)
{
	// Calculate divider
	uint16_t divider = 0;

	while (div > 2 && divider < 7) {
		divider++;
		div >>= 1;
	}

	// Clocks enable
	__HAL_RCC_SPI4_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	// SPI configuration
	CLEAR_BIT(SPI->CR1, SPI_CR1_SPE);
	if (count == 2)
		SET_BIT(SPI->CR1, SPI_CR1_DFF);
	else
		CLEAR_BIT(SPI->CR1, SPI_CR1_DFF);
	MODIFY_REG(SPI->CR1, SPI_CR1_BR,
		   (divider << SPI_CR1_BR_Pos) | SPI_CR1_MSTR | SPI_CR1_SPE | SPI_CR1_SSM | SPI_CR1_SSI);
	NVIC_EnableIRQ(SPI4_IRQn);
	HAL_NVIC_SetPriority(SPI4_IRQn, 1, 0);

	// Pins configuration
	GPIO_InitTypeDef conf = {
		.Pin = GPIO_PIN_12 | GPIO_PIN_14,
		.Mode = GPIO_MODE_AF_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH,
		.Alternate = GPIO_AF5_SPI4,
	};
	HAL_GPIO_Init(GPIOE, &conf);
}

bool spi_send(uint16_t data)
{
	if (!spi_can_send())
		return false;
	SPI->DR = data;
	SET_BIT(SPI->CR2, SPI_CR2_TXEIE);
	return true;
}

bool spi_can_send()
{
	return READ_BIT(SPI->SR, SPI_SR_TXE);
}

bool spi_is_tx()
{
	if (READ_BIT(SPI->SR, SPI_SR_BSY | SPI_SR_TXE) != SPI_SR_TXE)
		return true;
	else
		return false;
}

void SPI4_IRQHandler()
{
	CLEAR_BIT(SPI->CR2, SPI_CR2_TXEIE);
	spi_tx_cplt();
}

__attribute__((weak)) void spi_tx_cplt(){}
