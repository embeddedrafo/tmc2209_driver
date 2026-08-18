#include "stm32f4xx.h"
#include "uart_config.h"
#include "tmc2209.h"
#include "timer.h"

int main(void)
{
	usart1_halfduplex_init();

	for(volatile int i = 0; i < 60000; i++){}

	timer3_ch1_pwm_init();

	for(volatile int i = 0; i < 60000; i++){}

	tmc2209_t motor1;

	motor1.addr = UART_ADDRESS_0;
	motor1.send = send_array;
	motor1.receive = reccive_array;

	tmc2209_init_default(&motor1);

	for(volatile int i = 0; i < 60000; i++){}

	tmc2209_enable(&motor1);

	for(volatile int i = 0; i < 60000; i++){}

	while(1)
	{

	}
}
