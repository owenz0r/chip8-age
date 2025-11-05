#include "core/Engine.h"

class Chip8Engine : public age::Engine
{
  public:
	void Run() override;

	bool drawInterrupt = false;
	double displayRate = 1.0 / 60.0;
	double timeSinceLastDraw = 0.0;

  private:
	void _TimeStep() override;
};
