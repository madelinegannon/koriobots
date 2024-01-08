#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxGizmo.h"
#include "ofxXmlSettings.h"

#include "controllers/robot/RobotController.h"

#define DEBUG


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void windowResized(int w, int h);

	RobotController* robots;

	// 3D World
	ofEasyCam cam;
	ofRectangle bounds_safety;
	ofRectangle bounds_motion;

	// Communications
	vector<ofxOscReceiver> receivers;
	vector<ofxOscSender> senders;
	ofxOscReceiver osc_receiver;
	ofxOscSender osc_sender;
	void check_for_messages();

	void setup_camera();
	void key_pressed_camera(int key);
	void disable_camera(bool val);

	void on_set_camera_view(glm::vec3 position, glm::vec3 target, float distance = 5000);
	void on_print_camera_view();
	void update_camera_target();

	glm::vec3 camera_side = glm::vec3(1000, 0, 0);
	glm::vec3 camera_top = glm::vec3(0, 1000, 0);
	glm::vec3 camera_front = glm::vec3(0, 0, 1000);
	glm::vec3 camera_perspective = glm::vec3(4000, 1800, 4000);
	glm::vec3 camera_target = glm::vec3(0, 0, 0);

	ofColor background_inner = ofColor(238);
	ofColor background_outer = ofColor(118);

	void setup_gui();
	bool enable_hi_res = false;

	ofxPanel panel;
	ofParameterGroup params;
	ofParameter<string> osc_port_receiving = "";
	ofParameter<string> osc_port_sending = "";
	ofParameter<string> osc_host = "127.0.0.1";
	ofParameter<void> osc_connect;
	ofParameter<string> osc_status;
	void on_osc_connect();

private:
	// Configuration Parameters
	ofxXmlSettings config;
	ofxXmlSettings load_settings(string filename = "configuration.xml");
	
};
