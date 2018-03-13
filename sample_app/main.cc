// This file is published under public domain.

#include "base/command_line.h"
#include "nativeui/nativeui.h"
#include "radar_view.h"

#if defined(OS_WIN)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  base::CommandLine::Init(0, nullptr);
#else
int main(int argc, const char *argv[]) {
  base::CommandLine::Init(argc, argv);
#endif

  RadarView *view = new RadarView();
  view->run();

  return 0;
}
