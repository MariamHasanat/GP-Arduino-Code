#include <Wire.h>
#include <math.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
// #include <FirebaseArduino.h>
// #include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <SD.h>

////////////////////////////////////////////////// Firestore code ??????????????

#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

//////////////////////////////////////////////
#define SENSOR1_X_PIN 33
#define SENSOR1_Y_PIN 34
#define SENSOR1_Z_PIN 35
#define NUM_SAMPLES 100
#define NUM_OF_ITERATIONS 25
#define WIFI_SSID "Ross"
#define WIFI_PASSWORD "dm3j9f@35"
#define API_KEY "AIzaSyBgpUQqMd8SQj7ZsB33GVh8WJcD0jlD0pk"
#define DATABASE_URL "https://app2-d200f-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_PROJECT_ID "app2-d200f"                                //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define USER_EMAIL "earhtquakegraduationproject@gmail.com"
#define USER_PASSWORD "earthquake123@456"

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
unsigned long dataMillis = 0;
int count = 0;
int FinalResult = 0;
int old_FinalResult = 0;
int NUM_OF_ONES = 0;
int num_of_iteration = 0;
int predictedLabel;
Parameters theta;
DataPoint point;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;
FirebaseData fbdo_othernode;
// FirebaseJson json;
// FirebaseFirestore firestore;
float AVG_ALERT = 0;
float sensor1Data[3];                 // [X, Y, Z]
float sensor1Buffer[3][NUM_SAMPLES];  // [X, Y, Z]
float sensor1Features[3] = { 0 };     // Features for sensor 1 [stdX,  stdY, stdZ]

//////////////////////////////BUZZER ///////////////////////////////////////////
int other_node = 0;
const int buzzerPin = 32;  // You can change this to the pin you have connected the buzzer to

// Define the frequencies for alert and warning sounds
const int alertFrequency = 1000;   // Frequency in Hz for alert sound
const int warningFrequency = 500;  // Frequency in Hz for warning sound

// Define the duration for the sounds
const int alertDuration = 1000;   // Duration in milliseconds for alert sound
const int warningDuration = 500;  // Duration in milliseconds for warning sound

void playSound(int frequency, int duration) {
  int halfPeriod = 1000000 / (frequency * 2);    // Calculate the half period in microseconds
  int cycles = frequency * (duration / 1000.0);  // Calculate the number of cycles

  for (int i = 0; i < cycles; i++) {
    digitalWrite(buzzerPin, HIGH);  // Turn the buzzer on
    delayMicroseconds(halfPeriod);  // Wait for the half period
    digitalWrite(buzzerPin, LOW);   // Turn the buzzer off
    delayMicroseconds(halfPeriod);  // Wait for the other half period
  }
}

//////////////////////////////////////////////////////////////////////////////////

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
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
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
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;                  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
}

void loop() {

  digitalWrite(buzzerPin, LOW);

  num_of_iteration++;
  sensor1Data[0] = analogRead(SENSOR1_X_PIN);
  sensor1Data[1] = analogRead(SENSOR1_Y_PIN);
  sensor1Data[2] = analogRead(SENSOR1_Z_PIN);
  processSensorData();

  if (num_of_iteration == NUM_OF_ITERATIONS) {
    num_of_iteration = 0;

    AVG_ALERT = (NUM_OF_ONES / NUM_OF_ITERATIONS);
    if (AVG_ALERT > 0.5) {
      FinalResult = 1;
      playSound(alertFrequency, alertDuration);
      if (FinalResult != old_FinalResult) {
        Firebase.RTDB.setInt(&fbdo, F("/FinalResult1/int"), FinalResult);

        delay(500);
        old_FinalResult = FinalResult;
      }
      AVG_ALERT = 0;
      NUM_OF_ONES = 0;
      Serial.println("ALERT ALERT ALERT");
    } else {
      FinalResult = 0;
      if (FinalResult != old_FinalResult) {
        Firebase.RTDB.setInt(&fbdo, F("/FinalResult1/int"), FinalResult);
        old_FinalResult = FinalResult;
      }
      AVG_ALERT = 0;
      NUM_OF_ONES = 0;
      Serial.println("Everythin is good ");

      // Firebase.RTDB.getInt(fbdo_othernode, F("/FinalResult/int"));
      // other_node = Firebase.RTDB.getInt(fbdo_othernode, F("/FinalResult/int"));

      // Serial.printf(Firebase.RTDB.getInt(fbdo_othernode, F("/FinalResult/int")));
      if (Firebase.RTDB.getInt(&fbdo_othernode, F("/FinalResult/int")))
      other_node = fbdo_othernode.intData();
        if ((other_node == 1)) {
          playSound(warningFrequency, warningDuration);
        }
    }

    //// write document code ////
    if (Firebase.ready() && (millis() - dataMillis > 10000 || dataMillis == 0)) {
      dataMillis = millis();
      FirebaseJson content;
      String documentPath = "node_1/sensor_" + String(count);
      // double
      content.set("fields/xAxis/doubleValue", sensor1Features[0]);
      content.set("fields/yAxis/doubleValue", sensor1Features[1]);
      content.set("fields/zAxis/doubleValue", sensor1Features[2]);
      count++;

      Serial.print("Create a document... ");

      if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "(default)", documentPath.c_str(), content.raw()))
        Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      else
        Serial.println(fbdo.errorReason());
    }
  }
}