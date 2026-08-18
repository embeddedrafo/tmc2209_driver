#ifndef UART_CONFIG_H_
#define UART_CONFIG_H_

void usart1_halfduplex_init(void);
uint8_t send_array(uint8_t *arr, uint8_t size);
uint8_t reccive_array(uint8_t *arr, uint8_t size);

#endif /* UART_CONFIG_H_ */
