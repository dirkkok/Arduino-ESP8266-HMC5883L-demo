/*
  Mega multple serial test

 Receives from the main serial port, sends to the others.
 Receives from serial port 1, sends to the main serial (Serial 0).

 This example works only on the Arduino Mega

 The circuit:
 * Any serial device attached to Serial port 1
 * Serial monitor open on Serial port 0:

 created 30 Dec. 2008
 modified 20 May 2012
 by Tom Igoe & Jed Roach

 This example code is in the public domain.

 */

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  mySerial.begin(9600);
  
  Serial.println("asfasfas");
}

void loop() {  
  // read from port 1, send to port 0:
  if (mySerial.available()) {
    int inByte = mySerial.read();
    Serial.write(inByte);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    int inByte = Serial.read();
    mySerial.write(inByte);
  }
}
