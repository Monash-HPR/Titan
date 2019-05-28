/*
  Test.h - Test library for Wiring - description
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// ensure this library description is only included once
#ifndef HPDF_h
#define HPDF_h

//Library includes
#include "Arduino.h"

//Define baseclock in millis
#define HPDF_BCLK 50


///
/// Data structure classes
///
class HPDF_t_FrameData {

public:

  byte A;
  byte B;
  byte C;
  byte D;

  HPDF_t_FrameData(int posneg,int exponent,int significand1,int significand2,int significand3,int significand4,int significand5,int significand6,int channel);

};

class HPDF_t_float {

public:

  int sign;
  int exponent;
  int significand1;
  int significand2;
  int significand3;
  int significand4;
  int significand5;
  int significand6;

  HPDF_t_float(float f);

};


///
/// Library definition
///
class HPDF {

public:

  static void sendPacket(byte control,HPDF_t_FrameData data,Stream *port);

  static int val(char c);
  static int toDeci(char *str,int base);
  static byte decimalToNibble(int n);
  static byte joinNibble(byte n1, byte n2);
  static byte joinNibbleStream(byte n1, byte n2);
  static byte generateControl(char *sensorN, int feedN);
  static void sendFrame(byte data,Stream &port);
  static void sendFrameTerminator(Stream &port);

};

//Sensor class
class HPDF_Sensor {

public:
  char* id;
  bool enabled;
  void (*apiRead)(); //Sensor.apiRead = &somefunction; ,, Function defined by sensor OEM that reads data from sensor
  int freq; //Hz
  int wait;

  HPDF_Sensor(char* identification, int frequency);

};

//Feed class
class HPDF_Feed {

public:
  int id, channel;
  char* sensorID;
  float (*source)(); //Feed.source = &somefunction;  ,, Function defined by us that returns a single float from an apiFeed read
  byte controlStream;
  Stream &port;

  HPDF_Feed(char *parentID, int identification, byte controlByte, Stream &desiredPort);

  void transmit();

};

#endif
