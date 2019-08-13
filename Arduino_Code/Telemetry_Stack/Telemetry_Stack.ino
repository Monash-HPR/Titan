#include "I2Cdev.h"
#include "MPU6050.h"
#include "QMC5883L.h"
#include "BME280.h"
#include "Wire.h"
#include <EasyTransfer.h>
#include <NMEAGPS.h>
#include <string.h> // for strtok

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define GPS_PORT_NAME "Serial1"

//#define MIRROR_SD_WRITES_TO_SERIAL
//#define LOG_IN_BINARY_FORMAT
#define PRINT_ACC 1

#define POWER_LED_PIN 32

#define GPS_LED_PIN 33

#define SENDING_LED_PIN 25

//#define BATTERY_VOLTAGE_PIN 35
//#define BATTERY_VOLTAGE_MULT 2


#define DEBUG_PORT Serial
#define RFDPort Serial1
#define gpsPort Serial2

EasyTransfer ET_MPU;
EasyTransfer ET_HMC;
EasyTransfer ET_BME;
EasyTransfer ET_GPS;

#define RXD_GPS 18
#define TXD_GPS 19

#define RX_RFD 17
#define TX_RFD 16



MPU6050 accelgyro(0x68);
int16_t ax, ay, az;
int16_t gx, gy, gz;
int accelgyro_init_ok = 0;

QMC5883L mag;
int16_t mx, my, mz, mt;
int mag_init_ok = 0;

BME280 bme(0x76);
float bme_temp;
float bme_press;
float bme_hum;
int bme_init_ok = 0;

char gps_string[200];
int gps_string_end_index = 0;

static NMEAGPS  gps;
static gps_fix  fix;

// ------ predefs -----
struct MPU6050_data {
  uint8_t code; // b 1
  
  int16_t ax; // h 2
  int16_t ay; // h 2
  int16_t az; // h 2
  int16_t gx; // h 2
  int16_t gy; // h 2
  int16_t gz; // h 2
};

struct HMC5883L_data {
  uint8_t code; // b 1
  
  int16_t mx;
  int16_t my;
  int16_t mz;
  int16_t mt;
};

struct BME280_data {
  uint8_t code; // b 1
  float bme_temp;
  float bme_press;
  float bme_hum;
};

struct GPS_data {
  uint8_t code; // b 1
  int32_t altitudecm; //i 4
  int32_t latitude; //i 4
  int32_t longitude;  //i 4

  uint32_t speed_mkn; //I 4
  uint16_t heading_cd; //H 2

};

MPU6050_data myMPU;
HMC5883L_data myHMC;
BME280_data myBME;
GPS_data myGPS;

void setup() {

  DEBUG_PORT.begin(115200);
  gpsPort.begin(9600, SERIAL_8N1, RXD_GPS, TXD_GPS);
  RFDPort.begin( 115200, SERIAL_8N1, RX_RFD, TX_RFD );

  neosetup();
  pinMode(POWER_LED_PIN, OUTPUT);
  pinMode(GPS_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);

  //pinMode(BATTERY_VOLTAGE_PIN, INPUT);

  Wire.begin();

  accelgyro.initialize();
  DEBUG_PORT.println("Testing MPU6050 connection...");
  accelgyro_init_ok = accelgyro.testConnection();
  DEBUG_PORT.println(accelgyro_init_ok ? "MPU6050 connection successful" : "MPU6050 connection failed");
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  //  MPU6050_self_test(accelgyro);

  mag.init();
  mag.setSamplingRate(200);
  DEBUG_PORT.println("Testing HMC5883L connection...");
  mag_init_ok = mag.ready();
  DEBUG_PORT.println(mag_init_ok ? "HMC5883L connection successful" : "HMC5883L connection failed");

  bme.init();
  DEBUG_PORT.println("Testing BME280 connection...");
  bme_init_ok = bme.checkConnection();
  DEBUG_PORT.println(bme_init_ok ? "BME280 connection successful" : "BME280 connection failed");

  ET_MPU.begin(details(myMPU), &RFDPort);
  ET_HMC.begin(details(myHMC), &RFDPort);
  ET_BME.begin(details(myBME), &RFDPort);
  ET_GPS.begin(details(myGPS), &RFDPort);

  delay(1000);
}


void loop() {
  while (gps.available( gpsPort )) {
    fix = gps.read();
    doSomeWork();
  }

  if (PRINT_ACC == 1) {
    //if (Wire.available()) {
    MPUprint();
    HMCprint();
    BMEprint();
    //}
  }
  // just log stuff
}

static void doSomeWork()
{
  // Print all the things!

  // trace_all( DEBUG_PORT, gps, fix );
  gpsprint();
  if (fix.valid.location) {
    digitalWrite(GPS_LED_PIN, HIGH);
    }

}

void MPUprint()
{

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  myMPU.code = 10; //b 1

  myMPU.ax = ax; //
  myMPU.ay = ay; //
  myMPU.az = az; //

  myMPU.gx = gx; //
  myMPU.gy = gy; //
  myMPU.gz = gz; //


  ET_MPU.sendData();
}

void BMEprint()
{
  bme.getValues(&bme_temp, &bme_press, &bme_hum);

  myBME.code = 20; //b 1

  myBME.bme_temp = bme_temp;
  myBME.bme_press = bme_press;
  myBME.bme_hum = bme_hum;

  ET_BME.sendData();
}

void HMCprint()
{
  mag.readRaw(&mx, &my, &mz, &mt);

  myHMC.code = 30; //b 1

  myHMC.mx = mx;
  myHMC.my = my;
  myHMC.mz = mz;

  ET_HMC.sendData();
}

void gpsprint() {

  if (fix.valid.location) {
    myGPS.code = 254;

    myGPS.altitudecm = fix.altitude_cm();
    myGPS.latitude = fix.latitudeL();
    myGPS.longitude = fix.longitudeL();

    myGPS.speed_mkn = fix.speed_mkn();
    myGPS.heading_cd = fix.heading_cd();

    ET_GPS.sendData();
  }
}

//int MPU6050_self_test(MPU6050 *mpu) {
//  return 0;
//}
