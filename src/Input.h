#ifndef INPUT_H
#define INPUT_H

#include <gctypes.h>

namespace Input
{
	// \brief A class representing a GameCube gamepad
	class GamePad
	{
	public:
		// \fn Input::GamePad::GamePad()
		// \brief Gamepad constructor
		// \returns void
		GamePad() = default;
		// \fn Input::GamePad::GamePad(int pad)
		// \brief Gamepad constructor
		// \returns void
		GamePad(int pad);

		// \fn void Input::GamePad::Update()
		// \brief Polls input data from the controllers
		// \returns void
		void Update();
		
		// \fn bool Input::GamePad::A()
		// \brief Returns if the A key is pressed
		// \returns bool
		bool A() const;
		// \fn bool Input::GamePad::Start()
		// \brief Returns if the Start key is pressed
		// \returns bool
		bool Start() const;

		// \brief Current pad
		int pad = 0;
	private:
		// \brief Internal button data
		u16 m_buttonsDown = 0;
	};

	// \fn void Input::InitControllers()
	// \brief Initialises controller subsystem
	// \returns void
	void InitControllers();
}


#endif