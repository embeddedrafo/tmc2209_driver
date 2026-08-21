#include "stm32f4xx.h"
#include "uart_config.h"
#include "tmc2209.h"
#include "timer.h"
#include "systick.h"

int main(void)
{
	systick_init();

	usart1_halfduplex_init();
	timer3_ch1_pwm_init();

	delay_ms(50);

	tmc2209_t motor1;
	motor1.addr = UART_ADDRESS_0;
	motor1.send = send_array;
	motor1.receive = reccive_array;

	tmc2209_init_default(&motor1);
	delay_ms(20);

	tmc2209_set_chopper_mode(&motor1, TMC2209_MODE_SPREADCYCLE);
	delay_ms(20);

	tmc2209_enable(&motor1);
	delay_ms(20);

	while(1)
	{

	}
}
