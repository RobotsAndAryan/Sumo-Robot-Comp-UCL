/*
 * ============================================================================
 * SUMO ROBOT COMPETITION - SERVO HELPER CODE (ARDUINO UNO R3)
 * ============================================================================
 * Hardware & Pin Note:
 *   - On Arduino Uno R3, the standard <Servo.h> library uses Timer1, which
 *     disables PWM (analogWrite) on pins 9 and 10.
 *   - Since our DRV8833 Right Motor uses pins 9 and 10, this helper uses a
 *     non-blocking software pulse generator (`updateServo()`) so your servo
 *     and both DRV8833 motors work together without any timer conflicts!
 *
 * Wiring:
 *   - Servo Signal (Orange/Yellow) -> Pin 3 (or Pin 11)
 *   - Servo VCC (Red)              -> 5V / Battery + (via breadboard power rail)
 *   - Servo GND (Brown/Black)      -> Common GND
 *
 * Common Sumo Servo Uses:
 *   1. Drop-Down Wedge / Plow: Held upright before the match to fit inside the
 *      starting size box, then dropped flat as soon as the 5-second delay ends.
 *   2. Active Flipper / Lifter Arm: Sweeps upward when an opponent is right in
 *      front of the robot (`opponentDistCm <= 12`) to lift their wheels off the ground.
 * ============================================================================
 */

// ----------------------------------------------------------------------------
// 1. Pin & Angle Constants (TODO: Adjust angles for your mechanism!)
// ----------------------------------------------------------------------------

const uint8_t PIN_SERVO       = 3;    // Digital pin for Servo signal wire

const int SERVO_STOWED_ANGLE  = 90;   // Upright / retracted angle (0 to 180 deg)
const int SERVO_DEPLOY_ANGLE  = 15;   // Lowered wedge / resting angle (0 to 180 deg)
const int SERVO_FLIP_ANGLE    = 140;  // Raised flipper / attack angle (0 to 180 deg)

const unsigned long SERVO_FRAME_MS = 20; // Standard 50Hz servo refresh rate (20ms)

// ----------------------------------------------------------------------------
// 2. Servo State
// ----------------------------------------------------------------------------

int currentServoAngle = SERVO_STOWED_ANGLE;
unsigned long lastServoPulseMs = 0;

// ----------------------------------------------------------------------------
// 3. Provided Servo Helper Functions
// ----------------------------------------------------------------------------

/**
 * Sets the target servo angle (0 to 180 degrees).
 */
void setServoAngle(int angle) {
  currentServoAngle = constrain(angle, 0, 180);
}

/**
 * Sends a standard 50Hz servo pulse (500us to 2400us) every 20ms without using
 * Timer1, keeping PWM on pins 9 and 10 completely free for the DRV8833 motor driver.
 * Call `updateServo()` once inside `loop()`.
 */
void updateServo() {
  unsigned long now = millis();
  if (now - lastServoPulseMs < SERVO_FRAME_MS) return;
  lastServoPulseMs = now;

  // Map 0-180 degrees to 500-2400 microseconds pulse width
  int pulseWidthUs = map(currentServoAngle, 0, 180, 500, 2400);

  digitalWrite(PIN_SERVO, HIGH);
  delayMicroseconds(pulseWidthUs);
  digitalWrite(PIN_SERVO, LOW);
}

/**
 * Holds a servo angle for `durationMs` milliseconds (useful during `setup()`).
 */
void holdServoAngle(int angle, unsigned long durationMs) {
  setServoAngle(angle);
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    updateServo();
    delay(5);
  }
}

/**
 * Moves the servo smoothly from `startAngle` to `endAngle`.
 */
void sweepServo(int startAngle, int endAngle, int stepDelayMs) {
  if (startAngle < endAngle) {
    for (int angle = startAngle; angle <= endAngle; angle += 3) {
      holdServoAngle(angle, stepDelayMs);
    }
  } else {
    for (int angle = startAngle; angle >= endAngle; angle -= 3) {
      holdServoAngle(angle, stepDelayMs);
    }
  }
}

// ----------------------------------------------------------------------------
// 4. Student Servo Actions (TODO: Customize for your robot!)
// ----------------------------------------------------------------------------

/**
 * Deploys the wedge/plow down to the arena floor at the start of the match.
 */
void deployWedge() {
  setServoAngle(SERVO_DEPLOY_ANGLE);
}

/**
 * Triggers the flipper/lifter arm to destabilize a close opponent, then resets.
 */
void triggerFlipper() {
  // TODO: Customize angles or timing for your flipper mechanism!
  holdServoAngle(SERVO_FLIP_ANGLE, 350);   // Flip upward!
  holdServoAngle(SERVO_DEPLOY_ANGLE, 250); // Return wedge to floor
}

// ----------------------------------------------------------------------------
// 5. Setup & Test Loop
// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SERVO, OUTPUT);
  digitalWrite(PIN_SERVO, LOW);

  // 1. Hold servo in stowed position during the 5-second sumo start delay
  Serial.println(F("Stowing servo for 5-second match start delay..."));
  holdServoAngle(SERVO_STOWED_ANGLE, 5000);

  // 2. Drop the wedge as soon as the match begins
  Serial.println(F("Match started! Deploying wedge..."));
  deployWedge();
}

void loop() {
  // Keep refreshing the servo position every 20ms
  updateServo();

  // TODO: Combine with your ultrasonic sensor in `helper_code.ino`!
  // Example:
  //   if (opponentDistCm <= 10) {
  //     triggerFlipper();
  //   }
}
