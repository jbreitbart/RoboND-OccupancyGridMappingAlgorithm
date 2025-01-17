#include "src/matplotlibcpp.h" //Graph Library
#include <iostream>
#include <math.h>
#include <vector>

using namespace std;
namespace plt = matplotlibcpp;

// Defining Map Characteristics
double Zmax = 5000, Zmin = 170;
double l0 = 0, locc = 0.4, lfree = -0.4;
double gridWidth = 100, gridHeight = 100;
double mapWidth = 30000, mapHeight = 15000;
double robotXOffset = mapWidth / 5, robotYOffset = mapHeight / 3;
vector<vector<double>> l(mapWidth / gridWidth, vector<double>(mapHeight / gridHeight));

double inverseSensorModel(double x, double y, double theta, double xi, double yi, double sensorData[]) {
	// Defining Sensor Characteristics
	double Zk, thetaK, sensorTheta;
	double minDelta = -1;
	double alpha = 200, beta = 20;

	//******************Compute r and phi**********************//
	double r = sqrt(pow(xi - x, 2) + pow(yi - y, 2));
	double phi = atan2(yi - y, xi - x) - theta;

	// Scaling Measurement to [-90 -37.5 -22.5 -7.5 7.5 22.5 37.5 90]
	for (int i = 0; i < 8; i++) {
		if (i == 0) {
			sensorTheta = -90 * (M_PI / 180);
		} else if (i == 1) {
			sensorTheta = -37.5 * (M_PI / 180);
		} else if (i == 6) {
			sensorTheta = 37.5 * (M_PI / 180);
		} else if (i == 7) {
			sensorTheta = 90 * (M_PI / 180);
		} else {
			sensorTheta = (-37.5 + (i - 1) * 15) * (M_PI / 180);
		}

		if (fabs(phi - sensorTheta) < minDelta || minDelta == -1) {
			Zk = sensorData[i];
			thetaK = sensorTheta;
			minDelta = fabs(phi - sensorTheta);
		}
	}

	//******************Evaluate the three cases**********************//
	if (r > min((double)Zmax, Zk + alpha / 2) || fabs(phi - thetaK) > beta / 2 || Zk > Zmax || Zk < Zmin) {
		return l0;
	} else if (Zk < Zmax && fabs(r - Zk) < alpha / 2) {
		return locc;
	} else if (r <= Zk) {
		return lfree;
	}
}

void occupancyGridMapping(double Robotx, double Roboty, double Robottheta, double sensorData[]) {
	//******************Code the Occupancy Grid Mapping Algorithm**********************//
	for (int x = 0; x < mapWidth / gridWidth; x++) {
		for (int y = 0; y < mapHeight / gridHeight; y++) {
			double xi = x * gridWidth + gridWidth / 2 - robotXOffset;
			double yi = -(y * gridHeight + gridHeight / 2) + robotYOffset;
			if (sqrt(pow(xi - Robotx, 2) + pow(yi - Roboty, 2)) <= Zmax) {
				l[x][y] = l[x][y] + inverseSensorModel(Robotx, Roboty, Robottheta, xi, yi, sensorData) - l0;
			}
		}
	}
}

void visualization() {
	// TODO: Initialize a plot named Map of size 300x150
	plt::title("Map");
	plt::xlim(0, (int)(mapWidth / gridWidth));
	plt::ylim(0, (int)(mapHeight / gridHeight));

	// TODO: Loop over the log odds values of the cells and plot each cell state.
	// Unkown state: green color, occupied state: black color, and free state: red color
	for (int x = 0; x < mapWidth / gridWidth; x++) {
		for (int y = 0; y < mapHeight / gridHeight; y++) {
			std::string format = "";
			if (l[x][y] == 0) { // Green unkown state
				format = "g.";
			} else if (l[x][y] > 0) { // Black occupied state
				format = "k.";
			} else { // Red free state
				format = "k.";
			}
			plt::plot({static_cast<double>(x)}, {static_cast<double>(y)}, format);
		}
	}

	// TODO: Save the image and close the plot
	plt::save("./Images/Map.png");
	plt::clf();
}

int main() {
	double timeStamp;
	double measurementData[8];
	double robotX, robotY, robotTheta;

	FILE *posesFile = fopen("Data/poses.txt", "r");
	FILE *measurementFile = fopen("Data/measurement.txt", "r");

	// Scanning the files and retrieving measurement and poses at each timestamp
	while (fscanf(posesFile, "%lf %lf %lf %lf", &timeStamp, &robotX, &robotY, &robotTheta) != EOF) {
		fscanf(measurementFile, "%lf", &timeStamp);
		for (int i = 0; i < 8; i++) {
			fscanf(measurementFile, "%lf", &measurementData[i]);
		}
		occupancyGridMapping(robotX, robotY, (robotTheta / 10) * (M_PI / 180), measurementData);
	}

	// Visualize the map at the final step
	cout << "Wait for the image to generate" << endl;
	visualization();
	cout << "Done!" << endl;

	return 0;
}
