#include "stm32f4xx.h"

/* Documentation for configuring SysTick:
 * ARM Cortex-M4 Generic User Guide
 */

#define FRC_CLOCK		(16000000)

#define CTRL_ENABLE     (1U << 0)
#define CTRL_TICKINT    (1U << 1)
#define CTRL_CLKSOURCE  (1U << 2)

static volatile uint32_t ms_ticks = 0;

void systick_init(void)
{
	/* Set reload value for 1 ms interrupt */
    SysTick->LOAD = (FRC_CLOCK / 1000) - 1;

    /* Reset current counter value */
    SysTick->VAL  = 0;

    /* Set processor clock, enable interrupt, and enable counter */
    SysTick->CTRL = CTRL_CLKSOURCE | CTRL_TICKINT | CTRL_ENABLE;
}

void SysTick_Handler(void)
{
	/* Increment counter at each interrupt */
    ms_ticks++;
}

uint32_t get_ticks(void)
{
    return ms_ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;

    while ((ms_ticks - start) < ms) {}
}
