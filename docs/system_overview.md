System Overview

This document summarizes the architecture of the real-time IMU-based fall detection and alert system.

Objective

The objective of this project is to develop a non-invasive wearable system that can monitor user movement, classify posture and mobility states, detect fall events, and alert caregivers within a short response time.

The system is designed for elderly monitoring and emergency response support.

System Components

The system contains three main layers:

Wearable embedded hardware
Hybrid decision and classification pipeline
Mobile application and caregiver alert interface
Wearable Embedded Hardware

The wearable unit is mounted around the waist using a belt-based enclosure. This placement was selected because the waist is close to the body's center of mass and provides useful motion information for fall detection.

The hardware includes:

ESP32 microcontroller
MPU6050 accelerometer and gyroscope sensor
3.7 V LiPo battery
TP4056 charging module
Power switch
3D-printed enclosure

The ESP32 is responsible for:

Reading IMU data
Applying filtering
Segmenting data using sliding windows
Extracting features
Sending feature vectors over BLE
IMU Data Acquisition

The MPU6050 captures:

3-axis acceleration
3-axis angular velocity

The sampling frequency is set to 200 Hz.

The system does not continuously transmit raw IMU data. Instead, it extracts compact feature vectors from the sensor stream and transmits them to the mobile application. This reduces communication overhead and makes the system more suitable for wearable use.

Feature Extraction

The system extracts spatial and temporal motion features from each IMU window.

Important features include:

Signal Magnitude Area
Body tilt angle
Acceleration magnitude
Gyroscope magnitude
Axis-wise statistical features
Maximum jerk
Mean jerk

These features are used to distinguish static posture states, dynamic movement states, and possible fall events.

Hybrid Decision Model

The decision pipeline combines two methods:

Threshold-based static posture detection
Machine learning-based dynamic and fall classification

Static states are detected using deterministic rules. These include:

Sitting
Standing
Lying

Dynamic and fall-related states are classified using a trained Multilayer Perceptron model. These include:

Walking / Moving
Fast Walking / Fast Moving
Running
Fall

Candidate fall events are then verified using physical constraints such as acceleration peak, body tilt, and jerk.

This hybrid design helps reduce false alarms while keeping the system lightweight.

Mobile Application

The mobile application receives feature packets from the ESP32 through BLE.

The application provides:

Live motion state display
Elder, caregiver, and adult user roles
Elder-caregiver pairing
Emergency fall alert workflow
Local alarm on the elder device
Push notification to the paired caregiver

The full mobile application source code is not included in this repository for privacy and security reasons.

System Flow
The MPU6050 collects acceleration and gyroscope data.
The ESP32 samples the IMU data at 200 Hz.
The ESP32 applies filtering and sliding-window feature extraction.
Feature vectors are transmitted to the mobile application using BLE.
The mobile application applies the hybrid decision pipeline.
If a fall is verified, the system triggers a local alarm and caregiver notification.
Design Motivation

The system avoids camera-based monitoring to protect user privacy. It uses a lightweight wearable form factor and low-power embedded hardware to support daily use.

The hybrid threshold and machine learning design was chosen to reduce computational cost while preserving fall detection performance.

docs/hardware_layout.md
Hardware Layout

This document describes the hardware structure of the wearable fall detection prototype.

Hardware Components
Component	Purpose
ESP32	Main microcontroller, signal processing, BLE communication
MPU6050	3-axis accelerometer and 3-axis gyroscope sensing
3.7 V LiPo Battery	Portable power supply
TP4056 Charging Module	LiPo battery charging and power management
Power Switch	Manual on/off control
3D-Printed Enclosure	Mechanical protection and wearable housing
Belt Attachment	Waist-mounted use
Electrical Connections
Component Pin	Connection
MPU6050 VCC	ESP32 3V3
MPU6050 GND	ESP32 GND
MPU6050 SDA	ESP32 GPIO21
MPU6050 SCL	ESP32 GPIO22
LiPo +	TP4056 B+
LiPo -	TP4056 B-
TP4056 OUT+	Switch → ESP32 VIN
TP4056 OUT-	ESP32 GND
Communication Interfaces
I2C

The ESP32 communicates with the MPU6050 sensor using the I2C protocol.

Used pins:

SDA: GPIO21
SCL: GPIO22
BLE

Bluetooth Low Energy is used to transmit extracted feature vectors from the ESP32 to the mobile application.

The ESP32 acts as the BLE peripheral/server.
The mobile application acts as the BLE client.

Wearable Placement

The device is designed to be attached around the waist using a belt-mounted enclosure.

This placement was selected because:

It is close to the body’s center of mass.
It captures torso-level motion more reliably than wrist placement.
It reduces false motion artifacts caused by hand and arm movement.
It is suitable for fall detection and static posture estimation.
Prototype Notes

The prototype uses a perfboard-based implementation and a 3D-printed enclosure.

Future improvements may include:

Custom PCB design
Smaller enclosure
Improved battery protection
Better mechanical robustness
Additional sensing modules such as a barometric pressure sensor
docs/results.md
Results

This document summarizes the evaluation results of the real-time IMU-based fall detection and alert system.

Evaluation Setup

The final prototype was evaluated under controlled test conditions.

The evaluation included:

Five subjects
Activities of daily living
Simulated fall trials
Real-time BLE communication
Mobile application alert triggering

The tested activity classes were:

Walking / Moving
Fast Walking / Fast Moving
Running
Sitting
Standing
Lying
Fall
Main Performance Results
Metric	Result
Fall sensitivity	91.7%
Fall precision	100.0%
Fall F1 score	95.0%
Fall alert latency	Within 1.5 seconds
Observed fall false alarms during controlled ADL tests	0
Classification Results

The final hybrid system achieved the following controlled-test performance:

Activity	Precision	Sensitivity	Specificity	F1 Score
Walking / Moving	80.2%	77.7%	95.5%	77.6%
Running	96.9%	96.6%	99.2%	96.7%
Sitting	98.6%	84.5%	99.8%	90.4%
Standing	90.8%	99.4%	98.0%	94.5%
Lying Down	100.0%	88.0%	100.0%	93.6%
Fall	100.0%	91.7%	100.0%	95.0%
Fast Walk / Moving	79.5%	88.3%	94.0%	82.7%
Latency

The fall alert latency was evaluated using controlled trials with observation-based timing.

The response time ranged approximately between:

0.6 seconds – 1.5 seconds

The alert was generated within the required 1.5-second response window in observed fall trials.

False Alarm Mitigation

The system uses a physical validation layer after the MLP model predicts a candidate fall.

This validation layer checks motion features such as:

Peak acceleration
Maximum jerk
Body tilt
Post-impact orientation

This step helps reject high-impact activities of daily living that may produce acceleration spikes but are not actual falls.

Examples of false-alarm-prone activities considered during testing include:

Rapidly sitting down
Sudden balance changes
Dropping quickly onto a sofa
High-dynamic non-fall movements
Interpretation

The system is designed primarily as an emergency fall detection and caregiver alert system, not as a general-purpose fitness tracker.

For this reason, fall detection sensitivity, false alarm behavior, and alert latency are the most important success criteria.

The dynamic movement classes, such as Walking and Fast Walking, showed lower performance than fall detection because gait intensity varies between subjects. However, these errors are less critical than misclassifying falls or generating false fall alarms.

Limitations

The current prototype has the following limitations:

Controlled testing was performed with a limited number of subjects.
Real-world elderly trials were not performed.
BLE and mobile app timestamps were not fully synchronized.
Dynamic gait classes may vary between users.
The prototype uses a perfboard implementation instead of a custom PCB.
GPS-based location sharing was not included in the current version.
Future Improvements

Possible future improvements include:

Larger and more diverse user testing
User-specific calibration
Custom PCB design
Smaller and more robust enclosure
GPS-based caregiver location sharing
Barometric pressure sensor integration
Improved real-time latency measurement using synchronized timestamps
