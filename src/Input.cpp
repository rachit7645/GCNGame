#include "Input.h"

#include <gccore.h>

using namespace Input;

GamePad::GamePad(int pad)
	: pad(pad)
{
}

void GamePad::Poll() 
{
	PAD_ScanPads();
	m_buttonsDown = PAD_ButtonsDown(pad);
}

bool GamePad::A() const
{
	return static_cast<bool>(m_buttonsDown & PAD_BUTTON_A);
}

bool GamePad::Start() const
{
	return static_cast<bool>(m_buttonsDown & PAD_BUTTON_START);
}