#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

Uint32 Body::getRandomColor() {
	static RandCol randCol;

	return randCol();
}

void Body::draw(bool showv, bool showa) const {
	auto pos = p.toPoint();
	auto r = color >> 3 & 0xff;
	auto g = color >> 2 & 0xff;
	auto b = color >> 1 & 0xff;
	if(filledCircleRGBA(renderer, pos.x, pos.y, 10, r, g, b, 0xff) == -1) {
		std::cerr << "Could not draw circle: " << SDL_GetError();
		exit(1);
	}
}
