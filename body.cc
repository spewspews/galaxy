#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

void Body::draw(bool showv, bool showa) const {
	auto pos = p.toPoint();
	filledCircleColor(renderer, pos.x, pos.y, 10, color);
}
