#include "Chip8Screen.h"
#include "Chip8Engine.h"
#include "SDL/SDLInput.h"
#include "SDL_keycode.h"
#include "core/Engine.h"
#include "core/Renderer.h"
#include "core/ResourceManager.h"
#include "core/Utils.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define DEBUG 1
constexpr int display_width = 64;
constexpr int display_height = 32;
constexpr int display_size = display_width * display_height;
constexpr int memory_size = 4096;
constexpr int program_address = 0x0200;

// configurable instructions
constexpr bool shift_copy = true;
constexpr bool store_load_increment = true;
constexpr bool clipping = true;
constexpr int font_base = 0x0050;

static int display[display_height][display_width];
static char memory[memory_size];
static char keys[16];
static unsigned char delay_timer;
static unsigned char sound_timer;
static unsigned int PC = program_address;
static unsigned int I = 0x0000;
static unsigned char REG[16];
static unsigned int PEND = 0xFFFF;
static std::vector<unsigned int> stack;

static bool do_pause = false;
static int count = 0;

#ifdef DEBUG
#define DEBUG_LOG(x) std::cout << x << std::endl
#else
#define DEBUG_LOG(x)
#endif

using namespace age;

void Chip8Screen::Init()
{
	m_chip8 = static_cast<Chip8Engine*>(m_engine);
	m_input = std::make_unique<age::SDLInput>();

	m_input->m_keydownmap.insert({'1', [&] { keys[0x1] = 1; }});
	m_input->m_keydownmap.insert({'2', [&] { keys[0x2] = 1; }});
	m_input->m_keydownmap.insert({'3', [&] { keys[0x3] = 1; }});
	m_input->m_keydownmap.insert({'q', [&] { keys[0x4] = 1; }});
	m_input->m_keydownmap.insert({'w', [&] { keys[0x5] = 1; }});
	m_input->m_keydownmap.insert({'e', [&] { keys[0x6] = 1; }});
	m_input->m_keydownmap.insert({'a', [&] { keys[0x7] = 1; }});
	m_input->m_keydownmap.insert({'s', [&] { keys[0x8] = 1; }});
	m_input->m_keydownmap.insert({'d', [&] { keys[0x9] = 1; }});
	m_input->m_keydownmap.insert({'x', [&] { keys[0x0] = 1; }});
	m_input->m_keydownmap.insert({'z', [&] { keys[0xA] = 1; }});
	m_input->m_keydownmap.insert({'c', [&] { keys[0xB] = 1; }});
	m_input->m_keydownmap.insert({'4', [&] { keys[0xC] = 1; }});
	m_input->m_keydownmap.insert({'r', [&] { keys[0xD] = 1; }});
	m_input->m_keydownmap.insert({'f', [&] { keys[0xE] = 1; }});
	m_input->m_keydownmap.insert({'v', [&] { keys[0xF] = 1; }});

	m_input->m_keyupmap.insert({'1', [&] { keys[0x1] = 0; }});
	m_input->m_keyupmap.insert({'2', [&] { keys[0x2] = 0; }});
	m_input->m_keyupmap.insert({'3', [&] { keys[0x3] = 0; }});
	m_input->m_keyupmap.insert({'q', [&] { keys[0x4] = 0; }});
	m_input->m_keyupmap.insert({'w', [&] { keys[0x5] = 0; }});
	m_input->m_keyupmap.insert({'e', [&] { keys[0x6] = 0; }});
	m_input->m_keyupmap.insert({'a', [&] { keys[0x7] = 0; }});
	m_input->m_keyupmap.insert({'s', [&] { keys[0x8] = 0; }});
	m_input->m_keyupmap.insert({'d', [&] { keys[0x9] = 0; }});
	m_input->m_keyupmap.insert({'x', [&] { keys[0x0] = 0; }});
	m_input->m_keyupmap.insert({'z', [&] { keys[0xA] = 0; }});
	m_input->m_keyupmap.insert({'c', [&] { keys[0xB] = 0; }});
	m_input->m_keyupmap.insert({'4', [&] { keys[0xC] = 0; }});
	m_input->m_keyupmap.insert({'r', [&] { keys[0xD] = 0; }});
	m_input->m_keyupmap.insert({'f', [&] { keys[0xE] = 0; }});
	m_input->m_keyupmap.insert({'v', [&] { keys[0xF] = 0; }});

	m_input->SetQuitCallback([this]() { m_engine->Quit(); });
	m_input->m_keydownmap.insert({'m', [&]() -> void { m_chip8->Transition("select", ""); }});

	m_fontId = 0; // default font

	srand(time(0));

	Reset();
}

void Chip8Screen::Reset()
{
	m_initialized = false;

	memset(display, 0, sizeof(int) * display_size);
	memset(memory, 0, memory_size);
	memset(keys, 0, 16);

	PC = program_address;
	I = 0x0000;
	PEND = 0xFFFF;

	int font_address = font_base;

	// 0
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;

	// 1
	memory[font_address++] = 0x20;
	memory[font_address++] = 0x60;
	memory[font_address++] = 0x20;
	memory[font_address++] = 0x20;
	memory[font_address++] = 0x70;

	// 2
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;

	// 3
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0xF0;

	// 4
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0x10;

	// 5
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0xF0;

	// 6
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;

	// 7
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0x20;
	memory[font_address++] = 0x40;
	memory[font_address++] = 0x40;

	// 8
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;

	// 9
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x10;
	memory[font_address++] = 0xF0;

	// A
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;

	// B
	memory[font_address++] = 0xE0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xE0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xE0;

	// C
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;

	// D
	memory[font_address++] = 0xE0;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0x90;
	memory[font_address++] = 0xE0;

	// E
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;

	// F
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0xF0;
	memory[font_address++] = 0x80;
	memory[font_address++] = 0x80;

	delay_timer = 0;
	sound_timer = 0;

#if defined(__APPLE__)
	std::string path = getResourcesPath() + ROM_PATH + m_transitionData + ".ch8";
#else
	std::string path = getExecutablePath().parent_path().string() + "/Resources/Roms/" + m_transitionData + ".ch8";
#endif
	std::ifstream input(path, std::ios::binary);

	// Breakout [Carmelo Cortez, 1979].ch8

	if (input)
	{
		input.seekg(0, input.end);
		auto size = input.tellg();
		input.seekg(0, input.beg);
		input.read(&memory[program_address], size);
		input.close();

		PEND = program_address + size;
		m_initialized = true;
	}
	else
	{
		std::cout << "No ROM found - " << path << std::endl;
	}
}

void Chip8Screen::Update(const double dt)
{
	if (do_pause)
		return;

	if (PC >= PEND)
		return;

	// fetch
	unsigned char b1 = memory[PC++];
	unsigned char b2 = memory[PC++];

	unsigned char n1 = (b1 >> 4) & 0x0F;
	unsigned char n2 = b1 & 0x0F;
	unsigned char n3 = (b2 >> 4) & 0x0F;
	unsigned char n4 = b2 & 0x0F;

	// decode + execute

	switch (n1)
	{
		case 0x0:
			{
				if (n4 == 0x0)
				{
					DEBUG_LOG("Clear Screen");
					memset(display, 0, sizeof(int) * display_size);
					break;
				}
				else if (n4 == 0xE)
				{
					DEBUG_LOG("Subroutine return");
					PC = stack.back();
					stack.pop_back();
					break;
				}
				else
				{
					abort();
				}
			}
		case 0x1:
			DEBUG_LOG("Jump " << (int)((n2 << 8) | b2));
			PC = (n2 << 8) | b2;
			break;
		case 0x2:
			DEBUG_LOG("Subroutine " << (int)((n2 << 8) | b2));
			stack.push_back(PC);
			PC = (n2 << 8) | b2;
			break;
		case 0x3:
			DEBUG_LOG("Skip if REG " << (int)n2 << " is " << (int)b2);
			if (REG[n2] == b2)
				PC += 2;
			break;
		case 0x4:
			DEBUG_LOG("Skip if REG " << (int)n2 << " is NOT " << (int)b2);
			if (REG[n2] != b2)
				PC += 2;
			break;
		case 0x5:
			DEBUG_LOG("Skip if REG " << (int)n2 << " equals REG " << (int)n3);
			if (REG[n2] == REG[n3])
				PC += 2;
			break;
		case 0x6:
			DEBUG_LOG("Load " << (int)b2 << " into REG " << (int)n2);
			REG[n2] = b2;
			break;
		case 0x7:
			DEBUG_LOG("Add " << (int)b2 << " to REG " << (int)n2);
			REG[n2] += b2;
			break;
		case 0x8:
			{
				switch (n4)
				{
					case 0x0:
						DEBUG_LOG("Set REG " << (int)n2 << " to value of REG " << (int)n3);
						REG[n2] = REG[n3];
						break;
					case 0x1:
						DEBUG_LOG("Set REG " << (int)n2 << " to OR of REG " << (int)n3);
						REG[n2] |= REG[n3];
						REG[0xF] = 0x0;
						break;
					case 0x2:
						DEBUG_LOG("Set REG " << (int)n2 << " to AND of REG " << (int)n3);
						REG[n2] &= REG[n3];
						REG[0xF] = 0x0;
						break;
					case 0x3:
						DEBUG_LOG("Set REG " << (int)n2 << " to XOR of REG " << (int)n3);
						REG[n2] ^= REG[n3];
						REG[0xF] = 0x0;
						break;
					case 0x4:
						{
							DEBUG_LOG("Set REG " << (int)n2 << " to += value of REG " << (int)n3);
							auto flag = (REG[n2] + REG[n3]) > 255 ? 1 : 0;
							REG[n2] += REG[n3];
							REG[0xF] = flag;
							break;
						}
					case 0x5:
						{
							DEBUG_LOG("Set REG " << (int)n2 << " to -= value of REG " << (int)n3);
							unsigned char flag = REG[n2] >= REG[n3] ? 1 : 0;
							REG[n2] -= REG[n3];
							REG[0xF] = flag;
							break;
						}
					case 0x6:
						if (shift_copy)
						{
							DEBUG_LOG("Set REG " << (int)n2 << " to value of REG " << (int)n3 << " and SHIFT right 1 ");
							REG[n2] = REG[n3];
							auto flag = 0x01 & REG[n2];
							REG[n2] = REG[n2] >> 1;
							REG[0xF] = flag;
							break;
						}
						else
						{
							DEBUG_LOG("Shift REG " << (int)n2 << " right 1 ");
							auto flag = 0x01 & REG[n2];
							REG[n2] = REG[n2] >> 1;
							REG[0xF] = flag;
							break;
						}
					case 0x7:
						{
							DEBUG_LOG("Set REG " << (int)n2 << " to value of REG " << (int)n3 << " - REG " << (int)n2);
							auto flag = REG[n3] >= REG[n2] ? 1 : 0;
							REG[n2] = REG[n3] - REG[n2];
							REG[0xF] = flag;
							break;
						}
					case 0xE:
						if (shift_copy)
						{
							DEBUG_LOG("Set REG " << (int)n2 << " to value of REG " << (int)n3 << " and SHIFT left 1 ");
							REG[n2] = REG[n3];
							auto flag = (REG[n2] >> 7) & 0x01;
							REG[n2] = REG[n2] << 1;
							REG[0xF] = flag;
							break;
						}
						else
						{
							DEBUG_LOG("Shift REG " << (int)n2 << " left 1 ");
							auto flag = (REG[n2] >> 7) & 0x01;
							REG[n2] = REG[n2] << 1;
							REG[0xF] = flag;
							break;
						}
					default:
						abort();
				}
				break;
			}
		case 0x9:
			DEBUG_LOG("Skip if REG " << (int)n2 << " NOT equal REG " << (int)n3);
			if (REG[n2] != REG[n3])
				PC += 2;
			break;
		case 0xA:
			DEBUG_LOG("Load " << (int)(n2 << 8 | b2) << " into I");
			I = n2 << 8 | b2;
			break;
		case 0xB:
			DEBUG_LOG("Jump with offset in REG[0]");
			PC = (n2 << 8) | b2;
			PC += REG[0x0];
			break;
		case 0xC:
			{
				DEBUG_LOG("Random into REG" << (int)n2);
				auto r = rand() % 256;
				REG[n2] = b2 & r;
				break;
			}
		case 0xD:
			{
				DEBUG_LOG("Draw Sprite at (" << (int)REG[n2] << ", " << (int)REG[n3] << ")");

				if (m_chip8->drawInterrupt)
				{
					m_chip8->drawInterrupt = false;

					int x = REG[n2];
					int y = REG[n3];

					x = x % display_width;
					y = y % display_height;

					unsigned char flag = 0x0;
					for (int i = 0; i < n4; ++i)
					{
						if (!clipping || y < display_height)
						{
							int ycoord = y;
							char c = memory[I + i];
							for (int j = 0; j < 8; ++j)
							{
								if (!clipping || x + j < display_width)
								{
									int xcoord = x + j;
									int b = (c >> (7 - j)) & 0x01;
									if (b == 1)
									{
										if (!flag && display[ycoord][xcoord])
											flag = 0x1;
										display[ycoord][xcoord] = display[ycoord][xcoord] == 0 ? 1 : 0;
									}
								}
							}
						}
						++y;
					}
					REG[0xF] = flag;
				}
				else
				{
					PC = PC - 2;
				}
				break;
			}
		case 0xE:
			{
				switch (n3)
				{
					case 0x9:
						DEBUG_LOG("Skip if key " << std::hex << (int)REG[n2] << std::dec << " is pressed");
						if (keys[REG[n2]])
							PC += 2;
						break;
					case 0xA:
						DEBUG_LOG("Skip if key " << std::hex << (int)REG[n2] << std::dec << " is NOT pressed");
						if (!keys[REG[n2]])
							PC += 2;
						break;
					default:
						abort();
				}
				break;
			}
		case 0xF:
			switch (n3)
			{
				case 0x0:
					if (n4 == 0xA)
					{
						DEBUG_LOG("Get Key into " << (int)n2);
						for (int i = 0; i < 16; ++i)
						{
							if (keys[i])
							{
								REG[n2] = i;
								return;
							}
						}
						PC = PC - 2;
						break;
					}
					else if (n4 == 0x7)
					{
						DEBUG_LOG("Set REG " << (int)n2 << "to delay timer - " << delay_timer);
						REG[n2] = delay_timer;
						break;
					}
					else
					{
						abort();
					}
				case 0x1:
					{
						if (n4 == 0x5)
						{
							DEBUG_LOG("Set delay timer to " << (int)REG[n2]);
							delay_timer = REG[n2];
							break;
						}
						else if (n4 == 0x8)
						{
							DEBUG_LOG("Set sound timer to " << (int)REG[n2]);
							sound_timer = REG[n2];
							break;
						}
						else if (n4 == 0xE)
						{
							DEBUG_LOG("Add REG " << (int)n2 << " to I");
							auto flag = (I + REG[n2]) > 0x0FFF ? 1 : 0;
							I += REG[n2];
							REG[0xF] = flag;
							break;
						}
						else
						{
							abort();
						}
					}
				case 0x2:
					{
						DEBUG_LOG("Font character. Set I to REG " << (int)n2);
						I = font_base + (REG[n2] * 5);
						break;
					}
				case 0x3:
					{
						DEBUG_LOG("Binary-coded decimal REG " << (int)n2 << " into I");
						int tmp = REG[n2];
						int a = tmp / 100;
						tmp -= a * 100;
						int b = tmp / 10;
						tmp -= b * 10;
						int c = tmp;
						memory[I] = a;
						memory[I + 1] = b;
						memory[I + 2] = c;
						break;
					}
				case 0x5:
					if (store_load_increment)
					{
						DEBUG_LOG("Store " << (int)(n2) << " into I");
						for (int i = 0; i <= n2; ++i)
							memory[I++] = REG[i];
						break;
					}
					else
					{
						DEBUG_LOG("Store " << (int)(n2) << " into I");
						for (int i = 0; i <= n2; ++i)
							memory[I + i] = REG[i];
						break;
					}
				case 0x6:
					if (store_load_increment)
					{
						DEBUG_LOG("Load " << (int)(n2) << " from I into REG");
						for (int i = 0; i <= n2; ++i)
							REG[i] = memory[I++];
						break;
					}
					else
					{
						DEBUG_LOG("Load " << (int)(n2) << " from I into REG");
						for (int i = 0; i <= n2; ++i)
							REG[i] = memory[I + i];
						break;
					}
				default:
					abort();
			}
			break;
		default:
			abort();
	}
}

void Chip8Screen::Draw()
{
	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0)
		sound_timer--;

	for (int j = 0; j < display_height; ++j)
	{
		for (int i = 0; i < display_width; ++i)
		{
			Color c = Color::Yellow();
			switch (display[j][i])
			{
				case 0:
					c = Color::Black();
					break;
				case 1:
					c = Color::White();
					break;
				default:
					abort();
			}

			age::Rect r(i, j, 1, 1);
			m_renderer->DrawQuad(r, c);
		}
	}
}

void SelectScreen::Draw()
{
	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0)
		sound_timer--;

	for (int j = 0; j < display_height; ++j)
	{
		for (int i = 0; i < display_width; ++i)
		{
			Color c = Color::Black();
			switch (display[j][i])
			{
				case 0:
					c = Color::Black();
					break;
				case 1:
					c = Color::White();
					break;
				default:
					abort();
			}

			age::Rect r(i, j, 1, 1);
			m_renderer->DrawQuad(r, c);
		}
	}

	age::Rect r(3, 3, 58, 26);
	m_renderer->DrawQuad(r, Color::Red());

	TextParams params;
	if (m_num_roms < 1)
	{
		params.text = "No ROMs found";
		params.pos = {6, 6};
		params.height = 1.0f;
		params.color = Color::White();
		m_renderer->DrawText(params);
	}
	else
	{

		float y = 4.0f;
		int count = 0;
		for (const auto& name : m_rom_names)
		{
			params.text = name;
			params.pos = {6, y};
			params.height = 1.0f;
			params.color = Color::White();
			m_renderer->DrawText(params);
			if (count == m_selected)
			{
				r = age::Rect(5.25, y + 0.25, 0.5f, 0.5f);
				m_renderer->DrawQuad(r, Color::Yellow());
			}
			count++;
			y++;
		}
	}
}

void SelectScreen::Init()
{
	m_chip8 = static_cast<Chip8Engine*>(m_engine);

#if defined(__APPLE__)
	m_rom_path = getResourcesPath() + ROM_PATH;
#else
	m_rom_path = getExecutablePath().parent_path().string() + ROM_PATH;
#endif

	std::cout << "Rom Path: " << m_rom_path << std::endl;
	if (std::filesystem::exists(m_rom_path))
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_rom_path))
		{
			if (entry.path().extension().string() == ".ch8")
			{
				m_rom_names.push_back(entry.path().stem().string());
				m_num_roms++;
			}
		}
	}

	m_input = std::make_unique<age::SDLInput>();
	m_input->SetQuitCallback([this]() { m_engine->Quit(); });

	m_input->m_keydownmap.insert({'w', [&]() -> void {
									  if (m_selected > 0)
										  m_selected--;
								  }});
	m_input->m_keydownmap.insert({'s', [&]() -> void {
									  if (m_selected < m_num_roms - 1)
										  m_selected++;
								  }});
	m_input->m_keydownmap.insert(
		{SDLK_RETURN, [&]() -> void { m_chip8->Transition("chip8", m_rom_names[m_selected]); }});

	m_initialized = true;
}
