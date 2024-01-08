#include "TimeSeriesPlot.h"


TimeSeriesPlot::TimeSeriesPlot(int num_channels)
{
	this->num_channels = num_channels;
	setup();
}

void TimeSeriesPlot::setup()
{
	for (int i = 0; i < num_channels; i++) {
		ofColor col = ofColor(ofRandom(128) + 128, ofRandom(128) + 128, i % 2 * (255), 200);
		colors.push_back(col);
		vector<float> data_array;
		data.push_back(data_array);
	}
}

void TimeSeriesPlot::update(vector<float> incoming_data)
{
	for (int i = 0; i < data.size(); i++) {
		data[i].push_back(incoming_data[i]);

		// erase old data
		if (data[i].size() > resolution) {
			data[i].erase(data[i].begin());
		}
	}
}

void TimeSeriesPlot::draw()
{
	ofPushStyle();

	// Draw Labels
	ofDrawBitmapStringHighlight(name, 0, -15);
	ofFill();
	ofSetColor(255, 40);
	ofDrawRectangle(0, 0, width, height);
	ofDrawLine(0, height / 2, width, height / 2);
	float offset_y = 7;
	ofDrawBitmapStringHighlight(ofToString(max), -20, 0 + offset_y);
	ofDrawBitmapStringHighlight(ofToString((max + min) / 2), -23, height / 2 + offset_y);
	ofDrawBitmapStringHighlight(ofToString(min), -23, height + offset_y);


	// Draw Data
	ofNoFill();
	float step = width / resolution;
	for (int i = 0; i < data.size(); i++) {
		if (i % 2 == 0) {
			ofSetLineWidth(7);
			ofSetColor(colors[i], 120);
		}
		else {
			ofSetLineWidth(2);
			ofSetColor(colors[i]);
		}
		ofBeginShape();
		for (int j = 0; j < data[i].size(); j++) {
			float x = j * step;
			float y = ofMap(data[i][j], min, max, 0, height);
			ofVertex(x, y);
		}
		ofEndShape();
	}

	ofPopStyle();
}

void TimeSeriesPlot::reset()
{
	colors.clear();
	data.clear();
	setup();
}
