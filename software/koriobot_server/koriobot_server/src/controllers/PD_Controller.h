#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class PD_Controller
{
public:
	PD_Controller();
	~PD_Controller();

	ofParameterGroup params;
	ofParameter<float> kp, kd, steering_scalar;
	float time_diff = 1 / 20.;// 60.;

	void update(float setpoint = 0);
	void reset(float setpoint = 0);

	float get_setpoint() { return setpoint; }
	float get_smoothed_val() { return smoothed_val; }
	float get_pd_val() { return pd_val; }

private:
	float setpoint = 0;
	float smoothed_val = 0;
	float pd_val = 0;

	float last_time = 0;

};