#ifndef VERTEX_H
#define VERTEX_H

#include <gctypes.h>

namespace GFX
{
	struct Vertex
	{
		// Position
		f32 x, y, z;
		// Color
		u8 r, g, b, a;
		// TxCoords
		f32 u, v;
	};
}


#endif