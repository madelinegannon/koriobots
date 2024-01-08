#pragma once

#include "ofMain.h"
#include "pubSysCls.h"
#include "ClearPathMotor.h"

using namespace sFnd;

class ClearPathMotorController 
{
private:
	SysManager* myMgr;
	vector<ClearPathMotor*> motors;
	string com_port_names = "";
public:
	ClearPathMotorController();
	bool initialize();
	void shutdown();
	vector<ClearPathMotor*> get_motors();
	string get_com_ports();
};
