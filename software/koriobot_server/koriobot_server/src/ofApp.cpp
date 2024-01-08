#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_NOTICE);
	ofLogToConsole();
	ofSetCircleResolution(60);

	config = load_settings("configuration.xml");
	setup_camera();
	setup_gui();

	// auto connect to osc sender/receiver
	on_osc_connect();

	// pass the configuration settings to the robot controller
	robots = new RobotController(config);

	// Final touches to the GUI
	robots->set_gui_position(panel.getPosition().x, panel.getPosition().y + panel.getHeight() + 10);
	update_camera_target();
}

//--------------------------------------------------------------
void ofApp::update() {
	if (osc_status.get() == "CONNECTED") {
		check_for_messages();
	}
}

void ofApp::draw()
{
	ofBackgroundGradient(background_inner, background_outer);
	cam.begin();

	// draw the axis and planes
	ofPushStyle();
	//ofDrawGridPlane(1000, 10, true);
	ofSetLineWidth(3);
	ofSetColor(ofColor::blue, 200);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, 10000));
	ofSetColor(ofColor::blue, 80);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(0, 0, -10000));
	ofSetColor(ofColor::green, 200);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(0, 10000, 0));
	ofSetColor(ofColor::green, 80);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(0, -10000, 0));
	ofSetColor(ofColor::red, 200);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(10000, 0, 0));
	ofSetColor(ofColor::red, 80);
	ofLine(glm::vec3(0, 0, 0), glm::vec3(-10000, 0, 0));
	ofPopStyle();

	// draw the robots
	robots->draw();

	// draw all the gizmos
	for (auto gizmo : robots->get_gizmos())
		gizmo->draw(cam);

	cam.end();

	// draw 2D GUI 
	panel.draw();
	robots->set_gui_position(panel.getPosition().x, panel.getPosition().y + panel.getHeight() + 20);
	robots->draw_gui();
	ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), ofGetWidth() - 100, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	key_pressed_camera(key);

	// pass the key pressed to the motion & robot controller
	robots->key_pressed(key);

	switch (key)
	{
	case 'f':
	case 'F': {
		ofToggleFullscreen();
		break;
	}
	// keypress to scale the gui for 4K displays
	case 'k':
	case 'K': {
		enable_hi_res = !enable_hi_res;
		if (enable_hi_res)
			ofxGuiEnableHiResDisplay();
		else
			ofxGuiDisableHiResDisplay();
		break;
	}
	default:
		break;
	}
}


/**
 * @brief Loads system settings from a file.
 *
 * @param (string)  filename: file must be in local /bin/data folder (must end in .xml). Defaults to "settings.xml"
 */
ofxXmlSettings ofApp::load_settings(string filename)
{
	if (config.loadFile(filename)) {

		return config;
	}
	else {
		ofLogWarning("CableRobot::load_settings") << "No settings file found at: /bin/data/" << filename;
		ofxXmlSettings empty_config;
		return empty_config;
	}
}

/**
 * @brief Update the gizmos with new view dimentions.
 *
 * @param (int)  w:
 * @param (int)  h:
 */
void ofApp::windowResized(int w, int h) {
	if (robots)
		robots->windowResized(w, h);
}

void ofApp::setup_gui()
{
	panel.setup();
	panel.setName("Koriobot_Server");
	panel.setPosition(10, 10);
	panel.setWidthElements(250);

	params.setName("Communications");
	params.add(osc_port_receiving.set("Receive_Ports", osc_port_receiving.get()));
	params.add(osc_host.set("Send_Ports", osc_host.get()));
	params.add(osc_port_sending.set("Send_Hosts", osc_port_sending.get()));
	params.add(osc_connect.set("CONNECT"));
	params.add(osc_status.set("Status", "DISCONNECTED"));
	panel.add(params);

	osc_connect.addListener(this, &ofApp::on_osc_connect);
}

void ofApp::on_osc_connect()
{
	// Setup
	if (receivers.size() == 0) {
		config.pushTag("config");
		config.pushTag("osc");
		// setup receivers
		for (int i = 0; i < config.getNumTags("receive"); i++) {
			config.pushTag("receive", i);
			ofxOscReceiver r;
			ofxOscReceiverSettings settings;
			settings.port = config.getValue("port", 0);
			r.setup(settings);
			ofLogNotice(__FUNCTION__) << "ofxOscReceiver setup on port " << settings.port;
			receivers.push_back(r);
			config.popTag();

			if (osc_port_receiving.get() == "")
				osc_port_receiving.set(ofToString(settings.port));
			else
				osc_port_receiving.set(osc_port_receiving.get() + "," + ofToString(settings.port));
		}
		// setup senders
		for (int i = 0; i < config.getNumTags("send"); i++) {
			config.pushTag("send", i);
			ofxOscSender s;
			ofxOscSenderSettings settings;
			settings.host = config.getValue("host", "127.0.0.1");
			settings.port = config.getValue("port", 0);
			s.setup(settings);
			senders.push_back(s);
			ofLogNotice(__FUNCTION__) << "ofxOscSender setup on host:port " << settings.host << ":" << settings.port << endl;
			config.popTag();

			if (osc_port_sending.get() == "") {
				osc_host.set(ofToString(settings.host));
				osc_port_sending.set(ofToString(settings.port));
			}
			else {
				osc_host.set(osc_host.get() + "," + ofToString(settings.host));
				osc_port_sending.set(osc_port_sending.get() + "," + ofToString(settings.port));
			}
		}
		config.popTag();
		config.popTag();
		osc_status.set("CONNECTED");
	}
	// Reconnecting / Disconnecting	
	else {
		if (osc_status.get() == "DISCONNECTED") {
			for (auto& receiver : receivers) {
				receiver.start();
			}
			osc_status.set("CONNECTED");
		}
		else {
			for (auto& receiver : receivers) {
				receiver.stop();
			}
			osc_status.set("DISCONNECTED");
		}
	}

}

void ofApp::check_for_messages()
{
	for (auto& receiver : receivers) {
		while (receiver.hasWaitingMessages()) {
			ofxOscMessage m;
			receiver.getNextMessage(m);

			// trigger soft stop for all koriobots
			if (m.getAddress() == "/stop") {
				robots->move_vel_all(false);
				robots->pause();
			}
			// toggle move_vel for all koriobots
			else if (m.getAddress() == "/move_vel") {
				robots->move_vel_all(m.getArgAsBool(0));
			}
			// trigger reset for all koriobots
			else if (m.getAddress() == "/reset") {
				robots->reset();
			}
			// set normalized XY target in range { [0,1], [0,1] }
			else if (m.getAddress() == "/norm") {
				int i = m.getArgAsInt(0);
				float x = m.getArgAsFloat(1);
				float y = m.getArgAsFloat(2);
				robots->set_target(i, x, y);
			}
			// update the bounds of the safety region
			else if (m.getAddress() == "/bounds/pos") {
				float x = m.getArgAsFloat(0);
				float y = m.getArgAsFloat(1);
				x = ofMap(x, 0, 1, robots->bounds_motion.getMinX(), robots->bounds_motion.getMaxX());
				y = ofMap(y, 0, 1, robots->bounds_motion.getMinY(), robots->bounds_motion.getMaxY());
				robots->safety_bounds_pos.set(glm::vec3(x, y, robots->safety_bounds_pos.get().z));
			}
			else if (m.getAddress() == "/bounds/width") {
				float val = m.getArgAsFloat(0);
				val = ofMap(val, 0, 1, robots->safety_bounds_width.getMin(), robots->safety_bounds_width.getMax());
				robots->safety_bounds_width.set(val);
			}
			else if (m.getAddress() == "/bounds/height") {
				float val = m.getArgAsFloat(0);
				val = ofMap(val, 0, 1, robots->safety_bounds_height.getMin(), robots->safety_bounds_height.getMax());
				robots->safety_bounds_height.set(val);
			}
			// update the force limits
			else if (m.getAddress() == "/limits/velocity") {
				float val = m.getArgAsFloat(0);
				val = ofMap(val, 0, 1, robots->limit_velocity.getMin(), robots->limit_velocity.getMax());
				robots->limit_velocity.set(val);
			}
			else if (m.getAddress() == "/limits/acceleration") {
				float val = m.getArgAsFloat(0);
				val = ofMap(val, 0, 1, robots->limit_acceleration.getMin(), robots->limit_acceleration.getMax());
				robots->limit_acceleration.set(val);
			}
			else {
				// unrecognized message
				string msgString;
				msgString = m.getAddress();
				msgString += ":";
				for (size_t i = 0; i < m.getNumArgs(); i++) {

					// get the argument type
					msgString += " ";
					msgString += m.getArgTypeName(i);
					msgString += ":";

					// display the argument - make sure we get the right type
					if (m.getArgType(i) == OFXOSC_TYPE_INT32) {
						msgString += ofToString(m.getArgAsInt32(i));
					}
					else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT) {
						msgString += ofToString(m.getArgAsFloat(i));
					}
					else if (m.getArgType(i) == OFXOSC_TYPE_STRING) {
						msgString += m.getArgAsString(i);
					}
					else {
						msgString += "unhandled argument type " + m.getArgTypeName(i);
					}
				}
				ofLogWarning(__FUNCTION__) << msgString << endl;
			}
		}
	}
}

void ofApp::setup_camera()
{
	cam.setUpAxis(glm::vec3(0, 0, 1));
	cam.setPosition(camera_top);
	cam.setTarget(camera_target);
	cam.setDistance(5000);
	cam.setFarClip(30000);
	cam.setNearClip(.0001);
}

void ofApp::key_pressed_camera(int key)
{
	switch (key)
	{
	case 'v':
		on_print_camera_view();
		break;
	case '1':
		// TOP
		on_set_camera_view(camera_top, camera_target);
		break;
	case '2':
		// SIDE
		on_set_camera_view(camera_side, camera_target);
		break;
	case '3':
		// FRONT
		on_set_camera_view(camera_front, camera_target);
		break;
	case '4':
		// PERSPECTIVE		
		on_set_camera_view(camera_perspective, camera_target);
		break;
	default:
		break;
	}
}

void ofApp::disable_camera(bool val)
{
	if (val) {
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
	}
	else {
		if (!cam.getMouseInputEnabled()) cam.enableMouseInput();
	}
}

void ofApp::on_set_camera_view(glm::vec3 position, glm::vec3 target, float distance)
{
	cam.setPosition(position);
	cam.setTarget(target);
	cam.setDistance(distance);
}

void ofApp::on_print_camera_view()
{
	cout << "CAMERA VALUES:" << endl;
	cout << "\tpos:\t" << cam.getGlobalPosition() << endl;
	cout << "\ttarget:\t" << cam.getTarget().getGlobalPosition() << endl;
	cout << "\tdist:\t" << cam.getDistance() << endl;
	cout << "\tup axis:\t" << cam.getUpAxis() << endl;
	cout << "\tup dir:\t" << cam.getUpDir() << endl;
	cout << "\tnear clip:\t" << cam.getNearClip() << endl;
	cout << "\tfar clip:\t" << cam.getFarClip() << endl << endl;
}
/**
 * @brief Updates the camera values centered on the safety boundaries.
 */
void ofApp::update_camera_target()
{
	if (robots) {
		camera_target = robots->bounds_safety.getCenter();
	}
	camera_perspective.y = camera_target.y + 1800;
	camera_side = glm::vec3(camera_target.y, camera_target.y, 0);
	camera_front = glm::vec3(0, camera_target.y, -camera_target.y);
	on_set_camera_view(camera_perspective, camera_target);
}
