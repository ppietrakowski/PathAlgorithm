// PathTracing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Application.h"

static const float InitialWindowSizeX = Application::MapWidth * Application::CellSize;
static const float InitialWindowSizeY = Application::MapHeight * Application::CellSize;

int main()
{
    Application app(InitialWindowSizeX, InitialWindowSizeY, "Game");
    app.Run();
    return EXIT_SUCCESS;
}