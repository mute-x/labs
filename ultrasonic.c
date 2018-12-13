/* (c) 2018 ukrkyi */

/*
 * Pins used:
 *	С8 ---> GPIO    ---> TRIG
 *	C6 ---> TIM3CH1 ---> ECHO
 *
 * Peripherals used:
 *	TIM3 @ 1 MHz
 *        - CH1 - Reset
 *        - CH2 - Input Capture
 */

#include "ultrasonic.h"

#include <stm32f4xx.h>
#include <stdbool.h>

TIM_HandleTypeDef htim = {.Instance = TIM3};

static bool echo_started = false;

void ultrasonic_init() {
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef gpioConf = {
		.Pin = GPIO_PIN_6,
		.Mode = GPIO_MODE_AF_OD,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH,
		.Alternate = GPIO_AF2_TIM3,
	};
	HAL_GPIO_Init(GPIOC, &gpioConf);
	gpioConf.Pin = GPIO_PIN_8;
	gpioConf.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOC, &gpioConf);
	TIM_Base_InitTypeDef tim_binit = {
		.Prescaler = 99,
		.CounterMode = TIM_COUNTERMODE_UP,
		.Period = 65535,
		.ClockDivision = TIM_CLOCKDIVISION_DIV1,
	};
	htim.Init = tim_binit;
	HAL_TIM_IC_Init(&htim);
	TIM_SlaveConfigTypeDef tim_sinit = {
		.SlaveMode = TIM_SLAVEMODE_RESET,
		.InputTrigger = TIM_TS_TI1FP1,
		.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING,
		.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1,
		.TriggerFilter = 1,
	};
	HAL_TIM_SlaveConfigSynchronization(&htim, &tim_sinit);
	TIM_IC_InitTypeDef tim_cinit = {
		.ICPolarity = TIM_ICPOLARITY_FALLING,
		.ICSelection = TIM_ICSELECTION_INDIRECTTI,
		.ICPrescaler = TIM_ICPSC_DIV1,
		.ICFilter = 1,
	};
	HAL_TIM_IC_ConfigChannel(&htim, &tim_cinit, TIM_CHANNEL_2);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void ultrasonic_start()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
	for (volatile int i = 0; i < 2000; i++);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

	__HAL_TIM_CLEAR_IT(&htim, TIM_IT_UPDATE | TIM_IT_CC2);
	HAL_TIM_IC_Start_IT(&htim, TIM_CHANNEL_2);
	__HAL_TIM_ENABLE_IT(&htim, TIM_IT_UPDATE);
	echo_started = false;
}

void ultrasonic_stop()
{
	HAL_TIM_IC_Stop_IT(&htim, TIM_CHANNEL_2);
	__HAL_TIM_DISABLE_IT(&htim, TIM_IT_UPDATE);
}

void TIM3_IRQHandler(void)
{
	// Capture
	if(__HAL_TIM_GET_FLAG(&htim, TIM_FLAG_CC2) != RESET &&
	   __HAL_TIM_GET_IT_SOURCE(&htim, TIM_IT_CC2) != RESET)
	{
		__HAL_TIM_CLEAR_IT(&htim, TIM_IT_CC2);
		htim.Channel = HAL_TIM_ACTIVE_CHANNEL_2;

		/* Input capture event */
		if((htim.Instance->CCMR1 & TIM_CCMR1_CC2S) != 0x00U)
		{
			ultrasonic_completed(HAL_TIM_ReadCapturedValue(&htim, TIM_CHANNEL_2));
			ultrasonic_stop();
		}
		htim.Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
	}

	// Update
	if(__HAL_TIM_GET_FLAG(&htim, TIM_FLAG_UPDATE) != RESET &&
	   __HAL_TIM_GET_IT_SOURCE(&htim, TIM_IT_UPDATE) !=RESET)
	{
		__HAL_TIM_CLEAR_IT(&htim, TIM_IT_UPDATE);
		if (echo_started) {
			ultrasonic_error();
			ultrasonic_stop();
		} else {
			echo_started = true;
		}
	}
}

__attribute__((weak)) void ultrasonic_error(){}
__attribute__((weak)) void ultrasonic_completed(uint16_t time){}
