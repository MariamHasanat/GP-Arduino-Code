
#include <WiFi.h>
#include <math.h>
#include <Arduino.h>
#include <FirebaseESP32.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP32Tone.h>

#define SENSOR1_X_PIN 33
#define SENSOR1_Y_PIN 34
#define SENSOR1_Z_PIN 35
#define NUM_SAMPLES 100
#define NUM_OF_ITERATIONS 25
#define WIFI_SSID "Ross****"
#define WIFI_PASSWORD "dm3j9f@35*******"
#define API_KEY "AIzaSyBgpUQqMd8SQj7ZsB33GVh8WJcD0jlD0pk"
#define DATABASE_URL "https://app2-d200f-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_PROJECT_ID "app2-d200f"                                //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "earhtquakeg***********raduationproject@gmail.com"
#define USER_PASSWORD "earthqu*********ake123@456"

struct DataPoint {
  double x;
  double y;
  double z;
  int label;  // 1 for earthquake, 0 for non-earthquake
};
struct Parameters {       //,,,
  double w0 = -2.25237;   // Bias term
  double w1 = 0.334201;   // Coefficient for x
  double w2 = 0.0475398;  // Coefficient for y
  double w3 = 0.319727;   // Coefficient for z
};

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
int FinalResult = 0;
int old_FinalResult = 0;
int NUM_OF_ONES = 0;
int num_of_iteration = 0;
Parameters theta;
DataPoint point;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
float AVG_ALERT = 0;
float sensor1Data[3];                 // [X, Y, Z]
float sensor1Buffer[3][NUM_SAMPLES];  // [X, Y, Z]

// Sigmoid function
double sigmoid(double z) {
  return 1.0 / (1.0 + exp(-z));
}
// Hypothesis function
double hypothesis(const Parameters &theta, double x, double y, double z) {
  return sigmoid(theta.w0 + (theta.w1 * x) + (theta.w2 * y) + (theta.w3 * z));
}
// Function to predict labels
int predictLabel(const Parameters &theta, double x, double y, double z) {
  double prediction = hypothesis(theta, x, y, z);
  return (prediction >= 0.5) ? 1 : 0;
}
float mean(float *data, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / size;
}
float stddev(float *data, int size) {
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
    delay(0);  // Adjust delay based on sampling rate
  }
  // Extract features
  float sensor1Features[3] = { 0 };  // Features for sensor 1 [stdX,  stdY, stdZ]
  int predictedLabel;

  for (int axis = 0; axis < 3; axis++) {
    sensor1Features[axis] = stddev(sensor1Buffer[axis], NUM_SAMPLES);  // here I fill the array of three values of STD
  }
  predictedLabel = predictLabel(theta, sensor1Features[0], sensor1Features[1], sensor1Features[2]);  //
  if (predictedLabel == 1) {
    NUM_OF_ONES += 1;
  }
  for (int i = 0; i < 3; i++) {
    Serial.print(sensor1Features[i]);
    Serial.print(" ");
  }
  Serial.print(" ");
  Serial.print(predictedLabel);
  Serial.println();
}

void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);
}

void loop() {
  num_of_iteration++;
  sensor1Data[0] = analogRead(SENSOR1_X_PIN);
  sensor1Data[1] = analogRead(SENSOR1_Y_PIN);
  sensor1Data[2] = analogRead(SENSOR1_Z_PIN);
  processSensorData();
  if (num_of_iteration == NUM_OF_ITERATIONS) {
    num_of_iteration = 0;
    AVG_ALERT = (NUM_OF_ONES / NUM_OF_ITERATIONS);
    if (AVG_ALERT > 0.3) {
      FinalResult = 1;
      if (FinalResult != old_FinalResult) {
        Firebase.setInt(fbdo, F("/FinalResult1"), FinalResult);
        old_FinalResult = FinalResult;
      }
      AVG_ALERT = 0;
      NUM_OF_ONES = 0;
      Serial.println("ALERT ALERT ALERT");
    } else {
      FinalResult = 0;
      if (FinalResult != old_FinalResult) {
        Firebase.setInt(fbdo, F("/FinalResult1"), FinalResult);
        old_FinalResult = FinalResult;
      }
      AVG_ALERT = 0;
      NUM_OF_ONES = 0;
      Serial.println("Everythin is good ");
    }
  }
}