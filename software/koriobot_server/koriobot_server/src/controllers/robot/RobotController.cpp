#include "RobotController.h"


RobotController::RobotController(ofxXmlSettings config)
{
	this->config = config;
	int config_type = config.getValue("config:config_type", 2);
	system_config = Configuration(config_type);
	int num_bots = config.getValue("config:num_bots", 0);

	// Set the origin
	float x = config.getValue("config:origin:X", 0);
	float y = config.getValue("config:origin:Y", 0);
	float z = config.getValue("config:origin:Z", 0);
	float qx = config.getValue("config:origin:QX", 0);
	float qy = config.getValue("config:origin:QY", 0);
	float qz = config.getValue("config:origin:QZ", 0);
	float qw = config.getValue("config:origin:QW", 0);
	this->origin = new ofNode();
	this->origin->setGlobalPosition(glm::vec3(x, y, z));
	this->origin->setGlobalOrientation(glm::quat(qw, qx, qy, qz));
	this->origin->rotateAroundDeg(180, glm::vec3(1, 0, 0), glm::vec3(0, 0, 0));

	// Set the bases
	config.pushTag("config");
	config.pushTag("bases");
	for (int i = 0; i < config.getNumTags("pos"); i++) {
		config.pushTag("pos", i);
		glm::vec3 p = glm::vec3();
		p.x = config.getValue("X", 0);
		p.y = config.getValue("Y", 0);
		p.z = config.getValue("Z", 0);
		bases.push_back(p);
		config.popTag();
	}
	config.popTag();
	config.popTag();

	// Check for motor pairs for 2D configurations
	config.pushTag("config");
	config.pushTag("motor_pairs");
	for (int i = 0; i < config.getNumTags("pair"); i++) {
		vector<int> pair;
		config.pushTag("pair", i);
		pair.push_back(config.getValue("A", 0));
		pair.push_back(config.getValue("B", 0));
		motor_pairs.push_back(pair);
		config.popTag();
	}
	config.popTag();
	config.popTag();

	// Set the motion bounds
	x = config.getValue("config:bounds:motion:X", 0);
	y = config.getValue("config:bounds:motion:Y", 0);
	float width = config.getValue("config:bounds:motion:W", 0);
	float height = config.getValue("config:bounds:motion:H", 0);
	this->bounds_motion.setFromCenter(x, y, width, height);

	// Set the safety bounds
	x = config.getValue("config:bounds:safety:X", 0);
	y = config.getValue("config:bounds:safety:Y", 0);
	width = config.getValue("config:bounds:safety:W", 0);
	height = config.getValue("config:bounds:safety:H", 0);
	this->bounds_safety.setFromCenter(x, y, width, height);

	// Set the force limits
	limit_velocity = config.getValue("config:limits:velocity", 0);
	limit_acceleration = config.getValue("config:limits:acceleration", 0);
	limit_torque_min = config.getValue("config:limits:torque:MIN", 0);
	limit_torque_max = config.getValue("config:limits:torque:MAX", 0);

	// Set the shutdown position
	position_shutdown = config.getValue("config:shutdown:pos:Y", 0);


	gizmo_origin.setNode(*origin);
	gizmo_origin.setDisplayScale(.33);
	// add the origin gizmo to the gizmos list
	gizmos.push_back(&gizmo_origin);

	startThread();
}

void RobotController::initialize()
{
	// Create each cable robot
	for (int i = 0; i < motors.get_motors().size(); i++) {
		robots.push_back(new CableRobot(motors.get_motors()[i], config));
	}

	// Assign to cable robots to a particular configuration
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			gizmos.push_back(robots[i]->get_gizmo());
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < motor_pairs.size(); i++) {
			robots_2D.push_back(new CableRobot2D(origin,
				robots[motor_pairs[i][0]],
				robots[motor_pairs[i][1]],
				i,
				config
			));
			gizmos.push_back(robots_2D.back()->get_gizmo());
		}
	}

	// update the gui
	num_robots.set(ofToString(robots.size()));
	com_ports.set(motors.get_com_ports());
}

void RobotController::update()
{
	// update the gizmos
	update_gizmos();

	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->update();
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->update();
		}
	}
}

void RobotController::draw()
{
	// draw the boundaries
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(1);
	ofSetColor(ofColor::slateGray);
	ofDrawRectangle(bounds_motion);
	ofSetLineWidth(3);
	ofSetColor(ofColor::magenta);
	ofDrawRectangle(bounds_safety);
	ofPopStyle();

	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->draw();
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->draw();
		}
	}
}


void RobotController::reset()
{
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->reset();
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->reset();
		}
	}
}

void RobotController::shutdown()
{
	motors.shutdown();
}

void RobotController::windowResized(int w, int h)
{
	for (auto gizmo : gizmos)
		gizmo->setViewDimensions(w, h);
}

void RobotController::save_settings()
{
	ofLogNotice(__FUNCTION__) << "Saving Configuration File";

	// origin
	config.setValue("config:origin:X", origin->getGlobalPosition().x, 0);
	config.setValue("config:origin:Y", origin->getGlobalPosition().y, 0);
	config.setValue("config:origin:Z", origin->getGlobalPosition().z, 0);
	config.setValue("config:origin:QX", origin->getGlobalOrientation().x, 0);
	config.setValue("config:origin:QY", origin->getGlobalOrientation().y, 0);
	config.setValue("config:origin:QZ", origin->getGlobalOrientation().z, 0);
	config.setValue("config:origin:QW", origin->getGlobalOrientation().w, 0);

	// motion bounds
	config.setValue("config:bounds:motion:X", bounds_motion.getCenter().x);
	config.setValue("config:bounds:motion:Y", bounds_motion.getCenter().y);
	config.setValue("config:bounds:motion:W", bounds_motion.width);
	config.setValue("config:bounds:motion:H", bounds_motion.height);

	// safety bounds
	config.setValue("config:bounds:safety:X", bounds_safety.getCenter().x);
	config.setValue("config:bounds:safety:Y", bounds_safety.getCenter().y);
	config.setValue("config:bounds:safety:W", bounds_safety.width);
	config.setValue("config:bounds:safety:H", bounds_safety.height);
	cout << "H: " << config.getValue("bounds:safety:H", 1) << endl;

	// vel, accel, torque limits
	config.setValue("config:limits:velocity", limit_velocity.get(), 0);
	config.setValue("config:limits:acceleration", limit_acceleration.get(), 0);
	config.setValue("config:limits:torque:MIN", limit_torque_min.get(), 0);
	config.setValue("config:limits:torque:MAX", limit_torque_max.get(), 0);

	config.saveFile();
}

void RobotController::threadedFunction()
{
	while (isThreadRunning()) {
		if (!is_initialized) {

			// run once
			if (!is_gui_setup)
				setup_gui();

			// Initialize the motors
			if (motors.initialize()) {
				// Assign koriobots to configuration
				initialize();
				check_for_system_ready();
				is_initialized = true;
			}
			// add a delay before trying to initialize again
			else {
				float timer = ofGetElapsedTimeMillis();
				float delay = 5;
				ofLogWarning(__FUNCTION__) << "initialize() FAILED. Check that the motors are powered and connected to PC.\n\tRETRYING " << delay << " SECONDS.\n";
				while (ofGetElapsedTimeMillis() < timer + (delay * 1000))
				{
					// ... hold the thread for 5 seconds
				}
			}
		}
		update();
	}
}
/**
 * @brief Checks if all motors have been homed.
 * Updates the Controller State and updates the status in the GUI.
 */
void RobotController::check_for_system_ready()
{
	// check if motors are already enabled
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->check_for_system_ready();
		//robots[i]->enable.set(robots[i]->is_enabled());
	}

	// check if motors are homed and ready
	bool is_ready = true;
	for (int i = 0; i < robots.size(); i++) {
		if (robots[i]->status.get() == "ENABLED" || robots[i]->status.get() == "DISABLED") {
		}
		else {
			is_ready = false;
		}
	}
	state = is_ready ? ControllerState::READY : ControllerState::NOT_READY;
	// update the gui with the READY status
	status.set(state_names[state]);
}

void RobotController::setup_gui()
{
	mode_color_eStop = ofColor(250, 0, 0, 100);
	mode_color_normal = ofColor(60, 120);
	mode_color_disabled = ofColor(0, 200);
	mode_color_playing = ofColor(ofColor::blueSteel);

	int gui_width = 250;

	panel.setup("Controller");
	panel.setWidthElements(gui_width);
	panel.setPosition(10, 15);

	params_info.setName("System_Info");
	params_info.add(com_ports.set("COM_Ports", ""));
	params_info.add(num_robots.set("Num_Motors", ""));
	params_info.add(status.set("Status", ""));

	params_bounds.setName("Boundaries");
	params_bounds_motion.setName("Motion_Bounds");
	params_bounds_motion.add(motion_bounds_pos.set("Centroid", glm::vec3(bounds_motion.getCenter().x, bounds_motion.getCenter().y, 0), glm::vec3(0, -5000, 0), glm::vec3(0, 0, 0)));
	params_bounds_motion.add(motion_bounds_width.set("Width", bounds_motion.getWidth(), 0, 5000));
	params_bounds_motion.add(motion_bounds_height.set("Height", bounds_motion.getHeight(), 0, 5000));
	params_bounds.add(params_bounds_motion);

	params_bounds_safety.setName("Safety_Bounds");
	params_bounds_safety.add(safety_bounds_pos.set("Centroid", glm::vec3(bounds_safety.getCenter().x, bounds_safety.getCenter().y, 0), glm::vec3(-3000, -3000, 0), glm::vec3(3000, 3000, 3350)));
	params_bounds_safety.add(safety_bounds_width.set("Width", bounds_safety.getWidth(), 0, 4000));
	params_bounds_safety.add(safety_bounds_height.set("Height", bounds_safety.getHeight(), 0, 3000));
	params_bounds.add(params_bounds_safety);

	params_limits.setName("Limits");
	params_limits.add(limit_velocity.set("Velocity_(RPM)", limit_velocity.get(), 0, 100));
	params_limits.add(limit_acceleration.set("Acceleration_(RPM/s)", limit_acceleration.get(), 0, 500));
	params_limits.add(limit_torque_min.set("Torque_(MIN)", limit_torque_min.get(), 0, 40));
	params_limits.add(limit_torque_max.set("Torque_(MAX)", limit_torque_max.get(), 0, 40));

	params_homing.setName("Homing");
	params_homing.add(run_homing.set("Run_Homing"));

	params_shutdown.setName("Shutdown");
	params_shutdown.add(position_shutdown.set("Shutdown_Pos", position_shutdown.get(), 0, 5000));
	params_shutdown.add(run_shutdown.set("Run_Shutdown"));

	panel.add(params_info);
	panel.add(params_bounds);
	panel.add(params_limits);
	panel.add(params_homing);
	panel.add(params_shutdown);


	// Minimize all the groups
	panel.getGroup("System_Info").minimize();
	panel.getGroup("Boundaries").minimize();
	panel.getGroup("Limits").minimize();
	panel.getGroup("Homing").minimize();
	panel.getGroup("Shutdown").minimize();

	motion_bounds_pos.addListener(this, &RobotController::on_motion_bounds_pos_changed);
	motion_bounds_width.addListener(this, &RobotController::on_motion_bounds_width_changed);
	motion_bounds_height.addListener(this, &RobotController::on_motion_bounds_height_changed);
	safety_bounds_pos.addListener(this, &RobotController::on_safety_bounds_pos_changed);
	safety_bounds_width.addListener(this, &RobotController::on_safety_bounds_width_changed);
	safety_bounds_height.addListener(this, &RobotController::on_safety_bounds_height_changed);
	limit_velocity.addListener(this, &RobotController::on_limit_vel_changed);
	limit_acceleration.addListener(this, &RobotController::on_limit_accel_changed);
	limit_torque_min.addListener(this, &RobotController::on_limit_torque_changed);
	limit_torque_max.addListener(this, &RobotController::on_limit_torque_changed);

	is_gui_setup = true;
}

void RobotController::draw_gui()
{
	if (showGUI) {
		panel.draw();
		if (system_config == Configuration::ONE_D) {
			for (int i = 0; i < robots.size(); i++) {
				robots[i]->panel.draw();
			}
		}
		else if (system_config == Configuration::TWO_D) {
			for (int i = 0; i < robots_2D.size(); i++) {
				int padding = 20;
				int x = panel.getPosition().x + (i * panel.getWidth()) + (i * padding);
				int y = panel.getPosition().y + panel.getHeight() + padding;
				robots_2D[i]->panel.setPosition(x, y);
				robots_2D[i]->draw_gui();

				if (robots_2D[i]->debugging) {
					ofPushMatrix();
					ofTranslate(ofGetWidth() - 550, i * 150 + 60);
					robots_2D[i]->plot.draw();
					ofPopMatrix();
				}
			}
		}
	}
}


void RobotController::play()
{
	state = ControllerState::PLAY;
}

void RobotController::pause()
{
	ofLogNotice(__FUNCTION__) << "Stopping Robots ...";
	state = ControllerState::PAUSE;
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->stop();
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->stop();
		}
	}
}

void RobotController::set_e_stop(bool val)
{
	state = ControllerState::E_STOP;
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->set_e_stop(val);
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->set_e_stop(val);
		}
	}
}

void RobotController::key_pressed(int key)
{
	switch (key)
	{
	case '?':
		debugging = !debugging;
		break;
	case 'h':
	case 'H':
		showGUI = !showGUI;
		break;
	case 19:	// CRTL+s
		save_settings();
		break;
	default:
		break;
	}

	key_pressed_gizmo(key);

	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++)
			robots[i]->key_pressed(key);
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++)
			robots_2D[i]->key_pressed(key);
	}
}

void RobotController::move_vel_all(bool val)
{
	ofLogNotice(__FUNCTION__) << "Turning Velocity Moves [" << (val ? "ON" : "OFF") << "]";
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++)
			robots[i]->move_to_vel.set(val);
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++)
			robots_2D[i]->move_to_vel.set(val);
	}
}

void RobotController::key_pressed_gizmo(int key)
{
	switch (key)
	{
	case 'e':
	case 'E':
		for (auto gizmo : gizmos)
			gizmo->setType(ofxGizmo::ofxGizmoType::OFX_GIZMO_ROTATE);
		break;
	case 'w':
	case 'W':
		for (auto gizmo : gizmos) {
			gizmo->setType(ofxGizmo::ofxGizmoType::OFX_GIZMO_MOVE);
		}
		break;
	case '0':
		// reset the transform of the origin node
		gizmo_origin.setType(ofxGizmo::ofxGizmoType::OFX_GIZMO_MOVE);
		gizmo_origin.setMatrix(glm::mat4());
		break;
	default:
		break;
	}
}

bool RobotController::disable_camera()
{
	bool val = false;
	for (auto gizmo : gizmos)
		if (gizmo->isInteracting())
			val = true;
	return val;
}

void RobotController::set_origin(glm::vec3 pos, glm::quat orient)
{
	origin->setGlobalPosition(pos);
	origin->setGlobalOrientation(orient);
}

void RobotController::set_targets(vector<glm::vec3> targets)
{
	if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			ofNode node;
			node.setPosition(targets[i]);
			robots_2D[i]->get_gizmo()->setNode(node);
		}
	}
}


void RobotController::set_targets(vector<glm::vec3*> targets)
{
	if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			ofNode node;
			node.setPosition(*targets[i]);
			robots_2D[i]->get_gizmo()->setNode(node);
		}
	}
}

void RobotController::set_target(int i, float x, float y)
{
	if (system_config == Configuration::ONE_D) {
		ofLogNotice(__FUNCTION__) << "Not implemented yet." << endl;
	}
	else if (system_config == Configuration::TWO_D) {
		x = ofMap(x, 0, 1, bounds_safety.getMinX(), bounds_safety.getMaxX());
		y = ofMap(y, 0, 1, bounds_safety.getMaxY(), bounds_safety.getMinY());

		// move all the robots
		if (i == -1) {
			for (int index = 0; index < robots_2D.size(); index++) {
				auto pos = robots_2D[index]->get_gizmo()->getTranslation();
				pos.x = x;
				pos.y = y;
				ofNode node;
				node.setGlobalPosition(pos);
				robots_2D[index]->get_gizmo()->setNode(node);
			}
		}
		// move only one (if it's within range)
		else if (i < robots_2D.size()) {
			auto pos = robots_2D[i]->get_gizmo()->getTranslation();
			pos.x = x;
			pos.y = y;
			ofNode node;
			node.setGlobalPosition(pos);
			robots_2D[i]->get_gizmo()->setNode(node);
		}
		else {
			ofLogWarning(__FUNCTION__) << "Index [" << ofToString(i) << "] out of range." << endl;
		}
	}
}

/**
 * @brief Returns the *ROUGH ESTIMATE* of the end effector position (just the mid-point of the projected ends).
 *
 * @param ()  i:
 * @return (glm::vec3)
 */
glm::vec3 RobotController::get_target(int i) {
	if (system_config == Configuration::TWO_D) {
		if (i < robots_2D.size()) {
			return robots_2D[i]->estimated_target_actual;
		}
	}
	return glm::vec3();
}

void RobotController::set_target_x(int i, float x)
{
	if (i < robots_2D.size()) {
		auto pos = robots_2D[i]->get_gizmo()->getTranslation();
		set_target(i, x, pos.y);
	}
}

void RobotController::set_target_y(int i, float y)
{
	if (i < robots_2D.size()) {
		auto pos = robots_2D[i]->get_gizmo()->getTranslation();
		set_target(i, pos.x, y);
	}
}

/**
 * @brief Returns the estimated actual 2D positions of the robot's end effector.
 *
 * @return (vector<glm::vec3>)
 */
vector<glm::vec3> RobotController::get_actual_positions()
{
	return get_targets();
}

void RobotController::set_gui_position(int x, int y)
{
	panel.setPosition(x, y);
}


vector<glm::vec3> RobotController::get_targets()
{
	vector<glm::vec3> tgts;
	for (int i = 0; i < robots_2D.size(); i++) {
		tgts.push_back(get_target(i));
	}
	return tgts;
}

void RobotController::update_gizmos()
{
	// update the origin node to match the gizmo
	origin->setGlobalPosition(gizmos[0]->getTranslation());
	origin->setGlobalOrientation(gizmos[0]->getRotation());

	// override ee gizmo transforms if we're moving the origin gizmo
	bool val = gizmos[0]->isInteracting();
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
			robots[i]->override_gizmo = val;
			robots[i]->update_gizmo();
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->override_gizmo = val;
			robots_2D[i]->update_gizmo();
		}
	}
}


void RobotController::on_motion_bounds_pos_changed(glm::vec3& val)
{
	bounds_motion.setFromCenter(val, motion_bounds_width.get(), motion_bounds_height.get());

	float y = bounds_motion.getCenter().y;
	float height = bounds_motion.getHeight();
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->bounds_min.set((y + height / 2) * -1);
		robots[i]->bounds_max.set((y - height / 2) * -1);
	}
	if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->bounds_min.set((y + height / 2) * -1);
			robots_2D[i]->bounds_max.set((y - height / 2) * -1);
		}
	}
}

void RobotController::on_motion_bounds_width_changed(float& val)
{
	bounds_motion.setFromCenter(motion_bounds_pos.get(), val, motion_bounds_height.get());
}

void RobotController::on_motion_bounds_height_changed(float& val)
{
	bounds_motion.setFromCenter(motion_bounds_pos.get(), motion_bounds_width.get(), val);

	float y = bounds_motion.getCenter().y;
	float height = bounds_motion.getHeight();
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->bounds_min.set((y + height / 2) * -1);
		robots[i]->bounds_max.set((y - height / 2) * -1);
	}
	if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->bounds_min.set((y + height / 2) * -1);
			robots_2D[i]->bounds_max.set((y - height / 2) * -1);
		}
	}
}

void RobotController::on_safety_bounds_pos_changed(glm::vec3& val)
{
	bounds_safety.setFromCenter(val, safety_bounds_width.get(), safety_bounds_height.get());
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->set_safety_bounds(val, safety_bounds_width.get(), safety_bounds_height.get());
		}
	}
}

void RobotController::on_safety_bounds_width_changed(float& val)
{
	bounds_safety.setFromCenter(safety_bounds_pos.get(), val, safety_bounds_height.get());
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->set_safety_bounds(safety_bounds_pos.get(), val, safety_bounds_height.get());
		}
	}
}

void RobotController::on_safety_bounds_height_changed(float& val)
{
	bounds_safety.setFromCenter(safety_bounds_pos.get(), safety_bounds_width.get(), val);
	if (system_config == Configuration::ONE_D) {
		for (int i = 0; i < robots.size(); i++) {
		}
	}
	else if (system_config == Configuration::TWO_D) {
		for (int i = 0; i < robots_2D.size(); i++) {
			robots_2D[i]->set_safety_bounds(safety_bounds_pos.get(), safety_bounds_width.get(), val);
		}
	}
}

void RobotController::on_limit_vel_changed(float& val)
{
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->limit_velocity.set(val);
	}
}

void RobotController::on_limit_accel_changed(float& val)
{
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->limit_acceleration.set(val);
	}
}

void RobotController::on_limit_torque_changed(float& val)
{
	for (int i = 0; i < robots.size(); i++) {
		robots[i]->limit_torque_min.set(limit_torque_min);
		robots[i]->limit_torque_max.set(limit_torque_max);
	}
}