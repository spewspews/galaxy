#include "nbody.h"
#include <SDL2/SDL2_gfxPrimitives.h>

void Mouse::operator()(Galaxy& g) {
	update();
	switch(buttons_) {
	case SDL_BUTTON_LMASK:
		ui_.sim_.pause();
		body(g);
		ui_.sim_.unpause();
		break;
	case SDL_BUTTON_RMASK:
		move(g);
		break;
	}
}

void Mouse::update() {
	SDL_RenderPresent(ui_.renderer_);
	SDL_Delay(5);
	SDL_PumpEvents();
	buttons_ = SDL_GetMouseState(&p.x, &p.y);
	vp = ui_.toVector(p);
	SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
}

void Mouse::body(Galaxy& g) {
	auto b = Body{ui_.scale_};
	b.p = vp;
	for(;;) {
		ui_.draw(g);
		ui_.draw(b);
		ui_.draw(b, b.v);
		update();
		if(!(buttons_ & SDL_BUTTON_LMASK))
			break;
		if(buttons_ == (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			setSize(b, g);
		else if(buttons_ == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			setVel(b, g);
		else
			b.p = vp;
	}
	g.bodies.push_back(b);
	ui_.orig_ = ui_.toPoint(g.center());
	g.checkLimit(b.p);
	ui_.draw(g);
}

void Mouse::setSize(Body& b, const Galaxy& g) {
	auto oldp = p;
	for(;;) {
		auto d = vp - b.p;
		auto h = std::hypot(d.x, d.y);
		b.size = h == 0 ? 2.0 * ui_.scale_ : h;
		b.mass = b.size * b.size * b.size;
		ui_.draw(g);
		ui_.draw(b);
		ui_.draw(b, b.v);
		update();
		if(buttons_ != (SDL_BUTTON_LMASK | SDL_BUTTON_MMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::setVel(Body& b, const Galaxy& g) {
	auto oldp = p;
	for(;;) {
		b.v = (vp - b.p) / 10;
		ui_.draw(g);
		ui_.draw(b);
		ui_.draw(b, b.v);
		update();
		if(buttons_ != (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
			break;
	}
	SDL_WarpMouseInWindow(nullptr, oldp.x, oldp.y);
}

void Mouse::move(const Galaxy& g) {
	auto oldp = p;
	for(;;) {
		update();
		ui_.sim_.pause();
		ui_.orig_ += p - oldp;
		oldp = p;
		ui_.draw(g);
		ui_.sim_.unpause();
		if(buttons_ != SDL_BUTTON_RMASK)
			break;
	}
}

Vector UI::toVector(const Point& p) const {
	return Vector{static_cast<double>(p.x - orig_.x) * scale_,
	              static_cast<double>(p.y - orig_.y) * scale_};
}

Point UI::toPoint(const Vector& v) const {
	return Point{static_cast<int>(v.x / scale_) + orig_.x,
	             static_cast<int>(v.y / scale_) + orig_.y};
}

void UI::draw(const Galaxy& g) const {
	SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
	SDL_RenderClear(renderer_);
	for(auto& b : g.bodies) {
		draw(b);
		if(showv)
			draw(b, b.v);
		if(showa)
			draw(b, b.a);
	}
	SDL_RenderPresent(renderer_);
}

void UI::draw(const Body& b) const {
	auto pos = toPoint(b.p);
	auto drawSize = static_cast<int>(b.size / scale_);

	auto err = filledCircleRGBA(renderer_, pos.x, pos.y, drawSize, b.r, b.g,
	                            b.b, 0xcf);
	if(err) {
		std::cerr << "Could not draw circle: " << SDL_GetError() << "\n";
		exit(1);
	}
}

void UI::draw(const Body& b, const Vector& e) const {
	auto spos = toPoint(b.p);
	auto epos = toPoint(e * 10 + b.p);
	auto err = aalineRGBA(renderer_, spos.x, spos.y, epos.x, epos.y, b.r, b.g,
	                      b.b, 0xff);
	if(err == -1) {
		std::cerr << "Could not draw vector: " << SDL_GetError();
		exit(1);
	}
}

void UI::init() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		exit(1);
	}

	screen_ = SDL_CreateWindow("Galaxy", SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED, 640, 640,
	                           SDL_WINDOW_RESIZABLE);
	if(screen_ == nullptr) {
		std::cerr << "Could not create window: " << SDL_GetError() << "\n";
		exit(1);
	}

	renderer_ = SDL_CreateRenderer(screen_, -1, 0);
	if(renderer_ == nullptr) {
		std::cerr << "Could not create renderer: " << SDL_GetError() << "\n";
		exit(1);
	}
	SDL_GetWindowSize(screen_, &orig_.x, &orig_.y);
	orig_ /= 2;
}

void UI::loop(Galaxy& g) {
	SDL_GetWindowSize(screen_, &orig_.x, &orig_.y);
	orig_ /= 2;
	for(;;) {
		SDL_Event e;
		while(SDL_PollEvent(&e) != 0) {
			switch(e.type) {
			case SDL_QUIT:
				exit(0);
				break;

			case SDL_KEYDOWN:
				if(e.key.keysym.sym == SDLK_q)
					exit(0);
				break;

			case SDL_WINDOWEVENT:
				if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
					sim_.pause();
					SDL_SetWindowSize(screen_, e.window.data1, e.window.data2);
					orig_ = Point{e.window.data1, e.window.data2} / 2;
					sim_.unpause();
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouse_(g);
				break;
			}
		}
		SDL_Delay(100);
	}
}
