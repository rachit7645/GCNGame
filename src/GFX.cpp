#include "GFX.h"

#include <gccore.h>

static void*       xfb   = nullptr;
static GXRModeObj* rmode = nullptr;

void GFX::InitVideo()
{
	VIDEO_Init();
	
	rmode = VIDEO_GetPreferredMode(nullptr);
	xfb   = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	CON_Init
	(
		xfb,
		20,
		20,
		rmode->fbWidth,
		rmode->xfbHeight,
		rmode->fbWidth * VI_DISPLAY_PIX_SZ
	);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	}	
}

void GFX::WaitVBlank()
{
	VIDEO_WaitVSync();
}