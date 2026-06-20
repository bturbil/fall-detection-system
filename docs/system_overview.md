# System Overview

This document summarizes the architecture of the real-time IMU-based fall detection and alert system.

---

## Objective

The objective of this project is to develop a non-invasive wearable system that can monitor user movement, classify posture and mobility states, detect fall events, and alert caregivers within a short response time.

The system is designed for elderly monitoring and emergency response support.

---

## System Components

The system contains three main layers:

1. Wearable embedded hardware
2. Hybrid decision and classification pipeline
3. Mobile application and caregiver alert interface

---

## Wearable Embedded Hardware

The wearable unit is mounted around the waist using a belt-based enclosure. This placement was selected because the waist is close to the body's center of mass and provides useful motion information for fall detection.

The hardware includes:

* ESP32 microcontroller
* MPU6050 accelerometer and gyroscope sensor
* 3.7 V LiPo battery
* TP4056 charging module
* Power switch
* 3D-printed enclosure

The ESP32 is responsible for:

* Reading IMU data
* Applying filtering
* Segmenting data using sliding windows
* Extracting features
* Sending feature vectors over BLE

---

## IMU Data Acquisition

The MPU6050 captures:

* 3-axis acceleration
* 3-axis angular velocity

The sampling frequency is set to 200 Hz.

The system does not continuously transmit raw IMU data. Instead, it extracts compact feature vectors from the sensor stream and transmits them to the mobile application. This reduces communication overhead and makes the system more suitable for wearable use.

---

## Feature Extraction

The system extracts spatial and temporal motion features from each IMU window.

Important features include:

* Signal Magnitude Area
* Body tilt angle
* Acceleration magnitude
* Gyroscope magnitude
* Axis-wise statistical features
* Maximum jerk
* Mean jerk

These features are used to distinguish static posture states, dynamic movement states, and possible fall events.

---

## Hybrid Decision Model

The decision pipeline combines two methods:

1. Threshold-based static posture detection
2. Machine learning-based dynamic and fall classification

Static states are detected using deterministic rules. These include:

* Sitting
* Standing
* Lying

Dynamic and fall-related states are classified using a trained Multilayer Perceptron model. These include:

* Walking / Moving
* Fast Walking / Fast Moving
* Running
* Fall

Candidate fall events are then verified using physical constraints such as acceleration peak, body tilt, and jerk.

This hybrid design helps reduce false alarms while keeping the system lightweight.

---

## Mobile Application

The mobile application receives feature packets from the ESP32 through BLE.

The application provides:

* Live motion state display
* Elder, caregiver, and adult user roles
* Elder-caregiver pairing
* Emergency fall alert workflow
* Local alarm on the elder device
* Push notification to the paired caregiver

The full mobile application source code is not included in this repository for privacy and security reasons.

---

## System Flow

1. The MPU6050 collects acceleration and gyroscope data.
2. The ESP32 samples the IMU data at 200 Hz.
3. The ESP32 applies filtering and sliding-window feature extraction.
4. Feature vectors are transmitted to the mobile application using BLE.
5. The mobile application applies the hybrid decision pipeline.
6. If a fall is verified, the system triggers a local alarm and caregiver notification.

---

## Design Motivation

The system avoids camera-based monitoring to protect user privacy. It uses a lightweight wearable form factor and low-power embedded hardware to support daily use.

The hybrid threshold and machine learning design was chosen to reduce computational cost while preserving fall detection performance.

