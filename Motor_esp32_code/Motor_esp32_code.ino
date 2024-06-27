// Motor control pins
const int PWM_Pin = 5;     // PWM pin for motor speed control
const int dir1_Pin = 18;   // H-Bridge input 1
const int dir2_Pin = 19;   // H-Bridge input 2
const int encoderA_Pin = 2;  // Encoder channel A pin
const int encoderB_Pin = 3;  // Encoder channel B pin

// PID controller variables
float kp = 1.0, ki = 0.1, kd = 0.05;  // PID gains 
float integral = 0, derivative = 0, lastError = 0;
unsigned long lastTime = 0;

// Motor speed variables
int targetSpeed = 0;
volatile int encoderCount = 0;


void setupMotor();
void processSerialCommand();
void runPIDControl();
int parseSpeed();
void setMotorDirection(bool forward);
void setMotorSpeed(int speed);
void stopMotor();
void handleEncoderInterrupt();
int calculateEncoderSpeed();

void setup() {
  setupMotor();
}

void loop() {
  processSerialCommand();
  runPIDControl();
}

void setupMotor() {
  pinMode(PWM_Pin, OUTPUT);
  pinMode(dir1_Pin, OUTPUT);
  pinMode(dir2_Pin, OUTPUT);
  pinMode(encoderA_Pin, INPUT_PULLUP);
  pinMode(encoderB_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderA_Pin), handleEncoderInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderB_Pin), handleEncoderInterrupt, RISING);

  Serial.begin(9600);

  stopMotor();
}

void processSerialCommand() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'F':
        setMotorDirection(true);
        targetSpeed = parseSpeed();
        break;
      case 'B':
        setMotorDirection(false);
        targetSpeed = parseSpeed();
        break;
      case 'S':
        stopMotor();
        break;
      default:
        Serial.println("Invalid command");
    }
  }
}

void runPIDControl() {
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastTime) / 1000.0; 
  int currentSpeed = calculateEncoderSpeed();
  float error = targetSpeed - currentSpeed;
  
  integral += error * deltaTime;
  derivative = (error - lastError) / deltaTime;
  lastError = error;
  lastTime = currentTime;

  // PID output mapping and constraint
  int pwmValue = targetSpeed + kp * error + ki * integral + kd * derivative;
  pwmValue = constrain(pwmValue, 0, 255);

  setMotorSpeed(pwmValue);
}

int parseSpeed() {
  String speedString = Serial.readStringUntil('\n');
  return speedString.toInt();
}

void setMotorDirection(bool forward) {
  digitalWrite(dir1_Pin, forward ? HIGH : LOW);
  digitalWrite(dir2_Pin, forward ? LOW : HIGH);
}

void setMotorSpeed(int speed) {
  analogWrite(PWM_Pin, speed);
}

void stopMotor() {
  setMotorSpeed(0);
}

void handleEncoderInterrupt() {
  if (digitalRead(encoderB_Pin) == HIGH) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

int calculateEncoderSpeed() {
  int currentCount = encoderCount;
  int countsPerSecond = static_cast<int>(currentCount / (millis() - lastTime) * 1000.0);
  return countsPerSecond;
}
