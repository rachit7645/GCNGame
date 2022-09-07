#include "GFX.h"

#include <gccore.h>
#include <cmath>
#include <cstring>
#include <malloc.h>

#include "Util.h"

namespace GFX
{
	constexpr auto FIFO_SIZE = 256 * 1024;

	constexpr auto FOV    = 70.0f;
	constexpr auto ASPECT = static_cast<f32>(4.0 / 3.0f);
	constexpr auto NEAR   = 0.1f;
	constexpr auto FAR    = 500.0f;

	static void*       frameBuffer  = nullptr;
	static void*       fifoBuffer   = nullptr;
	static GXRModeObj* screenMode   = nullptr;
	static vu8         readyForCopy = GX_FALSE;

	static Mtx   view;
	static Mtx44 projection;

	static GXColor bgColor = {0, 0, 0, 255};
	
	static guVector camera = {0.0f, 0.0f, 0.0f};
	static guVector up     = {0.0f, 1.0f, 0.0f};
	static guVector look   = {0.0f, 0.0f, -1.0f};

	static s16 vertices[] ATTRIBUTE_ALIGN(32)
	{
		0, 15, 0,
		-15, -15, 0,
		15, -15, 0
	};

	static u8 colors[]	ATTRIBUTE_ALIGN(32)
	{
		255, 0,	0, 255, // red
		0, 255,	0, 255, // green
		0, 0, 255, 255  // blue
	};

	void InitScreen();
	void InitGPU();
	void LoadData();
	void CopyBuffers(u32 count);
	void UpdateScreen(Mtx& view, u16 vertexCount);
}

void GFX::InitVideo()
{
	VIDEO_Init();
	InitScreen();
	InitGPU();
	LoadData();
}

void GFX::InitScreen()
{
	screenMode  = VIDEO_GetPreferredMode(nullptr);
	frameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));
	
	VIDEO_Configure(screenMode);
	VIDEO_SetNextFramebuffer(frameBuffer);
	VIDEO_SetPostRetraceCallback(GFX::CopyBuffers);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
}

void GFX::InitGPU()
{
	fifoBuffer = MEM_K0_TO_K1(memalign(32, FIFO_SIZE));
	memset(fifoBuffer, 0, FIFO_SIZE);
	GX_Init(fifoBuffer, FIFO_SIZE);
	
	GX_SetCopyClear(bgColor, GX_MAX_Z24);
	GX_SetViewport(0, 0, screenMode->fbWidth, screenMode->efbHeight, 0, 1);
	GX_SetDispCopyYScale(static_cast<f32>(screenMode->xfbHeight) / static_cast<f32>(screenMode->efbHeight));
	GX_SetScissor(0, 0, screenMode->fbWidth, screenMode->efbHeight);
	GX_SetDispCopySrc(0, 0, screenMode->fbWidth, screenMode->efbHeight);
	GX_SetDispCopyDst(screenMode->fbWidth, screenMode->xfbHeight);
	GX_SetCopyFilter(screenMode->aa, screenMode->sample_pattern, true, screenMode->vfilter);
	GX_SetFieldMode(screenMode->field_rendering, ((screenMode->viHeight == 2 * screenMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer, GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_2_2);
}

void GFX::LoadData()
{
	guPerspective(projection, FOV, ASPECT, NEAR, FAR);
	GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,	GX_POS_XYZ,	GX_S16,	0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8,	0);
	GX_SetArray(GX_VA_POS, vertices, 3 * sizeof(s16));
	GX_SetArray(GX_VA_CLR0, colors, 4 * sizeof(u8));
	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

void GFX::Update()
{
	guLookAt(view, &camera,	&up, &look);
	GX_SetViewport(0, 0, screenMode->fbWidth, screenMode->efbHeight, 0, 1);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
	UpdateScreen(view, GCN_ARRAY_SIZE(vertices) / 3);
}

void GFX::UpdateScreen(Mtx& view, u16 vertexCount)
{
	Mtx	modelView;
	
	guMtxIdentity(modelView);
	guMtxTransApply(modelView, modelView, 0.0f,	0.0f, -50.0f);
	guMtxConcat(view, modelView, modelView);
	
	GX_LoadPosMtxImm(modelView,	GX_PNMTX0);
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, vertexCount);
	for (u8 i = 0; i < vertexCount; ++i)
	{
		GX_Position1x8(i);
		GX_Color1x8(i);
	}
	GX_End();
	GX_DrawDone();
	readyForCopy = GX_TRUE;

	VIDEO_WaitVSync();
}

void GFX::CopyBuffers(GCN_UNUSED u32 count)
{
	if (readyForCopy == GX_TRUE)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL,	GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer, GX_TRUE);
		GX_Flush();
		readyForCopy = GX_FALSE;
	}
}