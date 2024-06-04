// this code will test the model, run the code @ line 100 and make the code below as comment
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

// Define a structure for holding a single data point (x, y, z accelerations and label)
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

int main() {
    
    Parameters theta;
    char comma;
    DataPoint point;
    vector<DataPoint> data;
    cin >> point.x >> comma >> point.y >> comma >> point.z ;
    data.push_back(point);
   
    int correctPredictions = 0;
    for (const auto& point : data) {
        int predictedLabel = predictLabel(theta, point.x, point.y, point.z);
        cout << " Predicted Label: " << predictedLabel << endl;
    }
    
    return 0;
}
