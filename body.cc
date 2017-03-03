#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

Uint32 Body::getRandomColor() {
	static RandCol randCol;
	return randCol();
}

std::ostream& operator<<(std::ostream& os, const Body& b) {
	os << "Body{pos[" << b.p << "] vel[" << b.v << "]}";
	return os;
}
