#include "SparkFun_BNO08x_Arduino_Library.h"
#include <MadgwickAHRS.h>
BNO08x myIMU;

#define BNO08X_CS   53
#define BNO08X_INT  6
#define BNO08X_RST  5
int val = 0;
Madgwick filter;

struct position{
  float x;
  float y;
  float z;
};


void setup() {
  Serial.begin(115200);
  filter.begin(10);
  pinMode(3, INPUT_PULLUP);
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
  delay(50);

 

}
void setReports(void) {
  Serial.println("Setting desired reports");
  if (myIMU.enableGyro(100) == true) {
    Serial.println(F("Gyro enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable gyro");
  }
  if (myIMU.enableAccelerometer(100) == true) {
    Serial.println(F("Accelerometer enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable Accelerometer");
  }
  if (myIMU.enableMagnetometer(100) == true) {
    Serial.println(F("Magnetometer enabled"));
    Serial.println(F("Output in form x, y, z, in radians per second"));
  } else {
    Serial.println("Could not enable Magnetometer");
  }
  delay(50); // This delay allows enough time for the BNO086 to accept the new 
              // configuration and clear its reset status
}


void loop() {
  float roll, pitch, heading;
  const float ACC_CONV = 9.81;
  myIMU.serviceBus();

  if (myIMU.wasReset()) {
    Serial.println(" ------------------ BNO08x has reset. ------------------ ");
    Serial.print(F("Reset Reason: "));
    printResetReasonName(myIMU.getResetReason());
    Serial.println();

    setReports();  // We'll need to re-enable reports after any reset.
  }
  
  if(!(val =digitalRead(3))){
    Serial.println(val);
  }
  if (myIMU.getSensorEvent() == true) {

    // is it the correct sensor data we want?
    if (myIMU.getSensorEventID() == SENSOR_REPORTID_GYROSCOPE_CALIBRATED || myIMU.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER || myIMU.getSensorEventID() == SENSOR_REPORTID_MAGNETIC_FIELD) {

      //filter.update(myIMU.getGyroX(),myIMU.getGyroY(),myIMU.getGyroZ(),myIMU.getAccelX()/ACC_CONV,myIMU.getAccelY()/ACC_CONV,myIMU.getAccelZ()/ACC_CONV,myIMU.getMagX(),myIMU.getMagY(),myIMU.getMagZ());
      filter.updateIMU(myIMU.getGyroX(),myIMU.getGyroY(),myIMU.getGyroZ(),myIMU.getAccelX(),myIMU.getAccelY(),myIMU.getAccelZ());
      roll = filter.getRoll();
      pitch = filter.getPitch();
      heading = filter.getYaw();
      Serial.print("Orientation: ");
      Serial.print(heading);
      Serial.print(" ");
      Serial.print(pitch);
      Serial.print(" ");
      Serial.println(roll);
    }
  }
  
  
  /**Serial.printf("<ACC>%f,%f,%f/<GYRO>%f,%f,%f/<TEMP>%f/<CIN>%x%x%x%x/<COUT>%x%x%x%x\n",a.acceleration.x,a.acceleration.y,a.acceleration.z,g.gyro.x,g.gyro.y,g.gyro.z,temp.temperature, buf1[0],buf1[1], buf1[2], buf1[3], buf2[0],buf2[1],buf2[2],buf2[3]);  
  Serial.flush();**/
  
  
  delayMicroseconds(10);

}

void printResetReasonName(byte resetReasonNumber)
{
  if(resetReasonNumber == 1) Serial.print("POR");
  else if(resetReasonNumber == 2) Serial.print("Internal reset");
  else if(resetReasonNumber == 3) Serial.print("Watchdog");
  else if(resetReasonNumber == 4) Serial.print("External reset");
  else if(resetReasonNumber == 5) Serial.print("Other");
}
