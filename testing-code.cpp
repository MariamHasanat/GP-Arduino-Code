// this code will train the model, run the code @ line 100 and make the code below as comment
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
struct Parameters {
    double w0; // Bias term
    double w1; // Coefficient for x
    double w2; // Coefficient for y
    double w3; // Coefficient for z
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
    // Open the data file
    ifstream dataFile("data_for_testing.txt");
    if (!dataFile.is_open()) {
        cerr << "Failed to open data file." << endl;
        return 1;
    }
    
    // Read data from the file into a vector
    vector<DataPoint> data;
    string line;
    while (getline(dataFile, line)) {
        stringstream ss(line);
        DataPoint point;
        char comma;
        if (ss >> point.x >> comma >> point.y >> comma >> point.z >> comma >> point.label) {
            data.push_back(point);
        } else {
            cerr << "Invalid data format: " << line << endl;
        }
    }
    dataFile.close();
    
    // Load the trained model parameters
    Parameters theta;
    char ch_ar;
    ifstream modelFile("Learned.txt");
    if (!modelFile.is_open()) {
        cerr << "Failed to open model file." << endl;
        return 1;
    }
    modelFile >> theta.w0 >> ch_ar >> theta.w1 >> ch_ar >> theta.w2 >> ch_ar >> theta.w3;
    modelFile.close();
    
    
    ofstream resultOfTesting("test.txt");
    // resultOfTesting.open();
    // Test the model on the data
    int correctPredictions = 0;
    for (const auto& point : data) {
        int predictedLabel = predictLabel(theta, point.x, point.y, point.z);
        if (predictedLabel == point.label) {
            correctPredictions++;
        }
        resultOfTesting << "Actual Label: " << point.label << ", Predicted Label: " << predictedLabel << endl;
    }
    
    // Compute accuracy
    double accuracy = static_cast<double>(correctPredictions) / data.size() * 100.0;
    resultOfTesting << "Accuracy: " << accuracy << "%" << endl;
    
    resultOfTesting.close();
    
    return 0;
}