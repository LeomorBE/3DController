#include <Wire.h>
#include "SparkFun_MMA8452Q.h"
#include "HX711.h"
#include <math.h>
#include "usb_controller.h"

#define CAPTEUR_ACC_ADDRESS_LEFT 0x1D
#define CAPTEUR_ACC_ADDRESS_RIGHT 0x1C

#define SCALE_LEFT_DATA     3
#define SCALE_LEFT_CLOCK    4
#define SCALE_RIGHT_DATA     22
#define SCALE_RIGHT_CLOCK    23

#define PIN_LED   9
#define BLINK_MS  500

MMA8452Q accelLeft;
MMA8452Q accelRight;
HX711 scaleLeft;
HX711 scaleRight;

bool ifBeforeUSBConnected = false;
int countBlink = 0;
int tempsBlink = 0;

#define BUFFER_HID_12     1023
#define BUFFER_HID_64     65534

controller_report_t data = {0,0,0,0,0,0};
controller_report_t dataFiltered = {0,0,0,0,0,0};

struct rawDataFiltered{
  float rawPitchLeft;
  float rawRollLeft;
  float rawPitchRight;
  float rawRollRight;
  float weightLeft;
  float weightRight;
};

rawDataFiltered dataFilteredRaw={0.,0.,0.,0.,0.,0.};

const float coeffAlphaRIIAcc = 0.3;
const float coeffAlphaRIIWeight = 0.8;
bool isNewValue = true;

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

  tempsBlink = millis();
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

}

uint16_t scale_to_hid_axis(float weight){
  if (weight > maxWeight) weight=maxWeight;
  if (weight < 0.0f) weight=0;
  return (uint16_t)(weight * 65534.0f / maxWeight);
}

int16_t angle_to_hid_axis(float angle){
  if (angle > 90.0f)
    angle = 90.0f;

  if (angle < -90.0f)
    angle = -90.0f;

  return (int16_t)(angle * 32767.0f / 90.0f);
}



float filtreRII(float rawValue, float filteredOldValue, float coeff){
  if(filteredOldValue==0){
    return rawValue;
  }else{
    return (coeff * rawValue + (1 - coeff) * filteredOldValue);
  }
}





void loop() {
  if(accelLeft.available()){
    float x1 = accelLeft.getCalculatedX();
    float y1 = accelLeft.getCalculatedY();
    float z1 = accelLeft.getCalculatedZ();
    dataFilteredRaw.rawPitchLeft = filtreRII(atan2(x1, sqrt(y1 * y1 + z1 * z1)) * 180.0f / PI, dataFilteredRaw.rawPitchLeft, coeffAlphaRIIAcc);
    dataFilteredRaw.rawRollLeft = filtreRII(atan2(y1, sqrt(z1*z1+x1*x1)) * 180.0f / PI, dataFilteredRaw.rawRollLeft, coeffAlphaRIIAcc);
    data.pitch_left = angle_to_hid_axis(dataFilteredRaw.rawPitchLeft);
    data.roll_left  = angle_to_hid_axis(dataFilteredRaw.rawRollLeft);
  }
  if(accelRight.available()){
    float x2 = accelRight.getCalculatedX();
    float y2 = accelRight.getCalculatedY();
    float z2 = accelRight.getCalculatedZ();
    dataFilteredRaw.rawPitchRight = filtreRII(atan2(x2, sqrt(y2 * y2 + z2 * z2)) * 180.0f / PI, dataFilteredRaw.rawPitchRight, coeffAlphaRIIAcc);
    dataFilteredRaw.rawRollRight = filtreRII(atan2(y2, sqrt(z2*z2+x2*x2)) * 180.0f / PI, dataFilteredRaw.rawRollRight, coeffAlphaRIIAcc);
    data.pitch_right = angle_to_hid_axis(dataFilteredRaw.rawPitchRight);
    data.roll_right  = angle_to_hid_axis(dataFilteredRaw.rawRollRight);
  }
  if(scaleLeft.is_ready()){
    dataFilteredRaw.weightLeft = filtreRII(scaleLeft.get_units(),dataFilteredRaw.weightLeft, coeffAlphaRIIWeight);
    data.weight_left = scale_to_hid_axis(dataFilteredRaw.weightLeft);
  }
  if(scaleRight.is_ready()){
    dataFilteredRaw.weightRight = filtreRII(scaleRight.get_units(), dataFilteredRaw.weightRight, coeffAlphaRIIWeight);
    data.weight_right = scale_to_hid_axis(dataFilteredRaw.weightRight);
  }
  //printData();
  bool connected = (usb_configuration != 0);
  if( connected && !ifBeforeUSBConnected){
    if(millis() >= tempsBlink + BLINK_MS){
      digitalWrite(PIN_LED, countBlink % 2);
      if(countBlink>=6){
        countBlink = 0;
        ifBeforeUSBConnected = connected;
        tempsBlink = millis();
      }else{
        countBlink++;
        tempsBlink = millis();
      }
    }
  }
  else if(!connected && !ifBeforeUSBConnected){
    countBlink = 0;
    ifBeforeUSBConnected = connected;
  }
  else if(connected){
    usb_controller_send(&data);
  }
  


}

//pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
//    roll  = atan2(ay, az) * 180.0f / PI;
