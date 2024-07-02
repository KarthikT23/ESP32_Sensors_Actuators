// Define the pin numbers for the Gas Sensor
const int sensorPin = 15;

void setup() {
  Serial.begin(9600);  // Start serial communication at 9600 baud rate
}

void loop() {
  Serial.print("Analog output: ");
  Serial.println(analogRead(sensorPin));  // Read the analog value of the gas sensor and print it to the serial monitor
  delay(1000);                              // Wait for 50 milliseconds
}
