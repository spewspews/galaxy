#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <string>

class Vector {
public:
  double x, y;
};

class Body {
  Vector p, v, a, newa;
  double size, mass;
  Uint32 color;

public:
  void draw() const;
};

class Point {
public:
  int x, y;
  Point() : x{0}, y{0} {}
  Point(SDL_Point& p) : x{p.x}, y{p.y} {}
  Point(int a, int b) : x{a}, y{b} {}

  Point& operator=(const Point&);
  Point& operator/=(int);
};

class Mouse {
  Uint32 but;
  void update();
  void mkbody();
  void move();
public:
  Point pos;
  Vector vpos;
  void operator()();
};

extern SDL_Window* screen;
extern Mouse mouse;
extern Point orig;
