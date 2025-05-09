#include "Control.h"

Control::Control(Camera& camera) : camera(camera)
{
	key_state = SDL_GetKeyboardState(nullptr);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Control::handleInput()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		if (Event.type == SDL_MOUSEMOTION)
		{
			camera.pitch -= Event.motion.yrel * 0.01f;
			camera.yaw -= Event.motion.xrel * 0.01f;
		}
	}

	if (key_state[SDL_SCANCODE_W]) { camera.position += camera.direction; }
	if (key_state[SDL_SCANCODE_A]) { camera.position -= camera.right; }
	if (key_state[SDL_SCANCODE_S]) { camera.position -= camera.direction; }
	if (key_state[SDL_SCANCODE_D]) { camera.position += camera.right; }
}
