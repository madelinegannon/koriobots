#include "ClearPathMotor.h"

ClearPathMotor::ClearPathMotor(SysManager& SysMgr, INode* node):
	m_node(node),
	m_sysMgr(SysMgr) {

	printf("   Node[%d]: type=%d\n", m_node->Info.Ex.Addr(), m_node->Info.NodeType());
	printf("            userID: %s\n", m_node->Info.UserID.Value());
	printf("        FW version: %s\n", m_node->Info.FirmwareVersion.Value());
	printf("          Serial #: %d\n", m_node->Info.SerialNumber.Value());
	printf("             Model: %s\n", m_node->Info.Model.Value());
	printf("        Resolution: %d\n", m_node->Info.PositioningResolution.Value());
	printf("      Is E-Stopped: %s\n", is_estopped() ? "TRUE" : "FALSE");
	printf("        Is Enabled: %s\n", is_enabled() ? "TRUE" : "FALSE");
	printf("          Is Homed: %s\n", m_node->Motion.Homing.WasHomed() ? "TRUE":"FALSE");
	printf("  Current Position: %d\n\n", get_position());

	set_motion_params();
}

ClearPathMotor::~ClearPathMotor() {
	// Disable the node and wait for it to disable
	m_node->EnableReq(false);

	// Poll the status register to confirm the node's disable
	time_t timeout;
	timeout = time(NULL) + 3;
	m_node->Status.RT.Refresh();
	while (m_node->Status.RT.Value().cpm.Enabled) {
		if (time(NULL) > timeout) {
			printf("Error: Timed out waiting for disable\n");
			return;
		}
		m_node->Status.RT.Refresh();
	};
}

int ClearPathMotor::get_resolution()
{
	return m_node->Info.PositioningResolution.Value();;
}

void ClearPathMotor::set_motion_params(float imit_velocity, float imit_acceleration, int limit_trq_percent)
{
	// Set the user units to RPM and RPM/s
	m_node->VelUnit(INode::RPM);
	m_node->AccUnit(INode::RPM_PER_SEC);
	m_node->TrqUnit(INode::PCT_MAX);
	
	uint32_t jerk_limit = 3;

	m_node->Motion.VelLimit.Value(imit_velocity);
	m_node->Motion.AccLimit.Value(imit_acceleration);
	m_node->Motion.JrkLimit.Value(jerk_limit);
	m_node->Limits.PosnTrackingLimit.Value(m_node->Info.PositioningResolution.Value() / 4);
	m_node->Limits.TrqGlobal.Value(limit_trq_percent);

	m_node->Motion.TrqMeasured.AutoRefresh(true);
	m_node->Motion.VelMeasured.AutoRefresh(true);
	m_node->Motion.PosnMeasured.AutoRefresh(true);
}

void ClearPathMotor::enable()
{
	if (m_node != NULL) {
		// Clear alerts and node stops
		m_node->Status.AlertsClear();
		m_node->Motion.NodeStop(STOP_TYPE_ABRUPT);
		m_node->Motion.NodeStop(STOP_TYPE_CLR_ALL);

		// Enable the node
		m_node->EnableReq(true);

		// If the node is not currently ready, wait for it to get there
		time_t timeout;
		timeout = time(NULL) + 3;
		// Basic mode - Poll until disabled
		while (!m_node->Status.IsReady()) {
			if (time(NULL) > timeout) {
				ofLogWarning("ClearPathMotor::Enable()") << "Error: Timed out waiting for enable";
				return;
			}
		}
	}
}

void ClearPathMotor::disable()
{
	// Disable the node and wait for it to disable
	m_node->EnableReq(false);

	// Poll the status register to confirm the node's disable
	time_t timeout;
	timeout = time(NULL) + 3;
	m_node->Status.RT.Refresh();
	while (m_node->Status.RT.Value().cpm.Enabled) {
		if (time(NULL) > timeout) {
			printf("Error: Timed out waiting for disable\n");
			return;
		}
		m_node->Status.RT.Refresh();
	};
}

void ClearPathMotor::stop(nodeStopCodes stop_type)
{
	ofLogNotice("ClearPathMotor::stop") << "Stopping ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << ".";
	m_node->Motion.NodeStop(stop_type);		// this should clear buffer and stop, but not working with velocity move
	m_node->Motion.NodeStop(stop_type);
	m_node->Motion.MoveVelStart(0);			// ensure stop by setting velocity to zero
	m_node->Motion.MoveVelStart(0);
	m_node->Status.RT.Refresh();
}

void ClearPathMotor::set_e_stop(bool val)
{
	if (val) {
		ofLogNotice("ClearPathMotor::set_e_stop") << "Triggering E-Stop for ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << ".";
		m_node->Motion.NodeStop(STOP_TYPE_ESTOP_ABRUPT);
	}
	// Clear the E-Stop
	else {
		// Update the registers
		m_node->Status.RT.Refresh();
		m_node->Status.Alerts.Refresh();
		if (m_node->Status.Alerts.Value().cpm.Common.EStopped) {
			ofLogNotice("ClearPathMotor::clearMotionStop") << "Clearing E-Stop for ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << ".";
			m_node->Motion.NodeStopClear();
		}
		else {
			ofLogNotice("ClearPathMotor::clearMotionStop") << "ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << " is not E-Stopped.";
		}
	}
}

void ClearPathMotor::set_enabled(bool val)
{
	if (val && !is_enabled()) {
		ofLogNotice("ClearPathMotor::set_enabled") << "Enabling ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << ".";
		enable();
	}
	else if (!val) {
		ofLogNotice("ClearPathMotor::set_enabled") << "Disabling ClearPathMotor " << ofToString(m_node->Info.Ex.Addr()) << ".";
		disable();
	}
}

/**
 * @brief Returns either the target or actual motor position (in counts).
 * 
 * @param (bool)  get_actual_pos: returns the current actual position if true, the target position if false. True by default.
 * 
 * @return (int) motor position (in counts) 
 */
int ClearPathMotor::get_position(bool get_actual_pos)
{
	if (get_actual_pos) {
		m_node->Motion.PosnMeasured.Refresh();
		return int64_t(m_node->Motion.PosnMeasured.Value());
	}
	else {
		m_node->Motion.PosnCommanded.Refresh();
		return int64_t(m_node->Motion.PosnCommanded.Value());
	}
}

/**
 * @brief Get the desired velocity.
 * 
 * @return (float) desired velocity (RPM) 
 */
float ClearPathMotor::get_velocity()
{
	return m_node->Motion.VelLimit.Value();
}

/**
 * @brief Get the actual, measured velocity.
 * 
 * @return (float)  actual velocity (RPM)
 */
float ClearPathMotor::get_velocity_actual()
{
	return m_node->Motion.VelMeasured.Value();
}

/**
 * @brief Set the desired velocity.
 * 
 * @param (float)  val: desired velocity (RPM)
 */
void ClearPathMotor::set_velocity(float val)
{
	m_node->Motion.VelLimit.Value(val);
}

/**
 * @brief Get the desired acceleration.
 *
 * @return (float)  desired acceleration (RPM/s)
 */
float ClearPathMotor::get_acceleration()
{
	return m_node->Motion.AccLimit.Value();
}

/**
 * @brief Set the desired acceleration.
 * 
 * @param (float)  val: desired acceleration (RPM/s)
 */
void ClearPathMotor::set_acceleration(float val)
{
	m_node->Motion.AccLimit.Value(val);
}

/**
 * @brief Issues a trapezoidal position move. Will come to complete stop when target is reached.
 *
 * @param (int)  target_pos: target motor position in motor counts.
 * @param (bool)  is_absolute: target is abosulte or relative to current postion.
 * @param (bool)  add_dwell: whether to delay the next move being issued by DwellMs after this move profile completes.
 */
void ClearPathMotor::move_position(int target_pos, bool is_absolute, bool add_dwell)
{
	m_node->Motion.MovePosnStart(target_pos, is_absolute, add_dwell);
}

void ClearPathMotor::move_velocity(float target_vel)
{		
	// check that there is space in the motor's move buffer & then send vel command
	m_node->Status.RT.Refresh();
	if (m_node->Status.RT.Value().cpm.MoveBufAvail) {
		// filter out smalled changes
		float epsilon = 0.01;	
		if (abs(target_vel - get_velocity_actual()) >  epsilon)
			m_node->Motion.MoveVelStart(target_vel);
	}
	else {
		ofLogNotice(__FUNCTION__) << "ClearPathMotor " << ofToString(int(m_node->Info.Ex.Addr())) << ": Move Buffer Full. TimeStamp: " << ofGetElapsedTimeMillis() << endl;
	}
	if (target_vel == 0) {
		ofLogNotice(__FUNCTION__) << "stopping due to 0 RPM." << endl;
		stop();
	}
}

bool ClearPathMotor::is_moving()
{
	m_node->Status.RT.Refresh();
	return m_node->Status.RT.Value().cpm.InMotion != 0;
}

bool ClearPathMotor::is_enabled()
{
	m_node->Status.RT.Refresh();
	return m_node->Status.RT.Value().cpm.Enabled;
}

bool ClearPathMotor::is_estopped()
{
	m_node->Status.Alerts.Refresh();
	return m_node->Status.Alerts.Value().cpm.Common.EStopped;;
}

bool ClearPathMotor::is_homed()
{
	return m_node->Motion.Homing.WasHomed();
}

bool ClearPathMotor::run_homing_routine(int _timeout)
{
	// Enable the ClearPathMotor, if it's not already
	if (!is_enabled()) {
		enable();
	}
	// set a timeout(ms) in case the node is unable to home
	double timeout = m_sysMgr.TimeStampMsec() + (_timeout * 1000);	

	cout << "Initiating homing. Timeout in " << _timeout << " seconds." << endl;
	m_node->Motion.Homing.Initiate();
	while (!m_node->Motion.Homing.WasHomed()) {
		if (m_sysMgr.TimeStampMsec() > timeout) {
			printf("Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move.\n");
			return false;
		}
	}
	m_node->Motion.PosnMeasured.Refresh();		//Refresh our current measured position
	printf("Node completed homing, current position: \t%8.0f \n", m_node->Motion.PosnMeasured.Value());
	
	return true;
}
