#ifndef CAMERA_H
#define CAMERA_H

#include <gctypes.h>
#include <ogc/gu.h>

namespace GFX
{
	class Camera
	{
	public:
		Camera(const guVector& position, const guVector& up, const guVector& target);
		void CreateView();

		guVector position;
		guVector up;
		guVector target;
		Mtx      viewMat;
	};
}


#endif