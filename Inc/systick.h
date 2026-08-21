#ifndef SYSTICK_H_
#define SYSTICK_H_

void systick_init(void);
uint32_t get_ticks(void);
void delay_ms(uint32_t ms);

#endif /* SYSTICK_H_ */
