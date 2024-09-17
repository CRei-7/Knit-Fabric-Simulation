#include"Application.h"

int main(int, char**)
{
    Application app;
    if (!app.Init())
        return 1;
    app.MainLoop();
    app.Cleanup();
    return 0;
}
