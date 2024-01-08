#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "CableRobot.h"
#include "CableRobot2D.h"
#include "ofxGizmo.h"
#include "ofxXmlSettings.h"
#include "ClearPathMotorController.h"


class RobotController :
	public ofThread
{
private:
	vector<glm::vec3> bases;

	vector<CableRobot2D*> robots_2D;

	ofNode* origin;      // World reference frame 
	ofNode ee;

	bool is_initialized = false;
	void update();

	vector<vector<int>> motor_pairs;
	void initialize();


	void check_for_system_ready();
	bool is_gui_setup = false;
	void setup_gui();

	enum Configuration {
		ONE_D = 1,
		TWO_D,
		THREE_D
	};
	Configuration system_config = Configuration::TWO_D;

	bool auto_home = false;
	bool load_robots_from_file = true;
	bool run_offline = false;

	enum ControllerState {
		NOT_READY = 0,
		READY,
		PLAY,
		PAUSE,
		HOMING,
		E_STOP,
		SHUTDOWN
	};
	ControllerState state = ControllerState::NOT_READY;
	string state_names[7] = { "NOT_READY", "READY", "PLAY", "PAUSE", "HOMING", "E_STOP", "SHUTDOWN" };

	bool showGUI = true;
	ofxGizmo gizmo_origin;
	ofxGizmo gizmo_ee_0;
	vector<ofxGizmo*> gizmos;
	void update_gizmos();

	ofRectangle bounds;

	bool use_ee_target = true;

	bool debugging = true;

	// Configuration Parameters
	ofxXmlSettings config;
	float cable_drum_diameter = 0;
	int cable_drum_dir = 0;
	float cable_drum_length = 0;
	int cable_drum_turns = 0;

	ClearPathMotorController motors = ClearPathMotorController();

public:
	RobotController() = default;
	RobotController(ofxXmlSettings config);
	vector<CableRobot*> robots;

	void draw();
	void draw_gui();
	void reset();
	void shutdown();
	void windowResized(int w, int h);

	void threadedFunction();

	void play();
	void pause();
	void set_e_stop(bool val);

	void move_vel_all(bool val);

	void key_pressed(int key);
	void save_settings();

	ofRectangle bounds_safety;
	ofRectangle bounds_motion;

	vector<ofxGizmo*> get_gizmos() { return gizmos; }
	void key_pressed_gizmo(int key);
	bool disable_camera();
	void set_origin(glm::vec3 pos, glm::quat orient = glm::quat(0, 0, 0, 1));
	void set_ee(glm::vec3 pos, glm::quat orient = glm::quat(0, 0, 0, 1));

	void set_targets(vector<glm::vec3*> targets);
	void set_targets(vector<glm::vec3> targets);
	void set_target(int i, float x, float y);
	void set_target_x(int i, float x);
	void set_target_y(int i, float y);
	glm::vec3 get_target(int i);
	vector<glm::vec3> get_targets();
	vector<glm::vec3> get_actual_positions();

	ofxPanel panel;
	void set_gui_position(int x, int y);
	ofParameterGroup params;
	ofParameter<string> status;

	ofParameterGroup params_info;
	ofParameter<string> com_ports;
	ofParameter<string> num_com_hubs;
	ofParameter<string> num_robots;

	ofParameterGroup params_bounds;
	ofParameterGroup params_bounds_safety;
	ofParameter<glm::vec3> safety_bounds_pos = glm::vec3();
	ofParameter<float> safety_bounds_width = 3000;
	ofParameter<float> safety_bounds_height = 1000;
	void on_safety_bounds_pos_changed(glm::vec3& val);
	void on_safety_bounds_width_changed(float& val);
	void on_safety_bounds_height_changed(float& val);

	ofParameterGroup params_bounds_motion;
	ofParameter<glm::vec3> motion_bounds_pos = glm::vec3();
	ofParameter<float> motion_bounds_width = 3000;
	ofParameter<float> motion_bounds_height = 1000;
	void on_motion_bounds_pos_changed(glm::vec3& val);
	void on_motion_bounds_width_changed(float& val);
	void on_motion_bounds_height_changed(float& val);

	ofParameterGroup params_limits;
	ofParameter<float> limit_velocity = 30;
	ofParameter<float> limit_acceleration= 200;
	ofParameter<float> limit_torque_min;
	ofParameter<float> limit_torque_max;
	void on_limit_vel_changed(float& val);
	void on_limit_accel_changed(float& val);
	void on_limit_torque_changed(float& val);

	ofParameterGroup params_shutdown;
	ofParameter <void> run_shutdown;
	ofParameter <float> position_shutdown = 0;
	void on_run_shutdown();

	ofParameterGroup params_homing;
	ofParameter <void> run_homing;
	void on_run_homing();

	ofColor mode_color_disabled;
	ofColor mode_color_eStop;
	ofColor mode_color_normal;
	ofColor mode_color_playing;
};