#pragma once

#include "core/Screen.h"
#include <vector>

class Chip8Engine;

class Chip8Screen : public age::Screen
{
	Chip8Engine* m_chip8;
	int m_fontId = -1;
	void Init() override;
	void Reset() override;
	void Draw() override;
	void Update(const double dt) override;
	void SetTransitionData(std::string transition_data) override
	{
		m_transitionData = transition_data;
	}
};

class SelectScreen : public age::Screen
{
	Chip8Engine* m_chip8;
	int m_fontId = -1;
	int m_selected = 0;
	int m_num_roms = 0;
	std::vector<std::string> m_rom_names;
	std::string m_rom_path = "/media/roms";
	void Draw() override;
	void Init() override;
};
