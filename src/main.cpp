#include "Application.h"
#include <iostream>

int main(int, char**)
{
    Application app;
    if (!app.Init()) {
            std::cerr << "Initialization failed" << std::endl;
            return -1;
    }
    std::cout << "Application initialized" << std::endl;
    app.MainLoop();
    app.Cleanup();
    return 0;
}
