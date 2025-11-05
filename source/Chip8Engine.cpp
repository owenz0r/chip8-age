#include <chrono>
#include <thread>

#include <iostream>

#include "Chip8Engine.h"
#include "SDL/SDLDefaultScreen.h"

using namespace age;

void Chip8Engine::Run()
{
	if (!m_activeScreen)
	{
		m_activeScreen = new SDLDefaultScreen(m_renderer.get(), this);
		m_activeScreen->Init();
		if (!m_activeScreen->m_initialized)
		{
			std::cout << "Failed to initialize DefaultScreen" << std::endl;
			return;
		}
	}

	m_currentTime = Clock::now();
	srand(static_cast<unsigned>(time(nullptr)));

	while (!m_quit)
	{
		_TimeStep();
		_ProcessInput();
		_Render();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Chip8Engine::_TimeStep()
{
	auto newTime = Clock::now();
	std::chrono::duration<double> ft = newTime - m_currentTime;
	double frameTime = ft.count();
	timeSinceLastDraw += frameTime;
	m_currentTime = newTime;

	while (frameTime > 0.0)
	{
		double deltaTime = fmin(frameTime, tickRate);
		_Update(deltaTime);
		frameTime -= deltaTime;
	}

	if (timeSinceLastDraw > displayRate)
	{
		_Render();
		timeSinceLastDraw = 0;
		drawInterrupt = true;
	}
}
