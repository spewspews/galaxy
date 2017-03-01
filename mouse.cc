#include "nbody.h"

Mouse mouse;

void Mouse::operator()() {
  this->update();
  switch(but) {
  case SDL_BUTTON_LMASK:
    this->mkbody();
    break;
  case SDL_BUTTON_RMASK:
    this->move();
    break;
  }
}

void Mouse::update() {
  SDL_PumpEvents();
  but = SDL_GetMouseState(&this->pos.x, &this->pos.y);
  SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
}

void Mouse::mkbody() {
}

void Mouse::move() {
}
