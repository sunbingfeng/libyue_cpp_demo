// This file is published under public domain.
#include <iostream>
#include <cmath>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <cstddef>

//#include "radar_view.h"

#include "base/command_line.h"
// #include "string_number_conversions.h"

#include "nativeui/nativeui.h"

#include <observable/observable.hpp>

struct ObjectInfo_77{
    float Range;
    float Azimuth;
};


struct SonarData
{
    float left;
    float right;
};

const static float pi = 3.1415926;
static const float window_width = 600;
static const float window_height = 600;
static const float unit = 2;  // xx cm equals 1 pixel

static const float radar_view_size = window_width;

static const float width = 102 / unit; // 102 cm
static const float height = 175 / unit; // 175cm
static const float center_x = window_width / 2, center_y = window_height / 2;
static const float sonar_angle_range = pi/3;


class TestModel
{
  OBSERVABLE_PROPERTIES(TestModel)
public:
    // data-driven heartbeat
    observable_property<bool> dataHB { false };

public:
  typedef std::vector<ObjectInfo_77> objects_t;

  TestModel()
  {
    std::srand(std::time(nullptr)); // use current time as seed for random generator

    pAmpUpdateTh = new std::thread(&TestModel::gen_amp, this);
    pAmpUpdateTh->detach();
  }

  ~TestModel()
  {

  }

  SonarData m_sonar;
  std::unordered_map<int, objects_t> m_objs_map;

private:
  std::thread *pAmpUpdateTh;
  bool _toggle = false;

  void gen_amp(void)
  {
    while(1)
    {
      int cnt = std::rand() % 10;
      objects_t front, rear;
      for (int i = 0; i < cnt; ++i)
      {
          float Range = 0.1f * (std::rand() % 2000);  // cm
          float Azimuth = ((std::rand() % 180) - 90) * pi / 180;        
          front.push_back(ObjectInfo_77({Range,Azimuth}));
      }
      m_objs_map[0] = front;

      cnt = std::rand() % 10;
      for (int i = 0; i < cnt; ++i)
      {
          float Range = 0.1f * (std::rand() % 2000);  // cm
          float Azimuth = ((std::rand() % 180) - 90) * pi / 180;        
          rear.push_back(ObjectInfo_77({Range,Azimuth}));
      }
      m_objs_map[1] = rear;
      
      m_sonar = {
        0.1f * (std::rand() % 2000),
        0.1f * (std::rand() % 2000)
      };

      _toggle = !_toggle;

      dataHB = _toggle;

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
};

void drawSonarArc(nu::Painter *painter, nu::PointF center, float radius, float sa, float ea)
{
  painter->Save();

  painter->SetStrokeColor(nu::Color("#2029E9"));

  painter->BeginPath();
  painter->MoveTo(center);
  painter->LineTo(nu::PointF(center.x() + radius * std::cos(sa), center.y() +  + radius * std::sin(sa)));
  painter->Arc(center, radius, sa, ea);
  painter->ClosePath();
  painter->Stroke();

  painter->Restore();
}

void drawRadarObstacle(nu::Painter *painter, nu::PointF center, float radius)
{
  painter->Save();

  painter->SetStrokeColor(nu::Color("#550000")); //"#550000"
  painter->SetFillColor(nu::Color("#DD0000"));  //"#DD0000"

  painter->BeginPath();
  painter->Arc(center, radius, 0, 2*pi);
  painter->ClosePath();

  painter->Fill();

  painter->Restore();
}


void drawCar(nu::Painter *painter)
{
  painter->Save();

  painter->SetStrokeColor(nu::Color("#000000"));
  nu::RectF bounds = nu::RectF(-width/2, -height/2, width, height);
  bounds.Offset(center_x, center_y);
   
  painter->StrokeRect(bounds);

  painter->Restore();
}


void updateSonarArc(nu::Painter *painter, TestModel *model)
{
  static const float frontsonar_space = 38 / unit; // 38 cm
  static const float rearsonar_space = 44 / unit; // 44 cm
  static const float sidesonar_space = 125 / unit; // 125 cm

  // left sonar
  drawSonarArc(painter, nu::PointF(center_x - width/2, center_y), model->m_sonar.left/unit, pi - sonar_angle_range/2, pi + sonar_angle_range/2);

  // right sonar
  drawSonarArc(painter, nu::PointF(center_x + width/2, center_y), model->m_sonar.right/unit, 2*pi - sonar_angle_range/2, sonar_angle_range/2);
}

void updateRadarDetectedObjects(nu::Painter *painter, TestModel *model)
{
  std::vector<nu::PointF> pts;
  for (std::vector<ObjectInfo_77>::iterator itr = model->m_objs_map[0].begin(); itr != model->m_objs_map[0].end(); ++itr)
  {
    float r = (*itr).Range/unit;
    float thelta = (*itr).Azimuth;

    pts.push_back(nu::PointF(center_x + r * std::sin(thelta), center_y - height/2 - r * std::cos(thelta)));     
  }

  for (std::vector<ObjectInfo_77>::iterator itr = model->m_objs_map[1].begin(); itr != model->m_objs_map[1].end(); ++itr)
  {
    float r = (*itr).Range/unit;
    float thelta = (*itr).Azimuth;

    pts.push_back(nu::PointF(center_x - r * std::sin(thelta), center_y + height/2 + r * std::cos(thelta)));
  }

  for(auto pt : pts)
  {
     drawRadarObstacle(painter, pt, 5);
  }
}

#if defined(OS_WIN)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  base::CommandLine::Init(0, nullptr);
#else
int main(int argc, const char *argv[]) {
  base::CommandLine::Init(argc, argv);
#endif

  // base::CommandLine::Init(argc, argv);

  std::srand(std::time(nullptr)); // use current time as seed for random generator
  TestModel model;

  // Create GUI message loop.
  nu::Lifetime lifetime;

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create window with default options, and then show it.
  scoped_refptr<nu::Window> window(new nu::Window(nu::Window::Options()));

  model.dataHB.subscribe([&](auto hb){
    window->GetContentView()->Layout();
    window->GetContentView()->SchedulePaint();
  }); 

  scoped_refptr<nu::Container> radar_view(new nu::Container);
  radar_view->SetStyle("position", "absolute", "width", window_width, "height", window_height, "top", 0, "right", 0);

  radar_view->on_draw.Connect([&](nu::Container* self, nu::Painter* painter, const nu::RectF& dirty){
    drawCar(painter);
    updateSonarArc(painter, &model);  
    updateRadarDetectedObjects(painter, &model);
  });
  
  window->SetResizable(false);
  window->SetContentView(radar_view.get());
  window->SetContentSize(nu::SizeF(window_width, window_height));
  window->Center();
  window->Activate();

  // Quit when window is closed.
  window->on_close.Connect([&state](nu::Window*) {
    nu::MessageLoop::Quit();
  });

  // Enter message loop.
  nu::MessageLoop::Run();

  return 0;

}




