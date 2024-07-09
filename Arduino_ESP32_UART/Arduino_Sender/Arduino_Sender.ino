#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600);       // Initialize serial communication at 9600 bps for the serial monitor
  mySerial.begin(9600);     // Initialize software serial communication at 9600 bps
}

void loop() {
  mySerial.println("Hello from Arduino?!");  // Send data to ESP32
  delay(1000);                              // Wait for 1 second
}
