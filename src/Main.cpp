#include <cstdlib>

#include "Input.h"
#include "GFX.h"
#include "Util.h"

int main(GCN_UNUSED int argc, GCN_UNUSED char** argv)
{
	GFX::InitVideo();
	Input::InitControllers();

	Input::GamePad gamePad;

	while(true)
	{
		GFX::Render();
		gamePad.Update();

		if (gamePad.Start())
		{
			exit(0);
		}
	}

	return GCN_EXIT_SUCESS;
}