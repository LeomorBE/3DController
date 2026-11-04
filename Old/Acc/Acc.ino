#include "SparkFun_BNO08x_Arduino_Library.h"
BNO08x myIMU1;
BNO08x myIMU2;

#define BNO08X1_CS   22
#define BNO08X1_INT  25
#define BNO08X1_RST  24

#define BNO08X2_CS   41
#define BNO08X2_INT  43
#define BNO08X2_RST  42

int val = 0;

struct position{
  float x;
  float y;
  float z;
};


void setup() {
  Serial.begin(115200);
  pinMode(3, INPUT_PULLUP);
  if (myIMU1.beginSPI(BNO08X1_CS, BNO08X1_INT, BNO08X1_RST) == false && myIMU2.beginSPI(BNO08X2_CS, BNO08X2_INT, BNO08X2_RST) == false) {
    Serial.println("BNO08x not detected. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  Serial.println("BNO08x found!");

  Serial.print(F("Reset Reason: "));
  printResetReasonName(myIMU1.getResetReason());
  printResetReasonName(myIMU2.getResetReason());
  Serial.println();

  setReports();

  Serial.println("Reading events");
  delay(50);

 

}
void setReports(void) {
  Serial.println("Setting desired reports");
  if (myIMU1.enableAccelerometer(50)) { // 50 ms = 20 Hz
    Serial.println("Rotation vector enabled 1");
  } else {
    Serial.println("Could not enable rotation vector 1");
  }
  if (myIMU2.enableAccelerometer(50)) { // 50 ms = 20 Hz
    Serial.println("Rotation vector enabled 2");
  } else {
    Serial.println("Could not enable rotation vector 2");
  }
  delay(50); // This delay allows enough time for the BNO086 to accept the new 
              // configuration and clear its reset status
}


void loop() {
  myIMU1.serviceBus();
  myIMU2.serviceBus();

  if (myIMU1.wasReset()) {
    Serial.println(" ------------------ BNO08x has reset. ------------------ ");
    Serial.print(F("Reset Reason: "));
    printResetReasonName(myIMU1.getResetReason());
    Serial.println();

    setReports();  // We'll need to re-enable reports after any reset.
  }
  if (myIMU2.wasReset()) {
    Serial.println(" ------------------ BNO08x has reset. ------------------ ");
    Serial.print(F("Reset Reason: "));
    printResetReasonName(myIMU2.getResetReason());
    Serial.println();

    setReports();  // We'll need to re-enable reports after any reset.
  }
  
  if(!(val =digitalRead(3))){
    Serial.println(val);
  }
  if (myIMU1.getSensorEvent() && myIMU2.getSensorEvent()) {
    if (myIMU1.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER && myIMU2.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
      uint8_t qw = myIMU1.getAccelAccuracy();
      float qx = myIMU1.getAccelX();
      float qy = myIMU1.getAccelY();
      float qz = myIMU1.getAccelZ();

      // envoyer en UART
      //Serial.print(qw); Serial.print(",");
      //Serial.print(qx); Serial.print(",");
      //Serial.print(qy); Serial.print(",");
      //Serial.println(qz);

      uint8_t lw = myIMU2.getAccelAccuracy();
      float lx = myIMU2.getAccelX();
      float ly = myIMU2.getAccelY();
      float lz = myIMU2.getAccelZ();

      // envoyer en UART
      Serial.print(lx); Serial.print(",");
      Serial.print(ly); Serial.print(",");
      Serial.print(lz); Serial.print(",");
      //Serial.println(lz);
      Serial.print(qx); Serial.print(",");
      Serial.print(qy); Serial.print(",");
      Serial.println(qz);
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
