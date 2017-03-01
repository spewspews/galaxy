#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define DBlack		= 0x000000FF,
#define DWhite		= 0xFFFFFFFF,
#define DRed		= 0xFF0000FF,
#define DGreen		= 0x00FF00FF,
#define DBlue		= 0x0000FFFF,
#define DCyan		= 0x00FFFFFF,
#define DMagenta		= 0xFF00FFFF,
#define DYellow		= 0xFFFF00FF,
#define DPaleyellow	= 0xFFFFAAFF,
#define DDarkyellow	= 0xEEEE9EFF,
#define DDarkgreen	= 0x448844FF,
#define DPalegreen	= 0xAAFFAAFF,
#define DMedgreen	= 0x88CC88FF,
#define DDarkblue	= 0x000055FF,
#define DPalebluegreen= 0xAAFFFFFF,
#define DPaleblue		= 0x0000BBFF,
#define DBluegreen	= 0x008888FF,
#define DGreygreen	= 0x55AAAAFF,
#define DPalegreygreen	= 0x9EEEEEFF,
#define DYellowgreen	= 0x99994CFF,
#define DGreyblue	= 0x005DBBFF,
#define DPalegreyblue	= 0x4993DDFF,

class Vector {
  public:
	double x, y;
};

class Body {
  public:
	Body() : p{0, 0}, v{0, 0}, a{0, 0}, newa{0, 0}, size{1}, mass{1} {}
	void draw() const;
	Uint32 color;
	Vector p, v, a, newa;
	double size, mass;
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
	void mkbody();
	void move();

  public:
	Point pos;
	Vector vpos;
	void operator()();
	void update();
	void setPos(Body&);
};

class Galaxy {
	std::vector<Body> bodies;
  public:
	Body& newBody() { bodies.push_back({}); return bodies.back(); }
};

class RandCol {
	static const Uint32 cols[];
  public:
	Uint32 operator()();
};

extern SDL_Window* screen;
extern Mouse mouse;
extern Point orig;
extern Galaxy glxy;
extern RandCol randCol;
