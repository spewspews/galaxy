#include "nbody.h"

Mouse mouse;

void Mouse::update() {
	SDL_PumpEvents();
	buttons = SDL_GetMouseState(&p.x, &p.y);
	vp = p.toVector();
	SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
}

void Mouse::mkbody() {
	auto& b = glxy.newBody();
	b.p = vp;
	for(;;) {
		glxy.draw(false, false);
		update();
		if(!(buttons & SDL_BUTTON_LMASK))
			break;
		if(buttons == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			setSize(b);
		else if(buttons == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			setVel(b);
		else
			b.p = vp;
	}
}

void Mouse::setSize(Body& b) {}
void Mouse::setVel(Body& b) {}

void Mouse::move() {}

void Mouse::operator()() {
	update();
	switch(buttons) {
	case SDL_BUTTON_LMASK:
		mkbody();
		break;
	case SDL_BUTTON_RMASK:
		move();
		break;
	}
}
