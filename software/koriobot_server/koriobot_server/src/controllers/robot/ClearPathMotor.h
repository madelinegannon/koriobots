#pragma once

#include "ofMain.h"
#include <time.h>
#include "pubSysCls.h"

using namespace sFnd;

class ClearPathMotor
{
private:
    INode* m_node;				
    SysManager& m_sysMgr;

public:
    ClearPathMotor(SysManager& SysMgr, INode* node);
    ~ClearPathMotor();

    INode* get() { return m_node; };
    int get_resolution();

    void set_motion_params(float imit_velocity=200, float imit_acceleration=400, int limit_trq_percent=100);
    void enable();
    void disable();
    void stop(nodeStopCodes stop_type=STOP_TYPE_ABRUPT);
    void set_e_stop(bool val);
    void set_enabled(bool val);

    int get_position(bool get_actual_pos=true);

    float get_velocity();
    float get_velocity_actual();
    void set_velocity(float val);
    float get_acceleration();
    void set_acceleration(float val);

    void move_position(int target_pos, bool is_absolute, bool add_dwell=false);
    void move_velocity(float target_vel);

    bool is_estopped();
    bool is_homed();
    bool is_enabled();
    bool is_moving();

    bool run_homing_routine(int _timeout=20);

    int get_motor_id() {
        return int(get()->Info.Ex.Addr());
    }
};