#ifndef _I2C_H_
#define _I2C_H_

#define I2C_IO_BASE 0xc000

int I2CWriteBytetoRegister(u8 bPicAddressI2cFormat, u8 bRegister, u8 wDataToWrite);
void wait_us(DWORD ticks);

#endif // _I2C_H_