#pragma once

#include "ofMain.h"

class TimeSeriesPlot
{
public:
	TimeSeriesPlot(int num_channels);
	//~TimeSeriesPlot() {};

	string name = "";

	int num_channels = 1;

	float min = -1;
	float max = 1;
	int width = 500;
	int height = 100;
	float resolution = width * 0.25;

	vector<ofColor> colors;
	vector<vector<float>> data;

	void update(vector<float> incoming_data);
	void draw();
	void reset();

private:
	void setup();

};