// Arduino UNO with ESP8266 and HMC5883L

#include <Wire.h>
#include <HMC5883L.h>
#include <SoftwareSerial.h>

HMC5883L compass;

int error = 0;

char serialbuffer[100];

float calibrateX[10];
float calibrateY[10];
float calibrateZ[10];
int calibationIndex = 0;

SoftwareSerial mySerial(10, 11);

boolean parking_is_available = true;

void setup() {
  Serial.begin(9600); // Connection to PC
  mySerial.begin(9600); // Connection to ESP8266 
  
  Serial.println("Start");
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  mySerial.println("AT+RST");
  WaitForReady(2000);
  mySerial.println("AT+CWMODE=1");
  WaitForOK(2000);
  mySerial.println("AT+RST");
  WaitForReady(2000);
  
  mySerial.println("AT+CWJAP=\"<ssid>\",\"<password>\"");
  delay(2000);
  /*
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
  
  if (deviation > 5) {
    Serial.println("Parkeeplaats bezet");
    setParkingAvailable(false);
  } else {
    Serial.println("Parkeeplaats vrij");
    setParkingAvailable(true);
  } 
    
  delay(1000); // sleep 10 seconds
}

void setParkingAvailable(boolean is_available) {
  // Check if status had changed since last check
  if (parking_is_available != is_available) {
    if (is_available) {
      sendAvailableToServer();
    } else {
      sendNotAvailableToServer();
    }
  }
  
  parking_is_available = is_available;
}

void sendAvailableToServer() {
  sendStatusUpdate("true");
}

void sendNotAvailableToServer() {
  sendStatusUpdate("false");
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

void sendStatusUpdate(String data) {
  // data.sparkfun.com/input/<public_key>?private_key=<private_key>&is_available=false
  String getRequest = "GET /input/<public_key>?private_key=<private_key>&is_available=" + data + " HTTP/1.0\r\n";
  getRequest += "Host: data.sparkfun.com\r\n\r\n";
  int requestLength = getRequest.length();
  
  Serial.println("Starting get request: (" + String(requestLength) + ") " + getRequest);
  
  Serial.println("AT+CIPSTART=\"TCP\",\"data.sparkfun.com\",80");
  mySerial.println("AT+CIPSTART=\"TCP\",\"data.sparkfun.com\",80");
  delay(2000);
  WaitForOK(5000);
  
  Serial.println("AT+CIPSEND=" + String(requestLength));
  mySerial.println("AT+CIPSEND=" + String(requestLength));
  WaitForOK(5000);
  
  mySerial.println(getRequest);
  WaitForOK(10000);
  
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
