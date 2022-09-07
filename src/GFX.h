#ifndef GFX_H
#define GFX_H

#include <gctypes.h>
#include <ogc/gu.h>

namespace GFX
{
	void InitVideo();
	void CopyBuffers(u32 count);
	void Update();
	void UpdateScreen(Mtx& view);
}


#endif