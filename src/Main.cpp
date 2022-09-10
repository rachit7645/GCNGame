#include <cstdlib>

#include "Input.h"
#include "GFX.h"
#include "Util.h"

// \fn int main(GCN_UNUSED int argc, GCN_UNUSED char** argv)
// \brief Entry point
// \param argc Argument count (Unused)
// \param argv Arguments (Unused)
// \returns GCN_EXIT_SUCESS
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
			std::exit(GCN_EXIT_START);
		}
	}

	return GCN_EXIT_SUCESS;
}