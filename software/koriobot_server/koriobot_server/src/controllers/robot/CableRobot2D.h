#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxGizmo.h"
#include "ofxXmlSettings.h"
#include "CableRobot.h"

#include "../TimeSeriesPlot.h"


class CableRobot2D :
	public ofThread
{
private:

	int id;
	vector<CableRobot*> robots;

	void update_trajectories_2D();

	void draw_cables_actual(glm::vec3 start_0, glm::vec3 end_0, float dist_0, glm::vec3 start_1, glm::vec3 end_1, float dist_1);
	void draw_cables_2D();

	vector<vector<glm::vec3>> ee_path;

	ofNode* origin;     // World reference frame 
	ofNode* ee;			// World reference 
	ofxGizmo gizmo_ee;
	
	glm::vec3 base_top_left;
	glm::vec3 base_top_right;
	ofRectangle bounds;
	ofRectangle safety_bounds;

	void setup_gui();
	string state_names[5] = { "NOT_HOMED", "HOMING", "ENABLED", "DISABLED", "E_STOP" };

	ofxXmlSettings config;
   
public:

	CableRobot2D() {};	
	CableRobot2D(ofNode* _origin, CableRobot* top_left, CableRobot* top_right, int id, ofxXmlSettings config);

	void update();
	void draw();
	void reset();
	void shutdown();
	void update_gui(ofxPanel* _panel);
	void draw_gui();

	void threadedFunction();

	void set_safety_bounds(glm::vec3 bounds, int width, int height);

	void get_status();
	bool debugging = true;

	ofxGizmo* get_gizmo() { return &gizmo_ee; }
	bool override_gizmo = false;
	void update_gizmo();

	void key_pressed(int key);


	void move_position(float target_pos, bool absolute = true);
	void move_velocity(float target_pos);

	bool is_estopped();
	bool is_homed();
	bool is_enabled();
	bool run_homing_routine(int timeout = 20);

	void stop();
	void set_e_stop(bool val);
	void set_enabled(bool val);


	TimeSeriesPlot plot = TimeSeriesPlot(4);
	vector<float> plot_data = { 0, 0, 0, 0 };

	ofPolyline path;
	void add_to_path(glm::vec3 pos);
	void draw_path();

	glm::vec3 estimated_target_actual;
	glm::vec3 get_target_actual();

	// GUI Listeners
	void on_enable(bool& val);
	void on_e_stop(bool& val);
	void on_run_homing();
	void on_run_shutdown();
	void on_bounds_changed(float& val);
	void on_torque_limits_changed(float& val);
	void on_vel_limit_changed(float& val);
	void on_accel_limit_changed(float& val);

	void on_ee_offset_changed(float& val);

	void on_move_to_changed(glm::vec2& val);
	void on_move_to_pos();
	void on_move_to_vel(bool& val);
	void on_zone_changed(float& val);

	void on_gains_changed(float& val);

	ofxPanel panel;
	ofParameterGroup params_control;
	ofParameter<string> status;
	ofParameter<bool> enable;
	ofParameter<bool> e_stop;
	ofParameter<void> btn_run_homing;
	ofParameter<void> btn_run_shutdown;

	ofParameterGroup params_limits;
	ofParameter<float> limit_velocity;
	ofParameter<float> limit_acceleration;
	ofParameter<float> bounds_min;
	ofParameter<float> bounds_max;
	ofParameter<float> limit_torque_min;
	ofParameter<float> limit_torque_max;

	ofParameterGroup params_kinematics;
	ofParameter<float> ee_offset = 1;

	ofParameterGroup params_motion;
	ofParameter<float> zone = 50;
	ofParameter<float> kp = 1;
	ofParameter<float> kd = 30;
	ofParameter<float> steering_scalar = 1.75;
	

	ofParameterGroup params_move;
	ofParameter<glm::vec2> move_to;
	ofParameter<void> move_to_pos;
	ofParameter<bool> move_to_vel;

	ofColor mode_color_enabled;
	ofColor mode_color_disabled;
	ofColor mode_color_not_homed;
	ofColor mode_color_estopped;
};