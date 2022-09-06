#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <gccore.h>

static void*       xfb   = nullptr;
static GXRModeObj* rmode = nullptr;

void* Initialise();

int main(int argc, char** argv)
{
	xfb = Initialise();

	printf("\nHello World!\n");

	while(true)
	{
		VIDEO_WaitVSync();
		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(0);
		
		if(buttonsDown & PAD_BUTTON_A)
		{
			printf("Button A pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START)
		{
			exit(0);
		}
	}

	return 0;
}

void* Initialise()
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
