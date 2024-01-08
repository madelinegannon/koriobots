#include "CableDrum.h"

void CableDrum::initialize(Groove direction, float diameter_drum, float length, int turns)
{
	this->direction = direction;
	this->diameter_drum = diameter_drum;
	this->length = length;
	this->turns = turns;
	this->circumference = PI * diameter_drum;

	this->tangent_pt.x = diameter_drum / 2 * (direction == RIGHT_HANDED ? -1 : 1);
}

void CableDrum::draw()
{
	ofPushStyle();

	ofNoFill();
	ofSetColor(100);

	// draw drum
	ofSetLineWidth(2);
	ofSetColor(200);
	ofDrawEllipse(0, 0, diameter_drum, diameter_drum);
	
	// draw centroid and tangent
	ofFill();
	ofDrawLine(0, 0, tangent_pt.x, tangent_pt.y);
	ofSetColor(ofColor::red);
	ofDrawEllipse(0, 0, diameter_drum / 4.0, diameter_drum / 4.0);
	ofSetColor(ofColor::orange);
	ofDrawEllipse(tangent_pt, 10, 10);
	ofPopStyle();
}
