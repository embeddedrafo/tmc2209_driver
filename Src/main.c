#include "stm32f4xx.h"
#include "uart_config.h"
#include "tmc2209.h"
#include "timer.h"
#include "systick.h"

volatile uint32_t status = 0;

int main(void)
{
	systick_init();

	usart1_halfduplex_init();
	timer3_ch1_pwm_init();

	delay_ms(50);

	tmc2209_t motorX;
	motorX.addr = UART_ADDRESS_0;
	motorX.send = send_array;
	motorX.receive = reccive_array;

	tmc2209_init_default(&motorX);
	delay_ms(20);

	tmc2209_enable(&motorX);
	delay_ms(20);

	while(1)
	{

	}
}
