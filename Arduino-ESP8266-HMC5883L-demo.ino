// Arduino UNO with ESP8266 and HMC5883L

#include <Wire.h>
#include <HMC5883L.h>
#include <SoftwareSerial.h>

HMC5883L compass;

int error = 0;

char serialbuffer[1000];

float calibrateX[10];
float calibrateY[10];
float calibrateZ[10];
int calibationIndex = 0;

SoftwareSerial mySerial(10, 11);

void setup() {
  Serial.begin(9600); // Connection to PC
  mySerial.begin(9600); // Connection to ESP8266 
  
  Serial.println("Start");
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
/*
  mySerial.println("AT+RST");
  WaitForReady(2000);
  mySerial.println("AT+CWMODE=1");
  WaitForOK(2000);
  mySerial.println("AT+RST");
  WaitForReady(2000);
  
//  mySerial.println("AT+CWJAP=\"<ssid>\",\"<password>\"");
  
  if (WaitForOK(5000)) {
    digitalWrite(13, HIGH); // Connection succesful
    Serial.println("Connected to Wi-Fi network");
  }
  */
  
  
  Wire.begin(); // Start the I2C interface.
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  
  error = compass.SetScale(1.3); // Set the scale of the compass.
  if(error != 0) // If there is an error, print it out.
    Serial.println(compass.GetErrorText(error));
    
  error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(error != 0) // If there is an error, print it out.
    Serial.println(compass.GetErrorText(error));
    
  calibrateSensor();
}

void loop() {
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  float currentZAxisValue = scaled.ZAxis;
  float averageZAxisValue = calculateSensorAverage(calibrateZ);
  
  float deviation = calculateDeviationFromAverage(currentZAxisValue, averageZAxisValue);
  
  Serial.println("Deviation: " + String(deviation) + " " + String(currentZAxisValue) + " " + String(averageZAxisValue));
  /*
  if (deviation > 5) {
    Serial.println("Parkeeplaats bezet");
    //sendData(10.0);
  } else {
    Serial.println("Parkeeplaats vrij");
  } 
  */
  
  delay(100); // sleep 10 seconds
}

float calculateDeviationFromAverage(float value, float average) {
  float deviation = abs(value - average);
  float percentage = (deviation / average) * 100;
  
  return abs(percentage);
}

void calibrateSensor() {
  Serial.println("Calibrating .. ");
  for(int i = 0; i < 10; i++) {
    MagnetometerScaled scaled = compass.ReadScaledAxis();
    calibrateX[i] = scaled.XAxis;
    calibrateY[i] = scaled.YAxis;
    calibrateZ[i] = scaled.ZAxis;
    Serial.println("Calibrating .. " + String(i));
    delay(60);
  }
}

float calculateSensorAverage(float data[]) {
  int dataLength = sizeof(data);
  float total = 0;
  for(int i = 0; i < dataLength; i++) {
    total = total + data[i];
  }
  
  return total / dataLength;
}

void sendData(float value) {
  mySerial.println("AT+CIPSTART=\"TCP\",\"data.sparkfun.com\",80");
  WaitForOK(5000);
  mySerial.println("AT+CIPSEND=123");
  WaitForOK(5000);
  String getRequest = "GET /input/<public_key>?private_key=<private_key>&temp=" + String(value) + " HTTP/1.0\r\n";
  mySerial.print(getRequest);
  mySerial.print("Hostname: data.sparkfun.com\r\n\r\n");
  WaitForOK(5000);
  mySerial.println("AT+CIPCLOSE");
  WaitForOK(5000);
}

boolean WaitForOK(long timeoutamount) {
  return WaitForResponse("OK", timeoutamount);
}

boolean WaitForReady(long timeoutamount) {
  return WaitForResponse("ready", timeoutamount);
}

boolean WaitForResponse(String response, long timeoutamount) {
//  Serial.setTimeout(timeoutamount);
//  
//  return Serial.find(response);
  unsigned long timeout = millis() + timeoutamount;
  
  while (millis() <= timeout) {
    while (mySerial.available() > 0) {
      int len = mySerial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));
    
      String message = String(serialbuffer).substring(0,len-1);
       
      if (message == response) {
        return true;
      }
    }
  }
  
  return false;
}
