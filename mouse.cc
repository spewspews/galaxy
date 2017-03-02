#include "nbody.h"

Mouse mouse;

void Mouse::operator()() {
	std::lock_guard<std::mutex> lock{glxy.mutex};
	update();
	switch(buttons_) {
	case SDL_BUTTON_LMASK:
		body();
		break;
	case SDL_BUTTON_RMASK:
		move();
		break;
	}
	std::cerr << "Mouse::operator(): leaving mouse\n";
}

void Mouse::update() {
	SDL_RenderPresent(renderer);
	SDL_Delay(5);
	SDL_PumpEvents();
	buttons_ = SDL_GetMouseState(&p.x, &p.y);
	glxy.toVector(vp, p);
	SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
	std::cerr << "Mouse::update: mouse is at " << p << " " << vp << "\n";
}

void Mouse::body() {
	auto& b = glxy.newBody();
	b.p = vp;
	for(;;) {
		glxy.draw();
		b.drawVel();
		update();
		if(!(buttons_ & SDL_BUTTON_LMASK))
			break;
		if(buttons_ == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			setSize(b);
		else if(buttons_ == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			setVel(b);
		else
			b.p = vp;
	}
	glxy.orig = glxy.center();
	glxy.checkLimit(b.p);
	glxy.draw();
}

void Mouse::setSize(Body& b) {
	auto oldp = p;
	for(;;) {
		auto d = vp - b.p;
		auto h = std::hypot(d.x, d.y);
		b.size = h == 0 ? 1.0 / glxy.scale : h;
		b.mass = b.size * b.size * b.size;
		glxy.draw();
		b.drawVel();
		update();
		if(buttons_ != (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::setVel(Body& b) {
	auto oldp = p;
	for(;;) {
		b.v = vp - b.p;
		glxy.draw();
		b.drawVel();
		update();
		if(buttons_ != (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::move() {
	auto oldp = p;
	for(;;) {
		update();
		glxy.orig += p - oldp;
		oldp = p;
		glxy.draw();
		if(buttons_ != SDL_BUTTON_RMASK)
			break;
	}
}
