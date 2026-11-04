//
//    FILE: HX_is_ready.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711


#include "HX711.h"

HX711 scaleLeft;
HX711 scaleRight;


#define SCALE_LEFT_DATA     3
#define SCALE_LEFT_CLOCK    4
#define SCALE_RIGHT_DATA     22
#define SCALE_RIGHT_CLOCK    23

//  adjust pins if needed



void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("HX711_LIB_VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  scaleLeft.begin(SCALE_LEFT_DATA, SCALE_LEFT_CLOCK);
  scaleRight.begin(SCALE_RIGHT_DATA, SCALE_RIGHT_CLOCK);

  //  TODO find a nice solution for this calibration..
  //  load cell factor 20 KG
  //  scale.set_scale(127.15);

  //  load cell factor 5 KG
  scaleLeft.set_scale(420.0983);       //  TODO you need to calibrate this yourself.
  //  reset the scale to zero = 0
  scaleLeft.tare(20);
  scaleRight.set_scale(420.0983);
  scaleRight.tare(20);
}


void loop()
{
  if (scaleLeft.is_ready())
  {
    Serial.print("ScaleLeft: ");
    Serial.println(scaleLeft.get_units(1));
  }
  if (scaleRight.is_ready())
  {
    Serial.print("scaleRight: ");
    Serial.println(scaleRight.get_units(1));
  }
  delay(1000);
}


//  -- END OF FILE --
