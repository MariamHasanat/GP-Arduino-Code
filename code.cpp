#include <Wire.h>
#include <math.h>
#include <ESP32Tone.h>

// Define the analog pins for the two sensors
#define SENSOR1_X_PIN 33
#define SENSOR1_Y_PIN 34
#define SENSOR1_Z_PIN 35
// Define the number of samples to collect
#define NUM_SAMPLES 100
#define NUM_OF_ITERATIONS 25
//////////////////////////////////// Machine Learning CODE ////////////////////////////////////////////

/////////  the defintion of of vars ////////
Parameters theta;
DataPoint point;
int NUM_OF_ONES = 0;
float AVG_ALERT = 0;
int num_of_iteration = 0;
/////// END the defintion of of vars ///////


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
int predictLabel(const Parameters& theta, double x, double y, double z) {
    double prediction = hypothesis(theta, x, y, z);
    return (prediction >= 0.5) ? 1 : 0;
}

//////////////////////////////// END Machine Learning CODE ////////////////////////////////////////////


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
  float sensor1Features[3] = {0}; // Features for sensor 1 [stdX,  stdY, stdZ]
  int predictedLabel;

  for (int axis = 0; axis < 3; axis++) {
    sensor1Features[axis] = stddev(sensor1Buffer[axis], NUM_SAMPLES); // here I fill the array of three values of STD 
  }

  predictedLabel = predictLabel(theta, sensor1Features[0], sensor1Features[1], sensor1Features[2]);//

  if(predictedLabel == 1){
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
	// pinMode(BUZZZER_PIN, OUTPUT);
  Serial.begin(9600);	
}

void loop() {
	// put your main code here, to run repeatedly:
    // Read data from sensor 1
	// each 25 samples --- mean of them  >= 0.3 --- thats mean the threshold value of error is 0.3 --- less than this value or 0 value will consider as not earthquake .
	// gain samples 
	// calculate STD 
	// calulate NUM-OF-ONES
	// calculate the threshold value to detect the earthquake
	// Process data (e.g., feature extraction)
	// send an alert to the SERVER with 50% , since each node have the 50% of probability 
	// calculate mean value for 25 sample
	// summation(NUM_OF_ONES)/25
	if(num_of_iteration == NUM_OF_ITERATIONS){
		num_of_iteration = 0;
		AVG_ALERT = (NUM_OF_ONES/NUM_OF_ITERATIONS);
		if(AVG_ALERT > 0.3){
		//send alert
		AVG_ALERT = 0;
		NUM_OF_ONES = 0;
		Serial.println("ALERT ALERT ALERT");
		}
		else{
		AVG_ALERT = 0;
		NUM_OF_ONES = 0 ;
		Serial.println("Everythin is good ");
		}
	}
	num_of_iteration++;
    sensor1Data[0] = analogRead(SENSOR1_X_PIN);
    sensor1Data[1] = analogRead(SENSOR1_Y_PIN);
    sensor1Data[2] = analogRead(SENSOR1_Z_PIN);
	processSensorData();  
}

