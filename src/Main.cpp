#include <stdio.h>
#include <stdlib.h>

#include <ogcsys.h>
#include <gccore.h>
#include <gctypes.h>

#include "Input.h"
#include "Util.h"

static void*       xfb   = nullptr;
static GXRModeObj* rmode = nullptr;

Input::GamePad gamePad;

void* InitVideo();

int main(GCN_UNUSED int argc, GCN_UNUSED char** argv)
{
	xfb = InitVideo();
	
	printf("\nHello World!\n");

	while(true)
	{
		VIDEO_WaitVSync();
		
		gamePad.Poll();

		if(gamePad.A())
		{
			printf("Button A pressed.\n");
		}

		if (gamePad.Start())
		{
			exit(0);
		}
	}

	return 0;
}

void* InitVideo()
{
	void* framebuffer;

	VIDEO_Init();
	PAD_Init();
	
	rmode       = VIDEO_GetPreferredMode(nullptr);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	CON_Init
	(
		framebuffer,
		20,
		20,
		rmode->fbWidth,
		rmode->xfbHeight,
		rmode->fbWidth * VI_DISPLAY_PIX_SZ
	);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	}	

	return framebuffer;
}
