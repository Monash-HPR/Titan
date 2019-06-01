#include <HPDF.h>

#include <NMEAGPS.h>
#include <Wire.h>
#include <LSM6.h>
#include <LIS3MDL.h>
#include <LPS.h>
#include <Streamers.h>

LSM6 gyro_acc;
LIS3MDL mag;
LPS ps;

//print accelerometer data?
#define PRINT_ACC 1

int IMU[8];
float PressMb;
float tempC;
#define RFDPort Serial1
#define gpsPort Serial2

#define GPS_PORT_NAME "Serial1"
#define DEBUG_PORT Serial

#define TXGPS 18
#define RXGPS 19
#define TXRFD 16
#define RXRFD 17


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

 //HPDF initialisation
 //Gyroscope
 HPDF_Sensor gyroscope ("A",0.02);
 HPDF_Feed gyro_x (gyroscope.id,0,HPDF::generateControl(gyroscope.id,0),DEBUG_PORT);

float returnGyroX() 
{

  return static_cast<float>(gyro_acc.g.x);
  
}

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

  RFDPort.begin( 115200, SERIAL_8N1, TXRFD, RXRFD );
  delay(100);

  gpsSetup(); //configures an MTK3339 based GPS (Adafruit Ultimate) -- See other tab

  delay(1000);

  //HPDF function call assignment
  gyro_x.source = &returnGyroX;


  
}

void loop()
{
  //GPSloop();
  gyro_x.transmit();
  
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
  gpsprint(RFDPort);

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
    accprint(RFDPort);
    //}
  }
}
//------------------------------------


// prints accelerometer data to port specified in function call
void accprint(Stream &myPort)
{
  
  gyro_acc.readGyro();
  gyro_acc.readAcc();
  mag.read();

  myPort.print("IMU:");
  myPort.print(ps.readPressureMillibars());
  myPort.print(",");
  myPort.print(ps.readTemperatureRaw());
  myPort.print(",");

  myPort.print(gyro_acc.g.x);
  myPort.print(",");
  myPort.print(gyro_acc.g.y);
  myPort.print(",");
  myPort.print(gyro_acc.g.z);
  myPort.print(",");

  myPort.print(gyro_acc.a.x); //>> 4; // shift right 4 bits to use 12-bit representation (1 g = 256)
  myPort.print(",");
  myPort.print(gyro_acc.a.y); //>> 4;
  myPort.print(",");
  myPort.print(gyro_acc.a.z); //>> 4;
  myPort.print(",");

  myPort.print(mag.m.x);
  myPort.print(",");
  myPort.print(mag.m.y);
  myPort.print(",");
  myPort.print(mag.m.z);
  
  myPort.println();
  
}


// prints data stored in GPS fix structure to port specified in function call
void gpsprint(Stream &myPort) {
  
  if (fix.valid.location) {
    
    myPort.print("GPS:");
    //trace_all( RFDPort, gps, fix);
    myPort.print(fix.dateTime.date);
    myPort.print(fix.dateTime.month);
    myPort.print(fix.dateTime.year);
    myPort.print("-");
    myPort.print(fix.dateTime.hours);
    myPort.print(fix.dateTime.minutes);
    myPort.print(fix.dateTime.seconds);
    myPort.print(fix.dateTime_ms());
    myPort.print(",");
    
    myPort.print(fix.altitude_cm());
    myPort.print(",");
    myPort.print(fix.latitudeL());
    myPort.print(",");
    myPort.print(fix.longitudeL());
    myPort.print(",");
    myPort.print(fix.speed_mkn());
    myPort.print(",");
    myPort.print(fix.heading_cd());
    
    myPort.println();
  }
}
