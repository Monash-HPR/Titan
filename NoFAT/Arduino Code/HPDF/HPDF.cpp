/*
  Test.h - Test library for Wiring - implementation
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// include this library's description file
#include "HPDF.h"

// include description files for other libraries used (if any)
#include "HardwareSerial.h"

// Public Methods //////////////////////////////////////////////////////////////
//
HPDF_t_float::HPDF_t_float(float f) {

  int posneg, exponentN;

  if (f >= 0)
  posneg = 1;
  else
  posneg = 0;

  exponentN = (int)log10(abs(f));
  significand1 = (static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/100000)) % 10;
  significand2 = (static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/10000)) % 10;
  significand3 = (static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/1000)) % 10;
  significand4 = (static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/100)) % 10;
  significand5 = (static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/10)) % 10;
  significand6 = ((static_cast<int>((abs((f / pow(10, exponentN))* pow(10, 5)))/1)) % 10)-1;

  sign = posneg;

  //Fix exponentiation overflow
  if (exponentN < -7) {

    exponent = -7;

  } else if (exponentN > 8) {

    exponent = 8;

  } else {

   exponent = exponentN;

  }

}

HPDF_t_FrameData::HPDF_t_FrameData(int posneg,int exponent,int significand1,int significand2,int significand3,int significand4,int significand5,int significand6,int channel) {

  byte nySign = HPDF::decimalToNibble(((10 + (2*channel) - posneg)-1));
  byte nyExp = HPDF::decimalToNibble(exponent);

  byte a1 = HPDF::decimalToNibble(significand1);
  byte a2 = HPDF::decimalToNibble(significand2);
  byte b1 = HPDF::decimalToNibble(significand3);
  byte b2 = HPDF::decimalToNibble(significand4);
  byte c1 = HPDF::decimalToNibble(significand5);
  byte c2 = HPDF::decimalToNibble(significand6);

  byte byA = HPDF::joinNibble(nySign,nyExp);
  byte byB = HPDF::joinNibble(a1,a2);
  byte byC = HPDF::joinNibble(b1,b2);
  byte byD = HPDF::joinNibble(c1,c2);

  A = byA;
  B = byB;
  C = byC;
  D = byD;

}

void HPDF::sendFrame(byte data,Stream &port) {

  port.flush();
  port.write(data);

}

void HPDF::sendFrameTerminator(Stream &port) {

  port.flush();
  port.write(255);

}

byte HPDF::generateControl(char *sensorN, int feedN) {

  byte feedC = static_cast<byte>(feedN);

  int sensorInt = HPDF::toDeci(sensorN,32);

  byte sensorC = HPDF::decimalToNibble(sensorInt);

  byte stream = HPDF::joinNibbleStream(sensorC,feedC);

  return stream;

};

void HPDF::sendPacket(byte control,HPDF_t_FrameData data, Stream &port) {


  HPDF::sendFrame(control,port);

  HPDF::sendFrame(data.A,port);
  HPDF::sendFrame(data.B,port);
  HPDF::sendFrame(data.C,port);
  HPDF::sendFrame(data.D,port);

  HPDF::sendFrameTerminator(port);

}

void HPDF_Feed::transmit() {

  float rawval = source();
  //HPDF_t_float hpdffloat = HPDF::floatToComponents(rawval);
  HPDF_t_float hpdffloat = HPDF_t_float(rawval);
  //HPDF_t_FrameData data = HPDF::createFrameData(hpdffloat.sign,hpdffloat.exponent,hpdffloat.significand1,hpdffloat.significand2,hpdffloat.significand3,hpdffloat.significand4,hpdffloat.significand5,hpdffloat.significand6,channel);
  HPDF_t_FrameData data = HPDF_t_FrameData(hpdffloat.sign,hpdffloat.exponent,hpdffloat.significand1,hpdffloat.significand2,hpdffloat.significand3,hpdffloat.significand4,hpdffloat.significand5,hpdffloat.significand6,this->channel);
  HPDF::sendPacket(this->controlStream,data,this->port);

}

// Private Methods //////////////////////////////////////////////////////////////
//
int HPDF::val(char c) {

  if (c >= '0' && c <= '9')
  return (int)c - '0';
  else
  return (int)c - 'A' + 10;

}

int HPDF::toDeci(char *str,int base) {

  int len = strlen(str);
  int power = 1; // Initialize power of base
  int num = 0;  // Initialize result
  int i;

  // Decimal equivalent is str[len-1]*1 +
  // str[len-1]*base + str[len-1]*(base^2) + ...
  for (i = len - 1; i >= 0; i--)
  {
    // A digit in input number must be
    // less than number's base
    if (HPDF::val(str[i]) >= base)
    {
      return -1;
    }

    num += HPDF::val(str[i]) * power;
    power = power * base;

  }

  return num;

}

byte HPDF::decimalToNibble(int n) {

  byte nibble = 0b00000000;
  byte rawnum = n;

  bitWrite(nibble,7,bitRead(rawnum,7));
  bitWrite(nibble,6,bitRead(rawnum,6));
  bitWrite(nibble,5,bitRead(rawnum,5));
  bitWrite(nibble,4,bitRead(rawnum,4));
  bitWrite(nibble,3,bitRead(rawnum,3));
  bitWrite(nibble,2,bitRead(rawnum,2));
  bitWrite(nibble,1,bitRead(rawnum,1));
  bitWrite(nibble,0,bitRead(rawnum,0));

  return nibble;

}

byte HPDF::joinNibble(byte n1, byte n2) {

  byte newbyte = 0b00000000;

  bitWrite(newbyte,7,bitRead(n1,3));
  bitWrite(newbyte,6,bitRead(n1,2));
  bitWrite(newbyte,5,bitRead(n1,1));
  bitWrite(newbyte,4,bitRead(n1,0));
  bitWrite(newbyte,3,bitRead(n2,3));
  bitWrite(newbyte,2,bitRead(n2,2));
  bitWrite(newbyte,1,bitRead(n2,1));
  bitWrite(newbyte,0,bitRead(n2,0));

  return newbyte;

}

byte HPDF::joinNibbleStream(byte n1, byte n2) {

  byte newbyte = 0b00000000;

  bitWrite(newbyte,7,bitRead(n1,4));
  bitWrite(newbyte,6,bitRead(n1,3));
  bitWrite(newbyte,5,bitRead(n1,2));
  bitWrite(newbyte,4,bitRead(n1,1));
  bitWrite(newbyte,3,bitRead(n1,0));
  bitWrite(newbyte,2,bitRead(n2,2));
  bitWrite(newbyte,1,bitRead(n2,1));
  bitWrite(newbyte,0,bitRead(n2,0));

  return newbyte;

}


// Class constructors /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

HPDF_Sensor::HPDF_Sensor(char *identification, int frequency)
{

  id = identification;
  enabled = true;
  freq = frequency;
  wait = ((1 / freq) * 1000);

}


HPDF_Feed::HPDF_Feed(char *parentID, int identification, byte controlByte, Stream &desiredPort)
{

  id = identification;
  sensorID = parentID;
  controlStream = controlByte;
  channel = 1;
  port = desiredPort;

}
