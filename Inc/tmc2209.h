#ifndef TMC2209_H_
#define TMC2209_H_

#include <stdint.h>

/* ==================== CHOPCONF ==================== */

typedef enum
{
    TMC2209_MICROSTEP_256      = 0x0,
    TMC2209_MICROSTEP_128      = 0x1,
    TMC2209_MICROSTEP_64       = 0x2,
    TMC2209_MICROSTEP_32       = 0x3,
    TMC2209_MICROSTEP_16       = 0x4,
    TMC2209_MICROSTEP_8        = 0x5,
    TMC2209_MICROSTEP_4        = 0x6,
    TMC2209_MICROSTEP_2        = 0x7,
    TMC2209_MICROSTEP_FULLSTEP = 0x8
} tmc2209_microstep_t;

typedef enum {
    TMC2209_INTPOL_DISABLE = 0,
    TMC2209_INTPOL_ENABLE  = 1
} tmc2209_intpol_t;

typedef enum
{
    TMC2209_TOFF_DISABLE = 0x0,
    TMC2209_TOFF_ENABLE  = 0x3
} tmc2209_toff_t;

typedef enum
{
	TMC2209_VSENSE_LOW  = 0,
	TMC2209_VSENSE_HIGH = 1
} tmc2209_vsense_t;

/* ==================== GCONF ==================== */

typedef enum
{
    TMC2209_I_SCALE_INTERNAL = 0,
    TMC2209_I_SCALE_ANALOG   = 1
} tmc2209_i_scale_t;

typedef enum
{
    TMC2209_MODE_STEALTHCHOP = 0,
    TMC2209_MODE_SPREADCYCLE = 1
} tmc2209_chop_mode_t;

typedef enum
{
    TMC2209_SHAFT_NORMAL   = 0,
    TMC2209_SHAFT_INVERTED = 1
} tmc2209_shaft_t;

/* ==================== HARDWARE / UART ==================== */

typedef uint8_t (*tmc2209_uart_send) (uint8_t *data, uint8_t length);
typedef uint8_t (*tmc2209_uart_receive) (uint8_t *data, uint8_t length);

typedef enum
{
    TMC2209_DISABLE = 0,
    TMC2209_ENABLE  = 1
} tmc2209_state_t;

typedef enum
{
    UART_ADDRESS_0 = 0x00,
    UART_ADDRESS_1 = 0x01,
    UART_ADDRESS_2 = 0x02,
    UART_ADDRESS_3 = 0x03
} tmc2209_uart_address_t;

typedef enum
{
	GCONF 	   = 0x00,
	IHOLD_IRUN = 0x10,
	CHOPCONF   = 0x6C,
	VACTUAL	   = 0x22
} tmc2209_registers_t;

typedef struct
{
	uint32_t value;
	uint8_t	 error;
} tmc2209_read_result_t;

/* ==================== DRIVER HANDLE ==================== */

typedef struct {
  tmc2209_uart_address_t addr;
  tmc2209_uart_send send;
  tmc2209_uart_receive receive;
  uint32_t _ihold_irun_shadow;
} tmc2209_t;

/* ==================== API FUNCTIONS ==================== */

/* ==================== INITIALIZATION ==================== */
void tmc2209_init_default(tmc2209_t *handler);

/* ==================== CURRENT CONTROL ==================== */
void tmc2209_set_current(tmc2209_t *handler, uint16_t ihold, uint16_t irun, uint8_t ihold_delay);
void tmc2209_set_ihold(tmc2209_t *handler, uint16_t ihold);
void tmc2209_set_irun(tmc2209_t *handler, uint16_t irun);
void tmc2209_set_ihold_delay(tmc2209_t *handler, uint16_t ihold_delay);

/* ==================== MOTION CONTROL ==================== */
void tmc2209_inverse_direction(const tmc2209_t *handler, tmc2209_shaft_t direction);
void tmc2209_set_microsteps(const tmc2209_t *handler, tmc2209_microstep_t microstep, tmc2209_intpol_t intpol);
void tmc2209_set_vactual(const tmc2209_t *handler, int32_t value);

/* ==================== CHOPPER / DRIVER STATE ==================== */
void tmc2209_set_chopper_mode(const tmc2209_t *handler, tmc2209_chop_mode_t chop_mode);
void tmc2209_set_toff(const tmc2209_t *handler, tmc2209_toff_t toff_value);
void tmc2209_enable(const tmc2209_t *handler);
void tmc2209_disable(const tmc2209_t *handler);

#endif /* TMC2209_H_ */
