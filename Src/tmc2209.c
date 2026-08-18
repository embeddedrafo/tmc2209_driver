#include "tmc2209.h"

uint8_t tmc2209_crc(uint8_t *data, uint8_t length);

/* ==================== TRANSPORT LAYER ==================== */

void tmc2209_write_register(const tmc2209_t *handler, tmc2209_registers_t reg_addr, uint32_t data)
{

	uint8_t packet[8] = {
			0x05,
			handler->addr,
			reg_addr | 0x80,
			(data >> 24) & 0xFF,
			(data >> 16) & 0xFF,
			(data >> 8) & 0xFF,
			data & 0xFF,
	};

	packet[7] = tmc2209_crc(packet, 7);

	handler->send(packet, 8);
}

uint32_t tmc2209_read_register(const tmc2209_t *handler, tmc2209_registers_t reg_addr)
{

	uint8_t  receive_data[8] = {0};
	uint32_t reg_value = 0;

	uint8_t packet[4] = {
			0x05,
			handler->addr,
			reg_addr & 0x7F,
	};

	packet[3] = tmc2209_crc(packet, 3);

	handler->send(packet, 4);

	handler->receive(receive_data, 8);

	reg_value |= (receive_data[3] & 0xFF) << 24;
	reg_value |= (receive_data[4] & 0xFF) << 16;
	reg_value |= (receive_data[5] & 0xFF) << 8;
	reg_value |= (receive_data[6] & 0xFF) << 0;

	return reg_value ;
}

uint8_t tmc2209_crc(uint8_t *data, uint8_t length)
{
    uint8_t crc = 0;

    for(uint8_t i = 0; i < length; i++)
    {
        uint8_t currentByte = data[i];

        for(uint8_t j = 0; j < 8; j++)
        {
            if ((crc >> 7) ^ (currentByte & 0x01))
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc = (crc << 1);
            }

            currentByte >>= 1;
        }
    }

    return crc;
}

/* ==================== INITIALIZATION ==================== */

void tmc2209_init_default(tmc2209_t *handler)
{
	//	uint32_t gconf = 0x000001C0;

	uint32_t gconf = 0;

	gconf &= ~(1U<<0); // i_scale_analog
	gconf &= ~(1U<<1); // internal_Rsense
	gconf &= ~(1U<<2); // en_spreadcycle
	gconf &= ~(1U<<3); // shaft
	gconf &= ~(1U<<4); // index_otpw
	gconf &= ~(1U<<5); // index_step
	gconf |=  (1U<<6); // pdn_disable
	gconf |=  (1U<<7); // mstep_reg_select
	gconf |=  (1U<<8); // multistep_filt
	gconf &= ~(1U<<9); // test_mode

	//    uint32_t chopconf = 0x140081B3;

	uint32_t chopconf = 0;

	chopconf |=  (0b0000 & 0xF) << 0; 	// toff
	chopconf |=  (0b011  & 0x7) << 4; 	// hstrt
	chopconf |=  (0b0011 & 0xF) << 7; 	// hend
	chopconf |=  (0b01 & 0x3) << 15; 	// tbl
	chopconf &= ~(1U << 17); 			// vsense
	chopconf |=  (0b0100 << 24); 		// mres
	chopconf |=  (1U << 28); 		    // intpol
	chopconf &= ~(1U << 29); 		    // dedge
	chopconf &= ~(1U << 30); 		    // diss2g
	chopconf &= ~(1U << 31); 		    // diss2vs

	tmc2209_write_register(handler, GCONF, gconf);

	//    uint32_t ihold_irun = 0x00061309;
	tmc2209_set_current(handler, 550, 1100, 6);

	tmc2209_write_register(handler, CHOPCONF, chopconf);
}

/* ==================== CURRENT CONTROL ==================== */

/* CS = round(I_rms[mA]/1000 * 32 * (Rsense+0.02) * sqrt(2) / V_FS) - 1
 * Rsense = 0.11 ohm (fixed for this board), V_FS = 325mV (vsense=0)
 * 5883 = 32 * (0.11+0.02) * sqrt(2) * 1000 -- precomputed coefficient for Rsense=0.11
 * +500 / 1000 -- rounding to nearest integer in integer division
 */

uint8_t tmc2209_cs_calc(uint16_t i_rms)
{
	uint32_t cs = ((((uint32_t)i_rms * 5883U) / 325) + 500U) / 1000U;
	if (cs <= 1U)
	{
		return 0;
	}
	if (cs > 31)
	{
		return 31;
	}
	return (uint8_t)(cs - 1U);
}

void tmc2209_set_ihold(tmc2209_t *handler, uint16_t ihold)
{
	handler->_ihold_irun_shadow &= ~(0x1F<<0);
	handler->_ihold_irun_shadow |= (tmc2209_cs_calc(ihold) & 0x1F) << 0;
    tmc2209_write_register(handler, IHOLD_IRUN, handler->_ihold_irun_shadow);
}

void tmc2209_set_irun(tmc2209_t *handler, uint16_t irun)
{
	handler->_ihold_irun_shadow &= ~(0x1F<<8);
	handler->_ihold_irun_shadow |= (tmc2209_cs_calc(irun) & 0x1F) << 8;
    tmc2209_write_register(handler, IHOLD_IRUN, handler->_ihold_irun_shadow);
}

void tmc2209_set_ihold_delay(tmc2209_t *handler, uint16_t ihold_delay)
{
	handler->_ihold_irun_shadow &= ~(0x1F<<16);
	handler->_ihold_irun_shadow |= (ihold_delay & 0x1F) << 16;
    tmc2209_write_register(handler, IHOLD_IRUN, handler->_ihold_irun_shadow);
}

void tmc2209_set_current(tmc2209_t *handler, uint16_t ihold, uint16_t irun, uint8_t ihold_delay)
{
	handler->_ihold_irun_shadow = 0;

	handler->_ihold_irun_shadow |= (tmc2209_cs_calc(ihold) & 0x1F) << 0;
	handler->_ihold_irun_shadow |= (tmc2209_cs_calc(irun)  & 0x1F) << 8;
	handler->_ihold_irun_shadow |= (ihold_delay & 0x1F) << 16;

    tmc2209_write_register(handler, IHOLD_IRUN, handler->_ihold_irun_shadow);
}

/* ==================== MOTION CONTROL ==================== */

void tmc2209_inverse_direction(const tmc2209_t *handler, tmc2209_shaft_t direction)
{
	uint32_t reg_value = tmc2209_read_register(handler, GCONF);

	if (direction == TMC2209_SHAFT_NORMAL)
	{
		reg_value &= ~(1U<<3);
	}
	else if (direction == TMC2209_SHAFT_INVERTED)
	{
		reg_value |= (1U<<3);
	}

    tmc2209_write_register(handler, GCONF, reg_value);
}

void tmc2209_set_microsteps(const tmc2209_t *handler, tmc2209_microstep_t microstep, tmc2209_intpol_t intpol)
{
	uint32_t reg_value  = tmc2209_read_register(handler, CHOPCONF);

	reg_value &= ~(0xF << 24);
	reg_value |=  (microstep & 0xF) << 24;

	if (intpol == TMC2209_INTPOL_ENABLE)
	{
		reg_value |= (1U << 28);
	}
	else if (intpol == TMC2209_INTPOL_DISABLE)
	{
		reg_value &= ~(1U << 28);
	}

	tmc2209_write_register(handler, CHOPCONF, reg_value);
}

void tmc2209_set_vactual(const tmc2209_t *handler, int32_t value)
{
	tmc2209_write_register(handler, VACTUAL, value);
}

/* ==================== CHOPPER / DRIVER STATE ==================== */

void tmc2209_set_chopper_mode(const tmc2209_t *handler, tmc2209_chop_mode_t chop_mode)
{
	uint32_t reg_value  = tmc2209_read_register(handler, GCONF);

	if (chop_mode == TMC2209_MODE_STEALTHCHOP)
	{
		reg_value &= ~(1U<<2);
	}
	else if (chop_mode == TMC2209_MODE_SPREADCYCLE)
	{
		reg_value |= (1U<<2);
	}

    tmc2209_write_register(handler, GCONF, reg_value);
}

void tmc2209_set_toff(const tmc2209_t *handler, tmc2209_toff_t toff_value)
{
	uint32_t reg_value  = tmc2209_read_register(handler, CHOPCONF);

	if (toff_value > 15)
	{
		toff_value = 15;
	}

	reg_value &= ~(0xF << 0);
	reg_value |=  (toff_value & 0xF) << 0;

	tmc2209_write_register(handler, CHOPCONF, reg_value);
}

void tmc2209_enable(const tmc2209_t *handler)
{
	tmc2209_set_toff(handler, TMC2209_TOFF_ENABLE);
}

void tmc2209_disable(const tmc2209_t *handler)
{
	tmc2209_set_toff(handler, TMC2209_TOFF_DISABLE);
}
