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

#define SCALE_RIGHT_DATA    23
#define SCALE_RIGHT_CLOCK   22

uint32_t val = 0;
HX711 scaleL;
HX711 scaleR;

struct position{
  float Lx;
  float Ly;
  float Lz;
  float Rx;
  float Ry;
  float Rz;
  float Rscale;
  float Lscale;
};

static struct position p = {0.,0.,0.,0.,0.,0.,0.,0.};

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


void setup() {
  //Serial.begin(115200);
  pinMode(3, INPUT_PULLUP);
  if (myIMU1.beginSPI(BNO08X1_CS, BNO08X1_INT, BNO08X1_RST) == false && myIMU2.beginSPI(BNO08X2_CS, BNO08X2_INT, BNO08X2_RST) == false) {
    //Serial.println("BNO08x not detected. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  //Serial.println("BNO08x found!");

  //Serial.print(F("Reset Reason: "));
  //printResetReasonName(myIMU1.getResetReason());
  //printResetReasonName(myIMU2.getResetReason());
  //Serial.println();

  setReports();

  //Serial.println("Reading events");
  delay(50);

  scaleL.begin(SCALE_LEFT_DATA,SCALE_LEFT_CLOCK);
  //scaleR.begin(SCALE_RIGHT_DATA,SCALE_RIGHT_CLOCK);
 

}
void setReports(void) {
  //Serial.println("Setting desired reports");
  if (myIMU1.enableAccelerometer(50)) { // 50 ms = 20 Hz
    //Serial.println("Rotation vector enabled 1");
  } else {
    //Serial.println("Could not enable rotation vector 1");
  }
  if (myIMU2.enableAccelerometer(50)) { // 50 ms = 20 Hz
    //Serial.println("Rotation vector enabled 2");
  } else {
    //Serial.println("Could not enable rotation vector 2");
  }
  delay(50); // This delay allows enough time for the BNO086 to accept the new 
              // configuration and clear its reset status
  val=micros();
}


void loop() {
  if(val>= 2000){
    val=0;
    buffer[0] = 0xAB;
    buffer[1] = 0xCD;
    //Serial.println("1");
    myIMU1.serviceBus();
    delayMicroseconds(10);
    //Serial.println("2");
    myIMU2.serviceBus();
    delayMicroseconds(10);
    //Serial.println("3");

    if (myIMU1.wasReset()) {
      //Serial.println(" ------------------ BNO08x has reset. ------------------ ");
      //Serial.print(F("Reset Reason: "));
      //printResetReasonName(myIMU1.getResetReason());
      //Serial.println();

      setReports();  // We'll need to re-enable reports after any reset.
    }
    if (myIMU2.wasReset()) {
      //erial.println(" ------------------ BNO08x has reset. ------------------ ");
      //Serial.print(F("Reset Reason: "));
      //printResetReasonName(myIMU2.getResetReason());
      //Serial.println();

      setReports();  // We'll need to re-enable reports after any reset.
    }
    
    if (myIMU1.getSensorEvent() && myIMU2.getSensorEvent() && scaleL.is_ready()) {
      //Serial.println("4");
      if (myIMU1.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER && myIMU2.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
        //Serial.println("5");
        p.Lx = myIMU1.getAccelX();
        p.Ly = myIMU1.getAccelY();
        p.Lz = myIMU1.getAccelZ();

        p.Rx = myIMU2.getAccelX();
        p.Ry = myIMU2.getAccelY();
        p.Rz = myIMU2.getAccelZ();

        p.Lscale=scaleL.get_units();

        buffer[2] = 0x20;
        buffer[3] = 0x20;
        memcpy(&buffer[4], &p.Lx, 4);
        buffer[8] = 0x20;
        buffer[9] = 0x20;
        memcpy(&buffer[10], &p.Ly, 4);
        buffer[14] = 0x20;
        buffer[15] = 0x20;
        memcpy(&buffer[16], &p.Lz, 4);
        buffer[20] = 0x20;
        buffer[21] = 0x20;
        memcpy(&buffer[22], &p.Rx, 4);
        buffer[26] = 0x20;
        buffer[27] = 0x20;
        memcpy(&buffer[28], &p.Ry, 4);
        buffer[32] = 0x20;
        buffer[33] = 0x20;
        memcpy(&buffer[34], &p.Rz, 4);
        buffer[38] = 0x20;
        buffer[39] = 0x20;
        memcpy(&buffer[40],&p.Lscale,4);
        buffer[44] = 0x20;
        buffer[45] = 0x20;
        memcpy(&buffer[46],&p.Rscale,4);
        buffer[50] = 0x0D;
        

        
        // fill the rest with zeros
        for (int i=51; i<63; i++) {
          buffer[i] = 0;
        }

        int n;
        n = RawHID.send(buffer, 100);
        if (n > 0) {
          //Serial.print(F("Transmit packet "));
          //Serial.println(packetCount);
          packetCount = packetCount + 1;
        } else {
          //Serial.println(F("Unable to transmit packet"));
        }

        // and put a count of packets sent at the end
        //buffer[62] = highByte(packetCount);
        //buffer[63] = lowByte(packetCount);
        // actually send the packet
        //int n;
        /**for(int i = 0; i<64; i++){
          Serial.print(buffer[i]);
        }
        Serial.println();**/
        /**n = RawHID.send(buffer, 100);
        if (n > 0) {
          //Serial.print(F("Transmit packet "));
          //Serial.println(packetCount);
          packetCount = packetCount + 1;
        } else {
          //Serial.println(F("Unable to transmit packet"));
        }**/
        

        //Serial.print(lx); Serial.print(",");
        //Serial.print(ly); Serial.print(",");
        //Serial.print(lz); Serial.print(",");
        //Serial.println(lz);
        //Serial.print(qx); Serial.print(",");
        //Serial.print(qy); Serial.print(",");
        //Serial.println(qz);


        
      }
    }

    
  }

  
  val=micros();

}
