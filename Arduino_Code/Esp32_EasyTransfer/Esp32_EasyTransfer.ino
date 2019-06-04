

#include <NMEAGPS.h>
#include <Wire.h>
#include <LSM6.h>
#include <LIS3MDL.h>
#include <LPS.h>
#include <Streamers.h>
#include <EasyTransfer.h>
//#include <HPDF.h>

LSM6 gyro_acc;
LIS3MDL mag;
LPS ps;

EasyTransfer ET_ACC;
EasyTransfer ET_GPS;

//print accelerometer data?
#define PRINT_ACC 1
#define DEBUG_PORT Serial
#define RFDPort Serial1
#define gpsPort Serial2

#define GPS_PORT_NAME "Serial1"


#define TXGPS 19
#define RXGPS 18
#define TXRFD 17
#define RXRFD 16


//------------------------------------------------------------
// This object parses received characters
//   into the gps.fix() data structure

static NMEAGPS  gps;

//------------------------------------------------------------
//  Define a set of GPS fix information.  It will
//  hold on to the various pieces as they are received from
//  an RMC sentence.  It can be used anywhere in your sketch.

static gps_fix  fix;


int ledPin = 13;

struct ACC_STRUCTURE {
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  uint8_t code; //B 1

  int16_t gyrox; //h 2
  int16_t gyroy; //h 2
  int16_t gyroz; //h 2

  int16_t accx; //h 2
  int16_t accy; //h 2
  int16_t accz; //h 2

  int16_t magx; //h 2
  int16_t magy; //h 2
  int16_t magz; //h 2

  int32_t pressuremb; //i 4
  int16_t tempC; //h 2
  // checksum B?


};

struct GPS_STRUCTURE {
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  uint8_t code; // b 1

  int32_t altitudecm; //i 4
  int32_t latitude; //i 4
  int32_t longitude;  //i 4

  uint32_t speed_mkn; //I 4
  uint16_t heading_cd; //H 2

};

ACC_STRUCTURE myAcc;


GPS_STRUCTURE myGPS;

void setup()
{
  delay(2000);

  DEBUG_PORT.begin(115200);

  i2cSetup();
  enableGyroAcc();
  enableMag();
  enableBaro();

  pinMode(ledPin, OUTPUT);    //define LED pin
  digitalWrite(ledPin, HIGH); // Turn on LED pin

  neosetup();

  RFDPort.begin( 115200, SERIAL_8N1, RXRFD, TXRFD );
  delay(100);

  gpsSetup(); //configures an MTK3339 based GPS (Adafruit Ultimate) -- See other tab

  delay(1000);

  ET_ACC.begin(details(myAcc), &RFDPort);
  ET_GPS.begin(details(myGPS), &RFDPort);
}

void loop()
{
  GPSloop();

}


//----------------------------------------------------------------
//  This function gets called about once per second, during the GPS
//  quiet time.  It's the best place to do anything that might take
//  a while: print a bunch of things, write to SD, send an SMS, etc.
//
//  By doing the "hard" work during the quiet time, the CPU can get back to
//  reading the GPS chars as they come in, so that no chars are lost.
static void doSomeWork()
{
  // Print all the things!

  // trace_all( DEBUG_PORT, gps, fix );
  gpsprint();

}


//------------------------------------
//  This is the main GPS parsing loop.
static void GPSloop()
{
  while (gps.available( gpsPort )) {
    fix = gps.read();
    doSomeWork();
  }

  if (PRINT_ACC == 1) {
    //if (Wire.available()) {
    accprint();
    //}
  }
}
//------------------------------------


// prints accelerometer data to port specified in function call
void accprint()
{

  gyro_acc.readGyro();
  gyro_acc.readAcc();
  mag.read();

  myAcc.code = 69; //b 1

  myAcc.gyrox = gyro_acc.g.x; //
  myAcc.gyroy = gyro_acc.g.y; //
  myAcc.gyroz = gyro_acc.g.z; //

  myAcc.accx = gyro_acc.a.x; //
  myAcc.accy = gyro_acc.a.y; //
  myAcc.accz = gyro_acc.a.z; //

  myAcc.magx = mag.m.x;
  myAcc.magy = mag.m.y;
  myAcc.magz = mag.m.z;

  myAcc.pressuremb = ps.readPressureMillibars();
  myAcc.tempC = ps.readTemperatureRaw();

  ET_ACC.sendData();
}


// prints data stored in GPS fix structure to port specified in function call
void gpsprint() {

  if (fix.valid.location) {
    //    myPort.print(fix.dateTime.date);
    //    myPort.print(fix.dateTime.month);
    //    myPort.print(fix.dateTime.year);
    //    myPort.print("-");
    //    myPort.print(fix.dateTime.hours);
    //    myPort.print(fix.dateTime.minutes);
    //    myPort.print(fix.dateTime.seconds);
    //    myPort.print(fix.dateTime_ms());
    //    myPort.print(",");



    myGPS.code = 254;

    myGPS.altitudecm = fix.altitude_cm();
    myGPS.latitude = fix.latitudeL();
    myGPS.longitude = fix.longitudeL();

    myGPS.speed_mkn = fix.speed_mkn();
    myGPS.heading_cd = fix.heading_cd();

    ET_GPS.sendData();
  }
}
