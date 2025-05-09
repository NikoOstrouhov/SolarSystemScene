#pragma once

#include <SDL2/SDL.h>
#include "model.h"
#include "view.h"


class Control {
private:
    Camera& camera;
    const Uint8* key_state;
public:
    void handleInput();
    Control(Camera& camera);
};
