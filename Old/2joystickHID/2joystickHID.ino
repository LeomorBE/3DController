#include "SparkFun_BNO08x_Arduino_Library.h"
#include "HX711.h"
#include <stdint.h>

#define BNO08X1_CS   15
#define BNO08X1_INT  16
#define BNO08X1_RST  17

#define BNO08X2_CS   18
#define BNO08X2_INT  19
#define BNO08X2_RST  20

#define SCALE_LEFT_DATA     3 
#define SCALE_LEFT_CLOCK    4

BNO08x myIMU1;
BNO08x myIMU2;

uint32_t timeµs = 0;
uint32_t timeScale = 0;
HX711 scaleL;
const float maxWeight = 5.0f;
//HX711 scaleR;


struct dataJoystick{
  float x;
  float y;
  float z;
  float scale;
};
dataJoystick joy1={0.,0.,0.,0.};
dataJoystick joy2={0.,0.,0.,0.};

void setReports(void) {
  myIMU1.enableAccelerometer(4); // 4 ms = 250 Hz
  myIMU2.enableAccelerometer(4); // 4 ms = 250 Hz
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  if (myIMU1.beginSPI(BNO08X1_CS, BNO08X1_INT, BNO08X1_RST) == false || myIMU2.beginSPI(BNO08X2_CS, BNO08X2_INT, BNO08X2_RST) == false) {
    while (1)
    ;
  }
  Serial.println("hello");
  delay(50);
  scaleL.begin(SCALE_LEFT_DATA,SCALE_LEFT_CLOCK);
  //scaleR.begin(SCALE_RIGHT_DATA,SCALE_RIGHT_CLOCK);
  setReports();
  delay(50);
}



int16_t float_to_hid_axis(float weight){
  float v = weight / maxWeight;
  if (v > 1.0f) v = 1.0f;
  if (v < -1.0f) v = -1.0f;
  return (int16_t)(v * 32767.0f);
}

int16_t accel_to_hid_axis(float accel){
    // Le BNO080 renvoie l'accélération en m/s².
    // 1g ≈ 9.81 m/s².
  float v = accel / 20.0f;

  if (v > 1.0f) v = 1.0f;
  if (v < -1.0f) v = -1.0f;

  return (int16_t)(v * 32767.0f);
}


void loop() {
  Serial.println("hello");
    myIMU1.serviceBus();
    myIMU2.serviceBus();

  if (myIMU1.wasReset()) {
    setReports();  // We'll need to re-enable reports after any reset.
  }
  if (myIMU2.wasReset()) {
    setReports();  // We'll need to re-enable reports after any reset.
  }
    
  if (myIMU1.getSensorEvent()) {
    if (myIMU1.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
      joy1.x=myIMU1.getAccelX();
      joy1.y=myIMU1.getAccelY();
      joy1.z=myIMU1.getAccelZ();
      usb_joystick1_send(
        accel_to_hid_axis(joy1.x),
        accel_to_hid_axis(joy1.y),
        accel_to_hid_axis(joy1.z),
        0
      );
      
    }
  }
  if (myIMU2.getSensorEvent()) {
    if (myIMU2.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
      joy2.x=myIMU2.getAccelX();
      joy2.y=myIMU2.getAccelY();
      joy2.z=myIMU2.getAccelZ();
      usb_joystick2_send(
        accel_to_hid_axis(joy2.x),
        accel_to_hid_axis(joy2.y),
        accel_to_hid_axis(joy2.z),
        0
      );
    }
  }
  if((millis()-timeScale)>= 50){
    if(scaleL.is_ready()){  //&& scaleR.is_ready()
      joy1.scale=scaleL.get_units();
    }
    timeScale=millis();
  }
  /**usb_joystick1_send(
    0,
    10000,
    -10000,
    20000
  );

  usb_joystick2_send(
    0,
    -10000,
    10000,
    -20000
  );

  delay(10);**/
  
  //delay(10);
}