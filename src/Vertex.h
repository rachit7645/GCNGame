#ifndef VERTEX_H
#define VERTEX_H

#include <gctypes.h>

namespace GFX
{
	// \brief
	// A struct representing a single vertex.
	struct Vertex
	{
		// \brief Position
		f32 x, y, z;
		// \brief Color
		u8 r, g, b, a;
		// \brief Texture Coordinates
		f32 u, v;
	};
}


#endif