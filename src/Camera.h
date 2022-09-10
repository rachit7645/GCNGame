#ifndef CAMERA_H
#define CAMERA_H

#include <gctypes.h>
#include <ogc/gu.h>

namespace GFX
{
	// \brief A class representing a camera
	class Camera
	{
	public:
		// \fn GFX::Camera::Camera(const guVector& position, const guVector& up, const guVector& target)
		// \brief Camera constructor
		// \returns void
		Camera(const guVector& position, const guVector& up, const guVector& target);
		
		// \fn GFX::Camera::CreateView()
		// \brief Creates the view matrix
		// \returns void
		void CreateView();

		// \brief Camera position
		guVector position;
		// \brief Camera up vector
		guVector up;
		// \brief Camera lookAt target
		guVector target;
		// \brief Camera view matrix generated with CreateView()
		Mtx viewMat;
	};
}


#endif