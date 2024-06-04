#include <Wire.h>
#include <math.h>
// Define the analog pins for the two sensors
#define SENSOR1_X_PIN 12
#define SENSOR1_Y_PIN 13
#define SENSOR1_Z_PIN 14
// Define the number of samples to collect
#define NUM_SAMPLES 100

//////////////////////////////////// Machine Learning CODE ////////////////////////////////////////////

struct DataPoint {
    double x;
    double y;
    double z;
    int label; // 1 for earthquake, 0 for non-earthquake
};

// Define a structure for holding the logistic regression parameters (weights)
struct Parameters { //,,, 
    double w0 = -2.25237; // Bias term
    double w1 = 0.334201; // Coefficient for x
    double w2 = 0.0475398; // Coefficient for y
    double w3 = 0.319727; // Coefficient for z
};

// Sigmoid function
double sigmoid(double z) {
    return 1.0 / (1.0 + exp(-z));
}

// Hypothesis function
double hypothesis(const Parameters& theta, double x, double y, double z) {
    return sigmoid(theta.w0 + (theta.w1 * x) + (theta.w2 * y) + (theta.w3 * z));
}

// Function to predict labels
int predictLabel(const Parameters& theta, double x, double y, double z, double threshold = 0.5) {
    double prediction = hypothesis(theta, x, y, z);
    return (prediction >= threshold) ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Variables to store the raw data
float sensor1Data[3]; // [X, Y, Z]
// Buffers to store samples
float sensor1Buffer[3][NUM_SAMPLES]; // [X, Y, Z]

float mean(float* data, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / size;
}

float stddev(float* data, int size) {
  float avg = mean(data, size);
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += pow(data[i] - avg, 2);
  }
  return sqrt(sum / size);
}

void processSensorData() {
  // Collect samples
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sensor1Buffer[0][i] = analogRead(SENSOR1_X_PIN);
    sensor1Buffer[1][i] = analogRead(SENSOR1_Y_PIN);
    sensor1Buffer[2][i] = analogRead(SENSOR1_Z_PIN);

     delay(0); // Adjust delay based on sampling rate
  }
// Extract features
  float sensor1Features[3] = {0}; // Features for sensor 1 [stdX, stdY, stdZ]

  for (int axis = 0; axis < 3; axis++) {
    sensor1Features[axis] = stddev(sensor1Buffer[axis], NUM_SAMPLES);
  
  }

  for (int i = 0; i < 3; i++) {
    Serial.print(sensor1Features[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Calibration code here, if needed
}

void loop() {
  // put your main code here, to run repeatedly:
  // Read data from sensor 1
  sensor1Data[0] = analogRead(SENSOR1_X_PIN);
  sensor1Data[1] = analogRead(SENSOR1_Y_PIN);
  sensor1Data[2] = analogRead(SENSOR1_Z_PIN);

  // Process data (e.g., feature extraction)
  processSensorData();

}
