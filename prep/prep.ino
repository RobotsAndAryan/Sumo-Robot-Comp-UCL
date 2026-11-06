#include "AlashMotorControlLite.h"

// PWM_PWM mode: IN1=PB4, IN2=PB5
AlashMotorControlLite motor(PWM_PWM, A5, A4);

void setup() {
  Serial.begin(115200);
  pinMode(A5,OUTPUT);
}

void loop() {
  digitalWrite(A5,1);
  motor.setSpeed(100);   // Full speed forward
  delay(2000);

  motor.brake();         // Active braking
  delay(500);

  motor.setSpeed(-100);  // Full speed reverse
  delay(2000);

  motor.stop();          // Coast stop
  delay(1000);
}