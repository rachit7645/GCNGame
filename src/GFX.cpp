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
	// \brief The size of the GPU FIFO buffer
	constexpr auto GPU_FIFO_SIZE = 256 * 1024;

	// \brief The field of view (in degrees) 
	constexpr auto FOV = 70.0f;
	// \brief The aspect ratio (4:3)
	constexpr auto ASPECT = static_cast<f32>(4.0 / 3.0);
	// \brief The near plane of the projection matrix
	constexpr auto NEAR = 0.1f;
	// \brief The far plane of the projection matrix
	constexpr auto FAR = 500.0f;

	// \brief The frame buffers
	static void* frameBuffers[] = {nullptr, nullptr};
	// \brief The current frame buffer
	static size_t currentFB = 0;

	// \brief The GPU FIFO buffer
	static void* gpuFifoBuffer = nullptr;
	// \brief The current screen mode
	static GXRModeObj* screenMode = nullptr;
	// \brief If GX_TRUE, copy the EFB to the XFB
	static vu8 readyForCopy = GX_FALSE;

	// \brief The GPU texture object
	static GXTexObj texture = {};
	// \brief The TPL file to load the texture
	static TPLFile skyTPL = {};

	// \brief The current projection matrix
	static Mtx44 projection = {};
	// \brief The current background color
	static GXColor bgColor = {0, 0, 0, 255};
	
	// \brief The camera
	static auto camera = Camera
	(
		{0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, -1.0f}
	);

	// \brief The cube vertices
	static Vertex vertices[]
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
		{
			1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			-1.0f, 1.0f, -1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			-1.0f, -1.0f, -1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
		{
			1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 0.0f
		},
		{
			-1.0f, -1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 0.0f
		},
		{
			-1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			1.0f, 1.0f
		},
		{
			1.0f, 1.0f, 1.0f,
			0, 0, 0, 0,
			0.0f, 1.0f
		},
	};
	// \brief The cube's rotation
	f32 rotation = 0.0f;

	// \fn void GFX::InitScreen()
	// \brief Initialises the display
	// \returns void
	void InitScreen();
	// \fn void GFX::InitGPU()
	// \brief Initialises the GPU
	// \returns void
	void InitGPU();
	// \fn void GFX::LoadData()
	// \brief Loads data to the GPU
	// \returns void
	void LoadData();
	
	// \fn void GFX::CopyBuffers()
	// \brief Vertical Interrupt callback, copies the EFB to the XFB
	// \returns void
	void CopyBuffers(u32 count);
	
	// \fn void GFX::BeginDraw()
	// \brief Begins rendering
	// \returns void
	void BeginDraw();
	// \fn void GFX::DrawCube()
	// \brief Draws a cube
	// \returns void
	void DrawCube();
	// \fn void GFX::EndDraw()
	// \brief Ends rendering
	// \returns void
	void EndDraw();

	// \fn void GFX::DrawVertex()
	// \brief Draws a vertex
	// \returns void
	void DrawVertex(const Vertex& vertex);
	// \fn void GFX::DrawQuad()
	// \brief Draws a quad with four vertices
	// \returns void
	void DrawQuad(size_t v0, size_t v1, size_t v2, size_t v3, const Vertex* vertices = GFX::vertices);
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
	gpuFifoBuffer = MEM_K0_TO_K1(memalign(32, GPU_FIFO_SIZE));
	memset(gpuFifoBuffer, 0, GPU_FIFO_SIZE);
	GX_Init(gpuFifoBuffer, GPU_FIFO_SIZE);
	
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

	GX_SetVtxDesc(GX_VA_POS,  GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,	 GX_POS_XYZ,  GX_F32,   0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0);
	
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY);
	GX_InvalidateTexAll();
	
	TPL_OpenTPLFromMemory(&skyTPL, Util::ToVoidPtr(textures_tpl), textures_tpl_size);
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

	guVector axis = {-1.0f, -1.0f, 0.0f};
	rotation++;
	
	Mtx	modelView;
	guMtxIdentity(modelView);
	guMtxRotAxisDeg(modelView, &axis, rotation);
	guMtxTransApply(modelView, modelView, 0.0f,	0.0f, -5.0f);
	guMtxConcat(camera.viewMat, modelView, modelView);
	GX_LoadPosMtxImm(modelView,	GX_PNMTX0);
}

void GFX::DrawCube()
{
	auto vertexCount = Util::ArraySize(vertices);
	// Draw vertexCount / 4 quads
	GX_Begin(GX_QUADS, GX_VTXFMT0, vertexCount);
		DrawQuad(0,  1,  2,  3);
		DrawQuad(4,  5,  6,  7);
		DrawQuad(8,  9,  10, 11);
		DrawQuad(12, 13, 14, 15);
		DrawQuad(16, 17, 18, 19);
		DrawQuad(20, 21, 22, 23);
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

void GFX::DrawQuad(size_t v0, size_t v1, size_t v2, size_t v3, const Vertex* vertices)
{
	DrawVertex(vertices[v0]);
	DrawVertex(vertices[v1]);
	DrawVertex(vertices[v2]);
	DrawVertex(vertices[v3]);
}

void GFX::CopyBuffers(GCN_UNUSED u32 count)
{
	if (readyForCopy == GX_TRUE)
	{
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffers[currentFB], GX_TRUE);
		GX_Flush();
		readyForCopy = GX_FALSE;
	}
}