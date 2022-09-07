#include "Camera.h"

using namespace GFX;

Camera::Camera(const guVector& position, const guVector& up, const guVector& target)
	: position(position),
	  up(up),
	  target(target)
{
}

void Camera::CreateView()
{
	guLookAt(viewMat, &position, &up, &target);
}