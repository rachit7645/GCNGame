#ifndef INPUT_H
#define INPUT_H

#include <gctypes.h>

namespace Input
{
	class GamePad
	{
	public:
		// Default constructor
		GamePad() = default;
		// Main Constructor
		GamePad(int pad);

		// Poll once every frame
		void Update();
		// Keys
		bool A()     const;
		bool Start() const;

		// Currently selected pad
		int pad = 0;
	private:
		// Internal button data
		u16 m_buttonsDown = 0;
	};

	void InitControllers();
}


#endif