#include "stm32f4xx.h"

#define GPIOA_EN 	(1U << 0)

#define TIM3_EN		(1U << 1)
#define CR1_CEN		(1U << 0)
#define CR1_ARPE	(1U << 7)

#define CCMR1_OC1PE	(1U << 3)

#define CCER_CC1E	(1U << 0)

#define DIER_UIE	(1U << 0)

#define	SR_UIF 		(1U << 0)

void timer3_ch1_pwm_init(void)
{
	/* Enable GPIOA peripheral clock */
	RCC->AHB1ENR |= GPIOA_EN;

	/* Configure GPIOA6 as Alternate function mode */
	GPIOA->MODER &= ~(1U<<12);
	GPIOA->MODER |=  (1U<<13);

	/* Configure GPIOA6 as push-pull */
	GPIOA->OTYPER &= ~(1U<<6);

	/* Configure GPIOA6 as No pull-up, no pull-down */
	GPIOA->PUPDR &= ~(1U<<12);
	GPIOA->PUPDR &= ~(1U<<13);

	/* Configure GPIOA6 speed as Medium speed */
	GPIOA->OSPEEDR |=  (1U<<12);
	GPIOA->OSPEEDR &= ~(1U<<13);

	/* Configure AFR for TIM3_CH1 (AF2) */
	GPIOA->AFR[0] &= ~(0xF << 24);
	GPIOA->AFR[0] |=  (0x2 << 24);

	/* Enable TIM3 peripheral clock */
	RCC->APB1ENR |= TIM3_EN;

	/* Set prescaler (PSC) value */
	TIM3->PSC = 16 - 1;

	/* Set auto-reload register (ARR) value */
	TIM3->ARR = 200 - 1;

	/* Set pulse width */
	TIM3->CCR1 = (TIM3->ARR + 1) / 2;

	/* Set PWM mode */
	TIM3->CCMR1 &= ~(0x7 << 4);
	TIM3->CCMR1 |=  (0x6 << 4);

	/*  Output compare 1 preload enable	 */
	TIM3->CCMR1 |= CCMR1_OC1PE;

	/* Auto-reload preload enable */
	TIM3->CR1 |= CR1_ARPE;

	/* Capture/Compare 1 output enable */
	TIM3->CCER |= CCER_CC1E;

	/* Clear counter */
	TIM3->CNT = 0;

	/* Enable Timer */
	TIM3->CR1 |= CR1_CEN;

	/* Enable Timer interrupt */
	TIM3->DIER |= DIER_UIE;

	/* Enable Timer interrupt in NVIC */
	NVIC_EnableIRQ(TIM3_IRQn);
}

void timer3_set_freq(uint16_t freq)
{
	if (freq < 16) return;

	TIM3->ARR = (1000000U / freq) - 1;

	TIM3->CCR1 = (TIM3->ARR + 1) / 2;
}

void TIM3_IRQHandler(void)
{
	/* Clear update interrupt flag */
    TIM3->SR &= ~SR_UIF;
}
