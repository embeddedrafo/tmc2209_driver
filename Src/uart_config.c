#include "stm32f4xx.h"

#define F_PCLK2 	16000000
#define BAUD_RATE	115200

#define GPIOA_EN 	(1U<<0)
#define USART1_EN	(1U<<4)
#define CR1_M		(1U<<12)

#define CR2_LINEN	(1U<<14)
#define CR2_CLKEN	(1U<<11)
#define CR3_SCEN	(1U<<5)
#define CR3_IREN	(1U<<1)

#define CR3_HDSEL	(1U<<3)

#define CR1_UE		(1U<<13)
#define CR1_TE		(1U<<3)
#define CR1_RE		(1U<<2)

#define SR_TXE		(1U<<7)
#define SR_TC		(1U<<6)
#define SR_RXNE		(1U<<5)

void usart1_halfduplex_init(void)
{
	/* Enable USART1 peripheral clock */
	RCC->APB2ENR |= USART1_EN;

	/* Enable GPIOA peripheral clock */
	RCC->AHB1ENR |= GPIOA_EN;

	/* Configure GPIOA9 as Alternate function mode */
	GPIOA->MODER &= ~(1U<<18);
	GPIOA->MODER |=  (1U<<19);

	/* Configure GPIOA9 as Open-Drain */
	GPIOA->OTYPER |= (1U<<9);

	/* Configure GPIOA9 as No pull-up, no pull-down */
	GPIOA->PUPDR &= ~(1U<<18);
	GPIOA->PUPDR &= ~(1U<<19);

	/* Configure GPIOA9 speed as Medium speed */
	GPIOA->OSPEEDR |=  (1U<<18);
	GPIOA->OSPEEDR &= ~(1U<<19);

	/* Configure AFR for USART1 (AF7) */
	GPIOA->AFR[1] &= ~(0xF << 4);
	GPIOA->AFR[1] |=  (0x7 << 4);

	/* Set Word length for USART1 as (1 Start bit, 8 Data bits, n Stop bit) */
	USART1->CR1 &= ~CR1_M;

	/* Set STOP bits for USART1 as (1 Stop bit) */
	USART1->CR2 &= ~(1U<<12);
	USART1->CR2 &= ~(1U<<13);

	/* Set baud rate */
	USART1->BRR = (F_PCLK2 + (BAUD_RATE / 2)) / BAUD_RATE;

	/* LIN mode disable */
	USART1->CR2 &= ~CR2_LINEN;

	/* Clock disable */
	USART1->CR2 &= ~CR2_CLKEN;

	/* Smartcard mode disable */
	USART1->CR3 &= ~CR3_SCEN;

	/* IrDA mode disable */
	USART1->CR3 &= ~CR3_IREN;

	/* Half-duplex selection */
	USART1->CR3 |= CR3_HDSEL;

	/*USART enable*/
	USART1->CR1 |= CR1_UE;

	/*Transmitter enable*/
	USART1->CR1 |= CR1_TE;

	/*Receiver enable*/
	USART1->CR1 |= CR1_RE;
}

uint8_t send_array(uint8_t *arr, uint8_t size)
{
	/*Receiver disable*/
	USART1->CR1 &= ~(CR1_RE);

	for (int i = 0; i < size; i++)
	{
		/*Wait for the Transmit Data Register to be empty*/
		while(!(USART1->SR & SR_TXE)){}

		/*Send byte*/
		USART1->DR = arr[i];
	}

	/*Wait for Transmission Complete*/
	 while(!(USART1->SR & SR_TC)){}

	/*Receiver enable*/
	USART1->CR1 |= CR1_RE;

	return 0;
}

uint8_t reccive_array(uint8_t *arr, uint8_t size)
{
	/*Transmitter disable*/
	USART1->CR1 &= ~(CR1_TE);

	(void)USART1->DR;

	for (int i = 0; i < size; i++)
	{
		/*Wait for the Received data is ready to be read*/
		while(!(USART1->SR & SR_RXNE)){}

		/*Receive byte*/
		arr[i] = (uint8_t)USART1->DR;
	}

	/*Transmitter enable*/
	USART1->CR1 |= CR1_TE;

	return 0;
}
