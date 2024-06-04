// to test the code make the code above as comment and run the code below

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

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

// Cost function
double costFunction(const vector<DataPoint>& data, const Parameters& theta) {
    double cost = 0;
    for (const auto& point : data) {
        double h = hypothesis(theta, point.x, point.y, point.z);
        cost += point.label * log(h) + (1 - point.label) * log(1 - h);
    }
    return -cost / data.size(); // Average cross-entropy loss
}

// Gradient descent function to update parameters
void gradientDescent(vector<DataPoint>& data, Parameters& theta, double alpha, int numIterations) {
    int m = data.size(); // Number of data points

    for (int iter = 0; iter < numIterations; ++iter) {
        double dw0 = 0, dw1 = 0, dw2 = 0, dw3 = 0;
        for (const auto& point : data) {
            double h = hypothesis(theta, point.x, point.y, point.z);
            dw0 += (h - point.label);
            dw1 += (h - point.label) * point.x;
            dw2 += (h - point.label) * point.y;
            dw3 += (h - point.label) * point.z;
        }
        dw0 /= m;
        dw1 /= m;
        dw2 /= m;
        dw3 /= m;

        theta.w0 -= alpha * dw0;
        theta.w1 -= alpha * dw1;
        theta.w2 -= alpha * dw2;
        theta.w3 -= alpha * dw3;
    }
}

int main() {
    
    // Open the data file
    ifstream dataFile("earthquake_data.txt");
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
    
    // Initial parameters (weights)
    double theta0, theta1, theta2, theta3;
    char not_important;
    
    ifstream learningParams1("Learned.txt");
    learningParams1 >> theta0 >> not_important  >> theta1  >> not_important >> theta2  >> not_important >> theta3;
    Parameters theta = {theta0, theta1, theta2, theta3};
    cout << theta0 << " " << theta1 << " " << theta2 << " " << theta3;
    learningParams1.close();
    
    // Hyperparameters
    double learningRate = 0.01;
    int numIterations = 1000;

    // Training
    gradientDescent(data, theta, learningRate, numIterations);

    // Output the learned parameters

    ofstream learningParams("Learned.txt");
    if (!learningParams.is_open()) {
        cerr << "Failed to open output file." << endl;
        return 1;
    }
    
    learningParams << theta.w0 << ", " << theta.w1 << ", " << theta.w2 << ", " << theta.w3 << endl;
    cout << endl << theta0 << " " << theta1 << " " << theta2 << " " << theta3;
    
    learningParams.close();
    
    return 0;
}