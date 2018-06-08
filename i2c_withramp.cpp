/*************************************************************************
i2ctest.cpp
Raspberry Pi I2C interface demo
Byron Jacquot @ SparkFun Electronics>
4/2/2014
https://github.com/sparkfun/Pi_Wedge

A brief demonstration of the Raspberry Pi I2C interface, using the SparkFun
Pi Wedge breakout board.

Resources:

This example makes use of the Wiring Pi library, which streamlines the interface
the the I/O pins on the Raspberry Pi, providing an API that is similar to the
Arduino.  You can learn about installing Wiring Pi here:
http://wiringpi.com/download-and-install/

The I2C API is documented here:
https://projects.drogon.net/raspberry-pi/wiringpi/i2c-library/

The init call returns a standard file descriptor.  More detailed configuration
of the interface can be performed using ioctl calls on that descriptor.
See the wiringPi I2C implementation (wiringPi/wiringPiI2C.c) for some examples.
Parameters configurable with ioctl are documented here:
http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/i2c/dev-interface

Hardware connections:

This file interfaces with the SparkFun MCP4725 breakout board:
https://www.sparkfun.com/products/8736

The board was connected as follows:
(Raspberry Pi)(MCP4725)
GND  -> GND
3.3V -> Vcc
SCL  -> SCL
SDA  -> SDA

An oscilloscope probe was connected to the analog output pin of the MCP4725.

To build this file, I use the command:
>  g++ i2ctest.cpp -lwiringPi

Then to run it, first the I2C kernel module needs to be loaded.  This can be 
done using the GPIO utility.
> gpio load i2c 400
> ./a.out

This will run the MCP through its output range several times.  A rising 
sawtooth will be seen on the analog output.

Development environment specifics:
Tested on Raspberry Pi V2 hardware, running Raspbian.
Building with GCC 4.6.3 (Debian 4.6.3-14+rpi1)

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <iostream>
#include <stdio.h>
#include "NB_input.h"
#include <errno.h>
#include <wiringPiI2C.h>
#include <limits>
//#define VOLT (0x07F)
//Added to use sleep function usleep(), which takes microsecond input units.
#include <unistd.h>

using namespace std;

// 4.87 volts is max, 0 is minimum
// 255		      0	
#define MAX 4.00
float V0;
float V1; 

// converts input voltage to integer to be sent to chair.  

int volt_convert(float volts)
{
	int output;
	if (volts > MAX) {
		cout << "Voltage exceeds chair limitations. \n";
		volts = 2.5;
		
	} else if (volts < 1) {
		cout << "Voltage below chair range. \n";
		volts = 2.5;
	} 
		
	float res1 = (volts/MAX);
	 
	int result = 255 * res1;
	//cout << result; 
	//cout << "\n"; 
	output = result;
	
	return output;
}

// Uses wasd keys on keyboard to control chair
// Granularity of acceleration can be changed by change incrementation value.

void wasd(char input)
{
 if (input == 'w' && (V0 < 4)) {
	// increment V0
	V0 = V0+0.005;  
} else if (input == 'a' && (V1 > 1)) {
	// decrement V1
	V1 = V1-0.005;
} else if (input == 's' && (V0 > 1)) {
	// decrement V0
	V0 = V0-0.005;
} else if (input == 'd' && (V1 < 4)) {
	// increment V1
	V1 = V1+0.005;
}
} 

int main()
{
   set_conio_terminal_mode();

   int fd, result;
   int fd1, result1;

	// Globals for chair input
	V0 = 2.5;
	V1 = 2.5;

   // input stream temp
   char x; 

   // Initialize the interface by giving it an external device ID.
   // The MCP4725 defaults to address 0x60.   
   // 
   // It returns a standard file descriptor.
   // 
   fd = wiringPiI2CSetup(0x60);

   cout << "Init result: "<< fd << endl;

   fd1 = wiringPiI2CSetup(0x61);

   cout << "Init result1: "<< fd1 << endl;
	
	
   while(1)
   {
      // I tried using the "fast write" command, but couldn't get it to work.  
      // It's not entirely obvious what's happening behind the scenes as
      // regards to endianness or length of data sent.  I think it's only 
      // sending one byte, when we really need two.
      //
      // So instead I'm doing a 16 bit register access.  It appears to 
      // properly handle the endianness, and the length is specified by the 
      // call.  The only question was the register address, which is the 
      // concatenation of the command (010x = write DAC output) 
      // and power down (x00x = power up) bits.
	
	
	
	x = getch(); /* consume the character */
	std::cout << x;
	std::cout << '\n';
	std::cout.flush();
	if (x == 0x0D) {

		return -1;
	}
	


	// Read in value from stdio
	//float VOLT0;
	//float VOLT1;
	char VOLT0;
	char VOLT1; 
	
	//cout<<"Input voltage 1: ";
	
	
	// UPdate globals
	wasd(x);


	int chair_input0 = volt_convert(V0);
	int chair_input1 = volt_convert(V1);

	cout << V0;
	cout << '\n';
	cout << V1;
	cout << '\n';
		     
      	//result = wiringPiI2CWriteReg16(fd, 0x40, (i & 0xfff) );
	result = wiringPiI2CWriteReg16(fd, 0x40, (chair_input0));
	result1 = wiringPiI2CWriteReg16(fd1, 0x40, (chair_input1));
	
	//cin.clear();
	//acin.ignore(numeric_limits<streamsize>::max(), '\n');

      if((result == -1) || (result1 == -1))
      {
         cout << "Error.  Errno is: " << errno << endl;
      }
   }
   reset_terminal_mode();	
}

