#include "nbody.h"

SDL_Window* screen;
SDL_Renderer* renderer;
Galaxy glxy;
bool showv, showa;

const std::vector<Uint32> RandCol::cols = {
    DWhite,         DRed,         DGreen,         DCyan,
    DMagenta,       DYellow,      DPaleyellow,    DDarkyellow,
    DDarkgreen,     DPalegreen,   DPalebluegreen, DPaleblue,
    DPalegreygreen, DYellowgreen, DGreyblue,      DPalegreyblue,
};

std::ostream& operator<<(std::ostream& os, const Point& p) {
	os << "Point{" << p.x << ", " << p.y << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Vector& v) {
	os << "Vector{" << v.x << ", " << v.y << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Body& b) {
	os << "Body{pos[" << b.p << "] vel[" << b.v << "]}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Quad& q) {
	os << "Quad{p[" << q.p << "]}";
	return os;
}

Point& Point::operator=(const Point& p) {
	x = p.x;
	y = p.y;
	return *this;
}

Point& Point::operator/=(int s) {
	x /= s;
	y /= s;
	return *this;
}

Point& Point::operator+=(const Point& p) {
	x += p.x;
	y += p.y;
	return *this;
}

Vector& Vector::operator=(const Vector& p) {
	x = p.x;
	y = p.y;
	return *this;
}

Vector& Vector::operator+=(const Vector& p) {
	x += p.x;
	y += p.y;
	return *this;
}

Vector& Vector::operator-=(const Vector& p) {
	x -= p.x;
	y -= p.y;
	return *this;
}
Vector& Vector::operator/=(double f) {
	x /= f;
	y /= f;
	return *this;
}

void Galaxy::draw() const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	std::cerr << "Galaxy::draw: ENTER\n";
	for(auto& b : bodies) {
		std::cerr << "Galaxy::draw: b " << b << "\n";
		b.draw();
		if(showv)
			b.drawVel();
		if(showa)
			b.drawAcc();
	}
	SDL_RenderPresent(renderer);
}

void Galaxy::checkLimit(const Vector& v) {
	auto f = fabs(v.x);
	if(f > limit / 2)
		limit = f * 2;
	f = fabs(v.y);
	if(f > limit / 2)
		limit = f * 2;
}

Point Galaxy::center() {
	Vector gc, gcv;
	double mass = 0;
	for(auto& b : glxy.bodies) {
		gc += b.p * b.mass;
		gcv += b.v * b.mass;
		mass += b.mass;
	}
	gc /= mass;
	gcv /= mass;
	for(auto& b : glxy.bodies) {
		b.p -= gc;
		b.v -= gcv;
	}
	return toPoint(gc);
}

void Galaxy::toVector(Vector& v, const Point& p) const {
	v.x = static_cast<double>(p.x - orig.x) / scale;
	v.y = static_cast<double>(p.y - orig.y) / scale;
}

void Galaxy::toPoint(Point& p, const Vector& v) const {
	p.x = static_cast<int>(v.x * scale) + orig.x;
	p.y = static_cast<int>(v.y * scale) + orig.y;
}

Vector Galaxy::toVector(const Point& p) const {
	return {static_cast<double>(p.x - orig.x) / scale,
	        static_cast<double>(p.y - orig.y) / scale};
}

Point Galaxy::toPoint(const Vector& v) const {
	return {static_cast<int>(v.x * scale) + orig.x,
	        static_cast<int>(v.y * scale) + orig.y};
}

bool initdraw() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		exit(1);
	}

	screen = SDL_CreateWindow("Galaxy", SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED, 640, 640,
	                          SDL_WINDOW_RESIZABLE);
	if(screen == nullptr) {
		std::cerr << "Could not create window: " << SDL_GetError() << "\n";
		exit(1);
	}

	renderer = SDL_CreateRenderer(screen, -1, 0);
	if(renderer == nullptr) {
		std::cerr << "Could not create renderer: " << SDL_GetError() << "\n";
		exit(1);
	}
	return true;
}

void simulate() {
	BHTree tree;
	for(;;) {
		glxy.mutex.lock();
		std::cerr << "top of simulate loop\n";
		glxy.draw();
		tree.insert(glxy);
		glxy.mutex.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds{10});
	}
}

int main() {
	initdraw();
	glxy.draw();
	std::thread simthread{simulate};

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
					SDL_SetWindowSize(screen, e.window.data1, e.window.data2);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouse();
				break;
			}
		}
		SDL_Delay(100);
	}
}
