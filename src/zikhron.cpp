#include <gtkgui/Application.h>

// gtk leaks like crazy - nothing we can do about - so leak detection is basically useless
extern "C" const char* __asan_default_options() { return "detect_leaks=0"; }

int main(int argc, char* argv[]) { return Application::run(argc, argv); }
