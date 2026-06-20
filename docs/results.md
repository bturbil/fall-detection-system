# Results

This document summarizes the evaluation results of the real-time IMU-based fall detection and alert system.

---

## Evaluation Setup

The final prototype was evaluated under controlled test conditions.

The evaluation included:

* Five subjects
* Activities of daily living
* Simulated fall trials
* Real-time BLE communication
* Mobile application alert triggering

The tested activity classes were:

* Walking / Moving
* Fast Walking / Fast Moving
* Running
* Sitting
* Standing
* Lying
* Fall

---

## Main Performance Results

| Metric                                                 | Result             |
| ------------------------------------------------------ | ------------------ |
| Fall sensitivity                                       | 91.7%              |
| Fall precision                                         | 100.0%             |
| Fall F1 score                                          | 95.0%              |
| Fall alert latency                                     | Within 1.5 seconds |
| Observed fall false alarms during controlled ADL tests | 0                  |

---

## Classification Results

The final hybrid system achieved the following controlled-test performance:

| Activity           | Precision | Sensitivity | Specificity | F1 Score |
| ------------------ | --------: | ----------: | ----------: | -------: |
| Walking / Moving   |     80.2% |       77.7% |       95.5% |    77.6% |
| Running            |     96.9% |       96.6% |       99.2% |    96.7% |
| Sitting            |     98.6% |       84.5% |       99.8% |    90.4% |
| Standing           |     90.8% |       99.4% |       98.0% |    94.5% |
| Lying Down         |    100.0% |       88.0% |      100.0% |    93.6% |
| Fall               |    100.0% |       91.7% |      100.0% |    95.0% |
| Fast Walk / Moving |     79.5% |       88.3% |       94.0% |    82.7% |

---

## Latency

The fall alert latency was evaluated using controlled trials with observation-based timing.

The response time ranged approximately between:

```text
0.6 seconds – 1.5 seconds
```

The alert was generated within the required 1.5-second response window in observed fall trials.

---

## False Alarm Mitigation

The system uses a physical validation layer after the MLP model predicts a candidate fall.

This validation layer checks motion features such as:

* Peak acceleration
* Maximum jerk
* Body tilt
* Post-impact orientation

This step helps reject high-impact activities of daily living that may produce acceleration spikes but are not actual falls.

Examples of false-alarm-prone activities considered during testing include:

* Rapidly sitting down
* Sudden balance changes
* Dropping quickly onto a sofa
* High-dynamic non-fall movements

---

## Interpretation

The system is designed primarily as an emergency fall detection and caregiver alert system, not as a general-purpose fitness tracker.

For this reason, fall detection sensitivity, false alarm behavior, and alert latency are the most important success criteria.

The dynamic movement classes, such as Walking and Fast Walking, showed lower performance than fall detection because gait intensity varies between subjects. However, these errors are less critical than misclassifying falls or generating false fall alarms.

---

## Limitations

The current prototype has the following limitations:

* Controlled testing was performed with a limited number of subjects.
* Real-world elderly trials were not performed.
* BLE and mobile app timestamps were not fully synchronized.
* Dynamic gait classes may vary between users.
* The prototype uses a perfboard implementation instead of a custom PCB.
* GPS-based location sharing was not included in the current version.

---

## Future Improvements

Possible future improvements include:

* Larger and more diverse user testing
* User-specific calibration
* Custom PCB design
* Smaller and more robust enclosure
