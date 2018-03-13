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

const static float pi = 3.1415926;
static const float window_width = 600;
static const float window_height = 700;
static const float unit = 2;  // xx cm equals 1 pixel


static const float tool_bar_width = 100;
static const float radar_view_size = 500;
static const float log_view_height = window_height - radar_view_size;

static const float width = 102 / unit; // 102 cm
static const float height = 175 / unit; // 175cm
static const float center_x = radar_view_size / 2, center_y = radar_view_size / 2;
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
          float Range = 0.1 * (std::rand() % 2000);  // cm
          float Azimuth = ((std::rand() % 180) - 90) * pi / 180;        
          front.push_back(ObjectInfo_77({0,Range,0.0f,0,Azimuth}));
      }
      m_objs_map[0] = front;

      cnt = std::rand() % 10;
      for (int i = 0; i < cnt; ++i)
      {
          float Range = 0.1 * (std::rand() % 2000);  // cm
          float Azimuth = ((std::rand() % 180) - 90) * pi / 180;        
          rear.push_back(ObjectInfo_77({0,Range,0.0f,0,Azimuth}));
      }
      m_objs_map[1] = rear;
      
      m_sonar = {
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000),
        0.1 * (std::rand() % 2000)
      };

      _toggle = !_toggle;

      dataHB = _toggle;

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
};


void drawLine(nu::Painter *painter) {
  painter->Save();

  painter->SetStrokeColor(nu::Color("#550000"));
  painter->SetFillColor(nu::Color("#D46A6A"));
  painter->SetLineWidth(20);
  painter->BeginPath();
  painter->MoveTo(nu::PointF(25, 25));
  painter->LineTo(nu::PointF(105, 25));
  // painter->LineTo(nu::PointF(25, 105));
  painter->ClosePath();
  painter->Stroke();

  painter->Restore();
}


void drawSonarArc(nu::Painter *painter, nu::PointF center, float radius, float sa, float ea)
{
  painter->SetStrokeColor(nu::Color("#2029E9"));

  painter->BeginPath();
  painter->MoveTo(center);
  painter->LineTo(nu::PointF(center.x() + radius * std::cos(sa), center.y() +  + radius * std::sin(sa)));
  painter->Arc(center, radius, sa, ea);
  painter->ClosePath();
  painter->Stroke();
}

void drawRadarObstacle(nu::Painter *painter, nu::PointF center, float radius)
{
  painter->SetStrokeColor(nu::Color("#550000")); //"#550000"
  painter->SetFillColor(nu::Color("#DD0000"));  //"#DD0000"

  painter->BeginPath();
  painter->Arc(center, radius, 0, 2*pi);
  painter->ClosePath();

  painter->Fill();
}



void drawRadarRange(nu::Painter *painter)
{
  painter->SetStrokeColor(nu::Color("#E92117"));

  painter->BeginPath();
  painter->Arc(nu::PointF(center_x, center_y - height / 2), 300/unit, pi, 2*pi);  // 3m range
  painter->Arc(nu::PointF(center_x, center_y - height / 2), 500/unit, pi, 2*pi);  // 5m range
  painter->ClosePath();

  painter->Stroke();
}

void drawCar(nu::Painter *painter)
{
  painter->SetStrokeColor(nu::Color("#000000"));
  nu::RectF bounds = nu::RectF(-width/2, -height/2, width, height);
  bounds.Offset(center_x, center_y);
   
  painter->StrokeRect(bounds);
}


void updateSonarArc(nu::Painter *painter, TestModel *model)
{
  static const float frontsonar_space = 38 / unit; // 38 cm
  static const float rearsonar_space = 44 / unit; // 44 cm
  static const float sidesonar_space = 125 / unit; // 125 cm

  // front sonar
  drawSonarArc(painter, nu::PointF(center_x - frontsonar_space/2, center_y - height/2), model->m_sonar.left_front/unit, 3*pi/2 - sonar_angle_range/2, 3*pi/2 + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + frontsonar_space/2, center_y - height/2), model->m_sonar.right_front/unit, 3*pi/2 - sonar_angle_range/2, 3*pi/2 + sonar_angle_range/2);

  // rear sonar
  drawSonarArc(painter, nu::PointF(center_x - frontsonar_space/2, center_y + height/2), model->m_sonar.left_rear/unit, pi/2 - sonar_angle_range/2, pi/2 + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + frontsonar_space/2, center_y + height/2), model->m_sonar.right_rear/unit, pi/2 - sonar_angle_range/2, pi/2 + sonar_angle_range/2);

  // left side sonar
  drawSonarArc(painter, nu::PointF(center_x - width/2, center_y - sidesonar_space/2), model->m_sonar.leftside_front/unit, pi - sonar_angle_range/2, pi + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x - width/2, center_y + sidesonar_space/2), model->m_sonar.leftside_rear/unit, pi - sonar_angle_range/2, pi + sonar_angle_range/2);

  // right side sonar
  drawSonarArc(painter, nu::PointF(center_x + width/2, center_y - sidesonar_space/2), model->m_sonar.rightside_front/unit, 2*pi - sonar_angle_range/2, sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + width/2, center_y + sidesonar_space/2), model->m_sonar.rightside_rear/unit, 2*pi - sonar_angle_range/2, sonar_angle_range/2);
}

void updateRadarDetectedObjects(nu::Painter *painter, TestModel *model)
{
  std::vector<nu::PointF> pts;
  for (std::vector<ObjectInfo_77>::iterator itr = model->m_objs_map[0].begin(); itr != model->m_objs_map[0].end(); ++itr)
  {
    float r = (*itr).Range/unit;
    float thelta = (*itr).Azimuth;

    pts.push_back(nu::PointF(center_x + r * std::sin(thelta), center_y - height/2 + r * std::cos(thelta)));     
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
void updateLogContent(nu::Label *label, std::string text)
{
  static std::string total_text;

  total_text += "\n";
  total_text += text;

  label->SetText(total_text);
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

  scoped_refptr<nu::Canvas> canvas(new nu::Canvas(nu::SizeF(400, 400), 1.0f));
  // nu::Painter *painter = canvas.get()->GetPainter();


  model.dataHB.subscribe([&](auto hb){
    // updateSonarArc(canvas.get()->GetPainter(), amp);

    window->GetContentView()->Layout();
    window->GetContentView()->SchedulePaint();
  }); 
//*/

  scoped_refptr<nu::Container> mainView(new nu::Container); 

  scoped_refptr<nu::Container> toolbar(new nu::Container); 
  toolbar->SetBackgroundColor(nu::Color("#9FA2A2")); 
  toolbar->SetStyle("position", "absolute", "width", tool_bar_width, "height", radar_view_size, "top", 0, "left", 0);

  for (int i = 0; i < 3; ++i)
  {
    scoped_refptr<nu::Button> btn(new nu::Button("button " + std::to_string(i), nu::Button::Type::Normal));
    // btn->SetStyle("flexgrow", 1);
    toolbar->AddChildView(btn.get());
  }

  for (int i = 0; i < 3; ++i)
  {
    scoped_refptr<nu::Button> btn(new nu::Button("checkbox " + std::to_string(i), nu::Button::Type::Checkbox));
    // btn->SetStyle("flexgrow", 1);
    toolbar->AddChildView(btn.get());
  }

  for (int i = 0; i < 3; ++i)
  {
    scoped_refptr<nu::Button> btn(new nu::Button("radio " + std::to_string(i), nu::Button::Type::Radio));
    // btn->SetStyle("flexgrow", 1);
    toolbar->AddChildView(btn.get());
  }

  mainView->AddChildView(toolbar.get());

  int scroll_size = log_view_height;
  scoped_refptr<nu::Scroll> logView(new nu::Scroll); 
  logView->SetBackgroundColor(nu::Color("#70DABB"));
  logView->SetStyle("position", "absolute", "width", window_width, "height", window_height - radar_view_size, "bottom", 0, "left", 0);
  logView->SetScrollbarPolicy(nu::Scroll::Policy::Always, nu::Scroll::Policy::Always);
  scoped_refptr<nu::Label> logLabel = new nu::Label("This is log area!");
  logLabel->SetStyle("position", "absolute", "height", 10000);
  logLabel->SetBackgroundColor(nu::Color("#70DABB"));
  logView->SetContentView(logLabel.get());
  logView->SetContentSize(nu::SizeF(window_width, scroll_size));
  mainView->AddChildView(logView.get());

  scoped_refptr<nu::Container> radarView(new nu::Container);
  radarView->SetStyle("position", "absolute", "width", radar_view_size, "height", radar_view_size, "top", 0, "right", 0);
  mainView->AddChildView(radarView.get());


  radarView->on_draw.Connect([&](nu::Container* self, nu::Painter* painter, const nu::RectF& dirty){
    std::cout << "container on_draw on rect: " << radarView->GetBounds().ToString() << std::endl;
    // std::cout << "container on_draw on rect: " << (self == nullptr) << std::endl;
    scoped_refptr<nu::Canvas> temp_canvas(new nu::Canvas(nu::SizeF(radar_view_size, radar_view_size), 1.0f));
    drawCar(temp_canvas.get()->GetPainter());
    drawRadarRange(temp_canvas.get()->GetPainter());
    updateSonarArc(temp_canvas.get()->GetPainter(), &model);  
    updateRadarDetectedObjects(temp_canvas.get()->GetPainter(), &model);

    nu::RectF bounds = radarView->GetBounds();
    painter->DrawCanvas(temp_canvas.get(), nu::RectF(0, 0, bounds.width(), bounds.height()));

    updateLogContent(logLabel.get(), "dirty area: " + dirty.ToString());
    logView->SetContentSize(nu::SizeF(window_width, ++scroll_size));
    // painter->DrawCanvas(canvas.get(), dirty);
  });

  
  window->SetResizable(false);
  window->SetContentView(mainView.get());
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




