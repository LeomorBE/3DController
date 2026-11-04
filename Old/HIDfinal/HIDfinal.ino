#include <Wire.h>
#include "SparkFun_MMA8452Q.h"
#include "HX711.h"
#include <math.h>

#define CAPTEUR_ACC_ADDRESS_LEFT 0x1D
#define CAPTEUR_ACC_ADDRESS_RIGHT 0x1C

#define SCALE_LEFT_DATA     3
#define SCALE_LEFT_CLOCK    4
#define SCALE_RIGHT_DATA     22
#define SCALE_RIGHT_CLOCK    23

MMA8452Q accelLeft;
MMA8452Q accelRight;
HX711 scaleLeft;
HX711 scaleRight;

#define BUFFER_HID_12     1023
#define BUFFER_HID_64     65534

struct data{
  uint16_t pitchLeft;
  uint16_t pitchRight;
  uint16_t rollLeft;
  uint16_t rollRight;
  uint16_t scaleLeft;
  uint16_t scaleRight;
};
struct dataAccFiltered{
  float x1Filtered;
  float y1Filtered;
  float z1Filtered;
  float x2Filtered;
  float y2Filtered;
  float z2Filtered;
  bool initOne;
  bool initTwo;
};
const float coeffFiltreEMI = 0.08;

dataAccFiltered acc={0,0,0,0,0,0,false,false};
data joy={0,0,0,0,0,0};
const float maxWeight = 2000.0f;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!accelLeft.begin(Wire,CAPTEUR_ACC_ADDRESS_LEFT)) {
    while (1);
  }
  if (!accelRight.begin(Wire, CAPTEUR_ACC_ADDRESS_RIGHT)) {
    while (1);
  }
  Joystick.useManualSend(true);
  scaleLeft.begin(SCALE_LEFT_DATA,SCALE_LEFT_CLOCK);
  scaleRight.begin(SCALE_RIGHT_DATA,SCALE_RIGHT_CLOCK);
  scaleLeft.set_scale(420.0983);
  scaleRight.set_scale(420.0983); 
  scaleLeft.tare(20);
  scaleRight.tare(20);   
  accelLeft.setScale(SCALE_2G);
  accelRight.setScale(SCALE_2G);
  accelLeft.setDataRate(ODR_400);
  accelRight.setDataRate(ODR_400);

}

uint16_t scale_to_hid_axis(float weight){
  if (weight > maxWeight) weight=maxWeight;
  if (weight < 0.0f) weight=0;
  return (uint16_t)(weight * 65534.0f / maxWeight);
}

uint16_t accel_to_hid_axis(float accel){

  if (accel > 2.0f) accel = 2.0f;
  if (accel < -2.0f) accel = -2.0f;

  return (uint16_t)((accel+2.0f) * 65534.0f/(2.0f*2.0f));
}

uint16_t angle_to_hid_axis(float angle){
  if (angle > 90.0f)
    angle = 90.0f;

  if (angle < -90.0f)
    angle = -90.0f;

  return (uint16_t)((angle + 90.0f) * 65534.0f / 180.0f);
}

/**void printData(){
  Serial.print("DATA: L pitch=");
  Serial.print(joy.pitchLeft);
  Serial.print(", L raw=");
  Serial.print(joy.rawLeft);
  Serial.print(", R pitch=");
  Serial.print(joy.pitchRight);
  Serial.print(", R raw=");
  Serial.print(joy.rawRight);
  Serial.print(", L scale=");
  Serial.print(joy.scaleLeft);
  Serial.print(", R scale=");
  Serial.println(joy.scaleRight);
}**/

void sendData(){
  Joystick.X(joy.pitchLeft);
  Joystick.Y(joy.rollLeft);
  Joystick.Z(joy.pitchRight);
  Joystick.Xrotate(joy.rollRight);
  Joystick.Yrotate(joy.scaleLeft);
  Joystick.Zrotate(joy.scaleRight);
  //Joystick.Zrotate(joy.rollRight);
  //Joystick.sliderLeft(joy.scaleLeft);
  //Joystick.sliderRight(joy.scaleRight);
  Joystick.send_now();
  Serial.println(joy.scaleLeft);
  Serial.println(joy.scaleRight);
}

void setFiltreEMA(float x,float y,float z, bool isOne){
  if(isOne){
    if(!acc.initOne){
      acc.x1Filtered = x;
      acc.y1Filtered = y;
      acc.z1Filtered = z;
      acc.initOne=true;
    }else{
      acc.x1Filtered = coeffFiltreEMI * x + (1-coeffFiltreEMI)*acc.x1Filtered;
      acc.y1Filtered = coeffFiltreEMI * y + (1-coeffFiltreEMI)*acc.y1Filtered;
      acc.z1Filtered = coeffFiltreEMI * z + (1-coeffFiltreEMI)*acc.z1Filtered;
    }
  }else{
    if(!acc.initTwo){
      acc.x2Filtered = x;
      acc.y2Filtered = y;
      acc.z2Filtered = z;
      acc.initTwo=true;
    }else{
      acc.x2Filtered = coeffFiltreEMI * x + (1-coeffFiltreEMI)*acc.x2Filtered;
      acc.y2Filtered = coeffFiltreEMI * y + (1-coeffFiltreEMI)*acc.y2Filtered;
      acc.z2Filtered = coeffFiltreEMI * z + (1-coeffFiltreEMI)*acc.z2Filtered;
    }
  }

}

void loop() {
  if(accelLeft.available()){
    float x1 = accelLeft.getCalculatedX();
    float y1 = accelLeft.getCalculatedY();
    float z1 = accelLeft.getCalculatedZ();
    joy.pitchLeft = angle_to_hid_axis(atan2(x1, sqrt(y1 * y1 + z1 * z1)) * 180.0f / PI);
    joy.rollLeft  = angle_to_hid_axis(atan2(y1, sqrt(z1*z1+x1*x1)) * 180.0f / PI);
  }
  if(accelRight.available()){
    float x2 = accelRight.getCalculatedX();
    float y2 = accelRight.getCalculatedY();
    float z2 = accelRight.getCalculatedZ();


    joy.pitchRight = angle_to_hid_axis(atan2(x2, sqrt(y2 * y2 + z2 * z2)) * 180.0f / PI);
    joy.rollRight  = angle_to_hid_axis(atan2(y2, sqrt(z2*z2+x2*x2)) * 180.0f / PI);
  }
  if(scaleLeft.is_ready()){
    joy.scaleLeft = scale_to_hid_axis(scaleLeft.get_units());
  }
  if(scaleRight.is_ready()){
    joy.scaleRight = scale_to_hid_axis(scaleRight.get_units());
  }
  //printData();
  sendData();

}

//pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
//    roll  = atan2(ay, az) * 180.0f / PI;
