#include "usb_controller.h"

controller_report_t mesInfos = {0,0,0,0,0,0};

void setup() {
  

}

void loop() {
  usb_controller_send(&mesInfos);
  mesInfos.pitch_left++;
  mesInfos.pitch_right++;
  mesInfos.roll_left++;
  mesInfos.roll_right++;
  mesInfos.weight_left++;
  mesInfos.weight_right++;
  delay(1000);

}
