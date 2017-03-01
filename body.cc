#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

Uint32 Body::getRandomColor() {
	static RandCol randCol;

	return randCol();
}

void Body::draw(bool showv, bool showa) const {
	auto pos = p.toPoint();
	auto drawSize = static_cast<int>(size * scale);
	if(drawSize == 0)
		drawSize = 2;
	auto err =
	    filledCircleRGBA(renderer, pos.x, pos.y, drawSize, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw circle: " << SDL_GetError();
		exit(1);
	}
	if(showv)
		drawVel();
}

void Body::drawVel() const {
	auto pos = p.toPoint();
	auto endvel = pos + v.toPoint();
	auto err =
	    aalineRGBA(renderer, pos.x, pos.y, endvel.x, endvel.y, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw velocity: " << SDL_GetError();
		exit(1);
	}
}
