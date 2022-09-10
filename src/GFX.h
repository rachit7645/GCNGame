#ifndef GFX_H
#define GFX_H

#include <gctypes.h>
#include <ogc/gu.h>

namespace GFX
{
	// \fn void GFX::InitVideo()
	// \brief Initialises video subsystem
	// \returns void
	void InitVideo();
	// \fn void GFX::Render()
	// \brief Renders current frame
	// \returns void
	void Render();
}


#endif