#include "ClearPathMotorController.h"

ClearPathMotorController::ClearPathMotorController()
{
}

/**
 * @brief Create the System Manger, then setup and open the port.
 * Logs how many nodes it finds on the port.
 *
 * @return (bool) True if the system was initialized properly.
 *				  False if there was a problem accessing the port.
 *
 */
bool ClearPathMotorController::initialize()
{
	// Create the CPM System Manager
	myMgr = SysManager::Instance();
	try
	{
		// Find how many SC-Hub Ports are connected
		int portCount = 0;
		vector<string> comHubPorts;
		SysManager::FindComHubPorts(comHubPorts);
		ofLogNotice(__FUNCTION__) << "Found " << ofToString(comHubPorts.size()) << " SC Hubs";

		// Define the first SC Hub port (port 0) to be associated with COM portnum (as seen in windows device manager)
		for (portCount = 0; portCount < comHubPorts.size() && portCount < NET_CONTROLLER_MAX; portCount++) {
			myMgr->ComHubPort(portCount, comHubPorts[portCount].c_str());

			// save the name of the com port for display in the gui
			com_port_names += comHubPorts[portCount].c_str();
			if (portCount != comHubPorts.size() - 1) {
				com_port_names += " , ";
			}
		}

		// Initialize and save each motor on the port
		if (portCount > 0) {
			// Open the port
			myMgr->PortsOpen(portCount);
			for (size_t i = 0; i < portCount; i++) {
				IPort& myPort = myMgr->Ports(i);
				ofLogNotice(__FUNCTION__) << "\tSTATUS: Port " << myPort.NetNumber() << ", state=" << myPort.OpenState() << ", nodes=" << myPort.NodeCount();
				for (size_t j = 0; j < myPort.NodeCount(); j++) {
					motors.push_back(new ClearPathMotor(*myMgr, &myPort.Nodes(j)));
				}
			}
		}
		else {
			ofLogWarning() << "Unable to locate any SC hub ports.\n\tCheck that ClearView is closed and no other Clearpath applications are running.";
			return false;
		}
	}
	catch (mnErr& theErr)
	{
		ofLogError(__FUNCTION__) << "Port Failed to open, Check to ensure correct Port number and that ClearView is not using the Port";
		ofLogError(__FUNCTION__) << "Caught error: addr=" << ofToString(theErr.TheAddr) << ", err=" << ofToString(theErr.ErrorCode) << ", msg=" << ofToString(theErr.ErrorMsg);
		return false;
	}
	return true;
}

void ClearPathMotorController::shutdown()
{
	if (myMgr != nullptr) {
		ofLogNotice() << "Closing HUB Ports...";
		myMgr->PortsClose();
	}
}

vector<ClearPathMotor*> ClearPathMotorController::get_motors()
{
	return motors;
}

string ClearPathMotorController::get_com_ports()
{
	return com_port_names;
}
