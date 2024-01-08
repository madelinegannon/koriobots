#pragma once

#include "ofMain.h"

enum Groove {
    NONE = 0,
    LEFT_HANDED, // UP=CCW(+), DOWN=CW(-)
    RIGHT_HANDED // UP=CW(-),  DOWN=CCW(+)
};

class CableDrum
{
private:
public:
    float diameter_drum = 99.95;
    float diameter_cable = 0.3048;
    float length = 30.0;
    int turns = 40;
    glm::vec3 tangent_pt = glm::vec3(0,0,0);
    CableDrum(){ direction = Groove::LEFT_HANDED; };
    void initialize(Groove direction, float diameter_drum = 99.95, float length = 30, int turns = 30);
    void draw();
    Groove direction = Groove::NONE;
    float get_diameter() { return diameter_drum; };
    void set_diameter(float val) { diameter_drum = val; }

    glm::vec3 get_tangent() { return tangent_pt; };
    void set_tangent(glm::vec3 tangent) {
        tangent_pt.x = tangent.x;
        tangent_pt.y = tangent.y;
        tangent_pt.z = tangent.z;
    }
    
    float circumference = 0;
};