#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
//#include "CanObj.h"
#include "nativeui/nativeui.h"
// #include "base/command_line.h"

struct ObjectInfo_77{
    int8_t ID;
    float Range;
    float RadialVelocity;
    int8_t Amplitude;
    int8_t Azimuth;
};

struct SonarData
{
    unsigned short left_front;
    unsigned short right_front;

    unsigned short leftside_front;
    unsigned short rightside_front;

    unsigned short leftside_rear;
    unsigned short rightside_rear;

    unsigned short left_rear;
    unsigned short right_rear;
};

class RadarView
{
public:
    RadarView();
    ~RadarView();

    typedef std::vector<ObjectInfo_77> objects_t;
    void run();
    void sonarUpdate(const SonarData& sonar);
    void radarUpdate(const int chl, const std::vector<ObjectInfo_77> &objs);  
private:
    
    void redraw();

    scoped_refptr<nu::Window> m_pwindow;
    scoped_refptr<nu::Container> radarView;

    SonarData latestSonar;
    std::unordered_map<int, objects_t> m_objs_map;

    void updateSonarArc(nu::Painter *painter);
    void updateRadarDetectedObjects(nu::Painter *painter);
};
