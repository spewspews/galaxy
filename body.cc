#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

Uint32 Body::getRandomColor() {
	static RandCol randCol;
	return randCol();
}

void Body::draw() const {
	auto pos = glxy.toPoint(p);
	auto drawSize = static_cast<int>(size * glxy.scale);
	auto err1 = aacircleRGBA(renderer, pos.x, pos.y, drawSize, r, g, b, 0xff);
	auto err2 =
	    filledCircleRGBA(renderer, pos.x, pos.y, drawSize, r, g, b, 0xff);
	if(err1 == -1 || err2 == -1) {
		std::cerr << "Could not draw circle: " << SDL_GetError();
		exit(1);
	}
}

void Body::drawVel() const {
	auto pos = glxy.toPoint(p);
	auto vvec = p + v;
	auto vel = glxy.toPoint(vvec);
	auto err = aalineRGBA(renderer, pos.x, pos.y, vel.x, vel.y, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw velocity: " << SDL_GetError();
		exit(1);
	}
}

void Body::drawAcc() const {
	auto pos = glxy.toPoint(p);
	auto endAcc = pos + glxy.toPoint(a);
	auto err =
	    aalineRGBA(renderer, pos.x, pos.y, endAcc.x, endAcc.y, r, g, b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw velocity: " << SDL_GetError();
		exit(1);
	}
}
