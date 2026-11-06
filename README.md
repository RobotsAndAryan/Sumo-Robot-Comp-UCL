# Sumo Robot Competition — Participant Manual

This repository contains the hardware documentation, wiring connections, and starter template for the Sumo Robot Workshop.

## Workshop Steps

1. **Select a Chassis:** Pick one of the available CAD chassis designs.
2. **Assemble:** Mount the motors, wheels, sensors, and Arduino Uno onto the chassis.
3. **Wire:** Connect all components to the Arduino via the breadboard using the pinout tables and datasheets below.
4. **Program:** Install the required libraries, open [`starter_code/starter_code.ino`](starter_code/starter_code.ino), and write your robot's control logic.

---

## Parts & Datasheets

| Component | Qty | Voltage | Datasheet / Documentation |
| :--- | :---: | :---: | :--- |
| **Arduino Uno R3** | 1 | 5V logic (7–12V VIN) | [Arduino Uno R3 Datasheet](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf) |
| **Solderless Breadboard** | 1 | — | [Breadboard Guide](https://learn.sparkfun.com/tutorials/how-to-use-a-breadboard/all) / [Datasheet (PDF)](https://www.busboard.com/pdfs/BPS-MAR-BB400-001.pdf) |
| **DRV8833 Motor Driver** | 1 | 2.7V – 10.8V (`VM`) | [TI DRV8833 Datasheet](https://www.ti.com/lit/ds/symlink/drv8833.pdf) |
| **N20 DC Gearmotors** | 2 | 6V | [N20 Gearmotor Specs](https://www.pololu.com/category/60/micro-metal-gearmotors) |
| **Pololu 2-Ch QTR Sensor** | 2 | 5V | [Pololu QTR User Guide](https://www.pololu.com/docs/0J19) |
| **HC-SR04 Ultrasonic Sensor** | 1 | 5V | [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf) |
| **MPU-6050 IMU** | 1 | 3.3V – 5V | [MPU-6050 Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf) |

---

## Breadboard & Wiring

* **Breadboard Layout:** The 5 holes in each numbered terminal row (`A–E` and `F–J`) are connected together internally, separated by the center gap. Place dual-row modules (like the DRV8833) straddling the center gap, and single-row modules (like the MPU-6050) across separate numbered rows so pins do not short together.
* **Power & Ground:** Connect Arduino `5V` and `GND` to the breadboard `+` and `-` power rails to share power across your sensors. Ensure the battery `-`, DRV8833 `GND`, and Arduino `GND` share a common ground. Power the DRV8833 `VM` pin from the battery pack, not the Arduino `5V` pin.

### DRV8833 Motor Driver
| DRV8833 Pin | Connects To |
| :--- | :--- |
| `VCC` / `VM` | Battery `+` |
| `GND` | Common `GND` |
| `EEP` / `SLP` | Pin `4` (or `5V`) |
| `AIN1`, `AIN2` | Pins `~5`, `~6` (Left Motor PWM) |
| `BIN1`, `BIN2` | Pins `~9`, `~10` (Right Motor PWM) |
| `AOUT1`, `AOUT2` | Left N20 Motor |
| `BOUT1`, `BOUT2` | Right N20 Motor |

### Sensors
| Component | Component Pin | Arduino Pin |
| :--- | :--- | :--- |
| **Front-Left QTR** | `VCC`, `GND`, `OUT1`, `OUT2` | `5V`, `GND`, `A0`, `A1` |
| **Front-Right QTR** | `VCC`, `GND`, `OUT1`, `OUT2` | `5V`, `GND`, `A2`, `A3` |
| **HC-SR04 Sonar** | `VCC`, `GND`, `Trig`, `Echo` | `5V`, `GND`, Pin `7`, Pin `8` |
| **MPU-6050 IMU** | `VCC`, `GND`, `SDA`, `SCL` | `5V`, `GND`, `A4`, `A5` |

---

## Required Libraries

Install the following libraries in the Arduino IDE (**Tools -> Manage Libraries...**):

1. **Adafruit MPU6050** — [github.com/adafruit/Adafruit_MPU6050](https://github.com/adafruit/Adafruit_MPU6050) (click *Install All* for dependencies).
2. **AlashMotorControlLite** — [github.com/Alash-electronics/AlashMotorControlLite](https://github.com/Alash-electronics/AlashMotorControlLite) (if not in Library Manager, download the ZIP from GitHub and add via **Sketch -> Include Library -> Add .ZIP Library...**).

---

## Starter Code Reference

Open [`starter_code/starter_code.ino`](starter_code/starter_code.ino) to begin programming your robot. The following helper functions are provided:

* `driveMotors(leftSpeed, rightSpeed)` — Sets wheel speeds from `-100` (full reverse) to `100` (full forward).
* `stopMotors()` — Brakes both motors.
* `readLineSensor(pin)` — Returns an analog reflectance value (`0` to `1023`).
* `readDistanceCm()` — Returns the ultrasonic distance in cm (or `999` if out of range).
