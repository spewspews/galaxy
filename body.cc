#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

Uint32 Body::getRandomColor() {
	static RandCol randCol;
	return randCol();
}

void Body::draw() const {
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
}

void Body::drawVel() const {
	auto pos = p.toPoint();
	auto vvec = p + v;
	auto vel = vvec.toPoint();
	auto err = aalineRGBA(renderer, pos.x, pos.y, vel.x, vel.y, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw velocity: " << SDL_GetError();
		exit(1);
	}
}

void Body::drawAcc() const {
	auto pos = p.toPoint();
	auto endAcc = pos + a.toPoint();
	auto err =
	    aalineRGBA(renderer, pos.x, pos.y, endAcc.x, endAcc.y, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw velocity: " << SDL_GetError();
		exit(1);
	}
}
