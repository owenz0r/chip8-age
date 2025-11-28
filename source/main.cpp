#include <iostream>
#include "Chip8Engine.h"
#include "Chip8Screen.h"
#include "core/globals.h"


int main(int, char**)
{
	Chip8Engine engine;
	// 700 tick rate/instrutions per second
	if (engine.Init(SCREEN_WIDTH, SCREEN_HEIGHT, 1.0 / 700.0))
	{
		auto chip8 = Chip8Screen();
		auto select = SelectScreen();

		engine.AddScreen("select", &select);
		engine.AddScreen("chip8", &chip8);

		engine.SetActiveScreen("select");

		engine.Run();
	}
	else
	{
		std::cout << "Failed to initialize engine. Aborting..." << std::endl;
	}
}
