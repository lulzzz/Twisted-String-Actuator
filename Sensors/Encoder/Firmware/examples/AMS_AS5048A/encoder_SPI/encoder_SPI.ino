  /*
   AS5047P Magnetic Rotary Encoder Chip
  
   Circuit:
   AS5047P sensor attached to pins 10 - 13:
   CS: pin 10
   MOSI: pin 11
   MISO: pin 12
   SCK: pin 13
  
   Simon Kalouche
   December 2015
   */
  
  // Transactions on the AS5047P chip are 16 bits, 2 bytes. // big endian in SPI -> means you send most significant bit first
  
  // the sensor communicates using SPI, so include the library:
  #include <SPI.h>
  
  const short READ = 0x3FFF;     // AS5047P measured angle with dynamic angle error compensation
  const short WRITE_MSB = 0x0016;   // write command for Zero Position MSB (most significant bit)
  const short WRITE_LSB = 0x0017;   // write command for Zero Position LSB
  
  
  // pins used for the connection with the sensor
  // the other you need are controlled by the SPI library):
  const int chipSelectPin = 10;
  
  void setup() {
    Serial.begin(9600);
    SPI.begin();          // start the SPI library:
    //    SPI.setBitOrder(MSBFIRST);
    //    SPI.setDataMode (SPI_MODE1) ;  //Configure chip to use fallin clock edge mode
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));
  
    // initalize the  data ready and chip select pins:
    pinMode(chipSelectPin, OUTPUT);
  
    // give the sensor time to set up:
    delay(100);
  }
  
  void loop() {
    digitalWrite(chipSelectPin, LOW);
    int result1 = SPI.transfer(0x3FFF);
    int angleData = SPI.transfer(0x3FFF);
    digitalWrite(chipSelectPin, HIGH);    // release SPI device.
    
    int angle = map(angleData,0,255,0,359);
  
    //int angleData = readRegister(READ, 2);
    Serial.print("Angle[deg]= ");
    Serial.println(angle);   

delay(100);

}
  










  //Read from or write to register from the SCP1000:
  unsigned int readRegister(short thisRegister, int bytesToRead) {
    byte inByte = 0;           // incoming byte from the SPI
    unsigned int result = 0;   // result to return
    Serial.print(thisRegister, BIN);
    Serial.print("\t");
    // SCP1000 expects the register name in the upper 6 bits
    // of the byte. So shift the bits left by two bits:
    thisRegister = thisRegister << 2;
    // now combine the address and the command into one byte
    byte dataToSend = thisRegister & READ;
    Serial.println(thisRegister, BIN);
    // take the chip select low to select the device:
    digitalWrite(chipSelectPin, LOW);
    // send the device the register you want to read:
    SPI.transfer(dataToSend);
    // send a value of 0 to read the first byte returned:
    result = SPI.transfer(0x00);
    // decrement the number of bytes left to read:
    bytesToRead--;
    // if you still have another byte to read:
    if (bytesToRead > 0) {
      // shift the first byte left, then get the second byte:
      result = result << 8;
      inByte = SPI.transfer(0x00);
      // combine the byte you just got with the previous one:
      result = result | inByte;
      // decrement the number of bytes left to read:
      bytesToRead--;
    }
    // take the chip select high to de-select:
    digitalWrite(chipSelectPin, HIGH);
    // return the result:
    return (result);
  }
  
  void writeRegister(byte thisRegister, byte thisValue) {
  
    // SCP1000 expects the register address in the upper 6 bits
    // of the byte. So shift the bits left by two bits:
    thisRegister = thisRegister << 2;
    // now combine the register address and the command into one byte:
    byte dataToSend = thisRegister | WRITE_MSB;
  
    // take the chip select low to select the device:
    digitalWrite(chipSelectPin, LOW);
  
    SPI.transfer(dataToSend); //Send register location
    SPI.transfer(thisValue);  //Send value to record into register
  
    // take the chip select high to de-select:
    digitalWrite(chipSelectPin, HIGH);
  }
