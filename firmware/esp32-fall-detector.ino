#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; Serial.println("BLE: Baglandi!"); };
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; pServer->getAdvertising()->start(); }
};

const int FS = 200;             
const int WINDOW_SIZE = 300;    
const int STEP_SIZE = 100;      

float ax_buf[WINDOW_SIZE], ay_buf[WINDOW_SIZE], az_buf[WINDOW_SIZE];
float gx_buf[WINDOW_SIZE], gy_buf[WINDOW_SIZE], gz_buf[WINDOW_SIZE];
volatile int sample_count = 0;
volatile bool buffer_ready = false; 
float features[28]; 

Adafruit_MPU6050 mpu;

const float b0 = 0.04613188, b1 = 0.09226376, b2 = 0.04613188;
const float a1 = -1.30728503, a2 = 0.49181255;

class BiquadFilter {
  private:
    float x1, x2, y1, y2;
  public:
    BiquadFilter() { x1 = 0; x2 = 0; y1 = 0; y2 = 0; }
    float process(float x) {
      float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
      x2 = x1; x1 = x; y2 = y1; y1 = y;
      return y;
    }
};

BiquadFilter filter_ax, filter_ay, filter_az, filter_gx, filter_gy, filter_gz;

float calculateMean(float* arr) {
  float sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) sum += arr[i];
  return sum / WINDOW_SIZE;
}
float calculateStd(float* arr, float mean) {
  float sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) sum += pow(arr[i] - mean, 2);
  return sqrt(sum / WINDOW_SIZE);
}
float calculateMax(float* arr) {
  float max_val = arr[0];
  for (int i = 1; i < WINDOW_SIZE; i++) { if (arr[i] > max_val) max_val = arr[i]; }
  return max_val;
}

void extractFeatures() {
  float acc_mag[WINDOW_SIZE], gyro_mag[WINDOW_SIZE];
  float mean_ax = calculateMean(ax_buf), mean_ay = calculateMean(ay_buf), mean_az = calculateMean(az_buf);
  float dynamic_sma_sum = 0;

  for (int i = 0; i < WINDOW_SIZE; i++) {
    acc_mag[i] = sqrt(pow(ax_buf[i], 2) + pow(ay_buf[i], 2) + pow(az_buf[i], 2));
    gyro_mag[i] = sqrt(pow(gx_buf[i], 2) + pow(gy_buf[i], 2) + pow(gz_buf[i], 2));
    dynamic_sma_sum += abs(ax_buf[i] - mean_ax) + abs(ay_buf[i] - mean_ay) + abs(az_buf[i] - mean_az);
  }

  features[0] = calculateMax(acc_mag);
  features[1] = calculateMean(acc_mag);
  features[2] = calculateStd(acc_mag, features[1]);
  features[3] = calculateMax(gyro_mag);
  features[4] = calculateMean(gyro_mag);
  features[5] = calculateStd(gyro_mag, features[4]);
  
  features[6] = mean_ax; features[7] = calculateMax(ax_buf); features[8] = calculateStd(ax_buf, mean_ax);
  features[9] = mean_ay; features[10] = calculateMax(ay_buf); features[11] = calculateStd(ay_buf, mean_ay);
  features[12] = mean_az; features[13] = calculateMax(az_buf); features[14] = calculateStd(az_buf, mean_az);
  
  float mean_gx = calculateMean(gx_buf); features[15] = mean_gx; features[16] = calculateMax(gx_buf); features[17] = calculateStd(gx_buf, mean_gx);
  float mean_gy = calculateMean(gy_buf); features[18] = mean_gy; features[19] = calculateMax(gy_buf); features[20] = calculateStd(gy_buf, mean_gy);
  float mean_gz = calculateMean(gz_buf); features[21] = mean_gz; features[22] = calculateMax(gz_buf); features[23] = calculateStd(gz_buf, mean_gz);

  float tilt_ratio = mean_ax / (features[1] + 0.000001);
  features[24] = acos(constrain(tilt_ratio, -1.0, 1.0)) * (180.0 / PI);
  features[25] = dynamic_sma_sum / WINDOW_SIZE;

  float jerk_max = 0, jerk_sum = 0;
  for (int i = 1; i < WINDOW_SIZE; i++) {
    float jerk = abs(acc_mag[i] - acc_mag[i-1]);
    if (jerk > jerk_max) jerk_max = jerk;
    jerk_sum += jerk;
  }
  features[26] = jerk_max;
  features[27] = jerk_sum / (WINDOW_SIZE - 1);
}

TaskHandle_t SensorTask;
hw_timer_t * timer = NULL;
void IRAM_ATTR onTimer() { vTaskNotifyGiveFromISR(SensorTask, NULL); }

void readSensorTask(void * parameter) {
  for(;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (sample_count >= WINDOW_SIZE) {
      for (int i = 0; i < WINDOW_SIZE - STEP_SIZE; i++) {
        ax_buf[i] = ax_buf[i + STEP_SIZE]; ay_buf[i] = ay_buf[i + STEP_SIZE]; az_buf[i] = az_buf[i + STEP_SIZE];
        gx_buf[i] = gx_buf[i + STEP_SIZE]; gy_buf[i] = gy_buf[i + STEP_SIZE]; gz_buf[i] = gz_buf[i + STEP_SIZE];
      }
      sample_count = WINDOW_SIZE - STEP_SIZE; 
    }
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    ax_buf[sample_count] = filter_ax.process(a.acceleration.x / 9.81);
    ay_buf[sample_count] = filter_ay.process(a.acceleration.y / 9.81);
    az_buf[sample_count] = filter_az.process(a.acceleration.z / 9.81);
    gx_buf[sample_count] = filter_gx.process(g.gyro.x * 57.2958);
    gy_buf[sample_count] = filter_gy.process(g.gyro.y * 57.2958);
    gz_buf[sample_count] = filter_gz.process(g.gyro.z * 57.2958);

    sample_count++;
    if (sample_count == WINDOW_SIZE) buffer_ready = true;
  }
}

void setup() {
  Serial.begin(115200); Wire.begin();
  if (!mpu.begin()) { while (1) delay(10); }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ); 

  BLEDevice::init("FallDetector_G7"); 
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start(); pServer->getAdvertising()->start();
  
  xTaskCreatePinnedToCore(readSensorTask, "SensorTask", 4096, NULL, 1, &SensorTask, 1);
  timer = timerBegin(1000000); timerAttachInterrupt(timer, &onTimer); timerAlarm(timer, 5000, true, 0);
}

void loop() {
  if (buffer_ready) {
    extractFeatures();
    if (deviceConnected) {
      pCharacteristic->setValue((uint8_t*)features, sizeof(features));
      pCharacteristic->notify();
    }
    buffer_ready = false;
  }
  delay(10); 
}
