#include "PD_Controller.h"

PD_Controller::PD_Controller()
{
	params.setName("PD_Parameters");
	params.add(kp.set("Proportional_Gains", 1, 0, 500));	// gains for Propotional Component
	params.add(kd.set("Derivative_Gains", 15, 0, 100));     // gains for Derivitive Component
	params.add(steering_scalar.set("Steering_Scalar", 1.25, 0, 5));
}

PD_Controller::~PD_Controller()
{
}

void PD_Controller::update(float setpoint)
{
	if (last_time != 0) {
		float diff = (ofGetElapsedTimef() - last_time);
		//cout << diff << endl;
		time_diff = MIN(diff, 1 / 15.); // <-- set a max time step so the PD controller doesn't explode
	}
	last_time = ofGetElapsedTimef();
	this->setpoint = setpoint;
	pd_val += (kp * (setpoint - smoothed_val) + kd * (-1 * pd_val)) * time_diff;
	smoothed_val += pd_val * steering_scalar;
}

void PD_Controller::reset(float setpoint)
{
	this->setpoint = setpoint;
	this->smoothed_val = setpoint;
	this->pd_val = 0;
	last_time = 0;
	time_diff = 1 / 60.;
}
