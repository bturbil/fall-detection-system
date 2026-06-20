# Hardware Layout

This document describes the hardware structure of the wearable fall detection prototype.

---

## Hardware Components

| Component              | Purpose                                                    |
| ---------------------- | ---------------------------------------------------------- |
| ESP32                  | Main microcontroller, signal processing, BLE communication |
| MPU6050                | 3-axis accelerometer and 3-axis gyroscope sensing          |
| 3.7 V LiPo Battery     | Portable power supply                                      |
| TP4056 Charging Module | LiPo battery charging and power management                 |
| Power Switch           | Manual on/off control                                      |
| 3D-Printed Enclosure   | Mechanical protection and wearable housing                 |
| Belt Attachment        | Waist-mounted use                                          |

---

## Electrical Connections

| Component Pin | Connection         |
| ------------- | ------------------ |
| MPU6050 VCC   | ESP32 3V3          |
| MPU6050 GND   | ESP32 GND          |
| MPU6050 SDA   | ESP32 GPIO21       |
| MPU6050 SCL   | ESP32 GPIO22       |
| LiPo +        | TP4056 B+          |
| LiPo -        | TP4056 B-          |
| TP4056 OUT+   | Switch → ESP32 VIN |
| TP4056 OUT-   | ESP32 GND          |

---

## Communication Interfaces

### I2C

The ESP32 communicates with the MPU6050 sensor using the I2C protocol.

Used pins:

* SDA: GPIO21
* SCL: GPIO22

### BLE

Bluetooth Low Energy is used to transmit extracted feature vectors from the ESP32 to the mobile application.

The ESP32 acts as the BLE peripheral/server.
The mobile application acts as the BLE client.

---

## Wearable Placement

The device is designed to be attached around the waist using a belt-mounted enclosure.

This placement was selected because:

* It is close to the body’s center of mass.
* It captures torso-level motion more reliably than wrist placement.
* It reduces false motion artifacts caused by hand and arm movement.
* It is suitable for fall detection and static posture estimation.

---

## Prototype Notes

The prototype uses a perfboard-based implementation and a 3D-printed enclosure.

Future improvements may include:

* Custom PCB design
* Smaller enclosure
* Improved battery protection
* Better mechanical robustness
* Additional sensing modules such as a barometric pressure sensor

* GPS-based caregiver location sharing
* Barometric pressure sensor integration
* Improved real-time latency measurement using synchronized timestamps
