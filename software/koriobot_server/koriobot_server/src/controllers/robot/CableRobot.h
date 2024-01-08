#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "ofxGizmo.h"
#include "CableDrum.h"
#include "ClearPathMotor.h"

#include "../TimeSeriesPlot.h"
#include "../PD_Controller.h"


class CableRobot
{
private:
    ofxXmlSettings config;

    // Motion Parameters
    float velocity_max = 300;   // RPM
    float accel_max = 800;      // RPM_PER_SEC
    float position_min = 0;     // absolute position in mm
    float position_max = 2000;  // absolute position in mm
    float position_shutdown = 2140;  // absolute position in mm

    bool is_in_bounds_absolute(float target_pos_absolute);
    bool is_in_bounds_relative(float target_pos_relative);
    bool is_in_bounds(float target_pos, bool absolute);

    int get_rotation_direction();

    float desired_velocity = 0.0;

    // Kinematics
    ofNode* origin = new ofNode();     // external parent node (in world coordinates)
    ofNode* ee = new ofNode();         // external parent ee node
    ofNode base = ofNode();        // center of cable drum
    ofNode tangent = ofNode();     // tangent point on the cable drum
    ofNode target = ofNode();      // desired ee position
    ofNode actual = ofNode();      // actual ee position
   
    void setup_plots();


    ClearPathMotor* motor;

    void setup_gui();
    ofxGizmo gizmo_ee;

    bool shutdown(int timeout=20);

    float mm_per_count;
    float count_to_mm(int val, bool use_unsigned = false);
    int mm_to_count(float val, bool use_unsigned = false);

    void draw_cable(glm::vec3 _anchor, glm::vec3 _target);

    enum RobotState {
        NOT_HOMED,
        HOMING,
        ENABLED,
        DISABLED,
        E_STOP
    };
    RobotState state = RobotState::NOT_HOMED;
    string state_names[5] = { "NOT_HOMED", "HOMING", "ENABLED", "DISABLED", "E_STOP"};

    bool auto_home = false;
    bool debugging = true;

    enum MoveType {
        POS,
        VEL
    };
    MoveType move_type = MoveType::POS;    
    float ee_offset = 0;
    ofRectangle bounds_motion;
public:

    CableRobot(ClearPathMotor* motor, ofxXmlSettings config);

    CableDrum drum = CableDrum();

    ofxGizmo* get_gizmo() { return &gizmo_ee; }
    bool override_gizmo = false;
    void update_gizmo();
    void update_move_to();

    void update();
    void draw();
    void reset();
    void key_pressed(int key);
    int get_id();
    void configure(ofxXmlSettings config);    

    void check_for_system_ready();
    bool is_ready();

    bool is_setup = false;

    float position_actual = 0; // in mm (+) val only
    float get_position_actual();
    vector<float> get_motion_parameters();
    void set_motion_parameters(float velocity_max, float accel_max, float position_min, float position_max);
   
    vector<float> get_jogging_parameters();
    void set_jogging_parameters(float jogging_vel, float jogging_accel, float jogging_dist);

    void set_velocity_limit(float velocity_max);
    void set_accel_limit(float accel_max);
    void set_bounds(float min, float max);

    ofNode get_tangent() { return tangent; }
    ofNode* get_tangent_ptr() { return &tangent; }
    ofNode* get_target() { return &target; }
    void set_ee(ofNode* _ee) { this->ee = _ee; }
    ofPolyline trajectory_world_coords;
    glm::vec3 actual_world_pos;

    ClearPathMotor* get_motor() { return motor; }

    ofNode get_base() { return base; }
    void set_base_position(glm::vec3 pos) { base.setPosition(pos); }

    bool is_estopped();
    bool is_homed();
    bool is_enabled();
    bool is_moving();
    bool is_torque_in_limits();
    bool run_homing_routine(int timeout=20);

    void move_position(float target_pos, bool absolute = true);
    void move_velocity(float target_pos);
    void move_velocity_rpm(float rpm);
    void set_desired_velocity(float rpm);
    float compute_velocity();
    float velocity_scalar = 1.0;
    float actual_to_desired_distance = 0;
    PD_Controller velocity_controller;

    TimeSeriesPlot plot_vel = TimeSeriesPlot(2);
    vector<float> plot_data_vel = { 0, 0 };

    void stop();
    void set_e_stop(bool val);
    void set_enabled(bool val);

    void jog_down(bool override = false);

    void set_zone(float val);

    // GUI and Listeners
    void on_enable(bool& val);
    void on_e_stop(bool& val);
    void on_run_homing();
    void on_run_shutdown();
    void on_jog_up();
    void on_jog_down();
    void on_move_to_changed(float& val);
    void on_move_to_pos();
    void on_move_to_vel(bool& val);
    void on_bounds_changed(float& val);
    
    ofxPanel panel;
    ofParameterGroup params_control;
    ofParameter<string> status;
    ofParameter<bool> enable = false;
    ofParameter<bool> e_stop = false;
    ofParameter<void> btn_run_homing;
    ofParameter<void> btn_run_shutdown;

    ofParameterGroup params_info;
    ofParameter<string> info_position_mm;
    ofParameter<string> info_position_cnt;
    ofParameter<string> info_vel_limit;
    ofParameter<string> info_accel_limit;
    ofParameter<string> info_velocity_target;
    ofParameter<string> info_velocity_actual;
    ofParameter<string> info_torque_actual;

    ofParameterGroup params_move;
    ofParameter<float> move_to;
    ofParameter<void> move_to_pos;
    ofParameter<bool> move_to_vel;

    ofParameterGroup params_motion;
    ofParameter<float> zone;

    ofParameterGroup params_boundary;
    ofParameter<float> bounds_min = 100;
    ofParameter<float> bounds_max = 1800;

    ofParameterGroup params_limits;
    ofParameter<float> limit_velocity = 30;
    ofParameter<float> limit_acceleration= 200;
    ofParameter<float> limit_torque_min;
    ofParameter<float> limit_torque_max;
    void on_limit_vel_changed(float& val);
    void on_limit_accel_changed(float& val);
    void on_limit_torque_changed(float& val);

        ofParameterGroup params_jog;
    ofParameter<float> jog_vel = 20;
    ofParameter<float> jog_accel = 200;
    ofParameter<float> jog_dist;
    ofParameter<void> btn_jog_up;
    ofParameter<void> btn_jog_down;

    ofParameterGroup params_homing;
    ofParameter<string> homing_status;
    ofParameter<bool> rehome;
    ofParameter<bool> reset_home_position;

    ofColor mode_color_enabled;
    ofColor mode_color_disabled;
    ofColor mode_color_not_homed;
    ofColor mode_color_estopped;

    
};