void gpsSetup() {
  gpsPort.begin( 9600, SERIAL_8N1, RXGPS, TXGPS);
  delay(100);
  gpsPort.print("$PMTK251,38400*27\r\n");
  delay(100);
  gpsPort.begin( 38400, SERIAL_8N1, RXGPS, TXGPS );
  delay(100);
  gpsPort.print("$PMTK220,100*2F\r\n");
  delay(100);
  gpsPort.print("PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2C\r\n");
  delay(100);
}

void i2cSetup() {
  Wire.begin();
  Wire.setClock(100000);
  DEBUG_PORT.println("I2C Bus Initialized");
}

void enableGyroAcc() {
  gyro_acc.init();
  gyro_acc.enableDefault();
  gyro_acc.writeReg(LSM6::CTRL2_G, 0x4C); // 104 Hz, 2000 dps full scale
  gyro_acc.writeReg(LSM6::CTRL1_XL, 0x44); // 104 Hz, +-16 g full scale
  DEBUG_PORT.println("Gyro / Accelerometer Enabled");
}

void enableMag() {
  mag.init();
  mag.enableDefault();
  DEBUG_PORT.println("Magnetometer Enabled");
}

void enableBaro() {
  ps.init();
  ps.enableDefault();
  DEBUG_PORT.println("Barometer Enabled");
}
