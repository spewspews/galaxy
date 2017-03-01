#include "nbody.h"

SDL_Window* screen;
SDL_Renderer* renderer;
Galaxy glxy;
double scale{10};

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

Vector Vector::operator-(const Vector& p) const { return {x - p.x, y - p.y}; }

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

Point Vector::toPoint() const {
	return {static_cast<int>(x * scale), static_cast<int>(y * scale)};
}

void Galaxy::draw(bool showv, bool showa) const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	for(auto& b : bodies)
		b.draw(showv, showa);
	SDL_RenderPresent(renderer);
}

bool initdraw(std::string& err) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::ostringstream ss;
		ss << "Could not initialize SDL: " << SDL_GetError();
		err = ss.str();
		return false;
	}

	screen =
	    SDL_CreateWindow("Galaxy", SDL_WINDOWPOS_UNDEFINED,
	                     SDL_WINDOWPOS_UNDEFINED, 640, 640, SDL_WINDOW_RESIZABLE);
	if(screen == nullptr) {
		std::ostringstream ss;
		ss << "Could not create window: " << SDL_GetError();
		err = ss.str();
		return false;
	}
	renderer = SDL_CreateRenderer(screen, -1, 0);
	if(renderer == nullptr) {
		std::ostringstream ss;
		ss << "Could not create renderer: " << SDL_GetError();
		err = ss.str();
		return false;
	}
	return true;
}

int main(void) {
	std::string err;
	if(!initdraw(err)) {
		std::cerr << err << "\n";
		exit(1);
	}

	glxy.draw(false, false);

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
