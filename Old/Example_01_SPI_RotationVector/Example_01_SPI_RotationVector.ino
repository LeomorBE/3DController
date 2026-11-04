/*
  Using the BNO08x IMU

  This example shows how to communicate with the sensor over SPI.

  It requires a few more connections than the qwiic cable.
  In addition to your usual SPI lines (CS/PICO/POCI/SCK),
  This also requires INT and RST. These are crucial for timing when talking to
  this sensor.

  It outputs the i/j/k/real parts of the rotation vector.
  https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation

  By: Nathan Seidle
  SparkFun Electronics
  Date: December 21st, 2017
  SparkFun code, firmware, and software is released under the MIT License.
	Please see LICENSE.md for further details.

  Originally written by Nathan Seidle @ SparkFun Electronics, December 28th, 2017

  Adjusted by Pete Lewis @ SparkFun Electronics, June 2023 to incorporate the
  CEVA Sensor Hub Driver, found here:
  https://github.com/ceva-dsp/sh2

  Also, utilizing code from the Adafruit BNO08x Arduino Library by Bryan Siepert
  for Adafruit Industries. Found here:
  https://github.com/adafruit/Adafruit_BNO08x

  Also, utilizing I2C and SPI read/write functions and code from the Adafruit
  BusIO library found here:
  https://github.com/adafruit/Adafruit_BusIO

  Hardware Connections:
  IoT RedBoard --> BNO08x
  D5  --> CS
  PICO --> SI
  POCI --> SO
  SCK  --> SCK
  A4  --> INT
  A5  --> RST
  3V3  --> 3V3
  GND  --> GND

  BNO08x "mode" pins set for SPI:
  PSO --> 3V3
  PS1 --> 3V3

  Serial.print it out at 115200 baud to serial monitor.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/22857
*/

//#include <Wire.h>

#include "SparkFun_BNO08x_Arduino_Library.h"  // CTRL+Click here to get the library: http://librarymanager/All#SparkFun_BNO08x
BNO08x myIMU;

// For SPI, we need some extra pins defined:
// Note, these can be other GPIO if you like.
#define BNO08X_CS   53
#define BNO08X_INT  6
#define BNO08X_RST  5

// If using the MicroMod Machine Learning Carrier Board, use these pins:
// for more info see https://github.com/sparkfun/SparkFun_BNO08x_Arduino_Library/issues/19
//#define BNO08X_CS   PWM0
//#define BNO08X_INT  D0 // Note, D0 is also the CS pin for the camera, so you'll need to change that if you're using both.
// a good alternative for D0 is PWM1, (A0 and A1 are only inputs on the carrier board).
//#define BNO08X_RST  D1

void setup() {
  Serial.begin(115200);
  
  while(!Serial) delay(10); // Wait for Serial to become available.
  // Necessary for boards with native USB (like the SAMD51 Thing+).
  // For a final version of a project that does not need serial debug (or a USB cable plugged in),
  // Comment out this while loop, or it will prevent the remaining code from running.
  
  Serial.println();
  Serial.println("BNO08x Read Example");

  if (myIMU.beginSPI(BNO08X_CS, BNO08X_INT, BNO08X_RST) == false) {
    Serial.println("BNO08x not detected. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  Serial.println("BNO08x found!");

  Serial.print(F("Reset Reason: "));
  printResetReasonName(myIMU.getResetReason());
  Serial.println();

  setReports();

  Serial.println("Reading events");
  delay(100);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (myIMU.enableGyro(50) == true) {
    Serial.println(F("Gyro enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable gyro");
  }
  if (myIMU.enableAccelerometer(50) == true) {
    Serial.println(F("Accelerometer enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable Accelerometer");
  }
  if (myIMU.enableMagnetometer(50) == true) {
    Serial.println(F("Magnetometer enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable Magnetometer");
  }
  delay(100); // This delay allows enough time for the BNO086 to accept the new 
              // configuration and clear its reset status
}

void loop() {
  delay(10);
  myIMU.serviceBus();

  // One of these will appear at the very start because of the power on reset.
  // Use requestResetReason() and getResetReason() for the difference between
  // different resets.
  if (myIMU.wasReset()) {
    Serial.println(" ------------------ BNO08x has reset. ------------------ ");
    Serial.print(F("Reset Reason: "));
    printResetReasonName(myIMU.getResetReason());
    Serial.println();

    setReports();  // We'll need to re-enable reports after any reset.
  }

  // Has a new event come in on the Sensor Hub Bus?
  if (myIMU.getSensorEvent() == true) {

    // is it the correct sensor data we want?
    if (myIMU.getSensorEventID() == SENSOR_REPORTID_GYROSCOPE_CALIBRATED || myIMU.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER || myIMU.getSensorEventID() == SENSOR_REPORTID_MAGNETIC_FIELD) {

      float gyrox = myIMU.getGyroX();
      float gyroy = myIMU.getGyroY();
      float gyroz = myIMU.getGyroZ();
      Serial.print("Gyroscope:");
      Serial.print(gyrox, 2);
      Serial.print(F(","));
      Serial.print(gyroy, 2);
      Serial.print(F(","));
      Serial.print(gyroz, 2);
      Serial.println();

      float accx = myIMU.getAccelX();
      float accy = myIMU.getAccelY();
      float accz = myIMU.getAccelZ();
      Serial.print("Acceleration:");
      Serial.print(accx, 2);
      Serial.print(F(","));
      Serial.print(accy, 2);
      Serial.print(F(","));
      Serial.print(accz, 2);
      Serial.println();

      float magx = myIMU.getMagX();
      float magy = myIMU.getMagY();
      float magz = myIMU.getMagZ();
      Serial.print("Magnitude:");
      Serial.print(magx, 2);
      Serial.print(F(","));
      Serial.print(magy, 2);
      Serial.print(F(","));
      Serial.print(magz, 2);
      Serial.println();


    }
    
  }
}
void printResetReasonName(byte resetReasonNumber)
{
  if(resetReasonNumber == 1) Serial.print("POR");
  else if(resetReasonNumber == 2) Serial.print("Internal reset");
  else if(resetReasonNumber == 3) Serial.print("Watchdog");
  else if(resetReasonNumber == 4) Serial.print("External reset");
  else if(resetReasonNumber == 5) Serial.print("Other");
}
