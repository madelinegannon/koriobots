#include "CableRobot.h"


CableRobot::CableRobot(ClearPathMotor* motor, ofxXmlSettings config)
{
	this->motor = motor;

	configure(config);
	setup_gui();
	motor->set_motion_params(limit_velocity.get(), limit_acceleration.get());

	check_for_system_ready();

	setup_plots();
}

void CableRobot::configure(ofxXmlSettings config)
{
	this->config = config;
	// Save the origin (Global Coordinate
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

	// Set the base (Local Coordinates)
	config.pushTag("config");
	config.pushTag("bases");
	config.pushTag("pos", motor->get_motor_id());
	glm::vec3 p = glm::vec3();
	p.x = config.getValue("X", 0);
	p.y = config.getValue("Y", 0);
	p.z = config.getValue("Z", 0);
	base.setParent(*origin);
	base.setPosition(p);
	config.popTag();
	config.popTag();
	config.popTag();

	// Setup the Cable Drum
	int cable_drum_dir = config.getValue("config:cable_drum:direction", 0);
	drum.direction = cable_drum_dir < 2 ? Groove::LEFT_HANDED : Groove::RIGHT_HANDED;
	drum.diameter_drum = config.getValue("config:cable_drum:diameter_drum", 0);
	drum.diameter_cable = config.getValue("config:cable_drum:diameter_cable", 0.0);
	drum.length = config.getValue("config:cable_drum:length", 0);
	drum.turns = config.getValue("config:cable_drum:turns", 0);
	drum.initialize(drum.direction, drum.diameter_drum, drum.length, drum.turns);

	// Define the linear length per motor step
	float step_resolution = motor->get_resolution();
	mm_per_count = drum.circumference / step_resolution;

	// Setup the kinematic chain
	tangent.setParent(this->base);
	//tangent.setPosition(drum.get_tangent());
	target.setPosition(0, 0, 0);
	actual.setParent(tangent);
	actual.setPosition(0, 0, 0);

	ee_offset = config.getValue("config:ee:offset:X", 0);

	x = config.getValue("config:bounds:motion:X", 0);
	y = config.getValue("config:bounds:motion:Y", 0);
	float width = config.getValue("config:bounds:motion:W", 0);
	float height = config.getValue("config:bounds:motion:H", 0);
	bounds_motion.setFromCenter(x, y, width, height);
	this->bounds_min = (y + height / 2) * -1;
	this->bounds_max = (y - height / 2) * -1;

	// Set the force limits
	limit_velocity = config.getValue("config:limits:velocity", 0);
	limit_acceleration = config.getValue("config:limits:acceleration", 0);
	limit_torque_min = config.getValue("config:limits:torque:MIN", 0);
	limit_torque_max = config.getValue("config:limits:torque:MAX", 0);

	// if the external ee node is empty, create a new one that is internal and add a gizmo
	if (ee->getGlobalPosition() == glm::vec3() && ee->getGlobalOrientation() == glm::quat()) {
		ee = new ofNode();
		ee->setParent(tangent);
		ee->setPosition(0, -1 * (bounds_max.get() - bounds_min.get()) / 2, tangent.getGlobalPosition().z);
		target.setParent(*ee);

		gizmo_ee.setNode(*ee);
		gizmo_ee.setDisplayScale(.5);
		gizmo_ee.setTranslationAxisMask(IGizmo::AXIS_Y);
		gizmo_ee.setRotationAxisMask(IGizmo::AXIS_Z);
	}
	// otherwise, parent the target to the external ee and offset by ee_offset value
	else {
		target.setParent(*ee);
		target.setGlobalPosition(ee->getGlobalPosition());
		if (motor->get_motor_id() % 2 == 0)
			target.setPosition(-1 * ee_offset, 0, 0);
		else
			target.setPosition(ee_offset, 0, 0);

		gizmo_ee.setNode(*ee);
		gizmo_ee.setDisplayScale(.5);
		gizmo_ee.setRotationAxisMask(IGizmo::AXIS_Z);
	}
}

float CableRobot::get_position_actual()
{
	return count_to_mm(motor->get_position(false), true);
}

/**
 * @brief Returns the main motion parameters for the robot.
 *
 * @return (vector<float>)  params: { limit_velocity, limit_acceleration, bounds_min, bounds_max }
 */
vector<float> CableRobot::get_motion_parameters()
{
	vector<float> params;
	params.push_back(limit_velocity.get());
	params.push_back(limit_acceleration.get());
	params.push_back(bounds_min.get());
	params.push_back(bounds_max.get());
	return params;
}

/**
 * @brief Sets the main motion parameters for the robot.
 *
 * @param (float)  velocity_max: velocity limit (RPM)
 * @param (float)  accel_max: acceleration limit (RPM/s)
 * @param (float)  position_min: minimum position bounds
 * @param (float)  position_max: maximum position bounds
 */
void CableRobot::set_motion_parameters(float velocity_max, float accel_max, float position_min, float position_max)
{
	limit_velocity.set(velocity_max);
	limit_acceleration.set(accel_max);
	bounds_min.set(position_min);
	bounds_max.set(position_max);
}

/**
 * @brief Returns the main motion parameters for the robot.
 *
 * @return (vector<float>)  params: { jogging_vel_limit, jogging_accel_limit, jogging_distance }
 */
vector<float> CableRobot::get_jogging_parameters()
{
	vector<float> params;
	params.push_back(jog_vel.get());
	params.push_back(jog_accel.get());
	params.push_back(jog_dist.get());
	return params;
}

void CableRobot::set_jogging_parameters(float jogging_vel, float jogging_accel, float jogging_dist)
{
	jog_vel.set(jogging_vel);
	jog_accel.set(jogging_accel);
	jog_dist.set(jogging_dist);
}

void CableRobot::set_velocity_limit(float velocity_max)
{
	limit_velocity.set(velocity_max);
}

void CableRobot::set_accel_limit(float accel_max)
{
	limit_acceleration.set(accel_max);
}

/**
 * @brief Set minimum and maximum travel bounds .
 *
 * @param (float)  min: minimum bounds (in mm)
 * @param (float)  max: maximum bounds (in mm)
 */
void CableRobot::set_bounds(float min, float max)
{
	bounds_min.set(min);
	bounds_max.set(max);
}

bool CableRobot::is_torque_in_limits()
{
	auto torque_measured = motor->get()->Motion.TrqMeasured.Value();
	//info_torque_actual.set(ofToString(torque_measured));
	if (torque_measured > limit_torque_max.get() || torque_measured < limit_torque_min.get()) {
		ofLogWarning(__FUNCTION__) << "\tRobot " << ofToString(motor->get_motor_id()) << " is OUT OF TORQUE RANGE with value of " << ofToString(torque_measured) << endl;
		return false;
	}
	return true;
}

void CableRobot::update()
{
	// check if we should be homing
	if (state == RobotState::HOMING) {
		// if we successfully homed, update the state and gui color
		if (run_homing_routine(60)) {
			bool _is_enabled = is_enabled();
			state = _is_enabled ? RobotState::ENABLED : RobotState::DISABLED;
			status.set(state_names[state]);
			auto color = _is_enabled ? mode_color_enabled : mode_color_disabled;
			panel.setBorderColor(color);
		}
		else {
			//state = RobotState::NOT_HOMED;
			//status.set(state_names[state]);
			//panel.setBorderColor(mode_color_not_homed);
		}
	}

	if (move_type == MoveType::VEL) {

		if (state != RobotState::E_STOP) {
			if (is_in_bounds(abs(actual.getPosition().y), true)) {
				// @FIXME: do move

			}
			else {
				stop();
				stop();
				ofLogWarning("CableRobot::update") << "Stopping CableRobot " << motor->get_motor_id() << ": trying to send past home position." << endl;
			}
		}
	}
	else {
		//ofLogNotice(__FUNCTION__) << motor->get_motor_id() << " target global position: " << ofToString(target.getGlobalPosition());
		//ofLogNotice(__FUNCTION__) << motor->get_motor_id() << " ee global position: " << ofToString(ee->getGlobalPosition());
		//ofLogNotice(__FUNCTION__) << motor->get_motor_id() << " gizmo_ee global position: " << ofToString(gizmo_ee.getTranslation()) << endl;
	}

	// update the actual positions
	position_actual = get_position_actual();
	actual.setPosition(0, -1 * position_actual, 0);

	// update the gui
	//info_velocity_actual.set(ofToString(motor->get()->Motion.VelMeasured.Value()));
}

void CableRobot::draw()
{
	ofPushStyle();

	ofSetLineWidth(2);
	ofSetColor(255);
	ofNoFill();

	// draw base and tangent nodes
	ofDrawEllipse(base.getGlobalPosition(), drum.get_diameter(), drum.get_diameter());
	ofDrawLine(base.getGlobalPosition(), tangent.getGlobalPosition());
	base.draw();
	tangent.draw();


	// draw current ee

	// draw ghosted line between tangent and target & ee and target
	// show RED if the target is out of bounds
	if (-1 * ee->getPosition().y > bounds_min.get() &&
		-1 * ee->getPosition().y < bounds_max.get()) {
		ofSetLineWidth(1);
		ofSetColor(120);
	}
	else {
		ofSetLineWidth(5);
		ofSetColor(ofColor::red, 30);
	}
	ofDrawLine(tangent.getGlobalPosition(), target.getGlobalPosition());
	ofDrawLine(ee->getGlobalPosition(), target.getGlobalPosition());


	// draw the target
	ofFill();
	ofSetColor(ofColor::red, 200);
	ofDrawEllipse(target.getGlobalPosition(), 50, 50);



	if (debugging) {
		// draw the 1D trajectory and actual position
		draw_cable(tangent.getGlobalPosition(), actual.getGlobalPosition());

		// draw distance to target
		ofSetColor(60);
		string msg = ofToString(glm::distance(tangent.getGlobalPosition(), target.getGlobalPosition())) + " (mm)";
		auto pt = target.getGlobalPosition();
		int offset = 50;
		if (motor->get_motor_id() % 2 == 0) {
			offset *= -5.5;
		}
		ofDrawBitmapString(msg, pt.x + offset, pt.y - 10, pt.z);

		// draw distance to actual
		msg = ofToString(glm::distance(tangent.getGlobalPosition(), actual.getGlobalPosition())) + " (mm)";
		pt = (tangent.getGlobalPosition() + actual.getGlobalPosition()) / 2.0;
		ofDrawBitmapString(msg, pt.x + offset, pt.y - 10, pt.z);
	}

	// draw the trajectory and actual position in (simulated) world coordinates
	if (trajectory_world_coords.getVertices().size() > 0) {
		ofSetLineWidth(5);
		ofSetColor(255, 0, 255, 100);
		trajectory_world_coords.draw();

		float dist = actual.getPosition().y;
		glm::vec3 heading = glm::normalize(tangent.getGlobalPosition() - trajectory_world_coords.getVertices()[0]) * dist;
		actual_world_pos = tangent.getGlobalPosition() + heading;
		draw_cable(tangent.getGlobalPosition(), actual_world_pos);
	}

	//// draw distance to actual in (simulated) world coordinates
	//msg = ofToString(glm::distance(tangent.getGlobalPosition(), actual.getGlobalPosition())) + " (mm)";
	//pt = (tangent.getGlobalPosition() + actual.getGlobalPosition()) / 2.0;
	//ofDrawBitmapString(msg, pt.x + offset, pt.y - 10, pt.z);

	ofPopStyle();


	// update gui
	//info_position_mm.set(ofToString(get_position_actual()));
	//info_position_cnt.set(ofToString(motor->get_position(false)));	// for debugging
}

/**
 * @brief Reset to the center of the bounds.
 */
void CableRobot::reset()
{
	float y = (bounds_min - bounds_max) / 2;
	ee->setPosition(glm::vec3(0, y, 0));
	gizmo_ee.setNode(*ee);
}

void CableRobot::update_gizmo()
{
	if (override_gizmo) {
		gizmo_ee.setNode(*ee);
	}
	else if (gizmo_ee.getTranslation() != ee->getGlobalPosition() ||
		gizmo_ee.getRotation() != ee->getGlobalOrientation()) {
		ee->setGlobalPosition(gizmo_ee.getTranslation());
		ee->setGlobalOrientation(gizmo_ee.getRotation());

		if (gizmo_ee.isInteracting()) {
			// update the gui
			update_move_to();
		}
	}
	// update bounds here too?
}

void CableRobot::update_move_to() {
	float dist = glm::distance(tangent.getGlobalPosition(), target.getGlobalPosition());
	// don't go past the min bounds
	if (dist < bounds_min.get())
		dist = bounds_min.get();
	move_to.set(dist);
}

bool CableRobot::shutdown(int timeout)
{
	// turn off any velocity moves
	move_to_vel.set(false);
	move_type = MoveType::POS;

	ofLogNotice(__FUNCTION__) << "Shutting Down CableRobot " << motor->get_motor_id();
	limit_velocity.set(20);
	limit_acceleration.set(200);

	jog_vel.set(50);
	jog_accel.set(200);
	jog_dist.set(bounds_max.get());
	jog_down();

	auto diff = position_shutdown - bounds_max.get();
	jog_vel.set(10);
	jog_accel.set(50);
	jog_dist.set(diff);
	jog_down(true);

	float timer = ofGetElapsedTimeMillis();
	bool shutting_down = true;
	while (shutting_down) {
		// check if we've timed out
		if (ofGetElapsedTimeMillis() > timer + (timeout * 1000)) {
			ofLogWarning(__FUNCTION__) << "CableRobot " << motor->get_motor_id() << " did not finish shutting down. Timed out after " << timeout << " seconds.";
			return false;
		}
		auto pos = count_to_mm(motor->get_position(), true);
		if (position_shutdown - pos < 1) {
			shutting_down = false;
		}
	}
	// wait a brief moment before disabling the motor
	float start_time = ofGetElapsedTimeMillis();
	float delay = 1500;
	while (ofGetElapsedTimeMillis() < start_time + delay)
	{
	}
	// disbale the motor and update the GUI
	enable.set(shutting_down);
	return true;
}

void CableRobot::key_pressed(int key)
{
	switch (key)
	{
		// trigger stop with SPACEBAR
	case ' ':
		stop();
		stop();
		break;
	case 'r':
	case 'R':
		reset();
		break;
		// minimize all gui groups, expect control
	case '-':
		panel.minimizeAll();
		panel.getGroup("Control").maximize();
		break;
		// maximize all gui groups
	case '+':
		panel.maximizeAll();
		break;
	case '?':
		debugging = !debugging;
		break;
	default:
		break;
	}
}

int CableRobot::get_id()
{
	return motor->get_motor_id();
}

/**
 * @brief Returns whether the target position is in bounds based on absolute coordinates.
 *
 * @param (float)  target_pos_absolute: target position in absolute coordinates.
 * @return (bool)
 */
bool CableRobot::is_in_bounds_absolute(float target_pos_absolute)
{
	return target_pos_absolute >= bounds_min.get() && target_pos_absolute <= bounds_max.get();
}

/**
 * @brief Returns whether the target position would be in bounds based on a relative move.
 *
 * @param (float)  target_pos_relative: (-) for relative move UP and (+) for relative move DOWN
 * @return (bool)
 */
bool CableRobot::is_in_bounds_relative(float target_pos_relative)
{
	int curr_pos = count_to_mm(motor->get_position(), true);
	// (-) relative move is UP
	if (target_pos_relative < 0) {
		return curr_pos - abs(target_pos_relative) >= bounds_min.get();
	}
	// (+) relative move is DOWN
	else {
		return curr_pos + abs(target_pos_relative) <= bounds_max.get();
	}
}

/**
 * @brief Returns whether the target position is in bounds.
 * Use (-) for relative move UP and (+) for relative move DOWN.
 *
 * @param (float)  target_pos: target position in absolute or relative coordinates (in mm).
 * @param (bool) is_aboslute: target_pos is in absolute or relative coordinates
 * @return (bool)
 */
bool CableRobot::is_in_bounds(float target_pos, bool is_absolute)
{
	if (is_absolute) {
		return target_pos >= bounds_min.get() && target_pos <= bounds_max.get();
	}
	else {
		int curr_pos = count_to_mm(motor->get_position(), true);
		// (-) relative move is UP
		if (target_pos < 0) {
			return curr_pos - abs(target_pos) >= bounds_min.get();
		}
		// (+) relative move is DOWN
		else {
			return curr_pos + abs(target_pos) <= bounds_max.get();
		}
	}
}

/**
 * @brief Returns the sign for moving the motor based on the cable drum's groove direction. LEFT_HANDED = -1, RIGHT_HANDED = 1.
 *
 * A homed, left-handed drum operates in negative rotation space.
 * @return (int)  -/+ 1 depending on configuration
 */
int CableRobot::get_rotation_direction()
{
	return drum.direction == Groove::LEFT_HANDED ? -1 : 1;
}

/**
 * @brief Checks if the robot is homed, updates the state,
 * and updates the GUI.
 */
void CableRobot::check_for_system_ready()
{
	RobotState _state;
	ofColor color;

	// check for ESTOP first
	if (is_estopped())
	{
		_state = RobotState::E_STOP;
		color = mode_color_estopped;
		e_stop.set(true);
	}
	else if (!is_homed()) {

		_state = RobotState::NOT_HOMED;
		color = mode_color_not_homed;
	}
	else {
		bool _is_enabled = is_enabled();
		_state = _is_enabled ? RobotState::ENABLED : RobotState::DISABLED;
		color = _is_enabled ? mode_color_enabled : mode_color_disabled;

		// check that the gui matched the current state
		if (enable.get() != _is_enabled)
			enable.set(_is_enabled);
	}
	state = _state;
	status.set(state_names[state]);
	panel.setBorderColor(color);
}

/**
 * @brief Returns whether the motor is ready for a movement command.
 * Motor is ready when there's no E-Stop, it's Homed, and it's Enabled.
 *
 * @return (bool)
 */
bool CableRobot::is_ready()
{
	return is_homed() && is_enabled();
}

void CableRobot::setup_plots()
{
	plot_vel.name = "Motor " + ofToString(get_id()) + ": Raw (RED), Filtered (YELLOW)";
	plot_vel.min = limit_velocity.get() * -1;
	plot_vel.max = limit_velocity.get() * 1;
	plot_vel.colors[0] = ofColor(ofColor::red);
	plot_vel.colors[1] = ofColor::yellow;
}

void CableRobot::setup_gui()
{
	mode_color_estopped = ofColor(250, 0, 0, 100);
	mode_color_not_homed = ofColor::orangeRed;
	mode_color_enabled = ofColor(200, 120);
	mode_color_disabled = ofColor(0, 200);

	int gui_width = 250;

	panel.setup("Motor_" + ofToString(get_id()));
	panel.setWidthElements(gui_width);
	panel.setPosition(10, 15);
	panel.add(status.set("Status", state_names[0]));

	params_control.setName("Control");
	params_control.add(enable.set("Enable", false));
	params_control.add(e_stop.set("E_Stop", false));
	params_control.add(btn_run_homing.set("Run_Homing"));
	params_control.add(btn_run_shutdown.set("Run_Shutdown"));

	params_info.setName("Info");
	params_info.add(info_position_mm.set("Position_(mm)", ""));
	params_info.add(info_position_cnt.set("Position_(cnt)", ""));
	string vel_lim = ofToString(motor->get_velocity());
	string accel_lim = ofToString(motor->get_acceleration());
	params_info.add(info_vel_limit.set("Vel_Limit_(RPM)", vel_lim));
	params_info.add(info_accel_limit.set("Accel_Limit_(RPM/s)", accel_lim));
	params_info.add(info_velocity_target.set("Vel_Target_(RPM)", "0"));
	params_info.add(info_velocity_actual.set("Vel_Actual_(RPM)", "0"));
	params_info.add(info_torque_actual.set("Torque_Actual_(%)", "0"));

	params_boundary.setName("Boundary");
	params_boundary.add(bounds_min.set("Bounds_Min", bounds_min, 0, 3000));
	params_boundary.add(bounds_max.set("Bounds_Max", bounds_max, 0, 4000));

	params_limits.setName("Limits");
	params_limits.add(limit_velocity.set("Vel_Limit_(RPM)", limit_velocity, 0, 300));
	params_limits.add(limit_acceleration.set("Accel_Limit_(RPM/s)", limit_acceleration, 0, 1000));
	params_limits.add(limit_torque_min.set("Torque_Min", limit_torque_min, -5, 10));
	params_limits.add(limit_torque_max.set("Torque_Max", limit_torque_max, 0, 100));

	params_jog.setName("Jogging");
	params_jog.add(jog_vel.set("Jog_Vel", 30, 0, 200));			// RPM
	params_jog.add(jog_accel.set("Jog_Accel", 200, 20, 800));	// RPM_PER_SEC
	params_jog.add(jog_dist.set("Jog_Dist", 50, 0, 1000));		// MM
	params_jog.add(btn_jog_up.set("Jog_Up"));
	params_jog.add(btn_jog_down.set("Jog_Down"));

	params_move.setName("Move");
	int val = (bounds_min.get() + bounds_max.get()) / 2;
	params_move.add(move_to.set("Move_To", val, bounds_min.get(), bounds_max.get()));
	params_move.add(move_to_pos.set("Move_Pos"));
	params_move.add(move_to_vel.set("Move_Vel", false));

	// bind GUI listeners
	e_stop.addListener(this, &CableRobot::on_e_stop);
	enable.addListener(this, &CableRobot::on_enable);
	btn_run_homing.addListener(this, &CableRobot::on_run_homing);
	btn_run_shutdown.addListener(this, &CableRobot::on_run_shutdown);
	move_to_pos.addListener(this, &CableRobot::on_move_to_pos);
	move_to.addListener(this, &CableRobot::on_move_to_changed);
	move_to_vel.addListener(this, &CableRobot::on_move_to_vel);
	btn_jog_up.addListener(this, &CableRobot::on_jog_up);
	btn_jog_down.addListener(this, &CableRobot::on_jog_down);
	limit_velocity.addListener(this, &CableRobot::on_limit_vel_changed);
	limit_acceleration.addListener(this, &CableRobot::on_limit_accel_changed);
	bounds_max.addListener(this, &CableRobot::on_bounds_changed);
	bounds_min.addListener(this, &CableRobot::on_bounds_changed);

	params_motion.setName("Motion");
	params_motion.add(zone.set("Approach_Zone", 50, 0, 300));
	params_motion.add(velocity_controller.params);

	panel.add(params_control);
	panel.add(params_info);
	panel.add(params_boundary);
	panel.add(params_limits);
	panel.add(params_jog);
	panel.add(params_move);
	panel.add(params_motion);

	panel.getGroup("Info").minimize();
	panel.getGroup("Limits").minimize();
	panel.getGroup("Boundary").minimize();
	panel.getGroup("Jogging").minimize();
	panel.getGroup("Motion").minimize();

	is_setup = true;
}

/**
 * @brief Converts from motor position (count) to linear distance (mm).
 *
 * @param (int)  val: motor position (in step counts)
 * @param (bool) use_unsigned: returns abs(val) if true. False by default.
 *
 * @return (float)  linear distance (in mm)
 */
float CableRobot::count_to_mm(int val, bool use_unsigned)
{
	if (use_unsigned)
		return abs(val) * mm_per_count;
	return val * mm_per_count;
}

/**
 * @brief Converts from linear distance (mm) to motor position.
 *
 * @param (float)  val:  linear distance (in mm)
 * @param (bool) use_unsigned: returns abs(val) if true. False by default.
 *
 * @return (int) motor position (in step counts)
 */
int CableRobot::mm_to_count(float val, bool use_unsigned)
{
	if (use_unsigned)
		return abs(val) / mm_per_count;
	return val / mm_per_count;
}

bool CableRobot::is_enabled()
{
	bool val = motor->is_enabled();

	if (is_homed()) {
		if (val)
			panel.setBorderColor(mode_color_enabled);
		else
			panel.setBorderColor(mode_color_disabled);
	}
	return val;
}

bool CableRobot::is_moving()
{
	return motor->is_moving();
}

bool CableRobot::is_estopped()
{
	bool val = motor->is_estopped();
	if (val) {
		panel.setBorderColor(mode_color_estopped);
		state = RobotState::E_STOP;
		status.set(state_names[state]);
	}
	return val;
}

bool CableRobot::is_homed()
{
	bool val = motor->is_homed();
	if (!val) {
		panel.setBorderColor(mode_color_not_homed);
		state = RobotState::NOT_HOMED;
		status.set(state_names[state]);
	}
	return val;
}

/**
 * @brief Runs the robot's homing routine (retracts until it feels a hard stop).
 *
 * @param (int)  timeout: homing routine timeout (in seconds)
 * @return (bool)
 */
bool CableRobot::run_homing_routine(int timeout)
{
	state = RobotState::HOMING;
	status.set(state_names[state]);
	return motor->run_homing_routine(timeout);
}
/**
 * @brief Issues a position move command: converts from mm to motor counts.
 *
 * This is a trapezoidal move, meaning that the motor with accelerate,
 * then decelerate and stop at the target position.
 *
 * NOTE: Use only for Point-to-Point / Waypoint moves. Use move_velocity for
 * streaming smooth trajectories.
 *
 * @param (int)  target_pos: target position (mm).
 * @param (bool)  is_absolute: target is abosulte or relative to current postion. TRUE by default.
 */
void CableRobot::move_position(float target_pos, bool is_absolute)
{
	if (!is_estopped() && is_enabled() && is_homed()) {
		// send move command
		if (is_absolute) {
			// convert from mm to motor counts and flip sign based on cable drum groove direction
			if (is_in_bounds(target_pos, true)) {
				glm::vec3 pos = glm::vec3(0, -1 * target_pos, 0);
				ee->setPosition(pos);
				//target.setPosition(glm::vec3(0, -1 * target_pos, 0));
				int count = mm_to_count(abs(target_pos)) * get_rotation_direction();
				motor->move_position(count, true);
			}
			else
				ofLogWarning(__FUNCTION__) << "Move not sent: The target move would have been out of bounds.";
		}
		else {
			// @TODO
			ofLogWarning(__FUNCTION__) << "Move not sent: Relative move not implemented yet.";
		}
	}
	else {
		string msg = "";
		if (is_estopped())
			msg = "Cannot move Robot " + ofToString(get_id()) + " while in an ESTOP state.";
		else if (!is_homed())
			msg = "Cannot move Robot " + ofToString(get_id()) + ". It must be homed first.";
		else
			msg = "Cannot move Robot " + ofToString(get_id()) + ". It must be enabled first.";
		ofLogWarning(__FUNCTION__) << msg;
	}
}


void CableRobot::draw_cable(glm::vec3 start, glm::vec3 end)
{
	ofPushStyle();
	ofSetColor(255);
	ofSetLineWidth(2);
	ofDrawLine(start, end);
	ofFill();
	ofSetColor(ofColor::orange, 200);
	ofDrawEllipse(end, 40, 40);
	ofPopStyle();
}

void CableRobot::move_velocity_rpm(float rpm)
{
	if (!is_estopped() && is_enabled() && is_homed()) {
		//cout << "RPM from Trajectory: " << rpm << endl;

		//// get whether we're moving up (1) or down (-1)
		//if (trajectory.get_heading().y != 0)
		//	rpm *= trajectory.get_heading().y * -1;
		//// convert for cable drum direction 
		//rpm *= get_rotation_direction();
		// clamp to velocity limit
		//cout << "MOTOR " << get_id() << ", incoming RPM: " << rpm << endl;
		rpm = ofClamp(rpm, -limit_velocity.get(), limit_velocity.get());
		//cout << "\tfinal RPM: " << rpm << endl;

		// send velocity command to motor
		motor->move_velocity(rpm);

		// update the gui
		//info_velocity_target.set(ofToString(rpm));
	}
	else {
		string msg = "";
		if (is_estopped())
			msg = "Cannot move Robot " + ofToString(get_id()) + " while in an ESTOP state.";
		else if (!is_homed())
			msg = "Cannot move Robot " + ofToString(get_id()) + ". It must be homed first.";
		else
			msg = "Cannot move Robot " + ofToString(get_id()) + ". It must be enabled first.";
		ofLogWarning(__FUNCTION__) << msg;
	}
}

float CableRobot::compute_velocity()
{
	// Get distance from actual to desired position
	position_actual = get_position_actual();
	float pos_desired = glm::distance(tangent.getGlobalPosition(), target.getGlobalPosition());
	float dist = abs(pos_desired - position_actual);
	actual_to_desired_distance = dist;
	float heading = (pos_desired > position_actual) ? -1 : 1;

	// Scale velocity to sync with external motors (scalar = 1.0 for 1D configurations)
	// @NOTE: velocity_scalar is updated by 2D / 3D configurations
	float rpm = limit_velocity.get() * velocity_scalar;

	// reset the velocity scalar every time to 1.0
	velocity_scalar = 1.0;

	// Map to 0 when arriving at the target
	float dist_threshold = zone.get();
	if (dist < dist_threshold) {
		rpm = ofMap(dist, dist_threshold, 0, rpm, 0);
	}

	// Set the velocity heading
	rpm *= heading;

	// Update the velocity controller
	velocity_controller.update(rpm);

	float smoothed_val = velocity_controller.get_smoothed_val();
	if (smoothed_val < -1 * velocity_max) {
		smoothed_val = -1 * velocity_max;
	}
	else if (smoothed_val > velocity_max) {
		smoothed_val = velocity_max;
	}

	// record the raw and filtered rpm for visualization
	if (debugging) {
		plot_data_vel[0] = rpm;
		plot_data_vel[1] = smoothed_val;// velocity_controller.get_smoothed_val();
		plot_vel.update(plot_data_vel);
	}


	// return the smoothed velocity
	return smoothed_val;// velocity_controller.get_smoothed_val();
}

void CableRobot::stop()
{
	move_type = MoveType::POS;
	motor->stop();

	// upadate the gui
	info_velocity_target.set("0");
	velocity_controller.reset();
}

void CableRobot::set_e_stop(bool val)
{
	if (val) {
		stop();
		stop();
	}
	motor->set_e_stop(val);
}

void CableRobot::set_enabled(bool val)
{
	motor->set_enabled(val);
}

void CableRobot::on_enable(bool& val)
{
	set_enabled(val);
	if (is_homed()) {
		if (val) {
			panel.setBorderColor(mode_color_enabled);
			state = RobotState::ENABLED;
		}
		else {
			panel.setBorderColor(mode_color_disabled);
			state = RobotState::DISABLED;
		}
		status.set(state_names[state]);
	}
}

/**
 * @brief E Stop triggered / cleared through GUI.
 *
 * @param (bool)  val: triggers or clears an E Stop.
 */
void CableRobot::on_e_stop(bool& val)
{
	set_e_stop(val);
	ofColor color;
	if (val) {
		color = mode_color_estopped;
		state = RobotState::E_STOP;
	}
	else {
		bool _is_enabled = is_enabled();
		state = _is_enabled ? RobotState::ENABLED : RobotState::DISABLED;
		color = _is_enabled ? mode_color_enabled : mode_color_disabled;;
	}
	panel.setBorderColor(color);
	status.set(state_names[state]);
}

/**
 * @brief  Changes the state so the homing routing runs in update() thread.
 *
 */
void CableRobot::on_run_homing()
{
	state = RobotState::HOMING;
	status.set(state_names[state]);
}

/**
 * @brief The Shutdown Routine moves to bounds_max,
 * moves slowly to the shutdown_position,
 * and then disables the motor.
 *
 */
void CableRobot::on_run_shutdown()
{
	int timeout = 20;
	shutdown(timeout);
}

/**
 * @brief Jogs the robot up relative to its current postion.
 * Compensates for sign based on RIGHT- or LEFT-HANDED cable drum.
 * Checks if it is within bounds before sending the move.
 *
 */
void CableRobot::on_jog_up()
{
	if (!is_enabled()) {
		ofLogWarning(__FUNCTION__) << "Enable Motor " << motor->get_motor_id() << " before trying to jog.";
	}
	else {
		// check if the target jogging distance is in bounds
		// (-) jog move is UP
		bool in_bounds = is_in_bounds(jog_dist.get() * -1, false);	// multiply (jog_dist.get() * -1) to signal jog UP direction
		if (in_bounds) {
			// save the current vel and accel values
			float curr_vel_limit = motor->get_velocity();
			float curr_accel = motor->get_acceleration();
			// switch to the jogging vel and accel values
			motor->set_velocity(jog_vel.get());
			motor->set_acceleration(jog_accel.get());
			// send the move
			float curr_pos_mm = count_to_mm(motor->get_position(), true);
			float pos = curr_pos_mm - jog_dist.get();
			move_position(pos, true);
			// switch back to previous vel and accel values
			motor->set_velocity(curr_vel_limit);
			motor->set_acceleration(curr_accel);
		}
		else {
			ofLogWarning(__FUNCTION__) << "Moving CableRobot " << motor->get_motor_id() << " to bounds_min: " << bounds_min.get();
			move_position(bounds_min.get(), true);
		}
	}
}

/**
 * @brief Jogs the robot up relative to its current postions.
 * Compensates for sign based on RIGHT- or LEFT-HANDED cable drum.
 * Checks if it is within bounds before sending the move. Use the @param override to jog past the bounds_max.
 *
 * @param (bool)  override: if true, overrides checking for bounds_max (false by default)
 */
void CableRobot::jog_down(bool override) {
	if (!is_enabled()) {
		ofLogWarning(__FUNCTION__) << "Enable Motor " << motor->get_motor_id() << " before trying to jog.";
	}
	else {
		// check if the target jogging distance is in bounds
		// (+) jog move is DOWN
		bool in_bounds = is_in_bounds(jog_dist.get(), false);
		float curr_pos_mm = count_to_mm(motor->get_position(), true);
		float pos = curr_pos_mm + jog_dist.get();
		if (!override) {
			if (in_bounds) {
				// save the current vel and accel values
				float curr_vel_limit = motor->get_velocity();
				float curr_accel = motor->get_acceleration();
				// switch to the jogging vel and accel values
				motor->set_velocity(jog_vel.get());
				motor->set_acceleration(jog_accel.get());
				// send the move
				move_position(pos, true);
				// switch back to previous vel and accel values
				motor->set_velocity(curr_vel_limit);
				motor->set_acceleration(curr_accel);
			}
			else {
				ofLogWarning(__FUNCTION__) << "Moving CableRobot " << motor->get_motor_id() << " to bounds_max: " << bounds_max.get();
				move_position(bounds_max.get(), true);
			}
		}
		// override bounds_max for shutdown routine; send the move directly to the motor
		else {
			int count = mm_to_count(jog_dist.get()) * get_rotation_direction();
			motor->move_position(count, false);
		}
	}
}

/**
 *  @brief Sets the precision radius for when to move to the next point
 *  in the trajectory. Smaller radius is more precise, but will move slower.
 *
 * @param (float) val: radius of precision zone
 */
void CableRobot::set_zone(float val)
{

}

/**
 * @brief GUI Callback to Jog Down.
 */
void CableRobot::on_jog_down()
{
	jog_down();
}

void CableRobot::on_move_to_changed(float& val)
{
	glm::vec3 pos = glm::vec3(0, -1 * val, 0);
	ee->setPosition(pos);
	gizmo_ee.setNode(*ee);
}

/**
 * @brief GUI Callback to Move To absolute position.
 */
void CableRobot::on_move_to_pos()
{
	move_to_vel.set(false);
	move_type = MoveType::POS;
	float dist = glm::distance(tangent.getGlobalPosition(), target.getGlobalPosition());;
	move_position(dist, true);// move_to.get(), true);
}

void CableRobot::on_move_to_vel(bool& val)
{
	if (val)
		// velocity moves are handled in update()
		move_type = MoveType::VEL;
	else {
		stop();
		stop();
	}
}

void CableRobot::on_bounds_changed(float& val)
{
	move_to.setMin(bounds_min.get());
	move_to.setMax(bounds_max.get());
}

/**
 * @brief Updates the desired velocity limit.
 *
 * @param (float)  val: desired velocity limit (RPM)
 */
void CableRobot::on_limit_vel_changed(float& val)
{
	motor->set_velocity(val);
	// update the gui
	info_vel_limit.set(ofToString(val));
}

/**
 * @brief Updates the desired acceleration limit.
 *
 * @param (float)  val: desired acceleration limit (RPM/s)
 */
void CableRobot::on_limit_accel_changed(float& val)
{
	motor->set_acceleration(val);
	// update the gui
	info_accel_limit.set(ofToString(val));
}


