

#include <stdio.h>
//#include <unistd.h> // for isatty()
#include <stdlib.h> // for strtof()
//#include <math.h> // for M_PI
#include <roboticscape.h>


#define US_ADDRESS 0x70 //i2cdetect -r 1
#define I2C_BUS 1
#define COMMAND_REGISTER 0x00
#define RANGE_HIGH_BYTE 0x02
#define RANGE_LOW_BYTE 0x03
#define US_MODE_CENTEMETERS 0x51

const uint64_t ns = 70000; // 70 millisek 


void Init_ultrasonic()
{
	rc_i2c_init(I2C_BUS, US_ADDRESS);

}
int get_ultrasonic_value()
{
	uint8_t data_low, data_high;
	uint8_t range, range_l;
	int a,b;

	rc_i2c_init(I2C_BUS, US_ADDRESS);
	rc_i2c_write_byte(I2C_BUS, COMMAND_REGISTER, US_MODE_CENTEMETERS);
	//TWI_stop();
	rc_usleep(ns);
	//TWI_Start();
	//rc_i2c_write_byte(I2C_BUS, COMMAND_REGISTER, RANGE_HIGH_BYTE);
	//TWI_Start();
	//rc_i2c_write_byte(I2C_BUS, COMMAND_REGISTER, RANGE_LOW_BYTE);
	a = rc_i2c_read_byte(I2C_BUS, RANGE_HIGH_BYTE, &data_high);
	b = rc_i2c_read_byte(I2C_BUS, RANGE_LOW_BYTE, &data_low);
	rc_i2c_close(I2C_BUS);

	range = data_high;
	range_l = data_low;
	range =(range << 8) | range_l;

	if(a == -1){
		printf("I2C High byte ERROR\n");
	}
	if(b == -1){
		printf("I2C Low byte ERROR\n");
	}
	return range; 

}
