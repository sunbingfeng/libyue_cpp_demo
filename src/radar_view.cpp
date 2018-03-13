// This file is published under public domain.
#include <iostream>
#include <cmath>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <cstddef>

#include "radar_view.h"

// #include "base/command_line.h"
// #include "string_number_conversions.h"

#include "nativeui/nativeui.h"

#include <observable/observable.hpp>

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

class testModel
{
  OBSERVABLE_PROPERTIES(testModel)
public:
    observable_property<float> amplitude { 0.0f };

public:
  testModel()
  {
    std::srand(std::time(nullptr)); // use current time as seed for random generator

    pAmpUpdateTh = new std::thread(&testModel::gen_amp, this);
    pAmpUpdateTh->detach();
  }

  ~testModel()
  {

  }
  
  float amp = 0.0f;
private:
  std::thread *pAmpUpdateTh;

  void gen_amp(void)
  {
    while(1)
    {
      int random_variable = std::rand();
      
      amp = 0.1 * (random_variable % 2000);  // cm
      amplitude = amp;

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
  nu::RectF bounds = nu::RectF(-width/2, -height/2, width, height);
  bounds.Offset(center_x, center_y);
   
  painter->StrokeRect(bounds);
}

RadarView::RadarView()
{

}
RadarView::~RadarView()
{

}

void RadarView::updateSonarArc(nu::Painter *painter)
{
  static const float frontsonar_space = 38 / unit; // 38 cm
  static const float rearsonar_space = 44 / unit; // 44 cm
  static const float sidesonar_space = 125 / unit; // 125 cm

  // front sonar
  drawSonarArc(painter, nu::PointF(center_x - frontsonar_space/2, center_y - height/2), latestSonar.left_front, 3*pi/2 - sonar_angle_range/2, 3*pi/2 + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + frontsonar_space/2, center_y - height/2), latestSonar.right_front, 3*pi/2 - sonar_angle_range/2, 3*pi/2 + sonar_angle_range/2);

  // rear sonar
  drawSonarArc(painter, nu::PointF(center_x - frontsonar_space/2, center_y + height/2), latestSonar.left_rear, pi/2 - sonar_angle_range/2, pi/2 + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + frontsonar_space/2, center_y + height/2), latestSonar.right_rear, pi/2 - sonar_angle_range/2, pi/2 + sonar_angle_range/2);

  // left side sonar
  drawSonarArc(painter, nu::PointF(center_x - width/2, center_y - sidesonar_space/2), latestSonar.leftside_front, pi - sonar_angle_range/2, pi + sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x - width/2, center_y + sidesonar_space/2), latestSonar.leftside_rear, pi - sonar_angle_range/2, pi + sonar_angle_range/2);

  // right side sonar
  drawSonarArc(painter, nu::PointF(center_x + width/2, center_y - sidesonar_space/2), latestSonar.rightside_front, 2*pi - sonar_angle_range/2, sonar_angle_range/2);
  drawSonarArc(painter, nu::PointF(center_x + width/2, center_y + sidesonar_space/2), latestSonar.rightside_rear, 2*pi - sonar_angle_range/2, sonar_angle_range/2);
}

void RadarView::updateRadarDetectedObjects(nu::Painter *painter)
{
  std::vector<nu::PointF> pts;
  for (std::vector<ObjectInfo_77>::iterator itr = m_objs_map[0].begin(); itr != m_objs_map[0].end(); ++itr)
  {
    float r = (*itr).Range;
    float thelta = (*itr).Azimuth;

    pts.push_back(nu::PointF(center_x + r * std::sin(thelta), center_y - height/2 + r * std::cos(thelta)));     
  }

  for (std::vector<ObjectInfo_77>::iterator itr = m_objs_map[1].begin(); itr != m_objs_map[1].end(); ++itr)
  {
    float r = (*itr).Range;
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

void RadarView::run()
{
  // base::CommandLine::Init(argc, argv);

  std::srand(std::time(nullptr)); // use current time as seed for random generator
  testModel model;

  // Initialize the global instance of nativeui.
  nu::State state;

  // Create GUI message loop.
  nu::Lifetime lifetime;

  // Create window with default options, and then show it.
  m_pwindow = scoped_refptr<nu::Window>(new nu::Window(nu::Window::Options()));

  scoped_refptr<nu::Canvas> canvas(new nu::Canvas(nu::SizeF(400, 400), 1.0f));
  // nu::Painter *painter = canvas.get()->GetPainter();

  model.amplitude.subscribe([&](auto amp){
    // updateSonarArc(canvas.get()->GetPainter(), amp);

    m_pwindow->GetContentView()->Layout();
    m_pwindow->GetContentView()->SchedulePaint();
  }); 

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

  radarView = scoped_refptr<nu::Container>(new nu::Container);
  radarView->SetStyle("position", "absolute", "width", radar_view_size, "height", radar_view_size, "top", 0, "right", 0);
  mainView->AddChildView(radarView.get());


  radarView->on_draw.Connect([&](nu::Container* self, nu::Painter* painter, const nu::RectF& dirty){
    std::cout << "container on_draw on rect: " << radarView->GetBounds().ToString() << std::endl;
    // std::cout << "container on_draw on rect: " << (self == nullptr) << std::endl;
    scoped_refptr<nu::Canvas> temp_canvas(new nu::Canvas(nu::SizeF(radar_view_size, radar_view_size), 1.0f));
    // drawCar(temp_canvas.get()->GetPainter());
    // drawRadarRange(temp_canvas.get()->GetPainter());
    // updateSonarArc(temp_canvas.get()->GetPainter());  
    // updateRadarDetectedObjects(temp_canvas.get()->GetPainter());

    nu::RectF bounds = radarView->GetBounds();
    painter->DrawCanvas(temp_canvas.get(), nu::RectF(0, 0, bounds.width(), bounds.height()));

    // updateLogContent(logLabel.get(), "dirty area: " + dirty.ToString());
    // logView->SetContentSize(nu::SizeF(window_width, ++scroll_size));
    // painter->DrawCanvas(canvas.get(), dirty);
  });

  
  m_pwindow->SetResizable(false);
  m_pwindow->SetContentView(mainView.get());
  m_pwindow->SetContentSize(nu::SizeF(window_width, window_height));
  m_pwindow->Center();
  m_pwindow->Activate();


  // Quit when window is closed.
  m_pwindow->on_close.Connect([&state](nu::Window*) {
    nu::MessageLoop::Quit();
  });


  // Enter message loop.
  nu::MessageLoop::Run();
}

void RadarView::redraw()
{
  if(!m_pwindow)
  {
    return;
  }

  m_pwindow->GetContentView()->Layout();
  m_pwindow->GetContentView()->SchedulePaint();  
}

void RadarView::sonarUpdate(const SonarData& sonar)
{
  latestSonar = sonar;
  redraw();
}

void RadarView::radarUpdate(const int chl, const std::vector<ObjectInfo_77> &objs)
{
  m_objs_map[chl] = objs;
  redraw();
}

