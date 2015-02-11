#ifndef __I2C_H
#define __I2C_H

void i2c_trigger_capture(uint8_t);
uint16_t i2c_read_values(void);
void i2c_init(void);

#endif
