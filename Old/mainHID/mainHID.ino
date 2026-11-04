#include "SparkFun_BNO08x_Arduino_Library.h"
#include "HX711.h"
BNO08x myIMU1;
BNO08x myIMU2;

#define BNO08X1_CS   15
#define BNO08X1_INT  16
#define BNO08X1_RST  17

#define BNO08X2_CS   18
#define BNO08X2_INT  19
#define BNO08X2_RST  20

#define SCALE_LEFT_DATA     3 
#define SCALE_LEFT_CLOCK    4

//#define SCALE_RIGHT_DATA    22
//#define SCALE_RIGHT_CLOCK   23

uint32_t timeµs = 0;
uint32_t currentµs = 0;
HX711 scaleL;
//HX711 scaleR;

bool hasToUp = false;

byte buffer[64];
unsigned int packetCount = 0;

void printResetReasonName(byte resetReasonNumber)
{
    if(resetReasonNumber == 1) Serial.print("POR");
    else if(resetReasonNumber == 2) Serial.print("Internal reset");
    else if(resetReasonNumber == 3) Serial.print("Watchdog");
    else if(resetReasonNumber == 4) Serial.print("External reset");
    else if(resetReasonNumber == 5) Serial.print("Other");
}

void constructBuffer(){
    buffer[0] = 0xAB;
    buffer[1] = 0xCD;
    buffer[2] = 0x20;
    buffer[3] = 0x20;
    buffer[4] = 0x00;
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x20;
    buffer[9] = 0x20;
    buffer[10] = 0x00;
    buffer[11] = 0x00;
    buffer[12] = 0x00;
    buffer[13] = 0x00;
    buffer[14] = 0x20;
    buffer[15] = 0x20;
    buffer[16] = 0x00;
    buffer[17] = 0x00;
    buffer[18] = 0x00;
    buffer[19] = 0x00;
    buffer[20] = 0x20;
    buffer[21] = 0x20;
    buffer[22] = 0x00;
    buffer[23] = 0x00;
    buffer[24] = 0x00;
    buffer[25] = 0x00;
    buffer[26] = 0x20;
    buffer[27] = 0x20;
    buffer[28] = 0x00;
    buffer[29] = 0x00;
    buffer[30] = 0x00;
    buffer[31] = 0x00;
    buffer[32] = 0x20;
    buffer[33] = 0x20;
    buffer[34] = 0x00;
    buffer[35] = 0x00;
    buffer[36] = 0x00;
    buffer[37] = 0x00;
    buffer[38] = 0x20;
    buffer[39] = 0x20;
    buffer[40] = 0x00;
    buffer[41] = 0x00;
    buffer[42] = 0x00;
    buffer[43] = 0x00;
    buffer[44] = 0x20;
    buffer[45] = 0x20;
    buffer[46] = 0x00;
    buffer[47] = 0x00;
    buffer[48] = 0x00;
    buffer[49] = 0x00;
    buffer[50] = 0x0D;

    for (int i=51; i<63; i++) {
        buffer[i] = 0;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("begin setup");
    if (myIMU1.beginSPI(BNO08X1_CS, BNO08X1_INT, BNO08X1_RST) == false && myIMU2.beginSPI(BNO08X2_CS, BNO08X2_INT, BNO08X2_RST) == false) {
        while (1)
        ;
    }
    setReports();
    delay(50);
    scaleL.begin(SCALE_LEFT_DATA,SCALE_LEFT_CLOCK);
    //scaleR.begin(SCALE_RIGHT_DATA,SCALE_RIGHT_CLOCK);
    constructBuffer();
    Serial.println("end setup");
}

void setReports(void) {
    myIMU1.enableAccelerometer(5); // 50 ms = 20 Hz
    myIMU2.enableAccelerometer(5); // 50 ms = 20 Hz
    delay(50); // This delay allows enough time for the BNO086 to accept the new configuration and clear its reset status
    timeµs=micros();
}

void loop() {

    if((micros()-timeµs)>= 2000){
        timeµs=micros();

        myIMU1.serviceBus();
        delayMicroseconds(10);
        myIMU2.serviceBus();
        delayMicroseconds(10);
        if (myIMU1.wasReset()) {
            setReports();  // We'll need to re-enable reports after any reset.
        }
        if (myIMU2.wasReset()) {
            setReports();  // We'll need to re-enable reports after any reset.
        }
    
    if (myIMU1.getSensorEvent() && myIMU2.getSensorEvent()) {
        if (myIMU1.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER && myIMU2.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
            hasToUp = true;

            memcpy(&buffer[4], &(float){myIMU1.getAccelX()}, 4);
            Serial.println(myIMU1.getAccelX());
            memcpy(&buffer[10], &(float){myIMU1.getAccelY()}, 4);
            Serial.println(myIMU1.getAccelY());
            memcpy(&buffer[16], &(float){myIMU1.getAccelZ()}, 4);
            Serial.println(myIMU1.getAccelZ());
            memcpy(&buffer[22], &(float){myIMU2.getAccelX()}, 4);
            memcpy(&buffer[28], &(float){myIMU2.getAccelY()}, 4);
            memcpy(&buffer[34], &(float){myIMU2.getAccelZ()}, 4);
        }
    }
    if(scaleL.is_ready() /**&& scaleR.is_ready()**/){
        hasToUp = true;
        memcpy(&buffer[40], &(float){scaleL.get_units()}, 4);
        //memcpy(&buffer[46], &(float){scaleR.get_units()}, 4);
    }

    if(hasToUp){
        hasToUp = false;
        RawHID.send(buffer, 100);
        Serial.println("send");
    }

    
  }


}
