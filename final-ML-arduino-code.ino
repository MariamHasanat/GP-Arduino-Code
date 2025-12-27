#include <Wire.h>
#include <math.h>
#include <ESP32Tone.h>

// ===================== CONFIG =====================
#define SENSOR1_X_PIN 33
#define SENSOR1_Y_PIN 34
#define SENSOR1_Z_PIN 35

#define NUM_SAMPLES 100
#define NUM_OF_ITERATIONS 25

#define ANALYSIS_INTERVAL 200   // ms (time window)
#define ALERT_THRESHOLD 0.3
// ==================================================

// ===================== ML STRUCTS =====================
struct DataPoint {
  double x;
  double y;
  double z;
  int label;
};

struct Parameters {
  double w0 = -2.25237;
  double w1 = 0.334201;
  double w2 = 0.0475398;
  double w3 = 0.319727;
};

Parameters theta;
// ==================================================

// ===================== GLOBAL VARS =====================
float sensor1Buffer[3][NUM_SAMPLES];

int NUM_OF_ONES = 0;
int num_of_iteration = 0;
float AVG_ALERT = 0;

unsigned long lastAnalysisTime = 0;
// ==================================================

// ===================== ML FUNCTIONS =====================
double sigmoid(double z) {
  return 1.0 / (1.0 + exp(-z));
}

double hypothesis(const Parameters& theta, double x, double y, double z) {
  return sigmoid(theta.w0 + theta.w1 * x + theta.w2 * y + theta.w3 * z);
}

int predictLabel(const Parameters& theta, double x, double y, double z) {
  return (hypothesis(theta, x, y, z) >= 0.5) ? 1 : 0;
}
// ==================================================

// ===================== STATS =====================
float mean(float* data, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) sum += data[i];
  return sum / size;
}

float stddev(float* data, int size) {
  float avg = mean(data, size);
  float sum = 0;
  for (int i = 0; i < size; i++)
    sum += pow(data[i] - avg, 2);
  return sqrt(sum / size);
}
// ==================================================

// ===================== CORE PROCESS =====================
void processSensorData() {

  // Collect samples
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sensor1Buffer[0][i] = analogRead(SENSOR1_X_PIN);
    sensor1Buffer[1][i] = analogRead(SENSOR1_Y_PIN);
    sensor1Buffer[2][i] = analogRead(SENSOR1_Z_PIN);
    delayMicroseconds(500); // sampling stability
  }

  // Feature extraction (STD)
  float features[3];
  for (int axis = 0; axis < 3; axis++)
    features[axis] = stddev(sensor1Buffer[axis], NUM_SAMPLES);

  int predictedLabel = predictLabel(theta, features[0], features[1], features[2]);

  if (predictedLabel == 1)
    NUM_OF_ONES++;

  Serial.print("STD: ");
  Serial.print(features[0]); Serial.print(" ");
  Serial.print(features[1]); Serial.print(" ");
  Serial.print(features[2]);
  Serial.print(" | Prediction: ");
  Serial.println(predictedLabel);
}
// ==================================================

void setup() {
  Serial.begin(9600);
}

void loop() {

  // Sensor is ALWAYS running, analysis is timed
  if (millis() - lastAnalysisTime >= ANALYSIS_INTERVAL) {
    lastAnalysisTime = millis();

    processSensorData();
    num_of_iteration++;

    if (num_of_iteration >= NUM_OF_ITERATIONS) {

      AVG_ALERT = (float)NUM_OF_ONES / NUM_OF_ITERATIONS;

      Serial.print("AVG_ALERT = ");
      Serial.println(AVG_ALERT);

      if (AVG_ALERT > ALERT_THRESHOLD) {
        Serial.println("🚨 ALERT ALERT ALERT 🚨");
        // here: buzzer / server / wifi
      } else {
        Serial.println("✅ Everything is good");
      }

      // reset window
      NUM_OF_ONES = 0;
      num_of_iteration = 0;
    }
  }
}
