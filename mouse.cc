#include "nbody.h"

Mouse mouse;

void Mouse::update() {
	SDL_Delay(5);
	SDL_PumpEvents();
	buttons = SDL_GetMouseState(&p.x, &p.y);
	vp = p.toVector();
	SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
}

void Mouse::body() {
	auto& b = glxy.newBody();
	b.p = vp;
	for(;;) {
		glxy.draw(false, false);
		update();
		if(!(buttons & SDL_BUTTON_LMASK))
			break;
		if(buttons == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			setSize(b);
		else if(buttons == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			setVel(b);
		else
			b.p = vp;
	}
}

void Mouse::setSize(Body& b) {
	auto oldp = p;
	for(;;) {
		auto d = vp - b.p;
		b.size = std::hypot(d.x, d.y);
		b.mass = b.size * b.size * b.size;
		glxy.draw(false, false);
		update();
		if(buttons != (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::setVel(Body& b) {
	auto oldp = p;
	for(;;) {
		b.v = vp - b.p;
		glxy.draw(false, false);
		b.drawVel();
		SDL_RenderPresent(renderer);
		update();
		if(buttons != (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::move() {
	auto oldp = vp;
	for(;;) {
		update();
		auto off = vp - oldp;
		oldp = vp;
		for(auto& b : glxy.bodies)
			b.p += off;
		glxy.draw(false, false);
		if(buttons != SDL_BUTTON_RMASK)
			break;
	}
}

void Mouse::operator()() {
	update();
	switch(buttons) {
	case SDL_BUTTON_LMASK:
		body();
		break;
	case SDL_BUTTON_RMASK:
		move();
		break;
	}
}
