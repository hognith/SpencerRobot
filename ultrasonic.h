#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define US_ADDRESS 0xE0
#define I2C_BUS 1
#define COMMAND_REGISTER 0x00
#define RANGE_HIGH_BYTE 0x02
#define RANGE_LOW_BYTE 0x03
#define US_MODE_CENTEMETERS 0x51

void Init_ultrasonic();
int get_ultrasonic_value();



#endif //ULTRASONIC_H