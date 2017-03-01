#include "nbody.h"

SDL_Window* screen;
SDL_Renderer* renderer;
Galaxy glxy;
double scale{10};
bool showv, showa;
std::mutex glxyMutex;

const std::vector<Uint32> RandCol::cols = {
    DWhite,         DRed,         DGreen,         DCyan,
    DMagenta,       DYellow,      DPaleyellow,    DDarkyellow,
    DDarkgreen,     DPalegreen,   DPalebluegreen, DPaleblue,
    DPalegreygreen, DYellowgreen, DGreyblue,      DPalegreyblue,
};

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

Point Point::operator+(const Point& p) const { return {x + p.x, y + p.y}; }

Vector Point::toVector() const {
	return {static_cast<double>(x) / scale, static_cast<double>(y) / scale};
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

Point Vector::toPoint() const {
	return {static_cast<int>(x * scale), static_cast<int>(y * scale)};
}

void Galaxy::draw() const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	for(auto& b : bodies) {
		b.draw();
		if(showv)
			b.drawVel();
		if(showa)
			b.drawAcc();
	}
	SDL_RenderPresent(renderer);
}

void Galaxy::center() {
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
		std::unique_lock<std::mutex> lock{glxyMutex};
		glxy.draw();
		tree.insert(glxy);
	}
}

int main() {
	initdraw();
	glxy.draw();

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
				std::unique_lock<std::mutex> lock{glxyMutex};
				mouse();
				break;
			}
		}
		SDL_Delay(100);
	}
}
