// PathTracing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Application.h"

static const int InitialWindowSizeX = static_cast<int>(Application::MapWidth * Application::CellSize);
static const int InitialWindowSizeY = static_cast<int>(Application::MapHeight * Application::CellSize);

int main()
{
    Application app(InitialWindowSizeX, InitialWindowSizeY, "Game");
    app.Run();
    return EXIT_SUCCESS;
}