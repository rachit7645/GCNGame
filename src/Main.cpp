#include <stdio.h>
#include <stdlib.h>

#include "Input.h"
#include "GFX.h"
#include "Util.h"

int main(GCN_UNUSED int argc, GCN_UNUSED char** argv)
{
	GFX::InitVideo();
	Input::InitControllers();

	Input::GamePad gamePad;

	printf("\nInitialised System.\n");

	while(true)
	{
		GFX::WaitVBlank();
		gamePad.Update();

		if(gamePad.A())
		{
			printf("Button A pressed.\n");
		}

		if (gamePad.Start())
		{
			exit(0);
		}
	}

	return GCN_EXIT_SUCESS;
}