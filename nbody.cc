#include "nbody.h"

SDL_Window* screen;
Point orig;

Point& Point::operator=(const Point& p) {
  this->x = p.x;
  this->y = p.y;
  return *this;
}

Point& Point::operator/=(int s) {
  this->x /= s;
  this->y /= s;
  return *this;
}

bool initdraw(std::string& err) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::ostringstream ss;
    ss << "Could not initialize SDL: " << SDL_GetError();
    err = ss.str();
    return false;
  }

  auto win =
      SDL_CreateWindow("Galaxy", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 10, 10, SDL_WINDOW_RESIZABLE);
  if(win == nullptr) {
    std::ostringstream ss;
    ss << "Could not initialize SDL: " << SDL_GetError();
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

  SDL_GetWindowSize(screen, &orig.x, &orig.y);
  orig /= 2;

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
          orig = {e.window.data1, e.window.data2};
          orig /= 2;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        mouse();
        break;
      }
    }
  }
}
