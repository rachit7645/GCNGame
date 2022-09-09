#include "GFX.h"

#include <cmath>
#include <cstring>

#include <gccore.h>
#include <malloc.h>
#include <ogc/tpl.h>

#include "Util.h"
#include "Camera.h"
#include "Vertex.h"

#include "textures_tpl.h"
#include "textures.h"

namespace GFX
{
	constexpr auto FIFO_SIZE = 256 * 1024;

	constexpr auto FOV    = 70.0f;
	constexpr auto ASPECT = static_cast<f32>(4.0 / 3.0);
	constexpr auto NEAR   = 0.1f;
	constexpr auto FAR    = 500.0f;

	static void*  frameBuffers[] = {nullptr, nullptr};
	static size_t currentFB      = 0;

	static void*       fifoBuffer    = nullptr;
	static GXRModeObj* screenMode    = nullptr;
	static vu8         readyForCopy  = GX_FALSE;

	static GXTexObj texture = {};
	static TPLFile  skyTPL  = {};

	static Mtx44   projection = {};
	static GXColor bgColor    = {0, 0, 0, 255};
	
	static Camera camera = Camera
	(
		{0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, -1.0f}
	);

	alignas(32) static Vertex vertices[]
	{
		{
			-1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			-1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			-1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			-1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
		{
			1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
		{
			-1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			-1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
		{
			-1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			-1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
	};

	void InitScreen();
	void InitGPU();
	void LoadData();

	void CopyBuffers(u32 count);
	
	void BeginDraw();
	void DrawCube();
	void EndDraw();

	void DrawVertex(const Vertex& vertex);
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
	screenMode      = VIDEO_GetPreferredMode(nullptr);
	frameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));
	frameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));
	
	VIDEO_Configure(screenMode);
	VIDEO_SetNextFramebuffer(frameBuffers[currentFB]);
	VIDEO_SetPostRetraceCallback(GFX::CopyBuffers);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(screenMode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	}
	currentFB ^= 1;
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
	GX_SetCullMode(GX_CULL_BACK);
	GX_CopyDisp(frameBuffers[currentFB], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
}

void GFX::LoadData()
{
	guPerspective(projection, FOV, ASPECT, NEAR, FAR);
	GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

	GX_ClearVtxDesc();

	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,	GX_POS_XYZ,	GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_InvalidateTexAll();
	
	TPL_OpenTPLFromMemory(&skyTPL, U8_TO_VOID_PTR(textures_tpl), textures_tpl_size);
	TPL_GetTexture(&skyTPL, sky, &texture);

	GX_LoadTexObj(&texture, GX_TEXMAP0);
}

void GFX::Render()
{
	BeginDraw();
	DrawCube();
	EndDraw();
}

void GFX::BeginDraw()
{
	camera.CreateView();
	GX_SetViewport(0, 0, screenMode->fbWidth, screenMode->efbHeight, 0, 1);

	GX_InvVtxCache();
	GX_InvalidateTexAll();
	
	GX_SetTevOp(GX_TEVSTAGE0, GX_DECAL);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetNumChans(1);
	GX_LoadTexObj(&texture, GX_TEXMAP0);

	guVector axis = {-1, -1, 0};
	static f32 rotation = 0.0f;
	rotation++;
	
	Mtx	modelView;
	guMtxIdentity(modelView);
	guMtxRotAxisDeg(modelView, &axis, rotation);
	guMtxTransApply(modelView, modelView, 0.0f,	0.0f, -5);
	guMtxConcat(camera.viewMat, modelView, modelView);
	GX_LoadPosMtxImm(modelView,	GX_PNMTX0);
}

void GFX::DrawCube()
{
	GX_Begin(GX_QUADS, GX_VTXFMT0, 6 * 4);
		DrawVertex(vertices[0]);
		DrawVertex(vertices[1]);
		DrawVertex(vertices[2]);
		DrawVertex(vertices[3]);

		DrawVertex(vertices[4]);
		DrawVertex(vertices[5]);
		DrawVertex(vertices[6]);
		DrawVertex(vertices[7]);

		DrawVertex(vertices[8]);
		DrawVertex(vertices[9]);
		DrawVertex(vertices[10]);
		DrawVertex(vertices[11]);

		DrawVertex(vertices[12]);
		DrawVertex(vertices[13]);
		DrawVertex(vertices[14]);
		DrawVertex(vertices[15]);

		GX_Position3f32(1.0f, -1.0f, -1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(0.0f,0.0f);
		GX_Position3f32(1.0f, 1.0f,-1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(1.0f, 0.0f);
		GX_Position3f32(-1.0f, 1.0f, -1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(1.0f, 1.0f);
		GX_Position3f32(-1.0f, -1.0f, -1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(0.0f, 1.0f);

		GX_Position3f32(1.0f, -1.0f, 1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(0.0f, 0.0f);
		GX_Position3f32(-1.0f, -1.0f, 1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(1.0f, 0.0f);
		GX_Position3f32(-1.0f, 1.0f, 1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(1.0f, 1.0f);
		GX_Position3f32(1.0f, 1.0f, 1.0f);
		GX_Color4u8(0, 0, 0, 0);
		GX_TexCoord2f32(0.0f, 1.0f);
	GX_End();
}

void GFX::EndDraw()
{
	GX_DrawDone();
	readyForCopy = GX_TRUE;

	VIDEO_SetNextFramebuffer(frameBuffers[currentFB]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	currentFB ^= 1;
}

void GFX::DrawVertex(const Vertex& vertex)
{
	GX_Position3f32(vertex.x, vertex.y, vertex.z);
	GX_Color4u8(vertex.r, vertex.g, vertex.b, vertex.a);
	GX_TexCoord2f32(vertex.u, vertex.v);
}

void GFX::CopyBuffers(GCN_UNUSED u32 count)
{
	if (readyForCopy == GX_TRUE)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL,	GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffers[currentFB], GX_TRUE);
		GX_Flush();
		readyForCopy = GX_FALSE;
	}
}